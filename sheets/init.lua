-- sheets - a Lua CSV library backed by libcsv, with a Python-`csv`-shaped API.
--
-- Quick start:
--
--   local csv = require("sheets")
--
--   -- reading
--   local f = io.open("data.csv")
--   for row in sheets.reader(f) do
--       print(row[1], row[2])
--   end
--   f:close()
--
--   -- reading as dicts
--   for rec in sheets.DictReader(io.open("data.csv")) do
--       print(rec.name, rec.age)
--   end
--
--   -- writing
--   local out = io.open("out.csv", "w")
--   local w = sheets.writer(out)
--   w:writerow({"name", "age"})
--   w:writerows({{"ABCD", 23}, {"Shivam", 24}})
--   out:close()
--
--   -- one-shot convenience (no manual file handling)
--   local rows = sheets.read_csv("data.csv")
--   csv.write_csv("out.csv", rows)

local core = require("sheets.core")
local dialect = require("sheets.dialect")
local Reader = require("sheets.reader")
local Writer = require("sheets.writer")
local DictReader = require("sheets.dictreader")
local DictWriter = require("sheets.dictwriter")
local sniffer = require("sheets.sniffer")


local M = {}

M._VERSION = "2.0.0"
M._LIBCSV_VERSION = core._LIBCSV_VERSION

-- -------------------------------------------------------------------
-- Core iterator/writer classes (Python-shaped)
-- -------------------------------------------------------------------

M.reader = Reader.new
M.writer = Writer.new
M.DictReader = DictReader.new
M.DictWriter = DictWriter.new

-- -------------------------------------------------------------------
-- Dialects
-- -------------------------------------------------------------------

M.excel = dialect.excel
M.excel_tab = dialect.excel_tab
M.unix_dialect = dialect.unix_dialect
M.register_dialect = dialect.register_dialect
M.get_dialect = dialect.get_dialect
M.unregister_dialect = dialect.unregister_dialect
M.list_dialects = dialect.list_dialects
M.sniff = sniffer.sniff
M.has_header = sniffer.has_header

-- -------------------------------------------------------------------
-- One-shot convenience functions (no manual file/iterator handling)
-- -------------------------------------------------------------------

-- sheets.read_csv(filename, opts) -> rows | nil, err
-- Reads and parses an entire file in one call. Each row is an array of
-- field strings. For large files prefer csv.reader() to avoid building
-- everything in memory at once... though note this implementation
-- already buffers the whole file either way (see README, "Streaming").
function M.read_csv(filename, opts)
    return core.parse_file(filename, dialect.resolve(opts))
end

-- sheets.parse(str, opts) -> rows | nil, err
function M.parse(str, opts)
    return core.parse_string(str, dialect.resolve(opts))
end



local function normalize_rows(rows)
    local out = {}

    for i, row in ipairs(rows) do
        local new_row = {}

        for j, v in ipairs(row) do
            if type(v) == "string" then
                new_row[j] = v
            else
                new_row[j] = tostring(v)
            end
        end

        out[i] = new_row
    end

    return out
end

-- csv.sheets(rows, opts) -> string
-- Serializes a rows table to CSV text (CRLF-joined, no trailing CRLF).
function M.write(rows, opts)
    local resolved = dialect.resolve(opts)

    if type(resolved) == "table" and opts and opts.quote_all ~= nil then
        resolved.quote_all = opts.quote_all
    end

    return core.write_rows(normalize_rows(rows), resolved)
end

-- sheets.write_csv(filename, rows, opts) -> true | nil, err
-- Convenience: writes rows straight to a file via csv.writer.
function M.write_csv(filename, rows, opts)
    local f, err = io.open(filename, "w")
    if not f then return nil, err end
    local w = M.writer(f, opts)
    w:writerows(rows)
    f:close()
    return true
end

