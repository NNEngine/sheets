# Design Philosophy of libcsv

Understanding the design philosophy of **libcsv** begins with understanding the problem it solves. CSV (Comma-Separated Values) is a widely used text format for storing tabular data. Each row represents a **record**, and each individual value inside a row is called a **field**. For example, in the row:

`Shivam,21,Gwalior`

there are three fields: `Shivam`, `21`, and `Gwalior`.

At first, CSV may seem simple because it looks like values are merely separated by commas. However, real-world CSV data is more complicated than simple string splitting. A comma may either act as a separator or be part of the actual data. For example:

`"Shivam, Sharma",21,Gwalior`

Here, the comma inside `"Shivam, Sharma"` belongs to the field itself and should not split the data. Similarly, newline characters usually indicate the end of a row, but they can also appear inside quoted fields. Because of these ambiguities, CSV processing requires careful interpretation of input rather than basic text splitting.

## Core Philosophy

The central philosophy behind libcsv is:

> CSV parsing is a stream interpretation problem, not simply a file processing problem.

This idea drives the entire design of the library. Instead of treating CSV as lines to split, libcsv treats input as a continuous stream of characters. Each character is processed one at a time, and its meaning is determined by the current context.

For example, consider the comma character:

- Outside quotation marks, a comma marks the end of a field.
- Inside quotation marks, the same comma is treated as ordinary data.

This means the parser cannot decide what a character means by looking at the character alone. It must also know its current context.

## The Role of the Parser

A **parser** is a system that reads raw input and interprets it according to a set of rules. In libcsv, the parser acts like an intelligent interpreter that continuously reads incoming characters and decides what they represent.

As characters arrive, the parser keeps answering questions such as:

- Am I at the start of a field?
- Am I inside a quoted field?
- Does this comma end a field?
- Does this newline end a record?
- Is this character actual data?

This decision-making process is implemented using a **state machine**, where the parser always maintains its current state. In libcsv, variables such as `pstate` and `quoted` are responsible for tracking this state.

This state-driven design is the heart of the library because CSV interpretation is entirely context-dependent.

## Building Fields Dynamically

As the parser reads characters belonging to a field, it must store them somewhere until the field is complete. libcsv uses an internal dynamic buffer called `entry_buf` for this purpose.

For example, while reading the field:

`Shivam`

characters arrive one by one:

`S → h → i → v → a → m`

Each character is appended to the internal buffer. Once a delimiter (such as a comma) or a record terminator (such as newline) is encountered, the parser knows the field is complete.

At that point, the buffer contains the full field value.

Since CSV fields can vary greatly in size, libcsv does not assume a fixed buffer size. Some fields may contain only a few bytes, while others may contain long text content spanning thousands of characters. To handle this efficiently, the buffer grows dynamically whenever needed.

This is managed through:

- `entry_pos` → current amount of data stored
- `entry_size` → total allocated buffer size
- `blk_size` → growth block size when expansion is needed

This design ensures memory efficiency while remaining flexible for large fields.

## Event-Driven Architecture

One of the most elegant design decisions in libcsv is its **event-driven architecture** using callbacks.

A **callback** is a user-provided function that the library calls automatically when a specific event occurs.

Instead of parsing an entire CSV file and returning a complete table structure, libcsv processes input incrementally and notifies the user whenever meaningful parsing events occur.

Two major events exist:

- **Field Completed (`cb1`)**
  Called whenever a field has been fully parsed.

- **Record Completed (`cb2`)**
  Called whenever an entire row has been parsed.

For example, while parsing:

`Shivam,21,Gwalior`

the parser generates events like:

- Field completed → `Shivam`
- Field completed → `21`
- Field completed → `Gwalior`
- Record completed

This means libcsv does not own or store the final dataset. Instead, it focuses only on parsing and leaves data storage or processing decisions to the user.

This design provides major advantages:

- Low memory usage
- Efficient streaming support
- Suitable for large CSV files
- Real-time processing capability

This makes libcsv highly scalable and efficient.

## Configurability and Flexibility

Another major aspect of libcsv’s design philosophy is flexibility.

Real-world CSV formats are not always identical. Different systems may use:

- commas
- semicolons
- tabs
- custom quote characters

Because of this, libcsv avoids hardcoded assumptions and provides configuration options for parser behavior.

Examples include:

- `csv_set_delim()` → set delimiter character
- `csv_set_quote()` → set quote character
- `csv_set_opts()` → configure parser behavior

This allows the library to support multiple CSV dialects instead of enforcing one rigid standard.

