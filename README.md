# sheets


![logo](docs/assets/logo.png)


A fast, memory-efficient CSV and tabular data toolkit for Lua.

Built on top of libcsv with a hybrid architecture:
- High-performance CSV parsing and writing in C
- Flexible Python-inspired API in Lua
- Lightweight tabular data transformation utilities

`sheets` combines native C performance with Lua’s simplicity, making it suitable for:

- Data preprocessing
- ETL pipelines
- Analytics workloads
- CSV-heavy backend services
- Machine learning workflows

```
               ┌──────────────────────┐
               │      User Code       │
               └──────────┬───────────┘
                          │
                    require("csv")
                          │
          ┌───────────────┴───────────────┐
          │                               │
   ┌──────▼──────┐                 ┌──────▼──────┐
   │  Reader API │                 │ Writer API  │
   └──────┬──────┘                 └──────┬──────┘
          │                               │
   ┌──────▼───────────────────────────────▼──────┐
   │             Lua API Layer                   │
   │ reader.lua writer.lua dictreader.lua ...    │
   └──────────────────────┬──────────────────────┘
                          │
                   require("csv.core")
                          │
             ┌────────────▼────────────┐
             │      C Core Layer       │
             │       csv/core.c        │
             └────────────┬────────────┘
                          │
                  ┌───────▼────────┐
                  │     libcsv     │
                  │ CSV Parser Lib │
                  └────────────────┘
```
---

## Features

- Fast CSV parsing with low memory usage
- High-performance CSV writing
- Python `csv`-style API
- Reader / Writer / DictReader / DictWriter
- Dialect support
- CSV sniffer
- Built-in tabular data utilities
- Cross-platform (Linux / Windows)
- Lua 5.2+

---

## Performance

`sheets` is designed for high-performance CSV processing with low memory overhead.

Benchmarks were run on a synthetic CSV file containing:

- **1,000,000 rows**
- **20 columns per row**
- Mixed field types:
  - integers
  - floats
  - strings
  - quoted fields
- Total size: ~200–300 MB (approx.)

Benchmark operations:
- **Parse** → Read and fully parse CSV into Lua/Python table structures
- **Write** → Serialize rows back into CSV text
- **Memory Usage** → Peak memory after full parse

### Benchmark Results

| Metric | sheets | Python csv |
|--------|---------|------------|
| Parse Time | ~9.2s | ~6.7s |
| Memory Usage | ~644 MB | ~1261 MB |
| Write Time | ~0.17s | ~0.59s |

### Key Takeaways

- **~2× lower memory usage** than Python’s built-in `csv` module
- **~3.5× faster CSV writing**
- Parse speed is competitive with Python’s highly optimized C implementation (`_csv.c`)

While Python’s parser is faster for full in-memory parsing, `sheets` offers significantly better memory efficiency and superior write performance, making it well-suited for:

- ETL pipelines
- Data preprocessing
- CSV-heavy backend workloads
- Memory-sensitive environments

### Benchmark Scripts

Benchmark scripts are available in:

```text
examples/comparison.lua
test/python_csv_bench.py
```

---

## Architecture

The library is split into two layers by design.

```text
sheets/
├── csv/
│   ├── core.c
│   ├── init.lua
│   ├── reader.lua
│   ├── writer.lua
│   ├── dictreader.lua
│   ├── dictwriter.lua
│   ├── dialect.lua
│   └── sniffer.lua
├── lib/
│   └── libcsv/
├── examples/
├── test/
└── README.md
```

### Core Layer (C)
Handles:
- CSV parsing
- CSV writing
- Conversion between C and Lua tables

Built using `rgamble/libcsv`.

### Lua Layer
Handles:
- Reader / Writer API
- DictReader / DictWriter
- Dialects
- Sniffer
- Data transformation utilities

This separation keeps:
- C code small and fast
- Lua code flexible and extensible

---

## Installation

### LuaRocks (recommended)

```bash
luarocks install sheets
```

### Manual Build

```bash
git clone <repo-url>
cd sheets
./build.sh
```

This builds:

```text
csv/core.so
```

---

## Quick Start

```lua
local csv = require("csv")
```

---

# Reading CSV

```lua
local csv = require("csv")

local f = io.open("data.csv")

for row in csv.reader(f) do
    print(row[1], row[2])
end

f:close()
```

Each row is returned as:

```lua
{"name", "age", "city"}
```

---

# DictReader

Reads rows as named records using header row.

```lua
local csv = require("csv")

for rec in csv.DictReader(io.open("data.csv")) do
    print(rec.name, rec.age)
end
```

