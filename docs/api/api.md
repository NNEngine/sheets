# API Reference

This document provides the complete API reference for `sheets`.

Import the library:

```lua
local csv = require("csv")
```

---

# Core Functions

These are the primary one-shot APIs.

---

## csv.read_csv(filename, opts)

Reads and parses an entire CSV file.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| filename | string | Path to CSV file |
| opts | table (optional) | Parse options |

### Returns

```lua
rows
```

or

```lua
nil, err
```

### Example

```lua
local rows = csv.read_csv("data.csv")
```

---

## csv.parse(str, opts)

Parse CSV data from a string.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| str | string | CSV text |
| opts | table (optional) | Parse options |

### Returns

```lua
rows
```

### Example

```lua
local rows = csv.parse("name,age\nAlice,30")
```

---

## csv.write(rows, opts)

Serialize rows into CSV text.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| rows | table | Array of rows |
| opts | table (optional) | Write options |

### Returns

```lua
string
```

### Example

```lua
local output = csv.write({
    {"name", "age"},
    {"Alice", 30}
})
```

---

## csv.write_csv(filename, rows, opts)

Write rows directly to a CSV file.

### Parameters

| Name | Type |
|------|------|
| filename | string |
| rows | table |
| opts | table (optional) |

### Returns

```lua
true
```

or

```lua
nil, err
```

---

## csv.read_dicts(filename, opts)

Read CSV directly as dictionaries.

### Example

```lua
local rows = csv.read_dicts("data.csv")
```

---

# Reader API

Create a reader:

```lua
local reader = csv.reader(source, opts)
```

---

## reader:next_row()

Returns next row.

### Returns

```lua
row
```

or

```lua
nil
```

---

## reader:readall()

Read all rows.

### Returns

```lua
table
```

---

## reader:get(index)

Get row by index.

### Parameters

| Name | Type |
|------|------|
| index | number |

---

## reader:count()

Return total row count.

---

## reader:reset()

Reset reader position.

---

## reader:column(index)

Extract column.

### Example

```lua
local names = reader:column(1)
```

---

## reader:filter(predicate)

Filter rows.

### Example

```lua
local filtered = reader:filter(function(row)
    return row[1] == "Alice"
end)
```

---

## reader:map(transformer)

Transform rows.

---

# Writer API

Create writer:

```lua
local writer = csv.writer(file, opts)
```

---

## writer:writerow(row)

Write single row.

### Example

```lua
writer:writerow({"Alice", 30})
```

---

## writer:writerows(rows)

Write multiple rows.

---

# DictReader API

Create:

```lua
local dr = csv.DictReader(source, opts)
```

Uses first row as headers.

---

## dictreader:next_row()

Returns next row as dictionary.

Example:

```lua
{
    name = "Alice",
    age = "30"
}
```

---

## dictreader:readall()

Read all records.

---

## dictreader:reset()

Reset cursor.

---

## dictreader:filter(predicate)

Filter records.

---

## dictreader:map(transformer)

Transform records.

---

# DictWriter API

Create:

```lua
local dw = csv.DictWriter(file, opts)
```

Required:

```lua
{
    fieldnames = {"name", "age"}
}
```

---

## dictwriter:writeheader()

Write header row.

---

## dictwriter:writerow(record)

Write dictionary row.

### Example

```lua
dw:writerow({
    name = "Alice",
    age = 30
})
```

---

## dictwriter:writerows(records)

Write multiple records.

---

# Dialects

Built-in dialects:

- `csv.excel`
- `csv.excel_tab`
- `csv.unix_dialect`

---

## csv.register_dialect(name, config)

Register custom dialect.

### Example

```lua
csv.register_dialect("pipes", {
    delim = "|"
})
```

---

## csv.get_dialect(name)

Get dialect config.

---

## csv.unregister_dialect(name)

Remove custom dialect.

---

## csv.list_dialects()

Returns available dialects.

---

# Sniffer

---

## csv.sniff(sample)

Auto-detect CSV format.

### Returns

```lua
dialect
```

---

## csv.has_header(sample)

Check whether sample likely contains headers.

### Returns

```lua
boolean
```

---

# Utility Functions

---

## csv.filter(rows, predicate)

Filter rows.

---

## csv.map(rows, transformer)

Transform rows.

---

## csv.merge(rows1, rows2)

Merge two row sets.

---

## csv.unique(rows, key_func)

Remove duplicates.

---

## csv.sort(rows, comparator)

Sort rows.

---

## csv.transpose(rows)

Transpose matrix.

Example:

```lua
{
    {"a", "b"},
    {"1", "2"}
}
```

becomes:

```lua
{
    {"a", "1"},
    {"b", "2"}
}
```

---

## csv.group_by(rows, key_func)

Group rows by key.

Example:

```lua
local grouped = csv.group_by(rows, function(row)
    return row[2]
end)
```

---

## csv.column(rows, index)

Extract column values.

---

# Options Reference

Most APIs accept an optional `opts` table.

---

## Parse Options

| Key | Type | Default |
|-----|------|---------|
| delim | string | `","` |
| quote | string | `"` |
| strict | boolean | false |
| dialect | string/table | nil |

---

## Write Options

| Key | Type | Default |
|-----|------|---------|
| delim | string | `","` |
| quote | string | `"` |
| quote_all | boolean | false |
| dialect | string/table | nil |

---

# Version Info

```lua
csv._VERSION
csv._LIBCSV_VERSION
```

Example:

```lua
print(csv._VERSION)
print(csv._LIBCSV_VERSION)
```# API Reference

