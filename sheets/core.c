/*
 * sheets/core.c - thin, fast Lua binding around libcsv (rgamble/libcsv).
 *
 * This is intentionally minimal: it does buffer<->table conversion only.
 * All ergonomics (reader/writer objects, DictReader, dialects, iterator
 * protocol) live in the pure-Lua layer (csv/init.lua and friends) so that
 * this C module stays small and easy to extend later (e.g. with true
 * streaming parse via csv_parse() fed chunk-by-chunk).
 *
 * Exposed as require("sheets.core"):
 *   core.parse_string(str, opts)  -> rows | nil, err
 *   core.parse_file(path, opts)   -> rows | nil, err
 *   core.write_rows(rows, opts)   -> string
 *   core.write_row(row, opts)     -> string   (single line, no terminator)
 *
 * opts (table, optional):
 *   delim   = single-char string, default ","
 *   quote   = single-char string, default '"'
 *   strict  = boolean, enable CSV_STRICT (parse only)
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Lua C API headers for embedding this module into a Lua runtime. */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/* The underlying C library (rgamble/libcsv) that handles RFC 4180 parsing. */
#include "../lib/libcsv/csv.h"

/* ============================================================================
 * OPTION APPLICATION
 * ============================================================================
 */

/*
 * Apply user-provided options from a Lua table onto a libcsv parser.
 * This is used for both parse_string and parse_file.
 *
 * @param L      The Lua state (for stack access).
 * @param opt_idx  Stack index of the opts table (or none/nil).
 * @param p      Pointer to the libcsv csv_parser to configure.
 */
static void apply_parse_opts(lua_State *L, int opt_idx, struct csv_parser *p)
{
    /* If no opts table was passed, use libcsv defaults (comma, double quote). */
    if (lua_isnoneornil(L, opt_idx))
        return;

    /* Ensure the caller actually passed a table, not a number/string. */
    luaL_checktype(L, opt_idx, LUA_TTABLE);

    /* --- delimiter --- */
    /* Push opts.delim onto the stack and check if it's a string. */
    lua_getfield(L, opt_idx, "delim");
    if (lua_isstring(L, -1)) {
        /* Extract the first character of the string as the delimiter byte. */
        const char *s = lua_tostring(L, -1);
        if (s && s[0]) csv_set_delim(p, (unsigned char)s[0]);
    }
    /* Pop the value we just pushed (opts.delim or nil). */
    lua_pop(L, 1);

    /* --- quote character --- */
    /* Same pattern: push opts.quote, extract first char if present. */
    lua_getfield(L, opt_idx, "quote");
    if (lua_isstring(L, -1)) {
        const char *s = lua_tostring(L, -1);
        if (s && s[0]) csv_set_quote(p, (unsigned char)s[0]);
    }
    lua_pop(L, 1);

    /* --- strict mode --- */
    /* If opts.strict is truthy, enable CSV_STRICT on the parser.
     * Strict mode rejects malformed CSV (e.g., unclosed quotes). */
    lua_getfield(L, opt_idx, "strict");
    if (lua_toboolean(L, -1)) {
        unsigned char cur = csv_get_opts(p);
        csv_set_opts(p, cur | CSV_STRICT);
    }
    lua_pop(L, 1);
}

/* ============================================================================
 * PARSING CONTEXT & CALLBACKS
 * ============================================================================
 */

/*
 * A growable buffer for one CSV field. libcsv gives us field data in chunks
 * via callbacks, so we accumulate each field into a dynamically allocated
 * buffer before flushing it to a Lua table.
 */
typedef struct {
    char *data;   /* Pointer to the accumulated field bytes (malloc'd). */
    size_t len;   /* Length of the field in bytes. */
} field_buf;

/*
 * Context structure passed through libcsv callbacks back into our code.
 * Holds the Lua state, the rows table being built, and a dynamic array of
 * field buffers for the current row.
 */
typedef struct {
    lua_State *L;        /* Lua state for pushing results onto the stack. */
    int rows_idx;        /* Stack index of the top-level rows table. */
    int row_count;       /* How many complete rows have been flushed so far. */

    field_buf *fields;   /* Dynamic array of field buffers for the current row. */
    int field_count;     /* How many fields are in the current row so far. */
    int capacity;        /* Allocated capacity of the fields array. */
} parse_ctx;

/* Initial capacity for the fields array. Doubles on overflow. */
#define INITIAL_FIELD_CAP 32

