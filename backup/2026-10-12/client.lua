local socket = require("socket")

-- REPLACE THIS with your Debian machine's IP address
local host = "0.0.0.0" 
local port = 12345

print("Attempting to connect to " .. host .. ":" .. port)

-- 1. Create the connection
local tcp = assert(socket.tcp())
tcp:settimeout(5) -- 5 second timeout for the initial connection

local s, err = tcp:connect(host, port)

if not s then
    print("Failed to connect: " .. err)
    os.exit()
end

print("Connected! Type a message and hit Enter:")

-- 2. Send a test message
local msg = io.read()
tcp:send(msg .. "\n")

-- 3. Wait for the server's response
local response, err = tcp:receive()
if not err then
    print("Server says: " .. response)
else
    print("Error receiving: " .. err)
end

tcp:close()
