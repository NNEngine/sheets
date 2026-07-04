--- sheets.writer
--- ----------
--- Python-style CSV writer for Lua.
---
--- This module provides a lightweight Writer object that serializes Lua rows
--- into CSV format using the C backend (`sheets.core`) for high-performance
--- row encoding.
---
--- Example:
---
---   local f = io.open("out.csv", "w")
---   local w = sheets.writer(f)
---   w:writerow({"name", "age"})
---   w:writerows({
---       {"ABCD", 23},
---       {"Shivam", 24}
---   })
---   f:close()
---
--- Responsibilities:
--- - Validate output file handle
--- - Resolve dialect/write options
--- - Convert Lua values to strings
--- - Write single or multiple rows

local core = require("sheets.core")
local dialect = require("sheets.dialect")

local Writer = {}
Writer.__index = Writer

----------------------------------------------------------------------
-- Internal Helpers
----------------------------------------------------------------------

--- Convert a Lua row into a string-only row.
---
--- The C writer (`core.write_row`) expects all fields as strings.
--- Numbers, booleans, and other values are converted via tostring(),
--- similar to Python's csv module.
---
--- Important:
--- Lua arrays cannot reliably contain nil values in the middle of a row.
--- Since iteration uses ipairs(), encountering nil truncates traversal.
---
--- Example:
---   {"A", nil, "B"}
---
--- becomes effectively:
---   {"A"}
---
--- If an explicit empty CSV field is needed, use "" instead of nil.
---
--- @param row table Array-like row
--- @return table String-converted row
local function to_string_row(row)
    local out = {}

    for i, v in ipairs(row) do
        if type(v) == "string" then
            out[i] = v
        else
            out[i] = tostring(v)
        end
    end

    return out
end

----------------------------------------------------------------------
-- Writer Constructor
----------------------------------------------------------------------

--- Create a new CSV writer.
---
--- @param file file Open writable file handle
--- @param opts table|nil CSV writer options / dialect settings
--- @return Writer
function Writer.new(file, opts)
    if io.type(file) ~= "file" then
        error("sheets.writer expects an open file handle", 2)
    end

    local resolved = dialect.resolve(opts)

    return setmetatable({
        file = file,
        opts = resolved,
        terminator = resolved.line_terminator or "\r\n",
    }, Writer)
end

----------------------------------------------------------------------
-- Row Writing
----------------------------------------------------------------------

--- Write a single row to the output file.
---
--- The row is converted to strings and serialized using the C backend.
--- A line terminator is automatically appended.
---
--- @param row table Array-like row
function Writer:writerow(row)
    local line = core.write_row(to_string_row(row), self.opts)
    self.file:write(line, self.terminator)
end

--- Write multiple rows to the output file.
---
--- @param rows table Array of row tables
function Writer:writerows(rows)
    for _, row in ipairs(rows) do
        self:writerow(row)
    end
end

return Writer
