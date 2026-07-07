???+ note "Message from the Author"

    Welcome to **sheets**.

    Lua is powerful, lightweight, and fast—but its ecosystem still lags behind modern developer needs.
    This project is my first contribution toward changing that.
    I’m here to build serious tools that push Lua beyond scripting into real engineering.

    — **Shivam Sharma**

# sheets

<div style="background-color: #161b22; color: #e2e8f0; padding: 2rem; border-radius: 0.5rem; font-family: 'Segoe UI', system-ui, -apple-system, sans-serif; font-size: 1.05rem; line-height: 1.6; margin: 1.5rem 0; box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.3); text-align:center;">

  <p style="color: #fc8181; font-size: 1.5rem; font-weight: 700; margin: 0 0 1rem 0; border-bottom: 2px solid #e53e3e; padding-bottom: 0.5rem; display: inline-block; width: 100%;">
    Fast, Production-Ready CSV Processing for Lua
  </p>

  <p style="margin: 0;">
    <b>sheets</b> is a high-performance, memory-efficient CSV and tabular data toolkit designed for production workloads. Built on top of the battle-tested <b>libcsv</b> C library, it combines native performance with an intuitive Python-inspired API.
  </p>

  <p style="margin: 1rem 0 0 0;">
    Whether you're building data pipelines, ETL systems, analytics platforms, or processing large datasets in Lua, sheets provides a robust, efficient solution for structured tabular data.
  </p>

</div>

---

## Why sheets?

The Lua ecosystem has several CSV libraries, but most fall into one of three categories:

=== "Minimal Parsers"
    Limited features, simple implementations, but lacking essential functionality for production systems

=== "Pure Lua Libraries"
    Flexible and easy to modify, but significantly slower when processing large datasets

=== "Incomplete Solutions"
    Missing modern data processing APIs or lacking consistent, intuitive interfaces

**sheets solves all of these problems** by combining:

- **Native C performance** via libcsv for parsing and writing
- **Pythonic API** inspired by Python's standard csv module
- **Complete feature set** for real-world data workflows
- **Production-ready** reliability and comprehensive testing

---

## Key Features

- **High Performance** — Parse and write CSV files with C-level speed...
- **Memory Efficient** — Streaming architecture and optimized buffering...
- **Pythonic API** — Familiar interfaces like Reader, Writer...
- **Flexible Dialects** — Support for different CSV formats...
- **Cross-Platform** — Works seamlessly on Linux, macOS, and Windows...
- **Production-Ready** — Battle-tested on large datasets...

---

## Quick Comparison

<table style="border-collapse:separate;border-spacing:0;background:#161b22;border-radius:6px;overflow:hidden;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Helvetica,Arial,sans-serif;font-size:0.9rem;color:#c9d1d9;width:100%;">
  <thead>
    <tr>
      <th style="background:#161b22;color:#e6edf3;font-weight:600;text-align:left;padding:12px 16px;border-bottom:2px solid #f78166;font-size:0.85rem;letter-spacing:0.3px;">Feature</th>
      <th style="background:#161b22;color:#e6edf3;font-weight:600;text-align:left;padding:12px 16px;border-bottom:2px solid #f78166;font-size:0.85rem;letter-spacing:0.3px;">sheets</th>
      <th style="background:#161b22;color:#e6edf3;font-weight:600;text-align:left;padding:12px 16px;border-bottom:2px solid #f78166;font-size:0.85rem;letter-spacing:0.3px;">Python csv</th>
      <th style="background:#161b22;color:#e6edf3;font-weight:600;text-align:left;padding:12px 16px;border-bottom:2px solid #f78166;font-size:0.85rem;letter-spacing:0.3px;">Pure Lua Libs</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><strong style="color:#e6edf3;">Parse Speed</strong></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Native C</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Native C</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Script</code></td>
    </tr>
    <tr>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;background:#1c2128;"><strong style="color:#e6edf3;">Memory Usage</strong></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;background:#1c2128;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">644 MB (1M rows)</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;background:#1c2128;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">1261 MB (1M rows)</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;background:#1c2128;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Variable</code></td>
    </tr>
    <tr>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><strong style="color:#e6edf3;">Python-style API</strong></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Yes</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Yes</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Partial</code></td>
    </tr>
    <tr>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;background:#1c2128;"><strong style="color:#e6edf3;">DictReader/DictWriter</strong></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;background:#1c2128;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Yes</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;background:#1c2128;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Yes</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;background:#1c2128;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Limited</code></td>
    </tr>
    <tr>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><strong style="color:#e6edf3;">Dialect Support</strong></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Auto-detect</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Auto-detect</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">No</code></td>
    </tr>
    <tr>
      <td style="padding:12px 16px;border-bottom:none;vertical-align:middle;background:#1c2128;"><strong style="color:#e6edf3;">Production Ready</strong></td>
      <td style="padding:12px 16px;border-bottom:none;vertical-align:middle;background:#1c2128;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Yes</code></td>
      <td style="padding:12px 16px;border-bottom:none;vertical-align:middle;background:#1c2128;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Yes</code></td>
      <td style="padding:12px 16px;border-bottom:none;vertical-align:middle;background:#1c2128;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">Limited</code></td>
    </tr>
  </tbody>