/*
 * Ensure the fields array has room for at least one more field.
 * Grows the array by doubling when full. Returns 1 on success, 0 on OOM.
 *
 * @param ctx  The parsing context whose fields array may need growth.
 * @return 1 if capacity is sufficient (or was grown), 0 if realloc failed.
 */
static int ensure_field_capacity(parse_ctx *ctx)
{
    /* If we still have room, nothing to do. */
    if (ctx->field_count < ctx->capacity)
        return 1;

    /* Double the capacity, or start at INITIAL_FIELD_CAP if currently zero. */
    int new_cap = (ctx->capacity == 0) ? INITIAL_FIELD_CAP : ctx->capacity * 2;

    /* Attempt to grow the fields array. */
    field_buf *new_fields =
        (field_buf *)realloc(ctx->fields, sizeof(field_buf) * new_cap);

    /* If realloc failed, signal OOM so the caller can abort gracefully. */
    if (!new_fields)
        return 0;

    ctx->fields = new_fields;
    ctx->capacity = new_cap;
    return 1;
}

/*
 * Free all field buffers in the current row and reset the field counter.
 * Called after a row is flushed to Lua, or during cleanup.
 *
 * @param ctx  The parsing context whose row buffers should be cleared.
 */
static void free_row_buffer(parse_ctx *ctx)
{
    /* If no fields array was ever allocated, nothing to free. */
    if (!ctx->fields)
        return;

    /* Free each field's individually malloc'd data buffer. */
    for (int i = 0; i < ctx->field_count; i++) {
        free(ctx->fields[i].data);
        ctx->fields[i].data = NULL;
        ctx->fields[i].len = 0;
    }

    /* Reset the field counter so the next row starts fresh. */
    ctx->field_count = 0;
}

/*
 * Complete cleanup of a parse_ctx. Frees all row buffers and the fields
 * array itself. Safe to call even on partially initialized contexts.
 *
 * @param ctx  The parsing context to destroy.
 */
static void destroy_parse_ctx(parse_ctx *ctx)
{
    free_row_buffer(ctx);
    free(ctx->fields);
    ctx->fields = NULL;
    ctx->capacity = 0;
}

/*
 * Flush the current row (all accumulated fields) into the Lua rows table.
 * Creates a new Lua table for the row, pushes each field as a string,
 * then appends the row to the top-level rows array.
 *
 * @param ctx  The parsing context holding the completed row.
 * @return 1 on success (always returns 1; errors are Lua longjmps).
 */
static int flush_row_to_lua(parse_ctx *ctx)
{
    lua_State *L = ctx->L;

    /* Skip blank rows (zero fields). This prevents empty trailing newlines
     * from producing spurious empty tables in the result. */
    if (ctx->field_count == 0)
        return 1; /* ignore blank row */

    /* Create a new Lua table to hold this row's fields (array part). */
    lua_createtable(L, ctx->field_count, 0);

    /* Push each field buffer as a Lua string and assign it to the row table.
     * lua_rawseti uses integer keys (1-based, matching Lua conventions). */
    for (int i = 0; i < ctx->field_count; i++) {
        lua_pushlstring(L, ctx->fields[i].data, ctx->fields[i].len);
        lua_rawseti(L, -2, i + 1);
    }

    /* Increment the row counter and append this row to the top-level rows table. */
    ctx->row_count++;
    lua_rawseti(L, ctx->rows_idx, ctx->row_count);

    /* Wipe the field buffers so the next row starts clean. */
    free_row_buffer(ctx);
    return 1;
}

/*
 * libcsv callback: called every time a complete field is parsed.
 * Receives the field bytes and copies them into our growable buffer.
 *
 * @param s    Pointer to the field data (may contain embedded nulls).
 * @param len  Length of the field data in bytes.
 * @param data Opaque user pointer (our parse_ctx).
 */
static void field_cb(void *s, size_t len, void *data)
{
    parse_ctx *ctx = (parse_ctx *)data;
    char *copy;

    /* Make sure we have room in the fields array. If OOM, we silently
     * return and let the parser fail later or produce incomplete output. */
    if (!ensure_field_capacity(ctx))
        return; /* OOM; parser will fail later or produce incomplete result */

    /* Allocate a private copy of the field data. libcsv may reuse its
     * internal buffer, so we must copy before the callback returns. */
    copy = (char *)malloc(len);
    if (!copy)
        return;

    /* Copy the field bytes. s may be NULL if len is 0 (empty field). */
    if (s && len > 0)
        memcpy(copy, s, len);

    /* Store the copy in the next slot of the fields array. */
    ctx->fields[ctx->field_count].data = copy;
    ctx->fields[ctx->field_count].len = len;
    ctx->field_count++;
}

