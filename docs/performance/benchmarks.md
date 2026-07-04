# Benchmarks

This page documents performance benchmarks for `sheets`.

The goal is to evaluate:

- Parsing speed
- Writing speed
- Memory efficiency

and compare against Python’s built-in `csv` module.

---

# Benchmark Setup

Benchmarks were run using a synthetic CSV file with:

- **1,000,000 rows**
- **20 columns per row**
- Mixed field types:
  - integers
  - floats
  - strings
  - quoted strings
- Approximate file size: **200–300 MB**

Example row structure:

```csv
id,count,price,name,quoted,city,...
1,2,0.0100,user_1,"quoted,value",city_1,...
```

---

# Test Operations

Three main benchmarks were measured.

---

## 1. Parse Benchmark

Measures:

- Reading CSV file
- Parsing all rows
- Fully loading into memory

Equivalent to:

```lua
local rows = csv.read_csv("benchmark_1m.csv")
```

---

## 2. Write Benchmark

Measures:

- Serializing rows into CSV text

Equivalent to:

```lua
local output = csv.write(rows)
```

Repeated **100,000 times** for stable measurements.

---

## 3. Memory Benchmark

Measures:

- Memory usage before parse
- Memory usage after full parse

This represents total in-memory footprint after loading all rows.

---

# Benchmark Results

| Metric | sheets | Python csv |
|--------|---------|------------|
| Parse Time | ~9.2s | ~6.7s |
| Memory Usage | ~644 MB | ~1261 MB |
| Write Time | ~0.17s | ~0.59s |

---

# Benchmark Analysis

## Parsing Performance

| Library | Parse Time |
|---------|------------|
| Python csv | ~6.7s |
| sheets | ~9.2s |

Python’s built-in `csv` module parses faster due to highly optimized C internals and decades of optimization.

However, `sheets` remains competitive while offering excellent memory efficiency.

---

## Memory Efficiency

| Library | Memory Usage |
|---------|--------------|
| Python csv | ~1261 MB |
| sheets | ~644 MB |

### Result

`sheets` uses approximately:

- **~49% less memory**
- Nearly **2× lower memory usage**

This is one of the strongest advantages of `sheets`.

For large datasets, lower memory usage can significantly improve stability and scalability.

---

## Writing Performance

| Library | Write Time |
|---------|------------|
| Python csv | ~0.59s |
| sheets | ~0.17s |

### Result

`sheets` achieves approximately:

- **~3.5× faster writing performance**

This makes it particularly effective for:

- Data export pipelines
- CSV generation workloads
- Reporting systems

---

# Key Takeaways

`sheets` provides:

- Competitive parsing speed
- Excellent memory efficiency
- Outstanding write performance

Main strengths:

- ~2× lower memory usage than Python csv
- ~3.5× faster writing
- High-performance C backend using libcsv

---

# Benchmark Scripts

Benchmark scripts are included in the repository.

Lua benchmark:

```text
examples/comparison.lua
```

Python benchmark:

```text
test/python_csv_bench.py
```

---

# Notes

Benchmark results may vary depending on:

- CPU
- RAM
- OS
- Lua version
- Compiler optimization flags

These benchmarks are intended to provide practical comparative insight rather than absolute performance guarantees.