</table>

## Use Cases

sheets is ideal for:

=== "Data Pipelines"
    ETL workflows that require fast, reliable CSV ingestion and transformation

=== "Analytics Platforms"
    Processing large datasets for real-time or batch analytics

=== "Backend Services"
    CSV handling in web applications, APIs, and data services

=== "Data Preprocessing"
    Preparing datasets for machine learning pipelines

=== "Reporting Systems"
    Generating and parsing CSV reports at scale

---

## Getting Started

### 1. Installation

```
luarocks install sheets
```

Or build from source. See [Installation](getting-started/installation.md) for detailed instructions.

### 2. Basic Reading

```lua
local csv = require("csv")

-- Simple row iteration
for row in csv.reader(io.open("data.csv")) do
    print(row[1], row[2], row[3])
end
```

### 3. Basic Writing

```lua
local csv = require("csv")

local f = io.open("output.csv", "w")
local writer = csv.writer(f)

writer:writerow({"name", "age", "city"})
writer:writerow({"Alice", 28, "New York"})
writer:writerow({"Bob", 35, "San Francisco"})

f:close()
```

### 4. Dictionary-Based Access

```lua
-- Access rows as dictionaries with header mapping
for record in csv.DictReader(io.open("data.csv")) do
    print(record.name, record.age)
end
```

---

## Performance at Scale

Tested on a **1 million row, 20 column** CSV file with mixed data types:


<table style="border-collapse:separate;border-spacing:0;background:#161b22;border-radius:6px;overflow:hidden;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Helvetica,Arial,sans-serif;font-size:0.9rem;color:#c9d1d9;width:100%;">
  <thead>
    <tr>
      <th style="background:#161b22;color:#e6edf3;font-weight:600;text-align:left;padding:12px 16px;border-bottom:2px solid #f78166;font-size:0.85rem;letter-spacing:0.3px;">Metric</th>
      <th style="background:#161b22;color:#e6edf3;font-weight:600;text-align:left;padding:12px 16px;border-bottom:2px solid #f78166;font-size:0.85rem;letter-spacing:0.3px;">sheets</th>
      <th style="background:#161b22;color:#e6edf3;font-weight:600;text-align:left;padding:12px 16px;border-bottom:2px solid #f78166;font-size:0.85rem;letter-spacing:0.3px;">Python csv</th>
      <th style="background:#161b22;color:#e6edf3;font-weight:600;text-align:left;padding:12px 16px;border-bottom:2px solid #f78166;font-size:0.85rem;letter-spacing:0.3px;">Ratio</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><strong style="color:#e6edf3;">Parse Time</strong></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">9.2s</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">6.7s</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;">1.4×</td>
    </tr>
    <tr>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><strong style="color:#e6edf3;">Memory Usage</strong></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">644 MB</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">1261 MB</code></td>
      <td style="padding:12px 16px;border-bottom:1px solid #30363d;vertical-align:middle;">0.51×</td>
    </tr>
    <tr>
      <td style="padding:12px 16px;border-bottom:none;vertical-align:middle;"><strong style="color:#e6edf3;">Write Time</strong></td>
      <td style="padding:12px 16px;border-bottom:none;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">0.17s</code></td>
      <td style="padding:12px 16px;border-bottom:none;vertical-align:middle;"><code style="font-family:'SFMono-Regular',Consolas,Menlo,monospace;background:#0d1117;padding:3px 6px;border-radius:4px;font-size:0.85em;color:#c9d1d9;border:1px solid #30363d;">0.59s</code></td>
      <td style="padding:12px 16px;border-bottom:none;vertical-align:middle;"><strong style="color:#e6edf3;">3.5×</strong></td>
    </tr>
  </tbody>
