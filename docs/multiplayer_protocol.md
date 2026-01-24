# Multiplayer Protocol (reference)

This document describes the minimal JSON-based protocol used between client and server.

- Transport: TCP (newline-delimited JSON messages)
- Protocol field: all messages include `protocol: <int>` (current: 1)
- Server-assigned sequence: server includes `seq` on important messages (e.g., `zone_sync`) to allow ordering

Common client -> server messages:
- ping
  {
    "type": "ping",
    "protocol": 1,
    "ts": "2026-01-24T12:00:00Z"
  }
- identify
  { "type": "identify", "protocol": 1, "username": "Alice" }
- chat
  { "type": "chat", "protocol": 1, "message": "Hello" }
- action
  { "type": "action", "protocol": 1, "action": "move", "data": { ... } }

Common server -> client messages:
- pong
  { "type": "pong", "protocol": 1, "ts": "2026-01-24T12:00:00Z" }
- player_join / player_leave
  { "type": "player_join", "protocol": 1, "username": "Bob" }
- chat
  { "type": "chat", "protocol": 1, "sender": "Bob", "message": "Hi" }
- zone_sync  (server includes `seq` on these)
  { "type": "zone_sync", "protocol": 1, "seq": 42, "data": { ... } }

Notes and recommendations:
- Clients must ignore messages with mismatched protocol versions (or request negotiation).
- Use sequence numbers (`seq`) to apply zone_syncs in order. Missing sequences should trigger a resync request.
- Messages are newline-delimited JSON for readability and debugging; consider migrating to length-prefixed framing for embedded newlines/binaries in future.
- Sanitize chat and other user-supplied strings on both client and server sides.