Example row:

```lua
{
    name = "Shivam",
    age = "23"
}
```

---

# Writing CSV

```lua
local csv = require("csv")

local out = io.open("result.csv", "w")
local writer = csv.writer(out)

writer:writerow({"name", "age"})
writer:writerow({"Shivam", 23})
writer:writerow({"Alice", 25})

out:close()
```

---

# DictWriter

```lua
local csv = require("csv")

local out = io.open("result.csv", "w")

local dw = csv.DictWriter(out, {
    fieldnames = {"name", "age"}
})

dw:writeheader()
dw:writerow({
    name = "Shivam",
    age = 23
})

out:close()
```

---

# One-shot Helpers

Read entire CSV:

```lua
local rows = csv.read_csv("data.csv")
```

Parse CSV string:

```lua
local rows = csv.parse(csv_string)
```

Write rows to CSV string:

```lua
local output = csv.write(rows)
```

Write rows to file:

```lua
csv.write_csv("output.csv", rows)
```

Read directly as dictionaries:

```lua
local rows = csv.read_dicts("data.csv")
```

---

## Dialects

Supports built-in and custom dialects.

Built-in:
- `excel`
- `excel_tab`
- `unix_dialect`

Example:

```lua
local rows = csv.parse(text, {
    dialect = "excel_tab"
})
```

Register custom dialect:

```lua
csv.register_dialect("pipes", {
    delim = "|"
})

local rows = csv.parse(text, {
    dialect = "pipes"
})
```

---

## Sniffer

Auto-detect CSV format.

```lua
local dialect = csv.sniff(csv_text)
```

Check if CSV likely contains headers:

```lua
local has_header = csv.has_header(csv_text)
```

---

## Options

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| delim | string | `","` | Field delimiter |
| quote | string | `"\""` | Quote character |
| strict | boolean | `false` | Enable strict parsing |
| dialect | string/table | - | CSV dialect |
| quote_all | boolean | `false` | Quote all output fields |

Example:

```lua
local rows = csv.read_csv("data.csv", {
    delim = ";",
    strict = true
})
```

---

# Data Transformation Utilities

`sheets` also provides lightweight utilities for tabular processing.

---

## Filter

```lua
local adults = csv.filter(rows, function(row)
    return tonumber(row[2]) >= 18
end)
```

---

## Map

```lua
local transformed = csv.map(rows, function(row)
    row[2] = tonumber(row[2])
    return row
end)
```

---

## Sort

```lua
local sorted = csv.sort(rows, function(a, b)
    return tonumber(a[2]) < tonumber(b[2])
end)
```

---

## Unique

```lua
local unique_rows = csv.unique(rows)
```

---

## Group By

```lua
local grouped = csv.group_by(rows, function(row)
    return row[3]
end)
```

---

## Column Extraction

```lua
local names = csv.column(rows, 1)
```

---

## Transpose

```lua
local transposed = csv.transpose(rows)
```

---

## API Reference

### Core Layer (`csv.core`)
- `parse_string(str, opts)`
- `parse_file(path, opts)`
- `write_row(row, opts)`
- `write_rows(rows, opts)`

---

### Public Layer (`csv`)
- `csv.reader(source, opts)`
- `csv.writer(file, opts)`
- `csv.DictReader(source, opts)`
- `csv.DictWriter(file, opts)`

---

### Convenience
- `csv.read_csv(filename, opts)`
- `csv.parse(str, opts)`
- `csv.write(rows, opts)`
- `csv.write_csv(filename, rows, opts)`
- `csv.read_dicts(filename, opts)`

---

### Utilities
- `csv.filter(rows, predicate)`
- `csv.map(rows, transformer)`
- `csv.merge(rows1, rows2)`
- `csv.unique(rows, key_func)`
- `csv.sort(rows, comparator)`
- `csv.transpose(rows)`
- `csv.group_by(rows, key_func)`
- `csv.column(rows, column_index)`

---

## Testing

Run:

```bash
lua test/test_csv.lua
```

Tests cover:
- Reader / Writer
- DictReader / DictWriter
- Quoted fields
- Dialects
- Sniffer
- Strict mode
- Utility functions

---

## Roadmap

- [x] High-performance CSV parsing
- [x] High-performance CSV writing
- [x] Reader / Writer
- [x] DictReader / DictWriter
- [x] Dialects
- [x] Sniffer
- [x] Data utilities
- [ ] True streaming parser
- [ ] Async reader
- [ ] Type inference

---

## License

MIT License.

---

## Acknowledgements

- `rgamble/libcsv`
- Python `csv` module for API inspiration