This document provides the complete API reference for `sheets`.

Import the library:

```lua
local csv = require("csv")
```

---

# Core Functions

These are the primary one-shot APIs.

---

## csv.read_csv(filename, opts)

Reads and parses an entire CSV file.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| filename | string | Path to CSV file |
| opts | table (optional) | Parse options |

### Returns

```lua
rows
```

or

```lua
nil, err
```

### Example

```lua
local rows = csv.read_csv("data.csv")
```

---

## csv.parse(str, opts)

Parse CSV data from a string.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| str | string | CSV text |
| opts | table (optional) | Parse options |

### Returns

```lua
rows
```

### Example

```lua
local rows = csv.parse("name,age\nAlice,30")
```

---

## csv.write(rows, opts)

Serialize rows into CSV text.

### Parameters

| Name | Type | Description |
|------|------|-------------|
| rows | table | Array of rows |
| opts | table (optional) | Write options |

### Returns

```lua
string
```

### Example

```lua
local output = csv.write({
    {"name", "age"},
    {"Alice", 30}
})
```

---

## csv.write_csv(filename, rows, opts)

Write rows directly to a CSV file.

### Parameters

| Name | Type |
|------|------|
| filename | string |
| rows | table |
| opts | table (optional) |

### Returns

```lua
true
```

or

```lua
nil, err
```

---

## csv.read_dicts(filename, opts)

Read CSV directly as dictionaries.

### Example

```lua
local rows = csv.read_dicts("data.csv")
```

---

# Reader API

Create a reader:

```lua
local reader = csv.reader(source, opts)
```

---

## reader:next_row()

Returns next row.

### Returns

```lua
row
```

or

```lua
nil
```

---

## reader:readall()

Read all rows.

### Returns

```lua
table
```

---

## reader:get(index)

Get row by index.

### Parameters

| Name | Type |
|------|------|
| index | number |

---

## reader:count()

Return total row count.

---

## reader:reset()

Reset reader position.

---

## reader:column(index)

Extract column.

### Example

```lua
local names = reader:column(1)
```

---

## reader:filter(predicate)

Filter rows.

### Example

```lua
local filtered = reader:filter(function(row)
    return row[1] == "Alice"
end)
```

---

## reader:map(transformer)

Transform rows.

---

# Writer API

Create writer:

```lua
local writer = csv.writer(file, opts)
```

---

## writer:writerow(row)

Write single row.

### Example

```lua
writer:writerow({"Alice", 30})
```

---

## writer:writerows(rows)

Write multiple rows.

---

# DictReader API

Create:

```lua
local dr = csv.DictReader(source, opts)
```

Uses first row as headers.

---

## dictreader:next_row()

Returns next row as dictionary.

Example:

```lua
{
    name = "Alice",
    age = "30"
}
```

---

## dictreader:readall()

Read all records.

---

## dictreader:reset()

Reset cursor.

---

## dictreader:filter(predicate)

Filter records.

---

## dictreader:map(transformer)

Transform records.

---

# DictWriter API

Create:

```lua
local dw = csv.DictWriter(file, opts)
```

Required:

```lua
{
    fieldnames = {"name", "age"}
}
```

---

## dictwriter:writeheader()

Write header row.

---

## dictwriter:writerow(record)

Write dictionary row.

### Example

```lua
dw:writerow({
    name = "Alice",
    age = 30
})
```

---

## dictwriter:writerows(records)

Write multiple records.

---

# Dialects

Built-in dialects:

- `csv.excel`
- `csv.excel_tab`
- `csv.unix_dialect`

---

## csv.register_dialect(name, config)

Register custom dialect.

### Example

```lua
csv.register_dialect("pipes", {
    delim = "|"
})
```

---

## csv.get_dialect(name)

Get dialect config.

---

## csv.unregister_dialect(name)

Remove custom dialect.

---

## csv.list_dialects()

Returns available dialects.

---

# Sniffer

---

## csv.sniff(sample)

Auto-detect CSV format.

### Returns

```lua
dialect
```

---

## csv.has_header(sample)

Check whether sample likely contains headers.

### Returns

```lua
boolean
```

---

# Utility Functions

---

## csv.filter(rows, predicate)

Filter rows.

---

## csv.map(rows, transformer)

Transform rows.

---

## csv.merge(rows1, rows2)

Merge two row sets.

---

## csv.unique(rows, key_func)

Remove duplicates.

---

## csv.sort(rows, comparator)

Sort rows.

---

## csv.transpose(rows)

Transpose matrix.

Example:

```lua
{
    {"a", "b"},
    {"1", "2"}
}
```

becomes:

```lua
{
    {"a", "1"},
    {"b", "2"}
}
```

---

## csv.group_by(rows, key_func)

Group rows by key.

Example:

```lua
local grouped = csv.group_by(rows, function(row)
    return row[2]
end)
```

---

## csv.column(rows, index)

Extract column values.

---

# Options Reference

Most APIs accept an optional `opts` table.

---

## Parse Options

| Key | Type | Default |
|-----|------|---------|
| delim | string | `","` |
| quote | string | `"` |
| strict | boolean | false |
| dialect | string/table | nil |

---

## Write Options

| Key | Type | Default |
|-----|------|---------|
| delim | string | `","` |
| quote | string | `"` |
| quote_all | boolean | false |
| dialect | string/table | nil |

---

# Version Info

```lua
csv._VERSION
csv._LIBCSV_VERSION
```

Example:

```lua
print(csv._VERSION)
print(csv._LIBCSV_VERSION)
```
