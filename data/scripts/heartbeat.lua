-- This runs every 10 seconds
local time = os.date("%X")
-- GameReleaseVersion is pushed from C++ (the semantic version, e.g. "2.0.0").
local version = "v" .. (GameReleaseVersion or "unknown")

-- Call the C++ function we registered
SetTitle("Blacklands " .. version .. " | System Time: " .. time)

print("Title updated by Lua at " .. time)
