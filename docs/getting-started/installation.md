# Installation

This guide explains how to install `sheets` using either:

- LuaRocks (recommended)
- Manual build from source

---

## Requirements

Before installing, ensure you have:

- Lua **5.2+**
- A C compiler
- Build tools

Supported platforms:

- Linux
- Windows
- macOS (experimental)

---

## Install via LuaRocks (Recommended)

The easiest way to install `sheets` is through LuaRocks.

```bash
luarocks install sheets
```

This will automatically:

- Build the C core (`csv.core`)
- Install Lua modules
- Make the library available system-wide

---

## Manual Installation

### 1. Clone Repository

```bash
git clone <repo-url>
cd sheets
```

---

### 2. Build Native Module

Run:

```bash
./build.sh
```

This compiles:

```text
csv/core.so
```

The native module is built from:

- `csv/core.c`
- `lib/libcsv/libcsv.c`

---

### 3. Verify Build

After successful compilation, you should see:

```text
csv/
├── core.so
```

(or `core.dll` on Windows)

---

## Windows Installation

You need:

- MinGW / GCC
or
- MSVC

Example using MinGW:

```bash
gcc -O2 -shared -o csv/core.dll csv/core.c lib/libcsv/libcsv.c -Ilib/libcsv -I<lua_include_path>
```

Make sure your Lua installation provides:

- `lua.h`
- `lauxlib.h`
- `lualib.h`

---

## Linux Installation

Install build dependencies.

### Ubuntu / Debian

```bash
sudo apt install build-essential lua5.4 liblua5.4-dev luarocks
```

Then:

```bash
./build.sh
```

---

### Arch Linux

```bash
sudo pacman -S base-devel lua luarocks
```

Then:

```bash
./build.sh
```

---

## Verify Installation

Run Lua:

```bash
lua
```

Then:

```lua
local csv = require("csv")
print(csv._VERSION)
```

Expected output:

```text
2.0.0
```

---

## Test Installation

Try a quick parse:

```lua
local csv = require("csv")

local rows = csv.parse("name,age\nShivam,23")

print(rows[2][1])  -- Shivam
print(rows[2][2])  -- 23
```

If this works, installation is successful.

---

## Troubleshooting

---

### Module `csv.core` not found

This means Lua cannot find the compiled C module.

Check:

- `package.cpath`
- Build completed successfully
- `csv/core.so` exists

---

### Missing Lua headers

Error:

```text
fatal error: lua.h: No such file or directory
```

Install Lua development headers.

Examples:

Ubuntu:

```bash
sudo apt install liblua5.4-dev
```

Arch:

```bash
sudo pacman -S lua
```

---

### Permission denied on build.sh

Run:

```bash
chmod +x build.sh
./build.sh
```

---

## Next Steps

After installation:

- Read **Quickstart** for basic usage
- Explore **API Reference**
- Check **Benchmarks**