/*
 * libcsv callback: called at the end of every row (when a newline or
 * EOF is reached). Triggers flushing the accumulated fields to Lua.
 *
 * @param c    The character that triggered the row end (unused).
 * @param data Opaque user pointer (our parse_ctx).
 */
static void row_cb(int c, void *data)
{
    parse_ctx *ctx = (parse_ctx *)data;
    (void)c; /* c is provided by libcsv but we don't need it. */

    flush_row_to_lua(ctx);
}

/* ============================================================================
 * CORE PARSING LOGIC
 * ============================================================================
 */

/*
 * Shared implementation for parse_string and parse_file.
 * Initializes libcsv, applies options, runs the parser, and builds the
 * Lua rows table. Handles both success and error paths.
 *
 * @param L        The Lua state.
 * @param data     Pointer to the raw CSV text buffer.
 * @param len      Length of the buffer in bytes.
 * @param opt_idx  Stack index of the opts table (or none).
 * @return Number of Lua return values: 1 (rows table) on success,
 *         2 (nil, err string) on failure.
 */
static int do_parse(lua_State *L, const char *data, size_t len, int opt_idx)
{
    struct csv_parser p;
    parse_ctx ctx;
    size_t parsed;

    /* Initialize the libcsv parser. Non-zero means initialization failed. */
    if (csv_init(&p, 0) != 0)
        return luaL_error(L, "csv_init failed");

    /* Apply delimiter/quote/strict overrides from the Lua opts table. */
    apply_parse_opts(L, opt_idx, &p);

    /* Create the top-level Lua table that will hold all parsed rows.
     * We pre-allocate 128 slots as a hint to the Lua VM (not a hard limit). */
    lua_createtable(L, 128, 0);

    /* Initialize the parse context. rows_idx points to the table we just created. */
    ctx.L = L;
    ctx.rows_idx = lua_gettop(L);
    ctx.row_count = 0;
    ctx.fields = NULL;
    ctx.field_count = 0;
    ctx.capacity = 0;

    /* Run the parser. libcsv calls field_cb and row_cb as it processes the buffer. */
    parsed = csv_parse(&p, data, len, field_cb, row_cb, &ctx);

    /* If parsed != len, libcsv hit a syntax error before consuming all input. */
    if (parsed != len) {
        /* Extract the human-readable error message from libcsv. */
        const char *err = csv_strerror(csv_error(&p));
        /* Clean up all allocated memory before returning to Lua. */
        destroy_parse_ctx(&ctx);
        csv_free(&p);

        /* Pop the partial rows table, then push nil + error string. */
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushfstring(L, "csv parse error: %s (at byte %d)", err, (int)parsed);
        return 2;
    }

    /* Finalize parsing. Handles any trailing field that wasn't followed by a newline. */
    if (csv_fini(&p, field_cb, row_cb, &ctx) != 0) {
        const char *err = csv_strerror(csv_error(&p));
        destroy_parse_ctx(&ctx);
        csv_free(&p);

        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushfstring(L, "csv finalize error: %s", err);
        return 2;
    }

    /* Success path: clean up and leave the rows table on the stack. */
    destroy_parse_ctx(&ctx);
    csv_free(&p);

    return 1;
}

/*
 * Lua binding: core.parse_string(str, opts)
 * Parses CSV text from a Lua string and returns a table of row tables.
 *
 * @param L  Lua state. Stack: [1] = string, [2] = opts table (optional).
 * @return 1 (rows table) or 2 (nil, error string).
 */
static int l_parse_string(lua_State *L)
{
    size_t len;
    /* Extract the input string and its length from the Lua stack. */
    const char *data = luaL_checklstring(L, 1, &len);
    /* Pass opts at stack index 2 (may be nil/none). */
    return do_parse(L, data, len, 2);
}

/*
 * Lua binding: core.parse_file(path, opts)
 * Reads an entire file into memory, then parses it as CSV.
 * Uses full-buffer parse (faster in benchmarks than chunk-by-chunk).
 *
 * @param L  Lua state. Stack: [1] = filename string, [2] = opts table (optional).
 * @return 1 (rows table) or 2 (nil, error string).
 */
