-- sheets.reader - Reads CSV data row by row, similar to Python's csv.reader.
--
-- You can loop through the rows like this:
--   local f = io.open("data.csv")
--   for row in sheets.reader(f) do
--       print(row[1], row[2])
--   end
--   f:close()
--
-- The returned object can be called directly in a `for` loop because
-- its `__call` metamethod is set up to act like an iterator.

-- Pull in the low-level C/Lua hybrid parser that handles quoting,
-- escaping, and delimiter splitting from raw text.
local core = require("sheets.core")        -- Low-level CSV parsing engine
-- Pull in the dialect system that defines named presets (e.g., "excel")
-- and merges them with user-provided overrides.
local dialect = require("sheets.dialect") -- Named preset system (like "excel" format)

-- Reader is our main object. We use a Lua "class" pattern where
-- Reader.__index lets missing methods fall back to this table.
local Reader = {}
Reader.__index = Reader

-- Helper: slurp the entire source into a single string.
-- We accept either an open file handle or a plain string of CSV text.
-- This way callers don't have to wrap in-memory data in a fake file object.
-- @param source Either an open file handle (io.file) or a raw CSV string.
-- @return The full contents of the source as a single Lua string.
local function read_all(source)
    -- If the caller passed a raw string, we can use it directly.
    if type(source) == "string" then
        return source
    -- If the caller passed an open file handle, read the entire file.
    elseif io.type(source) == "file" then
        return source:read("*a")
    -- Anything else is a programming error; raise immediately.
    else
local dialect = require("sheets.dialect") -- Named preset system (like "excel" format)
        error("sheets.reader expects a file handle or a string, got " .. type(source), 3)
    end
end

-- Create a new Reader from a source (file handle or string) and options.
-- `opts` is an optional table. You can pass format options directly,
-- or use `opts.dialect = "excel"` to pick a preset. You can also set
-- `opts.sniff = true` to auto-detect the delimiter from the data.
-- @param source A file handle or string containing CSV data.
-- @param opts Optional table with parsing options, dialect preset, or sniff flag.
-- @return A new Reader object ready for iteration.
function Reader.new(source, opts)
    -- Grab all the CSV text into memory first.
    local text = read_all(source)
    -- Normalize opts to an empty table if the caller didn't provide any.
    opts = opts or {}

    -- If sniffing is enabled, try to guess the format (delimiter, etc.)
    -- from the raw text instead of relying on defaults or presets.
    if opts.sniff then
        -- Load the sniffer module lazily so it's only required when needed.
        local sniffer = require("sheets.sniffer")
        -- Analyze the first few lines of text to guess the delimiter.
        local guessed, err = sniffer.sniff(text)
        if guessed then
            -- Start with whatever explicit options the caller gave,
            -- then fill in any missing fields from the sniffer's guess.
            -- This preserves caller overrides while filling gaps.
            opts = dialect.resolve(opts)
            for k, v in pairs(guessed) do
                if opts[k] == nil then opts[k] = v end
            end
        end
    end

    -- Finalize options: merge dialect preset + explicit overrides.
    -- This produces a complete, concrete options table for the parser.
    local resolved = dialect.resolve(opts)

    -- Hand the full text and resolved options to the C parser.
    -- `rows` becomes a Lua table of tables: each inner table is one row.
    local rows, err = core.parse_string(text, resolved)
    if not rows then
        -- Propagate parse errors up to the caller with a stack level
        -- that points back to the Reader.new call site.
        error(err, 2)
    end

    -- Build the Reader object. `_pos` tracks which row we are currently on.
    -- setmetatable wires up the class methods via __index.
    return setmetatable({
        rows = rows,          -- All parsed rows stored in memory
        line_num = 0,          -- Current row number (updated as we iterate)
        _pos = 0,              -- Internal cursor into the rows table
    }, Reader)
end

-- Return the next row as a table, or nil if we've run out of rows.
-- You can also call the reader object directly: `reader()` does the same thing.
-- @return A table of strings representing the next CSV row, or nil at EOF.
function Reader:next_row()
    -- Advance the internal cursor to the next row.
    self._pos = self._pos + 1
    -- Look up the row at the new cursor position.
    local row = self.rows[self._pos]
    if row then
        -- Update the public line_num so callers know which row we're on.
        self.line_num = self._pos
        return row
    end
    -- No more rows: return nil to signal the end of iteration.
    return nil
