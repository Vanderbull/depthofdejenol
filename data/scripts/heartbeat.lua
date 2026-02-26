-- This runs every 10 seconds
local time = os.date("%X")
local version = "v0.4.2"

-- Call the C++ function we registered
SetTitle("Blacklands " .. version .. " | System Time: " .. time)

print("Title updated by Lua at " .. time)
