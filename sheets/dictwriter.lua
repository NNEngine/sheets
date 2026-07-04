--- sheets.DictWriter - like csv.writer but takes {fieldname = value} tables.
--
--   local f = io.open("out.csv", "w")
--   local w = sheets.DictWriter(f, { fieldnames = {"name", "age"} })
--   w:writeheader()
--   w:writerow({ name = "ABCD", age = 23 })
--   f:close()

-- Pull in the base Writer class that handles raw row output to a file.
-- DictWriter is a thin wrapper that converts dictionary-style records
-- into ordered row tables before passing them down.
local Writer = require("sheets.writer")

-- DictWriter uses the same Lua "class" pattern as Reader: a table with
-- __index so missing methods fall back to the DictWriter prototype.
local DictWriter = {}
DictWriter.__index = DictWriter

--- Create a new DictWriter attached to an output file.
-- @param file An open file handle in write mode (e.g., from io.open).
-- @param opts A table of options. Must contain `fieldnames`, an ordered
--        array of column names that defines the output schema. May also
--        contain `extrasaction` ("raise" or "ignore") to control handling
--        of unexpected keys in input records.
-- @return A new DictWriter object ready to write dictionary records.
function DictWriter.new(file, opts)
    -- Normalize opts to an empty table if omitted.
    opts = opts or {}
    -- fieldnames is mandatory because it defines the column order and
    -- the mapping from dictionary keys to output positions.
    if not opts.fieldnames then
        error("sheets.DictWriter requires opts.fieldnames = {...}", 2)
    end
    -- Build the object: wrap a base Writer, store the ordered fieldnames,
    -- and set the default extrasaction policy to "raise" (strict mode).
    return setmetatable({
        writer = Writer.new(file, opts),  -- Underlying row-oriented writer
        fieldnames = opts.fieldnames,      -- Ordered list of column names
        extrasaction = opts.extrasaction or "raise", -- "raise" | "ignore"
    }, DictWriter)
end

--- Write the header row (the fieldnames themselves) to the CSV.
-- This should typically be called once, before any data rows.
function DictWriter:writeheader()
    -- The base writer expects a row table; fieldnames is already in
    -- the correct order, so we can pass it directly.
    self.writer:writerow(self.fieldnames)
end

--- Write multiple records
-- Convenience batch method: converts each dictionary record to a row
-- and writes them all in one call.
-- @param dicts An array of dictionary-style record tables.
function DictWriter:writerows(dicts)
    for _, rec in ipairs(dicts) do
        self:writerow(rec)
    end
end

--- Convert a dictionary record into an ordered row table.
-- This is the core mapping logic: it reads values from the input record
-- using the predefined fieldnames order, then optionally validates for
-- unexpected extra keys depending on the extrasaction policy.
-- @param rec A dictionary-style table like { name = "ABCD", age = 23 }.
-- @return An ordered row table suitable for the base Writer.
function DictWriter:_dict_to_row(rec)
    -- Build the ordered row by walking fieldnames in schema order.
    -- If a field is missing in the record, the slot becomes nil.
    local row = {}
    for i, name in ipairs(self.fieldnames) do
        row[i] = rec[name]
    end

    -- If strict mode is enabled, reject any keys in the record that
    -- are not part of the declared schema. This catches typos and
    -- schema drift early rather than silently dropping data.
    if self.extrasaction == "raise" then
        -- Build a lookup set of known/expected field names.
        local known = {}
        for _, name in ipairs(self.fieldnames) do known[name] = true end
        -- Scan every key in the input record; if any key is unknown,
        -- raise an error with a helpful message.
        for k in pairs(rec) do
            if not known[k] then
                error("sheets.DictWriter: unexpected field '" .. tostring(k) .. "' (set extrasaction='ignore' to allow)", 3)
            end
        end
    end

    -- Return the ordered row. Missing fields remain as nil; the base
    -- Writer will typically render them as empty strings.
    return row
end

--- Write a single dictionary record to the CSV.
-- Converts the record to an ordered row and delegates to the base writer.
-- @param rec A dictionary-style table representing one CSV row.
function DictWriter:writerow(rec)
    self.writer:writerow(self:_dict_to_row(rec))
end

--- Write multiple records
-- Duplicate of the earlier writerows; both variants do the same thing.
-- Accepts an array of dictionary records and writes each one.
-- @param recs An array of dictionary-style record tables.
function DictWriter:writerows(recs)
    for _, rec in ipairs(recs) do
        self:writerow(rec)
    end
end

-- Expose the DictWriter module. Callers use DictWriter.new().
return DictWriter
