-- Simple Standalone Lua Server Example
local socket = require("socket") -- Uses LuaSocket library

-- 1. Create the server on port 12345
local server = assert(socket.bind("*", 12345))
local ip, port = server:getsockname()
print("Server started on " .. ip .. ":" .. port)
server:settimeout(0) -- Make it non-blocking

local clients = {}

while true do
    -- 2. Check for new player connections
    local client = server:accept()
    if client then
        client:settimeout(0)
        table.insert(clients, client)
        print("New player connected! Total: " .. #clients)
    end

    -- 3. Loop through connected players to receive data
    for i, p in ipairs(clients) do
        local line, err = p:receive()
        if not err then
            print("Received from player: " .. line)
            -- Echo to everyone else
            for _, other_p in ipairs(clients) do
                other_p:send("Player says: " .. line .. "\n")
            end
        elseif err == "closed" then
            table.remove(clients, i)
            print("Player left.")
        end
    end
    
    socket.sleep(0.01) -- Don't melt the CPU
end