</table>

**Key Takeaways:**

- **2× better memory efficiency** than Python
- **3.5× faster writing performance**
- Competitive parsing with full C implementation backend
- Scales efficiently to multi-million row

See [Benchmarks](performance/benchmarks.md) for detailed performance analysis and additional test cases.

---

## Architecture Overview

sheets uses a **hybrid Lua + C architecture** for optimal performance and usability:

```
┌─────────────────────────────────────┐
│   Lua Layer (High-Level API)        │
│  Reader, Writer, DictReader, etc.   │
└────────────┬────────────────────────┘
             │
┌────────────▼────────────────────────┐
│   C Layer (Libcsv Bindings)         │
│  Fast parsing, writing, buffering   │
└────────────┬────────────────────────┘
             │
┌────────────▼────────────────────────┐
│   libcsv Backend                    │
│  Battle-tested, standards-compliant │
└─────────────────────────────────────┘
```

This design ensures:

- **Lua flexibility** for API design and ease of use
- **C performance** for I/O-bound operations
- **Modularity** for easy extension and maintenance

See [Design_Overview](architecture/design.md) for deep technical details.

---

## Core API Overview

=== "Reader"
    ```lua
    local csv = require("csv")

    -- Stream rows as arrays
    for row in csv.reader(file) do
        print(#row, row[1])
    end
    ```

=== "Writer"
    ```lua
    local csv = require("csv")

    local writer = csv.writer(file)
    writer:writerow({"col1", "col2"})
    writer:writerows({{1, 2}, {3, 4}})
    ```

=== "DictReader"
    ```lua
    local csv = require("csv")

    -- Maps rows to dictionaries with headers
    for record in csv.DictReader(file) do
        print(record.email, record.name)
    end
    ```

=== "DictWriter"
    ```lua
    local csv = require("csv")

    local writer = csv.DictWriter(file, {"name", "age"})
    writer:writeheader()
    writer:writerow({name="Alice", age=28})
    ```

See [api_red](api/api.md) for complete documentation of all functions and parameters.

---

## Documentation Structure

This documentation is organized into several sections:

!!! info "📖 Documentation Guide"

    - **[Installation](getting-started/installation.md)** — Setup, build from source, platform-specific notes

    - **[Quickstart](getting-started/quickstart.md)** — Hands-on tutorial with common patterns and examples

    - **[api_ref](api/api.md)** — Complete API documentation with parameters, return values, and examples

    - **[Architecture](architecture/architecture.md)** — Internal design, internals, and contribution guidelines

    - **[Benchmarks](performance/benchmarks.md)** — Performance data, comparison with other libraries, profiling results

---

## Project Philosophy

sheets is built on three core principles:

!!! quote ""

    **Performance**

    Native C implementation for I/O-bound operations. Stream processing architecture to minimize memory footprint.

!!! quote ""

    **Usability**

    Pythonic API familiar to developers across ecosystems. Clear, predictable behavior with sensible defaults.

!!! quote ""

    **Reliability**

    Production-tested on large real-world datasets. Comprehensive error handling and consistent cross-platform behavior.

---

## Version & License

**sheets v2.0.0**

Licensed under the MIT ((includes LGPL-2.1 licensed libcsv)) License. See LICENSE file for details.

---

## Get Started Now

Ready to start? Head over to the [Installation](getting-started/installation.md) guide to get sheets set up.

Already familiar with CSV libraries? Jump straight to the [api_ref](api/api.md) for complete documentation.

Have questions? Check out the [Quickstart](getting-started/quickstart.md) for common patterns and best practices.