/* reverted to full-buffer parse (faster in benchmark) */
static int l_parse_file(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    FILE *fp;
    char *buf;
    long size;
    size_t nread;
    int result;

    /* Open the file in binary mode to preserve exact byte sequences. */
    fp = fopen(filename, "rb");
    if (!fp) {
        /* fopen failed: push nil + a descriptive error message. */
        lua_pushnil(L);
        lua_pushfstring(L, "could not open '%s': %s", filename, strerror(errno));
        return 2;
    }

    /* Determine file size by seeking to the end. */
    if (fseek(fp, 0, SEEK_END) != 0 || (size = ftell(fp)) < 0) {
        fclose(fp);
        lua_pushnil(L);
        lua_pushfstring(L, "could not determine size of '%s'", filename);
        return 2;
    }

    /* Return to the beginning so we can read the whole file. */
    rewind(fp);

    /* Allocate a buffer large enough for the entire file (+1 for safety). */
    buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(fp);
        return luaL_error(L, "out of memory reading '%s'", filename);
    }

    /* Read the entire file into our buffer in one shot. */
    nread = fread(buf, 1, (size_t)size, fp);
    fclose(fp);

    /* Parse the buffer and return the result (rows table or error). */
    result = do_parse(L, buf, nread, 2);

    /* Free the temporary buffer regardless of parse success or failure. */
    free(buf);
    return result;
}

/* ============================================================================
 * WRITE OPTION APPLICATION
 * ============================================================================
 */

/*
 * Apply user-provided options from a Lua table for writing.
 * Unlike parsing, writing doesn't use a libcsv parser state, so we just
 * extract quote and delim into local C variables.
 *
 * @param L       The Lua state.
 * @param opt_idx Stack index of the opts table (or none/nil).
 * @param quote   Output: the quote character to use (default '"').
 * @param delim   Output: the delimiter string to use (default ",").
 */
static void apply_write_opts(lua_State *L, int opt_idx, unsigned char *quote, const char **delim)
{
    /* Set defaults before checking for overrides. */
    *quote = CSV_QUOTE;  /* libcsv's default quote char (usually '"') */
    *delim = ",";

    /* If no opts table, stick with the defaults. */
    if (lua_isnoneornil(L, opt_idx))
        return;

    luaL_checktype(L, opt_idx, LUA_TTABLE);

    /* --- quote character --- */
    lua_getfield(L, opt_idx, "quote");
    if (lua_isstring(L, -1)) {
        const char *s = lua_tostring(L, -1);
        if (s && s[0])
            *quote = (unsigned char)s[0];
    }
    lua_pop(L, 1);

    /* --- delimiter --- */
    lua_getfield(L, opt_idx, "delim");
    if (lua_isstring(L, -1)) {
        const char *s = lua_tostring(L, -1);
        if (s && s[0])
            *delim = s;
    }
    lua_pop(L, 1);
}

/* ============================================================================
 * ROW WRITING
 * ============================================================================
 */

/*
 * Write a single Lua row table into a LuaL_Buffer.
 * Handles quoting rules: fields containing the delimiter, quote char,
 * or line breaks are wrapped in quotes, with internal quotes doubled.
 *
 * @param L         The Lua state.
 * @param b         The LuaL_Buffer to append into.
 * @param row_idx   Stack index of the Lua row table.
 * @param quote     The quote character (e.g., '"').
 * @param delim     The delimiter string (e.g., ",").
 * @param quote_all If non-zero, force quotes around every field.
 */
static void write_one_row(lua_State *L, luaL_Buffer *b, int row_idx,
                          unsigned char quote, const char *delim, int quote_all)
{
    /* Determine how many fields are in this row (Lua array length). */
    lua_Integer nfields = (lua_Integer)lua_rawlen(L, row_idx);

    /* Iterate over each field in the row. */
    for (lua_Integer f = 1; f <= nfields; f++) {
        size_t flen;
        const char *fval;
        int needs_quote = quote_all;

        /* Push the field value onto the stack and extract as a string. */
        lua_rawgeti(L, row_idx, f);
        fval = luaL_checklstring(L, -1, &flen);

        /* If not forcing quotes on all fields, scan the field to see if it
         * contains any characters that require quoting per RFC 4180:
         *   - the quote character itself
         *   - the delimiter
         *   - line breaks (\n or \r) */
        if (!needs_quote) {
            for (size_t i = 0; i < flen; i++) {
                char c = fval[i];
                if (c == quote || c == delim[0] || c == '\n' || c == '\r') {
                    needs_quote = 1;
                    break;
                }
            }
        }

        /* If the field needs quoting, wrap it and escape internal quotes. */
        if (needs_quote) {
            /* Open the quote. */
            luaL_addchar(b, quote);

            /* Copy the field character by character, doubling any quote chars. */
            for (size_t i = 0; i < flen; i++) {
                char c = fval[i];
                if (c == quote)
                    luaL_addchar(b, quote);  /* Escape by doubling. */
                luaL_addchar(b, c);
            }

            /* Close the quote. */
            luaL_addchar(b, quote);
        } else {
            /* No quoting needed: append the raw field bytes directly. */
            luaL_addlstring(b, fval, flen);
        }

        /* Append the delimiter between fields, but not after the last field. */
        if (f < nfields)
            luaL_addstring(b, delim);

        /* Pop the field value off the stack to keep the stack clean. */
        lua_pop(L, 1);
    }
}

