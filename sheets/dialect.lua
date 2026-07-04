-- sheets.dialect - Named presets for CSV formatting rules.
--
-- What's a "dialect"? It's just a nickname for a set of CSV options
-- (like "use commas" or "use tabs"). Instead of typing out all the
-- settings every time, you can say "use the Excel style" and the
-- library fills in the details for you.
--
-- How it works: when you read or write a CSV, you pass in options.
-- If you say `dialect = "excel"`, the library looks up that preset,
-- copies its defaults, then applies any extra options you gave.
-- Your explicit options always win over the preset defaults.

-- The module table that will be returned at the end.
local M = {}

-- Internal "phonebook" that maps nicknames (like "excel") to their
-- actual option tables. Users can add their own entries here too.
-- This is kept private so the registry can only be modified through
-- the public API (register_dialect, unregister_dialect).
local registry = {}

-- Three built-in presets that match common real-world formats:
--   "excel"      = what Microsoft Excel exports (commas, double quotes)
--   "excel_tab"  = tab-separated files, like .tsv (tabs, double quotes)
--   "unix"       = clean Unix-style CSV (commas, double quotes, \n endings)
-- These are exposed on M so users can inspect or clone them if needed.
M.excel = { delim = ",", quote = '"' }
M.excel_tab = { delim = "\t", quote = '"' }
M.unix_dialect = { delim = ",", quote = '"', line_terminator = "\n" }

-- Helper to add a preset to our internal phonebook.
-- This is a thin wrapper used during module initialization to register
-- the built-in presets. It simply stores the dialect table under the given name.
-- @param name The string nickname for this preset.
-- @param dialect A table of CSV formatting options.
local function register(name, dialect)
    registry[name] = dialect
end

-- Register the three built-in presets so users can refer to them by name
-- when creating readers or writers (e.g., opts = {dialect = "excel"}).
register("excel", M.excel)
register("excel_tab", M.excel_tab)
register("unix", M.unix_dialect)

--- Add your own custom preset so you can reuse it by name later.
-- Example: M.register_dialect("pipe", {delim = "|", quote = "'"})
-- Then later: opts = {dialect = "pipe"}
-- @param name A string nickname for the new preset.
-- @param dialect A table of CSV formatting options.
function M.register_dialect(name, dialect)
    -- Validate inputs early to prevent confusing errors later.
    if type(name) ~= "string" then
        error("dialect name must be a string", 2)
    end
    if type(dialect) ~= "table" then
        error("dialect must be a table of options", 2)
    end
    -- Store the preset in the private registry so resolve() can find it.
    registry[name] = dialect
end

-- Look up a preset by its nickname. Returns the option table, or nil.
-- This is useful if you want to inspect a preset before using it,
-- or merge it manually with other options.
-- @param name The string nickname of the preset to look up.
-- @return The dialect table if found, nil otherwise.
function M.get_dialect(name)
    return registry[name]
end

-- Remove a preset from the phonebook if you no longer need it.
-- This only affects the registry; any existing readers/writers that
-- already resolved the dialect are unaffected.
-- @param name The string nickname of the preset to remove.
function M.unregister_dialect(name)
    registry[name] = nil
end

-- Get a sorted list of all available preset nicknames.
-- Useful for UI pickers, debugging, or validating user input.
-- @return An array table of sorted string names.
function M.list_dialects()
    local names = {}
    -- Collect all keys from the registry into a flat array.
    for k in pairs(registry) do names[#names + 1] = k end
    -- Sort alphabetically so the output is deterministic and readable.
    table.sort(names)
    return names
end

--- Build the final options table by merging a preset with your overrides.
--
-- Priority (highest to lowest):
--   1. Options you pass in explicitly (opts)
--   2. Defaults from the named preset (opts.dialect)
--   3. Hardcoded library defaults (handled elsewhere)
--
-- `dialect` can be a string (preset nickname) or a table (inline rules).
-- The "dialect" key itself is stripped out — it's only used for lookup.
-- @param opts A table of user-provided options, potentially containing `dialect`.
-- @return A new table with the merged, final set of CSV formatting options.
function M.resolve(opts)
    -- Normalize opts to an empty table if the caller passed nothing.
    opts = opts or {}
    -- Extract the dialect reference before merging. This could be a string
    -- nickname like "excel" or an inline table of options.
    local dialect = opts.dialect
    -- Start with an empty base table; we'll layer defaults and overrides onto it.
    local base = {}

    -- If a dialect was requested, copy its defaults into our result first.
    if dialect then
        -- If it's a nickname, look it up in the registry.
        if type(dialect) == "string" then
            local d = registry[dialect]
            -- Fail fast if the user referenced a non-existent preset.
            if not d then
                error("unknown dialect '" .. dialect .. "'", 2)
            end
            -- Replace the string name with the actual table for copying.
            dialect = d
        end
        -- Shallow-copy every setting from the preset into our base table.
        -- This ensures the preset table itself is never mutated.
        for k, v in pairs(dialect) do base[k] = v end
    end

    -- Now copy your explicit options on top, overriding the preset.
    -- We skip the "dialect" key because it's a meta-option used only for
    -- lookup; it is not a real CSV formatting parameter like delim or quote.
    for k, v in pairs(opts) do
        if k ~= "dialect" then base[k] = v end
    end

    -- Return the fully resolved options. Callers (like Reader.new or
    -- Writer.new) pass this directly to the core parser.
    return base
end

-- Expose the dialect module's public API.
return M
