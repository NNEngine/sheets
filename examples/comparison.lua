local csv = require("sheets")

local FILE = "benchmark_1m.csv"
local ROWS = 1000000
local COLS = 20

local function now()
    return os.clock()
end

local function mem_mb()
    return collectgarbage("count") / 1024
end

local function generate_csv()
    print("Generating CSV...")

    local t1 = now()
    local f = assert(io.open(FILE, "w"))

    -- header
    local header = {}
    for i = 1, COLS do
        header[i] = "col" .. i
    end
    f:write(table.concat(header, ","), "\n")

    for r = 1, ROWS do
        local row = {
            tostring(r),
            tostring(r * 2),
            string.format("%.4f", r / 100),
            "user_" .. r,
            "\"quoted,value\"",
            "city_" .. (r % 100),
            tostring(r % 500),
            tostring(r * 10),
            tostring(r / 3),
            "data_" .. r,
            tostring(r + 11),
            tostring(r + 12),
            tostring(r + 13),
            tostring(r + 14),
            tostring(r + 15),
            tostring(r + 16),
            tostring(r + 17),
            tostring(r + 18),
            tostring(r + 19),
            tostring(r + 20)
        }

        f:write(table.concat(row, ","), "\n")

        if r % 100000 == 0 then
            print(("Generated %d rows..."):format(r))
        end
    end

    f:close()

    local t2 = now()
    print(("CSV generated in %.3f sec"):format(t2 - t1))
end

local function benchmark_parse()
    collectgarbage("collect")

    print("\nBenchmarking parse...")
    print(("Memory before: %.2f MB"):format(mem_mb()))

    local t1 = now()
    local rows, err = csv.read_csv(FILE)
    local t2 = now()

    if not rows then
        error(err)
    end

    print(("Parse time: %.3f sec"):format(t2 - t1))
    print(("Rows parsed: %d"):format(#rows))
    print(("Columns in row 1: %d"):format(#rows[1]))
    print(("Memory after: %.2f MB"):format(mem_mb()))
end

local function benchmark_write()
    collectgarbage("collect")

    local rows = {
        {"name", "age", "city"},
        {"Shivam", "23", "Gwalior"},
        {"Alice", "25", "New York"},
        {"Bob", "27", "London"},
    }

    local ITER = 100000

    print("\nBenchmarking write...")

    local t1 = now()

    for _ = 1, ITER do
        csv.write(rows)
    end

    local t2 = now()
    print(("Write benchmark (%d iterations): %.3f sec"):format(ITER, t2 - t1))
end

-- Generate file only once
local f = io.open(FILE, "r")
if not f then
    generate_csv()
else
    f:close()
    print("Benchmark CSV already exists.")
end

benchmark_parse()
benchmark_write()
