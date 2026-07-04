-- Enhanced test suite for sheets v2.0.0
-- Tests new functions, bug fixes, and edge cases

local script_dir = (arg[0] or ""):match("(.*/)") or "./"
local project_root = script_dir .. "../"
package.path = package.path .. ";" .. project_root .. "?.lua;" .. project_root .. "?/init.lua"
package.cpath = package.cpath .. ";" .. project_root .. "?.so"

local csv = require("sheets")

local test_count = 0
local pass_count = 0
local fail_count = 0

local function assert_equal(actual, expected, msg)
    test_count = test_count + 1
    if actual == expected then
        pass_count = pass_count + 1
        print("_/ " .. (msg or "test passed"))
    else
        fail_count = fail_count + 1
        print("X " .. (msg or "test failed"))
        print("  Expected: " .. tostring(expected))
        print("  Actual: " .. tostring(actual))
    end
end

local function assert_true(value, msg)
    test_count = test_count + 1
    if value then
        pass_count = pass_count + 1
        print("_/ " .. (msg or "assertion passed"))
    else
        fail_count = fail_count + 1
        print("X " .. (msg or "assertion failed"))
    end
end

local function assert_array_equal(actual, expected, msg)
    test_count = test_count + 1
    if #actual ~= #expected then
        fail_count = fail_count + 1
        print("✗ " .. (msg or "array length mismatch"))
        return
    end
    for i, v in ipairs(expected) do
        if actual[i] ~= v then
            fail_count = fail_count + 1
            print("✗ " .. (msg or "array element mismatch at index " .. i))
            return
        end
    end
    pass_count = pass_count + 1
    print("_/ " .. (msg or "array match passed"))
end

print("=========================================================================")
print("sheets v" .. csv._VERSION .. " -  Test Suite")
print("=========================================================================")

-- Test 1: Version check
print("\n[1] Version and metadata")
assert_equal(csv._VERSION, "2.0.0", "Version is 2.0.0")
assert_true(csv._LIBCSV_VERSION ~= nil, "libcsv version available")