/* ============================================================================
 * LUA BINDINGS: WRITE FUNCTIONS
 * ============================================================================
 */

/*
 * Lua binding: core.write_row(row, opts)
 * Serializes a single Lua row table into a CSV string (no line terminator).
 *
 * @param L  Lua state. Stack: [1] = row table, [2] = opts table (optional).
 * @return 1 (CSV string).
 */
/* core.write_row(row, opts) -> string */
static int l_write_row(lua_State *L)
{
    unsigned char quote;
    const char *delim;
    int quote_all = 0;
    luaL_Buffer b;

    /* Ensure the first argument is a table (the row). */
    luaL_checktype(L, 1, LUA_TTABLE);
    apply_write_opts(L, 2, &quote, &delim);

    /* Check if opts.quote_all is set to force quoting on every field. */
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "quote_all");
        quote_all = lua_toboolean(L, -1);
        lua_pop(L, 1);
    }

    /* Initialize a LuaL_Buffer for efficient string building. */
    luaL_buffinit(L, &b);
    write_one_row(L, &b, 1, quote, delim, quote_all);
    /* Push the final accumulated string onto the Lua stack. */
    luaL_pushresult(&b);

    return 1;
}

/*
 * Lua binding: core.write_rows(rows, opts)
 * Serializes multiple Lua row tables into a CSV string with CRLF line endings.
 *
 * @param L  Lua state. Stack: [1] = rows table, [2] = opts table (optional).
 * @return 1 (CSV string).
 */
/* core.write_rows(rows, opts) -> string */
static int l_write_rows(lua_State *L)
{
    unsigned char quote;
    const char *delim;
    int quote_all = 0;
    luaL_Buffer b;
    lua_Integer nrows;

    /* Ensure the first argument is a table (array of rows). */
    luaL_checktype(L, 1, LUA_TTABLE);
    apply_write_opts(L, 2, &quote, &delim);

    /* Check if opts.quote_all is set. */
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "quote_all");
        quote_all = lua_toboolean(L, -1);
        lua_pop(L, 1);
    }

    luaL_buffinit(L, &b);
    /* Count how many rows are in the top-level array. */
    nrows = (lua_Integer)lua_rawlen(L, 1);

    /* Serialize each row, appending CRLF between rows (but not after the last). */
    for (lua_Integer r = 1; r <= nrows; r++) {
        /* Push the row table onto the stack. */
        lua_rawgeti(L, 1, r);
        /* Ensure it's actually a table (defensive check). */
        luaL_checktype(L, -1, LUA_TTABLE);

        /* Serialize this row into the buffer. */
        write_one_row(L, &b, lua_gettop(L), quote, delim, quote_all);

        /* Pop the row table. */
        lua_pop(L, 1);

        /* Append CRLF line terminator between rows, not after the last one. */
        if (r < nrows)
            luaL_addstring(&b, "\r\n");
    }

    luaL_pushresult(&b);
    return 1;
}

/* ============================================================================
 * MODULE REGISTRATION
 * ============================================================================
 */

/* Array of functions exposed to Lua. Must end with {NULL, NULL}. */
static const luaL_Reg core_funcs[] = {
    {"parse_string", l_parse_string},
    {"parse_file",   l_parse_file},
    {"write_row",    l_write_row},
    {"write_rows",   l_write_rows},
    {NULL, NULL}
};

/* Windows DLL export macro for shared library builds. */
#ifdef _WIN32
__declspec(dllexport)
#endif
/*
 * Module entry point: called by Lua when require("sheets.core") is executed.
 * Registers the four functions in a new table and attaches a version string.
 *
 * @param L  The Lua state.
 * @return 1 (the module table is left on the stack).
 */
int luaopen_sheets_core(lua_State *L)
{
    /* Create a new table and register all functions from core_funcs. */
    luaL_newlib(L, core_funcs);
    /* Attach the libcsv version as a metadata field. */
    lua_pushstring(L, "3.0.3");
    lua_setfield(L, -2, "_LIBCSV_VERSION");
    return 1;
}
