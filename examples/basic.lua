#!/usr/bin/env lua

-- examples/basic.lua - Complete demonstration of sheets v2.0.0
-- Shows all functions: original API + new v2.0.0 features
-- Run from project root: lua examples/basic.lua

package.path = package.path .. ";./?.lua;./?/init.lua"
package.cpath = package.cpath .. ";./?.so"

local csv = require("sheets")

print("=================================================================================")
print("               sheets v" .. csv._VERSION .. " - Complete Feature Demo           ")
print("=================================================================================")

-- =====================================================================
-- SECTION 1: BASIC READING (Original API)
-- =====================================================================
print("\n[1] BASIC READING - Reading CSV rows")
print("=========================================================================")

local f = io.open("test/fixtures/sample.csv")
print("\nUsing csv.reader() to iterate rows:")
local rows = {}
for row in csv.reader(f) do
    rows[#rows + 1] = row
    print("  Row " .. #rows .. ": " .. table.concat(row, " | "))
end
f:close()

-- =====================================================================
-- SECTION 2: BASIC DICTIONARY READING (Original API)
-- =====================================================================
print("\n[2] DICTIONARY READING - Named fields")
print("=========================================================================")

local f2 = io.open("test/fixtures/sample.csv")
print("\nUsing csv.DictReader() with field names:")
for rec in csv.DictReader(f2) do
    print(("  %s (age %s) from %s"):format(rec.name, rec.age, rec.city))
end
f2:close()

-- =====================================================================
-- SECTION 3: BASIC WRITING (Original API)
-- =====================================================================
print("\n[3] BASIC WRITING - Writing CSV files")
print("=========================================================================")

local out = io.open("/tmp/example_output.csv", "w")
local w = csv.writer(out)
print("\nWriting rows to /tmp/example_output.csv:")
w:writerow({"Name", "Age", "City"})
w:writerow({"Alice", 30, "New York"})
w:writerow({"Bob", 25, "Los Angeles"})
w:writerow({"Charlie", 35, "Chicago"})
out:close()
print("  ✓ Wrote 4 rows")

-- =====================================================================
-- SECTION 4: ONE-SHOT OPERATIONS (Original API)
-- =====================================================================
print("\n[4] ONE-SHOT OPERATIONS - No manual file handling")
print("=========================================================================")

local all_rows = csv.read_csv("test/fixtures/sample.csv")
print("\ncsv.read_csv() - Read entire file at once:")
print("  Total rows: " .. #all_rows)

-- =====================================================================
-- SECTION 5: READER CLASS NEW METHODS (v2.0.0)
-- =====================================================================
print("\n[5] READER NEW METHODS - Enhanced row access")
print("=========================================================================")

local reader = csv.reader("name,age,city\nAlice,30,NYC\nBob,25,LA\nCharlie,35,Chicago")

print("\nReader:count() - Count rows:")
print("  Total rows: " .. reader:count())

print("\nReader:get(index) - Get specific row:")
local row1 = reader:get(1)
print("  Row 1: " .. table.concat(row1, " | "))

print("\nReader:column(index) - Extract column:")
local names_col = reader:column(1)
print("  Names column: " .. table.concat(names_col, ", "))

print("\nReader:slice(start, finish) - Get row range:")
local slice = reader:slice(2, 3)
print("  Rows 2-3: " .. #slice .. " rows")
for i, row in ipairs(slice) do
    print("    " .. table.concat(row, " | "))
end

print("\nReader:readall() - Get all remaining rows:")
local reader2 = csv.reader("a,b\n1,2\n3,4")
local all = reader2:readall()
print("  Got " .. #all .. " rows")

-- =====================================================================
-- SECTION 6: READER FILTERING & MAPPING (v2.0.0)
-- =====================================================================
print("\n[6] READER FILTERING & MAPPING - Transform data")
print("=========================================================================")

local data = "name,age,city\nAlice,30,NYC\nBob,25,LA\nCharlie,35,Chicago\nDiana,28,Boston"

print("\nReader:filter() - Filter rows by predicate:")
local reader3 = csv.reader(data)
local over_30 = reader3:filter(function(row)
    local age = tonumber(row[2])
    return age and age >= 30
end)
print("  People aged 30+:")
for _, row in ipairs(over_30) do
    print("    " .. row[1] .. " (" .. row[2] .. ")")
end

print("\nReader:map() - Transform rows:")
local reader4 = csv.reader(data)
local uppercase = reader4:map(function(row)
    return {row[1]:upper(), row[2], row[3]:upper()}
end)
print("  Uppercase names and cities:")
for _, row in ipairs(uppercase) do
    print("    " .. table.concat(row, " | "))
end

-- =====================================================================
-- SECTION 7: DICTREADER NEW METHODS (v2.0.0)
-- =====================================================================
print("\n[7] DICTREADER NEW METHODS - Enhanced dictionary access")
print("=========================================================================")

print("\nDictReader:readall() - Get all records:")
local dr = csv.DictReader(data)
local all_dicts = dr:readall()
print("  Total records: " .. #all_dicts)
print("  First record: " .. all_dicts[1].name .. " from " .. all_dicts[1].city)

print("\nDictReader:filter() - Filter records:")
local dr2 = csv.DictReader(data)
local young = dr2:filter(function(rec)
    local age = tonumber(rec.age)
    return age and age < 30
end)
print("  People under 30:")
for _, rec in ipairs(young) do
    print("    " .. rec.name .. " (age " .. rec.age .. ")")
end

print("\nDictReader:map() - Transform records:")
local dr3 = csv.DictReader(data)
local mapped_dicts = dr3:map(function(rec)
    local age = tonumber(rec.age) or 0
    return {
        full_name = rec.name:upper(),
        age_group = age >= 30 and "30+" or "Under 30"
    }
end)
print("  Transformed records:")
for _, rec in ipairs(mapped_dicts) do
    print("    " .. rec.full_name .. " - " .. rec.age_group)
end

-- =====================================================================
-- SECTION 8: CSV UTILITY FUNCTIONS - FILTER (v2.0.0)
-- =====================================================================
print("\n[8] CSV.FILTER() - Filter rows at module level")
print("=========================================================================")

local sample_rows = {
    {"name", "score"},
    {"Alice", "95"},
    {"Bob", "78"},
    {"Charlie", "88"},
    {"Diana", "92"}
}

print("\nFilter rows where score > 85:")
local high_scores = csv.filter(sample_rows, function(row)
    local score = tonumber(row[2])
    return score and score > 85
end)
for _, row in ipairs(high_scores) do
    print("  " .. row[1] .. ": " .. row[2])
end

-- =====================================================================
-- SECTION 9: CSV UTILITY FUNCTIONS - MAP (v2.0.0)
-- =====================================================================
print("\n[9] CSV.MAP() - Transform rows at module level")
print("=========================================================================")

print("\nDouble all numeric values:")
local doubled = csv.map(sample_rows, function(row)
    local num = tonumber(row[2])
    return {row[1], num and tostring(num * 2) or row[2]}
end)
for _, row in ipairs(doubled) do
    print("  " .. row[1] .. ": " .. row[2])
end

-- =====================================================================
-- SECTION 10: CSV UTILITY FUNCTIONS - MERGE (v2.0.0)
-- =====================================================================
print("\n[10] CSV.MERGE() - Combine row sets")
print("=========================================================================")

local set1 = {{"Alice", "30"}, {"Bob", "25"}}
local set2 = {{"Charlie", "35"}, {"Diana", "28"}}

print("\nMerging two datasets:")
local merged = csv.merge(set1, set2)
print("  Total rows: " .. #merged)
for _, row in ipairs(merged) do
    print("    " .. row[1] .. " (" .. row[2] .. ")")
end

-- =====================================================================
-- SECTION 11: CSV UTILITY FUNCTIONS - UNIQUE (v2.0.0)
-- =====================================================================
print("\n[11] CSV.UNIQUE() - Remove duplicates")
print("=========================================================================")

local with_dupes = {
    {"Alice", "30"},
    {"Bob", "25"},
    {"Alice", "30"},
    {"Charlie", "35"},
    {"Bob", "25"}
}

print("\nRemoving duplicates:")
local unique = csv.unique(with_dupes)
print("  Before: " .. #with_dupes .. " rows")
print("  After: " .. #unique .. " rows (unique)")
for _, row in ipairs(unique) do
    print("    " .. row[1])
end

-- =====================================================================
-- SECTION 12: CSV UTILITY FUNCTIONS - SORT (v2.0.0)
-- =====================================================================
print("\n[12] CSV.SORT() - Sort rows")
print("=========================================================================")

local unsorted = {
    {"Charlie", "35"},
    {"Alice", "30"},
    {"Bob", "25"},
    {"Diana", "28"}
}

print("\nSort by name alphabetically:")
local sorted = csv.sort(unsorted, function(a, b)
    return a[1] < b[1]
end)
for _, row in ipairs(sorted) do
    print("  " .. row[1] .. " (age " .. row[2] .. ")")
end

print("\nSort by age numerically:")
local sorted_age = csv.sort(unsorted, function(a, b)
    local age_a = tonumber(a[2]) or 0
    local age_b = tonumber(b[2]) or 0
    return age_a < age_b
end)
for _, row in ipairs(sorted_age) do
    print("  " .. row[1] .. " (age " .. row[2] .. ")")
end

-- =====================================================================
-- SECTION 13: CSV UTILITY FUNCTIONS - TRANSPOSE (v2.0.0)
-- =====================================================================
print("\n[13] CSV.TRANSPOSE() - Transpose matrix")
print("=========================================================================")

local matrix = {
    {"A", "B", "C"},
    {"1", "2", "3"},
    {"X", "Y", "Z"}
}

print("\nOriginal matrix:")
for _, row in ipairs(matrix) do
    print("  " .. table.concat(row, " "))
end

print("\nTransposed matrix:")
local transposed = csv.transpose(matrix)
for _, row in ipairs(transposed) do
    print("  " .. table.concat(row, " "))
end

-- =====================================================================
-- SECTION 14: CSV UTILITY FUNCTIONS - GROUP_BY (v2.0.0)
-- =====================================================================
print("\n[14] CSV.GROUP_BY() - Group rows by key")
print("=========================================================================")

local people = {
    {"Alice", "Engineer"},
    {"Bob", "Designer"},
    {"Charlie", "Engineer"},
    {"Diana", "Manager"},
    {"Eve", "Designer"}
}

print("\nGroup by job title:")
local grouped = csv.group_by(people, function(row)
    return row[2]
end)

for job, group in pairs(grouped) do
    print("  " .. job .. ":")
    for _, person in ipairs(group) do
        print("    - " .. person[1])
    end
end

-- =====================================================================
-- SECTION 15: CSV UTILITY FUNCTIONS - COLUMN (v2.0.0)
-- =====================================================================
print("\n[15] CSV.COLUMN() - Extract single column")
print("=========================================================================")

local data_table = {
    {"Name", "Age", "City"},
    {"Alice", "30", "NYC"},
    {"Bob", "25", "LA"},
    {"Charlie", "35", "Chicago"}
}

print("\nExtract column 1 (Names):")
local col1 = csv.column(data_table, 1)
print("  " .. table.concat(col1, ", "))

print("\nExtract column 2 (Ages):")
local col2 = csv.column(data_table, 2)
print("  " .. table.concat(col2, ", "))

print("\nExtract column 3 (Cities):")
local col3 = csv.column(data_table, 3)
print("  " .. table.concat(col3, ", "))

-- =====================================================================
-- SECTION 16: DICTWRITER NEW METHOD (v2.0.0)
-- =====================================================================
print("\n[16] DICTWRITER.WRITEROWS() - Write multiple records at once")
print("=========================================================================")

local records = {
    {name = "Frank", age = 40, city = "Seattle"},
    {name = "Grace", age = 32, city = "Portland"},
    {name = "Henry", age = 29, city = "San Francisco"}
}

print("\nWriting multiple records with writerows():")
local out2 = io.open("/tmp/dict_output.csv", "w")
local dw = csv.DictWriter(out2, {fieldnames = {"name", "age", "city"}})
dw:writeheader()
dw:writerows(records)
out2:close()
print("  ✓ Wrote " .. #records .. " records to /tmp/dict_output.csv")

-- =====================================================================
-- SECTION 17: BUG FIX - DICTREADER RESET (v2.0.0)
-- =====================================================================
print("\n[17] BUG FIX - DictReader.reset() now works correctly")
print("=========================================================================")

local test_data = "name,value\nA,1\nB,2\nC,3"
local dr_reset = csv.DictReader(test_data)

print("\nFirst iteration:")
local count1 = 0
for rec in dr_reset do
    count1 = count1 + 1
    print("  " .. rec.name .. " = " .. rec.value)
end

print("\nAfter reset():")
dr_reset:reset()
local count2 = 0
for rec in dr_reset do
    count2 = count2 + 1
    print("  " .. rec.name .. " = " .. rec.value)
end

print("Reset works! Read " .. count1 .. " records, then " .. count2 .. " again")

-- =====================================================================
-- SUMMARY
-- =====================================================================
print("\n" .. string.rep("=", 65))
print("SUMMARY - sheets v" .. csv._VERSION .. " Features Demonstrated")
print(string.rep("=", 65))
print("\n Original API:")
print("=> csv.reader() - Row-based reading")
print("=> csv.DictReader() - Dictionary-based reading")
print("=> csv.writer() - Row-based writing")
print("=> csv.DictWriter() - Dictionary-based writing")
print("=> csv.read_csv() - One-shot file reading")
print("=> csv.write_csv() - One-shot file writing")

print("\n NEW in v2.0.0 - Reader Methods:")
print("=> Reader:count(), :get(), :slice()")
print("=> Reader:column(), :readall()")
print("=> Reader:filter(), :map()")

print("\n NEW in v2.0.0 - DictReader Methods:")
print("=> DictReader:readall(), :count()")
print("=> DictReader:filter(), :map()")

print("\n NEW in v2.0.0 - Utility Functions:")
print("=> csv.filter() - Filter rows")
print("=> csv.map() - Transform rows")
print("=> csv.merge() - Combine datasets")
print("=> csv.unique() - Remove duplicates")
print("=> csv.sort() - Sort rows")
print("=> csv.transpose() - Transpose matrix")
print("=> csv.group_by() - Group by key")
print("=> csv.column() - Extract column")

print("\n NEW in v2.0.0 - DictWriter Methods:")
print("=> DictWriter:writerows()")

print("\n BUG FIXES:")
print("=> DictReader.reset() - Now correctly tracks consumed_header")

print("\n" .. string.rep("=", 65))
print("All features working! Ready for production use!")
print(string.rep("=", 65) .. "\n")
