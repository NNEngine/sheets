# Contributing

Thank you for your interest in contributing to `sheets`.

Contributions of all kinds are welcome.

This includes:

- Bug reports
- Feature requests
- Performance improvements
- Documentation improvements
- Test coverage improvements
- Code contributions

---

# Development Setup

Clone the repository:

```bash
git clone https://github.com/NNEngine/sheets.git
cd sheets
```

Build the native module:

```bash
./build.sh
```

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
├── docs/
├── examples/
├── test/
└── lib/
```

---

# Running Tests

Run the test suite:

```bash
lua test/test_csv.lua
```

All tests should pass before submitting changes.

---

# Coding Guidelines

---

## Lua Code

- Use clear and readable code
- Prefer small focused functions
- Follow existing module style
- Add comments for non-obvious logic

Example style:

```lua
local result = {}
for _, row in ipairs(rows) do
    result[#result + 1] = row
end
```

---

## C Code

Performance-critical logic lives in:

```text
csv/core.c
```

Guidelines:

- Keep C code minimal
- Prioritize performance
- Avoid unnecessary allocations
- Document complex logic

---

# Contribution Workflow

1. Fork repository
2. Create feature branch
3. Make changes
4. Add/update tests
5. Run tests
6. Submit pull request

Example:

```bash
git checkout -b feature/improve-parser
```

---

# Reporting Bugs

When reporting issues, please include:

- Lua version
- Operating system
- Error message
- Reproduction steps
- Example CSV input (if relevant)

Example:

```text
Lua version: 5.4
OS: Ubuntu 24.04
Issue: Parsing quoted multiline CSV fails
```

---

# Feature Requests

Feature requests are welcome.

Examples:

- Streaming parser
- Type inference
- Schema validation
- Async reader
- Performance improvements

---

# Areas for Contribution

Some high-impact contribution areas:

- Performance optimization
- Streaming parser support
- Improved dialect detection
- Documentation improvements
- Additional tests

---

# Code of Conduct

Please be respectful and constructive.

We aim to maintain a welcoming and collaborative environment.

---

# Questions?

Open an issue or start a discussion on GitHub.

Thank you for helping improve `sheets`.