## Separation of Engine and Policy

libcsv also follows an important software design principle:

> Separate the parsing engine from parsing policy.

The parsing engine is responsible for reading characters and interpreting CSV structure. Policy defines the specific rules used during parsing.

For example, libcsv does not permanently define:

- what counts as whitespace
- what counts as a line terminator

Instead, users can provide custom functions such as:

- `is_space()`
- `is_term()`

This separation keeps the parser core lightweight while allowing extensive customization.

## Custom Memory Management

Another advanced design choice is customizable memory management.

Most libraries directly use standard allocation functions such as:

- `malloc`
- `realloc`
- `free`

However, some systems use specialized memory allocators, memory pools, or embedded allocation strategies. To support such environments, libcsv allows users to provide custom memory management functions.

This makes the library suitable for:

- embedded systems
- performance-critical applications
- custom runtime environments

This reflects a strong systems-programming mindset.

## Overall Design Philosophy

The design of libcsv follows classic C library principles:

- Minimal
- Fast
- Lightweight
- Memory-efficient
- Highly configurable

Most importantly, libcsv focuses on doing one job extremely well.

It does not attempt to manage high-level CSV data structures or provide complex abstractions. Instead, its single responsibility is clear:

> Convert a raw stream of characters into meaningful CSV parsing events.

By combining state-driven parsing, dynamic buffering, event-driven callbacks, and flexible configuration, libcsv achieves a design that is simple in concept yet powerful in practice. This makes it an excellent example of clean, efficient, and well-engineered systems-level library design.




---



# Working of libcsv Source Code (`csv.c`)

Understanding the header file of libcsv provides insight into the design and architecture of the library, but the source file (`csv.c`) reveals how the entire system actually works in practice. While individual functions such as `csv_init()`, `csv_parse()`, `csv_fini()`, and `csv_write()` may appear understandable when studied separately, the true challenge lies in understanding how all these functions interact as one complete system.

At a high level, the source code of libcsv can be divided into two major subsystems:

- **Parsing subsystem** → Converts CSV data into structured fields and rows
- **Writing subsystem** → Converts raw text into CSV-safe formatted output

The parsing subsystem is the core of the library and represents the primary complexity. The writing subsystem is comparatively straightforward and performs the reverse operation.

## Parsing Subsystem

The parsing subsystem is responsible for reading CSV input and transforming it into meaningful parsing events such as “field completed” and “row completed.” This subsystem revolves around the parser object (`struct csv_parser`), which stores the parser’s complete internal state.

The lifecycle of parsing generally follows this sequence:

`csv_init() → csv_parse() → csv_fini() → csv_free()`

Each function plays a specific role in this lifecycle.

### Parser Initialization (`csv_init`)

The first step in using libcsv is creating and initializing a parser object. This is done through `csv_init()`.

At this stage, the parser is essentially being prepared for work. Before parsing begins, it needs default configuration values such as delimiter character, quote character, memory allocation behavior, parser options, and internal state values.

When `csv_init()` is called, the parser starts in a clean state:

- No row has started
- No field has started
- No memory buffer is allocated yet
- No parsing errors exist

Internally, the parser state is initialized to `ROW_NOT_BEGUN`, which means parsing has not yet entered any row. The dynamic buffer (`entry_buf`) is initially set to `NULL`, meaning memory is allocated lazily only when needed. This design avoids unnecessary memory allocation before actual parsing begins.

This initialization step effectively prepares the parser to begin processing incoming CSV data.

### Parsing Input (`csv_parse`)

The core of libcsv lies inside `csv_parse()`. This function is responsible for processing CSV input character by character using a state-driven parsing mechanism.

A key design choice in libcsv is that input is treated as a stream rather than a complete file. This means data can arrive in chunks instead of all at once. For example, input may come from:

- files
- network sockets
- pipes
- real-time streams

This makes libcsv highly flexible because it does not assume the entire CSV file is available in memory.

For example, consider the following CSV input:

`name,comment`
`Shivam,"I love AI, math"`

This data may reach the parser in chunks such as:

Chunk 1:
`name,co`

Chunk 2:
`mment\nShiv`

Chunk 3:
`am,"I love AI, math"`

Even though the data arrives in pieces, libcsv processes it correctly because the parser maintains its internal state across multiple calls to `csv_parse()`.

This ability to preserve state across parsing calls is one of the most important design decisions in the library.

### Dynamic Buffer Management (`csv_increase_buffer`)

