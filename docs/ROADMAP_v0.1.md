# Roadmap v0.1 — Security and Latent Breakage

**Status:** ✅ Complete (2026-10-10)
**Version:** v0.1 (pre-release)
**Test suite:** 954 → 983 passed, 0 failed

## Scope

Small, independent fixes for real defects that were already in the tree. No new features.
Each fix addresses a specific bug or security hole that was identified during the v1.1 audit.

## Slices

### 0.1 Remove the Lua RCE — ✅

**Why:** `onServerDataReceived()` passed raw TCP bytes to `luaL_dostring()`. Any process that
could reach port 12345 got arbitrary Lua, and Lua has `os.execute`. The bundled server binds
`QHostAddress::Any`, so that was the whole LAN.

**Done:**
- `sandboxLua(lua_State*)` — called once after `luaL_openlibs()`. Removes `io`, `package`,
  `debug`, `dofile`, `loadfile`, `require`, `load`, and every `os.*` that can execute, spawn,
  read or write. Keeps `os.date`/`os.time` (used by `heartbeat.lua`).
- `onServerDataReceived()` now parses JSON and dispatches a fixed message set (`chat`,
  `player_join`, `player_leave`), re-emitting text via a new `systemMessage` signal.
- Nothing from the socket is ever executed. Unparseable input is logged and dropped.

**Verified:** `make check` section [64] — 11 checks. The test drives the engine's own Lua
state (non-vacuous: 968/6 without the fix).

**Found while doing this:** `src/network_manager/NetworkManager.cpp` is a complete, correct
client for exactly this protocol — and nothing uses it. Recorded as slice 3.4 in v1.1.1.

---

### 0.2 Remove the undefined `readyBodyForResurrection()` — ✅

**Why:** Declared at `gameStateManager.h:235`, never defined. The first caller would have been
a linker error, and the natural first caller is a death-recovery slice.

**Done:** Deleted the declaration. It had no callers and no body storage in `gameStateManager`
to implement it against — `DeathRecovery` owns bodies as `BodyLocation`.

**Verified:** `grep -rn readyBodyForResurrection` finds nothing outside `trash/`.

---

### 0.3 Collapse the duplicate teleporter tile set — ✅

**Why:** `DungeonDialog` held both `m_teleportPositions` and `m_teleporterPositions`. The
minimap iterated the first; the generator and the wireframe renderer used the second. The
first was therefore always empty, so **teleporters never appeared on the automap**.

**Done:** Deleted `m_teleportPositions`; `DungeonMinimap.cpp` now iterates
`m_teleporterPositions`, the set `generateSpecialTiles()` fills.

**Verified:** `grep` shows exactly one teleporter set, written and read by the same code path.

---

### 0.4 Remove the wrong `PartyHP` — ✅

**Why:** `m_gameStateData["PartyHP"] = QVariantList({50, 40, 30})` at two sites — three values
for a four-member party, and none of them the HP the members actually had.

**Done:** Deleted both assignments. Nothing read the key; HP lives per-character in `Party`.

**Verified:** `grep -rn PartyHP` returns only a stale comment in `GeneralStore.cpp`.

---

### 0.5 Purge the duplicate aging path — ✅

**Why:** `processAgingConsequences()` existed twice: a live version with a hardcoded
`age > 70` and a flat 10% chance, and a commented-out `m_PC`-era version. Meanwhile
`AgingRules` — race-aware thresholds, death at the race's max age — had been written, tested
(section [28]) and never called.

**Done:** Deleted the commented copy; the live one delegates to
`AgingRules::applyYearOfAging()` per living member.

**Verified:** `make check` section [65] — 6 checks. Reverting to the old hardcoded rule
produces exactly 3 failures (Human death at max age, 0 HP on that death, Elf decay at 100).

---

### 0.6 Fix the broken version string — ✅

**Why:** `version.h.in` put `$$VERSION_INT` and `$$VERSION_HASH` on adjacent lines, so C++
concatenated them into `"v6421ed6b1a"`. `UpdateManager::parseVersionNumber()` does `toInt()`
on that, gets `-1`, and silently disables the update check. `gameStateManager` also added a
second `"v"`.

**Done:** `version.h.in` emits `FULL_VERSION` alone; the hash stays in `GIT_HASH`. Removed the
extra `"v"` prefix in `gameStateManager.cpp`.

**Verified:** `make check` section [65] — 3 checks (non-empty, parses as int, at most one `v`).
Generated `version.h` reads `FULL_VERSION = "v642"`.

---

## Summary

| Slice | What | Impact |
|---|---|---|
| 0.1 | Lua RCE closed | Security — arbitrary code execution over LAN |
| 0.2 | Undefined function removed | Prevents future linker error |
| 0.3 | Teleporters visible on automap | Player-facing bug fix |
| 0.4 | Wrong PartyHP removed | Data integrity |
| 0.5 | Aging consolidated | Race-aware aging now live |
| 0.6 | Version string fixed | Update check works again |

**Net test delta:** +29 checks (954 → 983), 0 failures.

## Also included in v0.1

The following work was completed before the version scheme was formalized and is included in
the v0.1 commit:

- **Dungeon UI:** layout fills screen (viewport + 320px sidebar), trap-disarm button,
  item-identification (10 GP), monster wander/chase AI, opened-chest persistence,
  trap detection, alignment display
- **Combat:** front/back row, defend action, monster flee, useItem from combat,
  status flags (Poisoned/Blinded/OnFire/Snared), death handling
- **Character:** hunger system, starvation damage, food consumption
- **Town:** TempleDialog (heal/cure/resurrect via GoldSinks), QuestChainDialog
  (6-step main quest), QuestBoard turn-in/reportKill
- **Doors:** DoorState map (locked/keyed/secret), collision, locked chests
- **Traps:** 5 unique effects, detection (DC 8+floor), disarming via thieving
- **Options:** 9 QSettings checkboxes, SFX volume slider
- **Automap:** integrated into dungeon sidebar, cursor tracking
- **Network:** NetworkManager (unused — recorded as slice 3.4 in v1.1.1)
- **Removed:** entire CompanionRegistry/ (half-built, per user request)
- **Documentation:** RELEASE_NOTES.md, docs/VERSIONING.md, per-version roadmaps

## Commit

```
35cc244 Phase 0 (v0.1): security hardening + backlog of completed work
```
