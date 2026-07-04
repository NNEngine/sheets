# FAQ

This page answers common questions about `sheets`.

---

## What is sheets?

`sheets` is a high-performance CSV and tabular data toolkit for Lua.

It combines:

- Native C performance using `libcsv`
- Python-inspired APIs
- Lightweight tabular utilities

It is designed for:

- CSV parsing
- CSV writing
- Data preprocessing
- ETL workflows
- Analytics pipelines

---

## Which Lua versions are supported?

`sheets` currently supports:

- Lua 5.2
- Lua 5.3
- Lua 5.4

---

## Does sheets support streaming?

Not yet.

Currently, `sheets` reads the full input into memory before parsing.

Example:

```lua
local rows = csv.read_csv("data.csv")
```

Future versions may include:

- True streaming parser
- Chunk-based reading
- Async CSV processing

---

## Is sheets faster than Python’s csv module?

It depends on the workload.

Benchmark results:

| Metric | sheets | Python csv |
|--------|---------|------------|
| Parse Time | ~9.2s | ~6.7s |
| Memory Usage | ~644 MB | ~1261 MB |
| Write Time | ~0.17s | ~0.59s |

Key advantages of `sheets`:

- Lower memory usage
- Faster writing performance
- Competitive parsing speed

---

## Why use libcsv?

`sheets` uses `libcsv` because it offers:

- High performance
- RFC 4180 compliant behavior
- Mature implementation
- Minimal dependency footprint

It allows `sheets` to focus on API design while relying on a battle-tested parser.

---

## Does sheets support custom delimiters?

Yes.

Example:

```lua
local rows = csv.parse(text, {
    delim = ";"
})
```

Supported examples:

- Comma `,`
- Semicolon `;`
- Pipe `|`
- Tab `\t`

---

## Does sheets support quoted fields?

Yes.

Example:

```csv
name,note
Alice,"hello,world"
```

This is parsed correctly.

---

## Does sheets support multiline fields?

Yes.

Example:

```csv
name,note
Alice,"hello
world"
```

Multiline quoted fields are supported.

---

## Does sheets support Unicode?

Yes.

Example:

```csv
name,city
Shivam,東京
```

Unicode strings are supported as long as your Lua environment handles UTF-8 correctly.

---

## Can I write numbers and booleans?

Yes.

Values are automatically converted to strings.

Example:

```lua
csv.write({
    {"name", "active", "age"},
    {"Alice", true, 30}
})
```

Becomes:

```csv
name,active,age
Alice,true,30
```

---

## Why are all values strings after parsing?

CSV stores raw text.

Example:

```csv
age
30
```

Parsed result:

```lua
"30"
```

This is expected behavior.

If numeric conversion is needed:

```lua
local age = tonumber(row[2])
```

---

## Can I contribute?

Yes.

Contributions are welcome.

See the **Contributing** page for details.