-- Test 2: Basic CSV parsing from string
print("\n[2] CSV parsing from string")
local csv_text = "name,age,city\nAlice,30,NYC\nBob,25,LA"
local rows = csv.parse(csv_text)
assert_equal(#rows, 3, "Parsed 3 rows")
assert_equal(rows[1][1], "name", "Header row correct")
assert_equal(rows[2][1], "Alice", "First data row correct")

-- Test 3: CSV serialization
print("\n[3] CSV serialization")
local output = csv.write(rows)
assert_true(string.len(output) > 0, "Generated CSV string")
assert_true(string.find(output, "Alice"), "Output contains data")

-- Test 4: Reader class
print("\n[4] Reader class methods")
local reader = csv.reader(csv_text)
assert_equal(reader:count(), 3, "Reader count is correct")
local first = reader:get(1)
assert_equal(first[1], "name", "Reader:get() works")

-- Test 5: Reader.readall()
print("\n[5] Reader.readall() method")
local reader2 = csv.reader(csv_text)
local all_rows = reader2:readall()
assert_equal(#all_rows, 3, "readall() returns all rows")
assert_equal(all_rows[2][1], "Alice", "readall() preserves data")

-- Test 6: Reader.column()
print("\n[6] Reader.column() method")
local reader3 = csv.reader(csv_text)
local names = reader3:column(1)
assert_equal(#names, 3, "column() returns correct count")
assert_equal(names[1], "name", "column() data is correct")

-- Test 7: Reader.filter()
print("\n[7] Reader.filter() method")
local reader4 = csv.reader(csv_text)
local filtered = reader4:filter(function(row)
    return tonumber(row[2]) ~= nil and tonumber(row[2]) > 26
end)
assert_true(#filtered >= 1, "filter() works")

-- Test 8: Reader.map()
print("\n[8] Reader.map() method")
local reader5 = csv.reader(csv_text)
local mapped = reader5:map(function(row)
    return {row[1]:upper(), row[2]}
end)
assert_equal(mapped[1][1], "NAME", "map() transforms data")

-- Test 9: DictReader basic
print("\n[9] DictReader basic functionality")
local dr = csv.DictReader(csv_text)
local rec = dr:next_row()
assert_equal(rec.name, "Alice", "DictReader field access works")
assert_equal(rec.age, "30", "DictReader preserves data types as strings")

-- Test 10: DictReader reset fix
print("\n[10] DictReader.reset() fix")
local dr2 = csv.DictReader(csv_text)
local recs1 = dr2:readall()
dr2:reset()
local recs2 = dr2:readall()
assert_equal(#recs1, #recs2, "reset() works correctly")

-- Test 11: DictReader filter
print("\n[11] DictReader.filter() method")
local dr3 = csv.DictReader(csv_text)
local filtered_dicts = dr3:filter(function(rec)
    return rec.name == "Alice"
end)
assert_equal(#filtered_dicts, 1, "DictReader filter works")

-- Test 12: DictReader map
print("\n[12] DictReader.map() method")
local dr4 = csv.DictReader(csv_text)
local mapped_dicts = dr4:map(function(rec)
    return {name_upper = rec.name:upper(), age = rec.age}
end)
assert_equal(mapped_dicts[1].name_upper, "ALICE", "DictReader map works")

-- Test 13: DictWriter with writerows
print("\n[13] DictWriter.writerows() method")
local dicts = {
    {name = "Charlie", age = 35, city = "Boston"},
    {name = "Diana", age = 28, city = "Denver"}
}
local out_file = script_dir .. "_test_output.csv"
local f = io.open(out_file, "w")
local dw = csv.DictWriter(f, {fieldnames = {"name", "age", "city"}})
dw:writeheader()
dw:writerows(dicts)
f:close()

-- Verify written file
local verify_f = io.open(out_file, "r")
local content = verify_f:read("*a")
verify_f:close()
assert_true(string.find(content, "Charlie"), "DictWriter.writerows() works")
os.remove(out_file)

-- Test 14: csv.filter() function
print("\n[14] csv.filter() utility function")
local filtered_util = csv.filter(rows, function(row)
    return row[1] ~= "name"
end)
assert_equal(#filtered_util, 2, "csv.filter() works")

-- Test 15: csv.map() function
print("\n[15] csv.map() utility function")
local mapped_util = csv.map(rows, function(row)
    return {row[1]:upper(), row[2]}
end)
assert_equal(mapped_util[1][1], "NAME", "csv.map() works")

-- Test 16: csv.merge() function
print("\n[16] csv.merge() utility function")
local rows2 = {{"Eve", "32", "Miami"}}
local merged = csv.merge(rows, rows2)
assert_equal(#merged, #rows + #rows2, "csv.merge() works")

-- Test 17: csv.unique() function
print("\n[17] csv.unique() utility function")
local dupes = {
    {"name", "age"},
    {"Alice", "30"},
    {"Alice", "30"},
    {"Bob", "25"}
}
local unique = csv.unique(dupes)
assert_equal(#unique, 3, "csv.unique() removes duplicates")

-- Test 18: csv.sort() function
print("\n[18] csv.sort() utility function")
local unsorted = {
    {"Charlie", "35"},
    {"Alice", "30"},
    {"Bob", "25"}
}
local sorted = csv.sort(unsorted, function(a, b)
    return a[1] < b[1]
end)
assert_equal(sorted[1][1], "Alice", "csv.sort() works")

-- Test 19: csv.transpose() function
print("\n[19] csv.transpose() utility function")
local transpose_input = {
    {"a", "b"},
    {"1", "2"},
    {"x", "y"}
}
local transposed = csv.transpose(transpose_input)
assert_equal(transposed[1][2], "1", "csv.transpose() works")

-- Test 20: csv.group_by() function
print("\n[20] csv.group_by() utility function")
local people = {
    {"Alice", "Engineer"},
    {"Bob", "Designer"},
    {"Charlie", "Engineer"}
}
local grouped = csv.group_by(people, function(row)
    return row[2]
end)
assert_true(grouped.Engineer ~= nil, "csv.group_by() works")
assert_equal(#grouped.Engineer, 2, "csv.group_by() groups correctly")

-- Test 21: csv.column() function
print("\n[21] csv.column() utility function")
local col = csv.column(rows, 1)
assert_equal(#col, #rows, "csv.column() extracts column")

-- Test 22: Dialect handling
print("\n[22] Dialect system")
local unix_data = "name,age\nAlice,30"
local unix_rows = csv.parse(unix_data, {dialect = "unix"})
assert_equal(#unix_rows, 2, "Unix dialect works")

-- Test 23: Custom dialect
print("\n[23] Custom dialect registration")
csv.register_dialect("test_pipe", {delim = "|", quote = '"'})
local test_dialect = csv.get_dialect("test_pipe")
assert_true(test_dialect ~= nil, "Custom dialect registered")
assert_equal(test_dialect.delim, "|", "Custom dialect has correct delimiter")
csv.unregister_dialect("test_pipe")

-- Test 24: Sniffer
print("\n[24] CSV format sniffer")
local sample = "a,b,c\n1,2,3\n4,5,6"
local detected = csv.sniff(sample)
assert_true(detected ~= nil, "Sniffer detects format")
assert_equal(detected.delim, ",", "Sniffer detects comma")

-- Test 25: Error handling - missing file
print("\n[25] Error handling for missing files")
local result, err = csv.read_csv("nonexistent_file_xyz.csv")
assert_true(result == nil or err ~= nil, "Handles missing file gracefully")

print("\n[26] Quoted comma fields")
local quoted_csv = 'name,note\nAlice,"hello,world"'
local quoted_rows = csv.parse(quoted_csv)
assert_equal(quoted_rows[2][2], "hello,world", "Quoted comma parsed correctly")

print("\n[27] Escaped quotes")
local escaped_csv = 'name,note\nAlice,"hello ""world"""'
local escaped_rows = csv.parse(escaped_csv)
assert_equal(escaped_rows[2][2], 'hello "world"', "Escaped quotes parsed correctly")

print("\n[28] Empty fields")
local empty_csv = "a,b,c\n1,,3"
local empty_rows = csv.parse(empty_csv)
assert_equal(empty_rows[2][2], "", "Empty middle field handled")

print("\n[29] Trailing empty fields")
local trailing_csv = "a,b,c\n1,2,"
local trailing_rows = csv.parse(trailing_csv)
assert_equal(trailing_rows[2][3], "", "Trailing empty field handled")

print("\n[30] Multiline quoted fields")
local multiline_csv = 'name,note\nAlice,"hello\nworld"'
local multiline_rows = csv.parse(multiline_csv)
assert_true(multiline_rows[2][2]:find("world") ~= nil, "Multiline field parsed correctly")

print("\n[31] Strict mode error handling")
local bad_csv = 'name,age\nAlice,"30'
local result, err = csv.parse(bad_csv, {strict = true})
assert_true(result == nil, "Strict mode rejects malformed CSV")
assert_true(err ~= nil, "Strict mode returns error message")

print("\n[32] Unicode support")
local unicode_csv = "name,city\nShivam,東京"
local unicode_rows = csv.parse(unicode_csv)
assert_equal(unicode_rows[2][2], "東京", "Unicode parsed correctly")

print("\n[33] Mixed type writing")
local mixed_rows = {
    {"name", "active", "age"},
    {"Alice", true, 30}
}
local mixed_output = csv.write(mixed_rows)
assert_true(mixed_output:find("true") ~= nil, "Boolean converted correctly")
assert_true(mixed_output:find("30") ~= nil, "Number converted correctly")

print("\n[34] Custom delimiter parsing")
local pipe_csv = "a|b|c\n1|2|3"
local pipe_rows = csv.parse(pipe_csv, {delim = "|"})
assert_equal(pipe_rows[2][2], "2", "Pipe delimiter works")

print("\n[35] Single-column CSV")
local single_csv = "name\nAlice\nBob"
local single_rows = csv.parse(single_csv)
assert_equal(#single_rows, 3, "Single-column CSV parsed")
assert_equal(single_rows[2][1], "Alice", "Single-column values correct")

print("\n[36] Blank lines handling")
local blank_csv = "a,b\n1,2\n\n3,4"
local blank_rows = csv.parse(blank_csv)
assert_true(#blank_rows >= 3, "Blank lines handled")



print("\n=========================================================================")
print("Test Results:")
print("  Total:  " .. test_count)
print("  Passed: " .. pass_count)
print("  Failed: " .. fail_count)
print("=========================================================================")

if fail_count == 0 then
    print("\n ALL TESTS PASSED!")
    os.exit(0)
else
    print("\n" .. fail_count .. " test(s) failed")
    os.exit(1)
end