Before characters can be processed, the parser needs memory to store the current field being built. This storage is provided by the internal dynamic buffer called `entry_buf`.

Initially this buffer is `NULL`, meaning no memory exists. The first time parsing begins, memory is allocated using `csv_increase_buffer()`.

The default allocation block size is 128 bytes. This means the parser initially allocates 128 bytes for storing field data.

As characters are parsed, they are appended into this buffer. If the buffer becomes full, `csv_increase_buffer()` is called again to expand the buffer size.

This function grows the buffer incrementally and carefully handles edge cases such as:

- memory allocation failure
- size overflow (`SIZE_MAX`)
- partial allocation recovery

This ensures that libcsv remains efficient and robust even when processing extremely large fields.

### State-Driven Character Processing

The most important part of `csv_parse()` is the state machine. This is where actual CSV interpretation happens.

Each incoming character is processed according to the parser’s current state. libcsv primarily uses four states:

- `ROW_NOT_BEGUN`
- `FIELD_NOT_BEGUN`
- `FIELD_BEGUN`
- `FIELD_MIGHT_HAVE_ENDED`

These states determine how characters such as commas, quotes, spaces, and newlines should be interpreted.

Consider parsing the first field from:

`name,comment`

Initially, the parser is in `ROW_NOT_BEGUN`.

When the character `n` arrives, the parser checks whether it is whitespace, delimiter, quote, or newline. Since it is none of these, the parser recognizes that a field has started.

The parser transitions from `ROW_NOT_BEGUN` to `FIELD_BEGUN`.

Characters `n`, `a`, `m`, and `e` are sequentially stored in the internal buffer.

When the parser encounters a comma, interpretation depends entirely on context. Since the parser is not inside a quoted field, the comma indicates that the current field has ended.

At this point, the parser triggers field submission.

### Field Submission (`SUBMIT_FIELD`)

Field submission is performed through the `SUBMIT_FIELD` macro.

This marks the completion of one field and performs several important operations:

- Removes trailing spaces if necessary
- Appends null terminator if `CSV_APPEND_NULL` is enabled
- Handles empty field behavior
- Invokes the user’s field callback (`cb1`)
- Resets field-related parser state

For example, after parsing `name`, field submission triggers:

`cb1("name", 4, data)`

This callback informs the user that a complete field has been parsed.

Once submitted, field-related variables such as `entry_pos`, `quoted`, and `spaces` are reset so the parser can begin constructing the next field.

### Quoted Field Processing

Quoted fields introduce the primary complexity in CSV parsing.

Consider the field:

`"I love AI, math"`

When the opening quote is encountered at the start of a field, the parser marks the field as quoted by setting `quoted = 1`.

This changes the meaning of several characters.

For example, commas normally indicate field boundaries. However, inside a quoted field, commas are treated as ordinary data.

Thus while parsing:

`"I love AI, math"`

the comma inside the field becomes part of the field data rather than acting as a separator.

This is one of the central reasons why CSV parsing cannot be implemented using simple string splitting.

### Handling Closing Quotes (`FIELD_MIGHT_HAVE_ENDED`)

One of the most clever parts of libcsv’s design is the state `FIELD_MIGHT_HAVE_ENDED`.

This state exists because encountering a quote inside a quoted field creates ambiguity.

For example, after reading:

`"hello"`

and encountering the closing quote, the parser cannot immediately determine whether:

- the field has ended, or
- another quote follows, meaning the quote was escaped (`""`)

To resolve this uncertainty, libcsv enters the intermediate state `FIELD_MIGHT_HAVE_ENDED`.

The next character determines the outcome:

- Delimiter → field ended
- Newline → field ended and row ended
- Quote → escaped quote
- Invalid character → possible parsing error in strict mode

This intermediate state allows libcsv to correctly handle escaped quotes and quoted field termination.

### Row Submission (`SUBMIT_ROW`)

When a newline is encountered outside a quoted field, the parser recognizes that the current record has ended.

This triggers row submission through the `SUBMIT_ROW` macro.

Row submission performs two main tasks:

- Invokes the row callback (`cb2`)
- Resets row-related parser state

For example, after parsing:

`name,comment`

the parser generates:

- Field completed → `name`
- Field completed → `comment`
- Row completed

This event-driven design allows users to process CSV data incrementally without storing the entire dataset inside the library.

### Error Handling (`csv_error`, `csv_strerror`)

During parsing, invalid CSV structures may be encountered.

Examples include:

- unescaped quotes
- invalid quote placement
- unclosed quoted fields
- memory allocation failure

