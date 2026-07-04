# Quickstart

This guide covers the most common workflows in `sheets`.

By the end, you’ll know how to:

- Read CSV files
- Write CSV files
- Use `DictReader` / `DictWriter`
- Work with dialects
- Use built-in data utilities

---

## Import Library

```lua
local csv = require("csv")
```

---


# Reading CSV
There are two main ways to read CSV data:

- One-shot loading (`csv.read_csv`)
- Streaming via `csv.reader`

---

## Read Entire CSV File

```lua
local csv = require("csv")

local rows = csv.read_csv("data.csv")

for _, row in ipairs(rows) do
    print(row[1], row[2])
end
```

Example CSV:

```csv
name,age
Alice,30
Bob,25
```

Result:

```lua
{
    {"name", "age"},
    {"Alice", "30"},
    {"Bob", "25"}
}
```

---

## Parse CSV String

```lua
local csv = require("csv")

local text = "name,age\nAlice,30\nBob,25"
local rows = csv.parse(text)

print(rows[2][1]) -- Alice
print(rows[2][2]) -- 30
```

---

## Reader API

Use `csv.reader()` when you want a Python-style reader.

```lua
local csv = require("csv")

local f = io.open("data.csv")

for row in csv.reader(f) do
    print(row[1], row[2])
end

f:close()
```

---

# Writing CSV

You can either:

- Serialize rows into a CSV string
- Write directly to a file

---

## Write CSV String

```lua
local csv = require("csv")

local rows = {
    {"name", "age"},
    {"Alice", 30},
    {"Bob", 25}
}

local output = csv.write(rows)
print(output)
```

Output:

```csv
name,age
Alice,30
Bob,25
```

---

## Write CSV File

```lua
local csv = require("csv")

local rows = {
    {"name", "age"},
    {"Alice", 30},
    {"Bob", 25}
}

csv.write_csv("output.csv", rows)
```

---

## Writer API

```lua
local csv = require("csv")

local f = io.open("output.csv", "w")
local writer = csv.writer(f)

writer:writerow({"name", "age"})
writer:writerow({"Alice", 30})
writer:writerow({"Bob", 25})

f:close()
```

---

# DictReader

`DictReader` uses the first row as column headers.

Example CSV:

```csv
name,age,city
Alice,30,NYC
Bob,25,LA
```

---

## Read as Dictionaries

```lua
local csv = require("csv")

local f = io.open("data.csv")

for record in csv.DictReader(f) do
    print(record.name, record.age, record.city)
end

f:close()
```

Example record:

```lua
{
    name = "Alice",
    age = "30",
    city = "NYC"
}
```

---

## One-shot Dict Read

```lua
local csv = require("csv")

local records = csv.read_dicts("data.csv")
```

---

# DictWriter

Write dictionaries using named fields.

```lua
local csv = require("csv")

local f = io.open("output.csv", "w")

local dw = csv.DictWriter(f, {
    fieldnames = {"name", "age", "city"}
})

dw:writeheader()

dw:writerow({
    name = "Alice",
    age = 30,
    city = "NYC"
})

f:close()
```

---

# Working with Dialects

Dialects control CSV formatting.

Built-in dialects:

- `excel`
- `excel_tab`
- `unix_dialect`

---

## Use Custom Delimiter

```lua
local rows = csv.parse(text, {
    delim = ";"
})
```

Example:

```csv
name;age
Alice;30
```

---

## Register Custom Dialect

```lua
local csv = require("csv")

csv.register_dialect("pipes", {
    delim = "|"
})

local rows = csv.parse("a|b|c\n1|2|3", {
    dialect = "pipes"
})
```

---

# Sniffer

Automatically detect CSV format.

```lua
local csv = require("csv")

local sample = "name,age\nAlice,30\nBob,25"

local dialect = csv.sniff(sample)
print(dialect.delim)
```

---

# Utility Functions

`sheets` includes lightweight utilities for tabular data.

---

## Filter Rows

```lua
local adults = csv.filter(rows, function(row)
    return tonumber(row[2]) >= 18
end)
```

---

## Map Rows

```lua
local transformed = csv.map(rows, function(row)
    row[2] = tonumber(row[2])
    return row
end)
```

---

## Sort Rows

```lua
local sorted = csv.sort(rows, function(a, b)
    return tonumber(a[2]) < tonumber(b[2])
end)
```

---

## Group Rows

```lua
local grouped = csv.group_by(rows, function(row)
    return row[3]
end)
```

---

## Extract Column

```lua
local names = csv.column(rows, 1)
```

---

# Next Steps

You now know the basics of `sheets`.

Next:

- Read **Architecture** to understand internals
- Explore **API Reference** for complete documentation
- Check **Benchmarks** for performance analysis
