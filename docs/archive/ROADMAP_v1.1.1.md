# Roadmap v1.1.1 — Debt and Hygiene

**Status:** Not started
**Version:** v1.1.1 (patch — polish and fixes)
**Test suite at start:** TBD (after v1.1.0)

## Goal

Clean up technical debt. No new features. This is the polish release that makes the codebase
maintainable for future work.

## Slices

### 3.1 `initializeParty()` dead code and emit spam `S`

FIXLIST #4. Remove the commented lines and hoist the `emit` out of the loop (4 signals → 1).

**Files:** `gameStateManager.cpp`.

**Verify:** one signal per init; suite green.

---

### 3.2 Modern signal/slot `M`

FIXLIST #10. `setupControls()` builds buttons from a `QMap<QString, const char*>` of
`SLOT(...)` strings — a typo there fails silently at runtime. Convert to member pointers or
lambdas.

**Files:** `DungeonDialog.cpp`, `blacklands.cpp`.

**Verify:** build clean; every button still fires.

---

### 3.3 Purge commented-out legacy bodies `M`

FIXLIST #21. `gameStateManager.cpp` carries hundreds of commented `m_PC`-era lines. Delete
them.

**Files:** `gameStateManager.cpp`.

**Verify:** identical behaviour; suite green.

---

### 3.4 Collapse the two network clients `M`

**Why:** found during v0.1. `src/network_manager/NetworkManager.cpp` implements the client
correctly (JSON parse, typed dispatch, `chatReceived`/`playerJoined`/`playerLeft` signals)
and **nothing uses it**. `gameStateManager` opens its own `QTcpSocket` to the same host and
port, which is how the Lua-execution path got in. Two clients, one of them dead.

**Do:** wire the game to `NetworkManager`, delete `gameStateManager`'s socket and
`onServerDataReceived()`, and connect `systemMessage`/`chatReceived` to the chat UI.

**Files:** `gameStateManager.h/.cpp`, `src/network_manager/*`.

**Verify:** one socket in the process; chat round-trips through `NetworkManager`.

---

## Release criteria

v1.1.1 is ready when:

- [ ] All 4 slices are complete
- [ ] No commented-out legacy code remains in `gameStateManager.cpp`
- [ ] All signal/slot connections use modern syntax
- [ ] One network client (NetworkManager)
- [ ] All tests pass
- [ ] `RELEASE_NOTES.md` is updated