When errors occur, the parser stores an error code in its internal `status` field.

Users can retrieve the numeric error using `csv_error()` and convert it into a readable message using `csv_strerror()`.

This design keeps error handling simple and centralized.

### Finalization (`csv_fini`)

After all input chunks have been processed, parsing is not necessarily complete.

A common edge case occurs when a CSV file does not end with a newline.

Example:

`name,age`
`Shivam,21`

If the final row lacks a newline, `csv_parse()` will not automatically submit the final field and row.

This is why `csv_fini()` exists.

Its purpose is to flush any remaining data still stored in the parser.

During finalization, libcsv checks the parser’s current state and submits any pending field or row. It also performs strict finalization checks for malformed quoted fields.

Without `csv_fini()`, the final row of many CSV files could be lost.

### Cleanup (`csv_free`)

Once parsing is complete, the parser’s internal memory must be released.

This is handled by `csv_free()`.

Its responsibility is straightforward:

- free the dynamic buffer
- reset buffer-related state

This completes the lifecycle of the parsing subsystem.

## Writing Subsystem

The writing subsystem performs the reverse operation of parsing. Instead of reading CSV data, it converts raw input into properly formatted CSV-safe output.

This subsystem is built around four functions:

- `csv_write()`
- `csv_write2()`
- `csv_fwrite()`
- `csv_fwrite2()`

The core logic exists inside `csv_write2()` and `csv_fwrite2()`.

### Writing CSV-Safe Output

The purpose of the writing subsystem is to ensure that raw field data is correctly escaped according to CSV rules.

Consider the raw input:

`I love "AI"`

If written directly into CSV, the embedded quote would break CSV formatting.

To make it CSV-safe, quotes inside data must be escaped by doubling them.

Thus:

`I love "AI"`

becomes:

`"I love ""AI"""`

This transformation is handled by `csv_write2()`.

The function wraps input data inside quotes and scans characters one by one. If a quote character is encountered inside the input, it writes an additional quote before writing the actual character.

This ensures the output remains valid CSV.

The functions `csv_write()` and `csv_fwrite()` are convenience wrappers that use the default quote character (`"`), while `csv_fwrite2()` performs the same serialization logic directly to a file stream instead of writing to a memory buffer.

## Overall Working of libcsv

The complete working of libcsv can be summarized as a structured lifecycle.

For parsing:

`csv_init() → csv_parse() → csv_fini() → csv_free()`

For writing:

`raw data → csv_write()/csv_write2() or csv_fwrite()/csv_fwrite2() → valid CSV output`

At its core, libcsv is fundamentally a state machine combined with dynamic buffering and event-driven callbacks. CSV is simply the grammar that this machine understands.

This is the true architectural essence of libcsv. The library does not attempt to manage high-level table structures or complex abstractions. Instead, it focuses on one responsibility: efficiently transforming raw character streams into meaningful CSV parsing events and converting raw data back into valid CSV format.




---



# Working of Lua Binding (`csv/core.c`)

Understanding libcsv alone explains how CSV parsing works at the C level, but `csv/core.c` introduces another important layer: the bridge between **libcsv’s low-level event-driven C world** and **Lua’s high-level table-based world**.

The purpose of this file is not to replace libcsv or reimplement CSV parsing. Instead, its job is to expose libcsv’s parsing and writing capabilities to Lua in a form that feels natural to Lua programmers.

At its core, the design philosophy of this file is:

> Keep performance-critical CSV parsing and serialization in C, while exposing simple table-based APIs to Lua.

This means the C layer focuses only on the essential operations:

- parsing CSV text into Lua tables
- converting Lua tables back into CSV strings

Higher-level abstractions such as:

- reader objects
- writer objects
- iterator protocols
- dictionary readers
- dialect systems

are intentionally kept outside this file in pure Lua code.

This separation of responsibilities creates a clean architecture:

- **C layer** → fast, minimal, performance-critical operations
- **Lua layer** → user-friendly abstractions and ergonomic APIs

This design keeps the binding lightweight, efficient, and easy to extend in the future.

## Overall Architecture

Just like libcsv itself, `csv/core.c` consists of two major subsystems:

- **Parsing subsystem** → CSV text → Lua tables
- **Writing subsystem** → Lua tables → CSV text

The overall flow for parsing is:

`Lua Input → C Binding → libcsv Parser → Lua Table Construction → Lua Output`

The overall flow for writing is:

`Lua Tables → C Binding → CSV Serialization → CSV String`