-- sheets.read_dicts(filename, opts) -> array of {field=value} | nil, err
-- Convenience: one-shot DictReader over a whole file.
function M.read_dicts(filename, opts)
    local f, err = io.open(filename, "r")
    if not f then return nil, err end
    local ok, result = pcall(function()
        local dr = M.DictReader(f, opts)
        local out = {}
        for rec in dr do
            out[#out + 1] = rec
        end
        return out
    end)
    f:close()
    if not ok then return nil, result end
    return result
end





-- sheets.filter(rows, predicate) -> filtered_rows
-- Filter rows based on a predicate function
-- @param rows table Array of rows
-- @param predicate function(row) -> boolean
-- @return table Filtered rows
function M.filter(rows, predicate)
    if type(predicate) ~= "function" then
        error("filter predicate must be a function", 2)
    end
    local result = {}
    for i, row in ipairs(rows) do
        if predicate(row) then
            result[#result + 1] = row
        end
    end
    return result
end

-- sheets.map(rows, transformer) -> transformed_rows
-- Transform rows using a transformer function
-- @param rows table Array of rows
-- @param transformer function(row) -> modified_row
-- @return table Transformed rows
function M.map(rows, transformer)
    if type(transformer) ~= "function" then
        error("map transformer must be a function", 2)
    end
    local result = {}
    for i, row in ipairs(rows) do
        result[#result + 1] = transformer(row)
    end
    return result
end

-- sheets.merge(rows1, rows2) -> merged_rows
-- Merge two CSV row sets
-- @param rows1 table First array of rows
-- @param rows2 table Second array of rows
-- @return table Merged rows
function M.merge(rows1, rows2)
    local result = {}
    for i, row in ipairs(rows1) do
        result[#result + 1] = row
    end
    for i, row in ipairs(rows2) do
        result[#result + 1] = row
    end
    return result
end

-- sheets.unique(rows, key_func) -> unique_rows
-- Remove duplicate rows based on key function
-- @param rows table Array of rows
-- @param key_func function(row) -> key (optional, default uses whole row)
-- @return table Unique rows
function M.unique(rows, key_func)
    local seen = {}
    local result = {}

    for i, row in ipairs(rows) do
        local key
        if key_func then
            key = key_func(row)
        else
            key = table.concat(row, "\x00")  -- Use concatenated row as key
        end

        if not seen[key] then
            seen[key] = true
            result[#result + 1] = row
        end
    end
    return result
end

-- csv.sort(rows, comparator) -> sorted_rows
-- Sort rows using comparator function
-- @param rows table Array of rows
-- @param comparator function(a, b) -> boolean (a < b)
-- @return table Sorted rows
function M.sort(rows, comparator)
    local result = {}
    for i, row in ipairs(rows) do
        result[#result + 1] = row
    end

    if comparator then
        table.sort(result, comparator)
    else
        table.sort(result)
    end
    return result
end

-- csv.transpose(rows) -> transposed_rows
-- Transpose a CSV matrix (rows become columns)
-- @param rows table Array of rows
-- @return table Transposed rows
function M.transpose(rows)
    if #rows == 0 then return {} end

    local max_cols = 0
    for _, row in ipairs(rows) do
        max_cols = math.max(max_cols, #row)
    end

    local result = {}
    for col = 1, max_cols do
        local new_row = {}
        for row_idx, row in ipairs(rows) do
            new_row[row_idx] = row[col] or ""
        end
        result[col] = new_row
    end
    return result
end

-- csv.group_by(rows, key_func) -> grouped_rows
-- Group rows by key function
-- @param rows table Array of rows
-- @param key_func function(row) -> group_key
-- @return table Dictionary of {key = {rows}}
function M.group_by(rows, key_func)
    if type(key_func) ~= "function" then
        error("group_by key_func must be a function", 2)
    end

    local result = {}
    for i, row in ipairs(rows) do
        local key = key_func(row)
        if not result[key] then
            result[key] = {}
        end
        result[key][#result[key] + 1] = row
    end
    return result
end

-- sheets.column(rows, column_index) -> column_values
-- Extract a single column from rows
-- @param rows table Array of rows
-- @param column_index number Column index (1-based)
-- @return table Array of column values
function M.column(rows, column_index)
    if type(column_index) ~= "number" or column_index < 1 then
        error("column_index must be a positive number", 2)
    end

    local result = {}
    for i, row in ipairs(rows) do
        result[#result + 1] = row[column_index] or ""
    end
    return result
end

return M
