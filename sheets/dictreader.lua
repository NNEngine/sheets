--- sheets.DictReader - like csv.reader but yields {fieldname = value} tables.
--
--   local f = io.open("data.csv")
--   for rec in sheets.DictReader(f) do
--       print(rec.name, rec.age)
--   end
--
-- If opts.fieldnames is not given, the first row of the source is
-- consumed and used as the header (same default as Python's DictReader).

local Reader = require("sheets.reader")

local DictReader = {}
DictReader.__index = DictReader

function DictReader.new(source, opts)
    opts = opts or {}
    local reader = Reader.new(source, opts)

    local fieldnames = opts.fieldnames
    local consumed_header = false

    if not fieldnames then
        fieldnames = reader:next_row()
        if not fieldnames then
            error("sheets.DictReader: source is empty, no header row found", 2)
        end
        consumed_header = true
    end

    return setmetatable({
        reader = reader,
        fieldnames = fieldnames,
        restkey = opts.restkey,   -- key to stash extra fields under, if row is longer than fieldnames
        restval = opts.restval,   -- value to fill missing fields with, if row is shorter
        line_num = 0,
        _consumed_header = consumed_header,  -- FIX: Track if header was consumed
    }, DictReader)
end

function DictReader:next_row()
    local row = self.reader:next_row()
    if not row then return nil end
    self.line_num = self.reader.line_num

    local rec = {}
    local nf = #self.fieldnames

    for i, name in ipairs(self.fieldnames) do
        local v = row[i]
        rec[name] = (v ~= nil) and v or self.restval
    end

    if #row > nf and self.restkey then
        local rest = {}
        for i = nf + 1, #row do
            rest[#rest + 1] = row[i]
        end
        rec[self.restkey] = rest
    end

    return rec
end

function DictReader:reset()
    -- Re-point at the start; if fieldnames came from the first row,
    -- skip it again on the next pull.
    self.reader:reset()
    if self._consumed_header then
        self.reader:next_row()
    end
    self.line_num = 0
end

--- Get all remaining records as array
function DictReader:readall()
    local result = {}
    for rec in self do
        result[#result + 1] = rec
    end
    return result
end

--- Get record count (requires reading all records)
function DictReader:count()
    local count = 0
    local pos = self.line_num
    for _ in self do
        count = count + 1
    end
    return count
end

--- Filter records by predicate
function DictReader:filter(predicate)
    if type(predicate) ~= "function" then
        error("filter predicate must be a function", 2)
    end
    local result = {}
    for rec in self do
        if predicate(rec) then
            result[#result + 1] = rec
        end
    end
    return result
end

--- Map records with transformer
function DictReader:map(transformer)
    if type(transformer) ~= "function" then
        error("map transformer must be a function", 2)
    end
    local result = {}
    for rec in self do
        result[#result + 1] = transformer(rec)
    end
    return result
end

DictReader.__call = function(self)
    return self:next_row()
end

return DictReader
