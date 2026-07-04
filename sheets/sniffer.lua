-- sheets.sniffer - guesses the delimiter of a CSV sample, Python-Sniffer-style.
--
--   local dialect = sheets.sniff(sample_text)
--   local rows = sheets.parse(full_text, dialect)

local M = {}

-- List of candidate delimiters to test against the sample.
-- These are the most common CSV separators used in the wild.
local CANDIDATES = { ",", ";", "\t", "|" }

-- Escapes a string so it can be used safely inside a Lua pattern.
-- Lua's string.gsub uses patterns (not plain strings), so special
-- characters like $, %, ., etc. must be escaped with %.
-- @param s The raw delimiter string to escape.
-- @return A pattern-safe version of the string.
local function escape_pattern(s)
    return s:gsub("([%^%$%(%)%%%.%[%]%*%+%-%?])", "%%%1")
end

-- sheets.sniff(sample) -> { delim = "," } | nil, err
-- Looks at the first few lines of `sample` and picks whichever candidate
-- delimiter has the most *consistent* count per line (i.e. every line
-- has the same number of that character -- a strong signal it's the
-- real field separator, not just a character that happens to appear).
-- @param sample A string containing a snippet of CSV data to analyze.
-- @param opts Optional table. Supports `max_lines` to limit how many
--        lines are examined (default: 10).
-- @return A dialect table like { delim = "," } on success, or nil plus
--         an error message on failure.
function M.sniff(sample, opts)
    -- Normalize opts to an empty table if not provided.
    opts = opts or {}
    -- Limit analysis to the first N lines to avoid being skewed by
    -- malformed or unusual data deep in the file.
    local max_lines = opts.max_lines or 10

    -- Collect non-empty lines from the sample into a table.
    -- This ignores blank lines so they don't distort the count.
    local lines = {}
    for line in sample:gmatch("([^\r\n]+)") do
        lines[#lines + 1] = line
        if #lines >= max_lines then break end
    end

    -- If we couldn't extract any lines, the sample is unusable.
    if #lines == 0 then
        return nil, "sheets.sniff: sample is empty"
    end

    -- Track the best candidate found so far.
    -- best_delim: the delimiter character that seems most likely.
    -- best_score: how many lines agreed on the same count for that delim.
    local best_delim, best_score = nil, -1

    -- Test each candidate delimiter one by one.
    for _, delim in ipairs(CANDIDATES) do
        -- counts[i] = number of occurrences of this delimiter on line i.
        local counts = {}
        for _, line in ipairs(lines) do
            -- Count how many times the delimiter appears in this line.
            -- The trick: gsub returns the number of replacements made.
            local _, n = line:gsub(escape_pattern(delim), "")
            counts[#counts + 1] = n
        end

        -- Build a frequency table: how many lines had exactly N occurrences?
        local freq = {}
        for _, c in ipairs(counts) do freq[c] = (freq[c] or 0) + 1 end

        -- Find the mode (most common count) among lines that actually
        -- contain the delimiter at least once (count > 0).
        -- A delimiter that never appears (mode_count == 0) is discarded.
        local mode_count, mode_freq = 0, 0
        for c, f in pairs(freq) do
            if c > 0 and f > mode_freq then
                mode_count, mode_freq = c, f
            end
        end

        -- Prefer the delimiter with the highest consistency across lines.
        -- mode_freq tells us how many lines shared the same count.
        -- mode_count > 0 ensures the delimiter actually appears in the data.
        if mode_freq > best_score and mode_count > 0 then
            best_score = mode_freq
            best_delim = delim
        end
    end

    -- If no candidate delimiter appeared even once, we can't determine
    -- the format (e.g., single-column data or empty file).
    if not best_delim then
        return nil, "sheets.sniff: could not determine a delimiter"
    end

    -- Return a dialect descriptor compatible with csv.parse().
    return { delim = best_delim }
end

-- sheets.has_header(sample, opts) -> boolean
-- Heuristic: treats row 1 as a header if it looks "different" from the
-- rows below it -- specifically, if row 1 has no fields that parse as
-- pure numbers while later rows do. This is intentionally simple; refine
-- it later if you hit real-world false positives.
-- @param sample A string containing CSV data to inspect.
-- @param opts Optional table passed through to csv.sniff().
-- @return true if the first row appears to be a header, false if not,
--         or nil plus an error message if analysis fails.
function M.has_header(sample, opts)
    -- First, detect the delimiter so we can parse the sample correctly.
    local dialect, err = M.sniff(sample, opts)
    if not dialect then return nil, err end

    -- Use the core CSV parser to split the sample into rows.
    -- This ensures we respect quotes, escapes, and the detected delimiter.
    local core = require("sheets.core")
    local rows, perr = core.parse_string(sample, dialect)
    if not rows or #rows < 2 then
        return nil, perr or "sheets.has_header: not enough rows to decide"
    end

    -- Counts how many fields in a row are pure numbers.
    -- Header rows typically contain text labels, while data rows
    -- often contain numeric values. This difference is our signal.
    -- @param row A table of strings representing one CSV row.
    -- @return The number of fields that tonumber() accepts.
    local function row_numeric_count(row)
        local n = 0
        for _, v in ipairs(row) do
            -- tonumber() returns nil for non-numeric strings,
            -- so we use it as a cheap type classifier.
            if tonumber(v) then n = n + 1 end
        end
        return n
    end

    -- Compare the numeric-ness of the first row vs. the second row.
    local header_numeric = row_numeric_count(rows[1])
    local body_numeric = row_numeric_count(rows[2])

    -- If the first row has fewer numeric fields than the second,
    -- it's likely a header row (text labels vs. numeric data).
    return header_numeric < body_numeric
end

-- Expose the module's public API.
return M