end

-- Reset the cursor back to the start so you can iterate again.
-- Useful if you want to read the same data more than once.
function Reader:reset()
    self._pos = 0
    self.line_num = 0
end

-- Total number of rows in the file.
-- Since we parse everything upfront, counting is just the table length.
-- @return Integer count of all rows in the parsed data.
function Reader:count()
    return #self.rows
end

-- Get all remaining rows as array
-- Iterates from the current cursor position to the end and collects
-- every row into a flat array table.
-- @return A table of row tables.
function Reader:readall()
    local result = {}
    -- Uses the __call metamethod, so `self` here acts as an iterator.
    for row in self do
        result[#result + 1] = row
    end
    return result
end

-- Get specific row by index (1-based)
-- Allows random access to any row without affecting the iteration cursor.
-- @param index The 1-based row index to retrieve.
-- @return The row table at that index, or nil if out of bounds.
function Reader:get(index)
    -- Guard against invalid indices to prevent confusing silent failures.
    if type(index) ~= "number" or index < 1 then
        error("row index must be a positive number", 2)
    end
    return self.rows[index]
end

-- Get slice of rows
-- Returns a contiguous subsequence of rows without modifying the cursor.
-- @param start The 1-based starting index (defaults to 1).
-- @param finish The 1-based ending index (defaults to last row).
-- @return A table containing the requested slice of row tables.
function Reader:slice(start, finish)
    -- Default to the full range if boundaries aren't provided.
    start = start or 1
    finish = finish or #self.rows

    -- Clamp to valid bounds so callers don't get nil gaps.
    if start < 1 then start = 1 end
    if finish > #self.rows then finish = #self.rows end

    -- Build the result by copying references into a new table.
    local result = {}
    for i = start, finish do
        result[#result + 1] = self.rows[i]
    end
    return result
end

-- Get column by index
-- Extracts a single vertical column across all rows.
-- Missing values in shorter rows are filled with empty strings.
-- @param col_index The 1-based column index to extract.
-- @return A flat table of strings, one per row.
function Reader:column(col_index)
    -- Reject invalid column indices early.
    if type(col_index) ~= "number" or col_index < 1 then
        error("column index must be a positive number", 2)
    end

    -- Walk every row and pluck out the field at col_index.
    local result = {}
    for _, row in ipairs(self.rows) do
        -- If a row is shorter than col_index, default to empty string
        -- so the result stays aligned with the row count.
        result[#result + 1] = row[col_index] or ""
    end
    return result
end

-- Filter rows
-- Returns only the rows that satisfy a predicate function.
-- Does not modify the original Reader; returns a new array.
-- @param predicate A function(row) returning true to keep the row.
-- @return A table of row tables that passed the test.
function Reader:filter(predicate)
    -- Guard against accidental non-function arguments.
    if type(predicate) ~= "function" then
        error("filter predicate must be a function", 2)
    end
    local result = {}
    for _, row in ipairs(self.rows) do
        if predicate(row) then
            result[#result + 1] = row
        end
    end
    return result
end

-- Map rows
-- Transforms every row through a user-provided function.
-- Does not modify the original Reader; returns a new array.
-- @param transformer A function(row) returning the transformed value.
-- @return A table of transformed values, one per row.
function Reader:map(transformer)
    -- Guard against accidental non-function arguments.
    if type(transformer) ~= "function" then
        error("map transformer must be a function", 2)
    end
    local result = {}
    for _, row in ipairs(self.rows) do
        result[#result + 1] = transformer(row)
    end
    return result
end

-- This magic line makes the Reader object itself callable in a `for` loop.
-- `for row in reader do ... end` is shorthand for repeatedly calling `reader()`.
-- Lua's generic-for protocol calls this function to get the iterator state.
Reader.__call = function(self)
    return self:next_row()
end

-- Expose the Reader module. Callers typically use Reader.new().
return Reader