This file essentially acts as a translator between two very different programming models.

libcsv works with:

- parser states
- callbacks
- raw memory buffers
- low-level C logic

Lua works with:

- strings
- tables
- simple APIs
- high-level abstractions

The purpose of `csv/core.c` is to bridge these two worlds efficiently.

# Parsing Subsystem

The parsing subsystem is responsible for converting CSV input into nested Lua tables.

For example, given CSV input:

`name,age`
`Shivam,21`

the expected Lua output becomes:

```lua
{
    {"name", "age"},
    {"Shivam", "21"}
}
```

This transformation happens using libcsv’s parser combined with Lua table construction.

## Parse Context (`parse_ctx`)

The most important structure in the parsing subsystem is `parse_ctx`.

This structure acts as the bridge between libcsv and Lua while parsing is in progress.

It stores:

- the Lua state
- the current rows table
- the current row being built
- number of parsed fields
- number of completed rows

Its role is crucial because libcsv itself only generates parsing events such as:

- field completed
- row completed

However, Lua expects fully structured tables. The parse context stores enough information to gradually build those tables as parsing events occur.

### Lua State

The `lua_State *L` pointer represents the Lua interpreter state.

This is the entry point into the Lua runtime. Every interaction with Lua from C happens through this pointer.

Using this state, the C code can:

- create tables
- push strings
- insert values
- manipulate the Lua stack

Without this pointer, the C binding would have no way to communicate with Lua.

### Row and Field Tracking

The remaining fields in `parse_ctx` track table construction progress.

- `rows_idx` stores the Lua stack index of the final rows table
- `row_idx` stores the Lua stack index of the current row being built
- `field_count` tracks how many fields exist in the current row
- `row_count` tracks how many rows have been completed

Together, these variables allow the binding to gradually convert libcsv events into Lua tables.

## Callback-Based Table Construction

libcsv works using callbacks. Whenever a field is parsed, libcsv invokes a field callback. Whenever a row ends, it invokes a row callback.

This binding implements those callbacks as:

- `field_cb()`
- `row_cb()`

These functions are where actual Lua table construction happens.

## Field Callback (`field_cb`)

The field callback is called whenever libcsv completes parsing a field.

For example, while parsing:

`name,age`

after the parser reads `name`, libcsv invokes:

`field_cb("name", 4, &ctx)`

The purpose of `field_cb()` is to insert this field into the current Lua row.

The process is straightforward.

First, the callback retrieves the parse context from the generic callback data pointer. This gives access to all parsing state and the Lua interpreter.

Next, it checks whether a row is currently being built.

If no active row exists, a new Lua table is created to represent the current row.

For example, when the first field of a row is encountered:

```lua
{}
```

is created.

This becomes the current row table.

After ensuring a row exists, the callback converts the parsed field into a Lua string.

If libcsv passes `NULL`, the field is treated as an empty string. Otherwise, the field buffer from libcsv is pushed into Lua as a string.

Finally, the field is inserted into the row table using the next numeric index.

For example, while parsing:

`name,age`

the row evolves as:

```lua
{"name"}
```

then:

```lua
{"name", "age"}
```

Thus, every completed field extends the current Lua row.

## Row Callback (`row_cb`)

The row callback is called whenever libcsv detects the end of a row.

For example, after parsing:

`name,age\n`

libcsv invokes the row callback.

At this point, the current row is fully constructed and must be inserted into the final rows table.

The callback first checks whether a row actually exists.

If no active row exists, it means the parser encountered a blank line. In this implementation, blank rows are ignored.

If a valid row exists, it is inserted into the final rows table.

For example:

```lua
rows[1] = {"name", "age"}
```

After insertion, row-related state is reset so parsing can begin constructing the next row.

This callback is what converts a temporary row into a permanent part of the final Lua result.

## Parse Options

The function `apply_parse_opts()` is responsible for translating Lua configuration tables into libcsv parser settings.

For example, a Lua user may provide:

```lua
{
    delim = ";",
    quote = "'",
    strict = true
}
```

This function reads those values and applies them to libcsv using:

* `csv_set_delim()`
* `csv_set_quote()`
* `csv_set_opts()`

This allows Lua users to customize parsing behavior without directly interacting with libcsv’s C API.

## Core Parsing Engine (`do_parse`)

The main parsing logic is implemented inside `do_parse()`.

This function coordinates the entire parsing workflow.

The parsing lifecycle begins with parser initialization using `csv_init()`.

Once the parser is created, parse options are applied if provided by the user.

