# Architecture

`sheets` uses a layered hybrid architecture designed to balance:

- High performance
- Low memory usage
- Clean API design
- Easy extensibility

The core idea is simple:

- Performance-critical operations are implemented in **C**
- High-level APIs and utilities are implemented in **Lua**

This provides the speed of native code while preserving the flexibility of Lua.

---

# Architecture Overview

`sheets` consists of three main layers:

1. User-facing API Layer
2. Native C Core Layer
3. Backend CSV Engine

---

## High-Level Flow

```text
User Code
   ↓
Lua API Layer
   ↓
C Core Layer
   ↓
libcsv Backend
```

---

# Layer 1: User-Facing Lua API

This is the public interface exposed by:

```lua
require("csv")
```

This layer provides a Python-inspired API.

Modules:

```text
csv/
├── init.lua
├── reader.lua
├── writer.lua
├── dictreader.lua
├── dictwriter.lua
├── dialect.lua
└── sniffer.lua
```

---

## Responsibilities

The Lua layer handles:

- Reader API
- Writer API
- DictReader
- DictWriter
- Dialect resolution
- CSV format detection
- Utility functions

Examples:

```lua
csv.reader(file)
csv.writer(file)
csv.DictReader(file)
csv.sniff(text)
csv.filter(rows, fn)
```

This layer focuses on:

- Ergonomics
- API design
- Validation
- Convenience

---

# Layer 2: Native C Core

This layer contains performance-critical logic.

Core file:

```text
csv/core.c
```

Exposed as:

```lua
require("csv.core")
```

---

## Responsibilities

The C layer handles:

- CSV parsing
- CSV serialization
- Buffer management
- Lua table conversion

Core functions:

```lua
core.parse_string(str, opts)
core.parse_file(path, opts)
core.write_row(row, opts)
core.write_rows(rows, opts)
```

---

## Why C?

CSV parsing and serialization are highly repetitive operations involving:

- Character scanning
- Delimiter detection
- Quote handling
- Memory allocation

These workloads are significantly faster in C than in pure Lua.

This design keeps:

- Parsing fast
- Writing fast
- Memory overhead low

---

# Layer 3: Backend CSV Engine

The C core uses:

```text
lib/libcsv/
```

Specifically:

- `csv.h`
- `libcsv.c`

---

## Backend Library

`sheets` uses **libcsv**, a lightweight and battle-tested C CSV parser.

Why libcsv:

- Mature implementation
- RFC 4180 compliant behavior
- High performance
- Small dependency footprint

This backend performs the low-level CSV parsing logic.

---

# Data Flow

---

## Parsing Flow

Example:

```lua
local rows = csv.read_csv("data.csv")
```

Execution path:

```text
csv.read_csv()
    ↓
core.parse_file()
    ↓
libcsv parser
    ↓
field callbacks
    ↓
Lua tables
```

Final output:

```lua
{
    {"name", "age"},
    {"Alice", "30"}
}
```

---

## Writing Flow

Example:

```lua
local output = csv.write(rows)
```

Execution path:

```text
csv.write()
    ↓
core.write_rows()
    ↓
CSV serialization
    ↓
Output string
```

---

# Design Philosophy

`sheets` was designed around several core principles.

---

## 1. Performance First

Parsing and writing are implemented in C.

This minimizes overhead in critical paths.

---

## 2. Lua for Flexibility

High-level features remain in Lua.

This makes the library:

- Easy to modify
- Easy to extend
- Easy to maintain

---

## 3. Python-Inspired API

The API design takes inspiration from Python’s built-in `csv` module.

Examples:

- `reader`
- `writer`
- `DictReader`
- `DictWriter`
- `Sniffer`

This makes the library familiar to Python users.

---

## 4. Minimal Dependencies

`sheets` depends only on:

- Lua
- libcsv

No heavy external dependencies are required.

---

# Project Structure

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
│
├── lib/
│   └── libcsv/
│
├── docs/
├── examples/
└── test/
```

---

# Advantages of This Architecture

This hybrid design provides:

- High performance
- Lower memory usage
- Clean modular code
- Easy extensibility
- Production-ready reliability

---

# Future Extensions

The architecture is designed to support future improvements.

Potential enhancements:

- True streaming parser
- Async CSV reader
- Type inference
- Schema validation
- Parallel parsing

The current architecture allows these features to be added without major redesign.
