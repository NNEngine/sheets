# Changelog - sheets

All notable changes to sheets are documented in this file.

## [2.0.0] - 2024-07-02

### Added

#### New Utility Functions
- `csv.filter(rows, predicate)` - Filter rows based on predicate function
- `csv.map(rows, transformer)` - Transform rows using transformer function
- `csv.merge(rows1, rows2)` - Merge two CSV row sets
- `csv.unique(rows, key_func)` - Remove duplicate rows
- `csv.sort(rows, comparator)` - Sort rows using comparator function
- `csv.transpose(rows)` - Transpose CSV matrix (rows become columns)
- `csv.group_by(rows, key_func)` - Group rows by key function
- `csv.column(rows, column_index)` - Extract single column from rows

#### Enhanced Reader Class
- `Reader:readall()` - Get all remaining rows as array
- `Reader:get(index)` - Get specific row by 1-based index
- `Reader:slice(start, finish)` - Get slice of rows
- `Reader:column(col_index)` - Extract single column
- `Reader:filter(predicate)` - Filter rows
- `Reader:map(transformer)` - Transform rows

#### Enhanced DictReader Class
- `DictReader:readall()` - Get all remaining records as array
- `DictReader:count()` - Get record count
- `DictReader:filter(predicate)` - Filter records by predicate
- `DictReader:map(transformer)` - Map records with transformer
- **BUG FIX**: Fixed `reset()` method by properly tracking `_consumed_header` flag

#### Enhanced DictWriter Class
- `DictWriter:writerows(dicts)` - Write multiple records at once

#### Documentation
- Professional README_COMPLETE.md with full API reference
- CONTRIBUTING.md for developers
- Enhanced inline code comments
- Comprehensive test suite (test_csv_enhanced.lua)

#### Testing
- 25 comprehensive test cases in test_csv_enhanced.lua
- Tests for all new functions
- Edge case coverage
- Validation of bug fixes

### Changed
- Version bumped from 0.1.0 to 2.0.0
- Enhanced error handling and validation throughout
- Improved DictReader implementation with proper state tracking
- Better documentation and examples

### Fixed
- **DictReader.reset() bug**: The `_consumed_header` flag was not being set, causing reset to fail
- Improved null/empty field handling in DictReader
- Enhanced error messages for invalid parameters

### Deprecated
- None

### Removed
- None

### Security
- None

## [0.1.0] - 2024-06-01 (Initial Release)

### Added
- Core CSV reader with row-based iteration
- Core CSV writer with row-based output
- DictReader class for dictionary-based reading
- DictWriter class for dictionary-based writing
- Dialect system with built-in presets (excel, excel_tab, unix)
- Support for custom dialects
- CSV format sniffer for auto-detection
- Test suite with basic coverage
- Basic examples

---

## Version Guide

- **v2.0.0** (2024-07-02): Major release with new features and bug fixes - Production ready
- **v0.1.0** (2024-06-01): Initial release - Foundation version

## Upgrade Notes

### From 0.1.0 to 2.0.0

**No breaking changes!** Your existing code will continue to work.

#### New Capabilities
- Use new utility functions for data transformation
- Use new Reader methods for advanced queries
- Use new DictReader methods for filtering/mapping
- Benefit from bug fixes in reset() functionality

#### Recommended Updates
1. Update to v2.0.0 for bug fixes
2. Adopt new utility functions where applicable
3. Use new Reader/DictReader methods for cleaner code

Example of migration to new style:
```lua
-- Old way (still works)
local rows = csv.read_csv("file.csv")
local filtered = {}
for _, row in ipairs(rows) do
    if tonumber(row[2]) > 30 then
        table.insert(filtered, row)
    end
end

-- New way (cleaner)
local rows = csv.read_csv("file.csv")
local filtered = csv.filter(rows, function(row)
    return tonumber(row[2]) > 30
end)
```

## Future Roadmap

### Planned for v2.1.0
- Streaming support for large files
- More statistical functions
- Excel-specific features

### Planned for v3.0.0
- Lua coroutine-based streaming
- Async I/O support
- Performance optimizations

---

## Support

- For bug reports: Open an issue on GitHub
- For feature requests: Discuss in GitHub issues
- For help: Check README_COMPLETE.md and examples

## License

MIT (includes LGPL-2.1 licensed libcsv) License - See LICENSE file for details