Next, a Lua table is created to hold the final rows result.

This table eventually becomes the returned Lua value.

After that, the parse context is initialized. At this stage, the bridge between Lua and libcsv is fully prepared.

The core step is calling libcsv’s parser:

`csv_parse(&p, data, len, field_cb, row_cb, &ctx)`

This is the most important point in the entire binding.

At this stage, control shifts to libcsv.

libcsv processes the CSV input character by character. Whenever it completes a field, it calls `field_cb()`. Whenever it completes a row, it calls `row_cb()`.

These callbacks progressively construct the final Lua table.

For example, while parsing:

`name,age`
`Shivam,21`

runtime events occur in this sequence:

* field callback → `"name"`
* field callback → `"age"`
* row callback
* field callback → `"Shivam"`
* field callback → `"21"`
* row callback

These events gradually construct:

```lua
{
    {"name", "age"},
    {"Shivam", "21"}
}
```

This is the heart of the Lua binding.

## Error Handling

After parsing, the function checks whether libcsv successfully processed the entire input.

If parsing fails, the error is retrieved from libcsv using:

* `csv_error()`
* `csv_strerror()`

The binding then converts this into a Lua-friendly error result:

```lua
nil, "csv parse error: ..."
```

This follows Lua’s common convention of returning:

`result | nil, error`

which makes the API intuitive for Lua programmers.

## Finalization and Cleanup

After parsing completes, `csv_fini()` is called.

This step is necessary for the same reason it is required in libcsv: the final row may not end with a newline.

Without finalization, the last row could be lost.

After finalization, parser memory is released using `csv_free()`.

The fully constructed rows table is then returned to Lua.

## Public Parsing APIs

The binding exposes two public parsing functions.

### `parse_string()`

This function parses CSV directly from a Lua string.

Example:

```lua
csv.core.parse_string(csv_text)
```

Internally, it extracts the string data from Lua and passes it to `do_parse()`.

### `parse_file()`

This function parses CSV from a file.

Example:

```lua
csv.core.parse_file("data.csv")
```

It performs three steps:

* opens the file
* reads its contents into memory
* passes data to `do_parse()`

This provides a simple file-based API for Lua users.

# Writing Subsystem

The writing subsystem performs the reverse operation of parsing.

Instead of converting CSV text into Lua tables, it converts Lua tables into valid CSV text.

For example:

```lua
{
    {"name", "age"},
    {"Shivam", "21"}
}
```

becomes:

```csv
name,age
Shivam,21
```

## Write Options

The function `apply_write_opts()` reads Lua configuration options for writing.

These options include:

* delimiter
* quote character

This allows users to customize CSV serialization behavior.

## Quoting Logic (`needs_quoting`)

Before writing a field, the binding must determine whether quoting is required.

A field must be quoted if it contains:

* delimiter
* quote character
* newline
* carriage return

For example:

`hello`

does not require quotes.

However:

`hello,world`

must be written as:

`"hello,world"`

because it contains a delimiter.

This decision is handled by `needs_quoting()`.

## Writing a Row (`write_one_row`)

The function `write_one_row()` converts one Lua row table into CSV text.

Each field is processed sequentially.

If quoting is required, the function uses libcsv’s `csv_write2()` to perform proper CSV escaping.

For example:

```lua
{"Shivam", "I love AI, math"}
```

produces:

`Shivam,"I love AI, math"`

This function builds one complete CSV row without adding a line terminator.

## Public Writing APIs

Two public writing functions are exposed.

### `write_row()`

This converts one Lua row table into a CSV row string.

Example:

```lua
csv.core.write_row(row)
```

It returns a single CSV row without newline characters.

### `write_rows()`

This converts multiple rows into complete CSV text.

Example:

```lua
csv.core.write_rows(rows)
```

Rows are joined using CRLF (`\r\n`) separators.

This produces a complete CSV-formatted string.

# Overall Working of `csv/core.c`

The complete architecture of this Lua binding can be summarized through two workflows.

For parsing:

`Lua Input → do_parse() → libcsv Parser → field_cb()/row_cb() → Lua Table Output`

For writing:

`Lua Tables → write_one_row() → csv_write2() → CSV String`

At its core, `csv/core.c` is a translator between libcsv’s low-level parsing engine and Lua’s high-level table abstraction.

It preserves libcsv’s speed and efficiency while exposing an API that feels natural and convenient for Lua users.

This makes the binding both minimal and powerful, which is exactly what a well-designed C-to-Lua interface should achieve.
