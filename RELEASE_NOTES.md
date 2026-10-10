# Release Notes

Every change to this project is recorded here, newest release first.
Format: what changed, why it mattered, and how it was verified.

## Version scheme (agreed 2026-10-10)

Semantic versioning with game-studio conventions:

- **Major (X.0.0)** — first version that can be played start to finish, or a fundamental rework of the game's character.
- **Minor (X.Y.0)** — new features, mechanics, areas; backwards-compatible.
- **Patch (X.Y.Z)** — bug fixes, balance, text, polish; no new features.

| Phase | Version | Level | Content |
|---|---|---|---|
| Phase 0 | **v0.1** | Pre-release | Security and latent breakage |
| Phase 1 | **v1.0.0** | Major | Connect the orphaned systems — makes the game finishable |
| Phase 2 | **v1.1.0** | Minor | Combat reachability and depth |
| Phase 3 | **v1.1.1** | Patch | Debt and hygiene |

One commit per version boundary. Each phase lands as a release, not as a batch of
mid-development commits.

---

# v0.1 — 2026-10-10 ✅ RELEASED

**Phase 0 — security and latent breakage.** Small, independent fixes for real defects that
were already in the tree. Scope: `docs/ROADMAP_v0.1.md`.

**Test suite before:** 954 passed, 0 failed.
**Test suite after:** 983 passed, 0 failed.
**Commits:** `35cc244` (Phase 0), `051e478` (versioning + docs)

---

## Phase 0 — Security and latent breakage

### 0.1 Remove the Lua RCE — ✅

**Why.** `onServerDataReceived()` passed raw TCP bytes straight into
`luaL_dostring()`. Anything that could reach port 12345 got arbitrary Lua in the client,
and Lua has `os.execute`. The bundled server binds `QHostAddress::Any`, so that was the
whole LAN, not just localhost. This was the oldest open item on `docs/FIXLIST.md` (#1).

**Changed.**
- Added `gameStateManager::sandboxLua(lua_State*)`, called once after `luaL_openlibs()`.
  It removes `io`, `package`, `debug`, `dofile`, `loadfile`, `require`, `load` and every
  `os.*` that can execute, spawn, read or write (`execute`, `exit`, `remove`, `rename`,
  `tmpname`, `getenv`, `setlocale`). It keeps `os.date`/`os.time`, which the shipped
  `data/scripts/heartbeat.lua` uses.
- `onServerDataReceived()` now parses JSON and dispatches a fixed message set (`chat`,
  `player_join`, `player_leave`), re-emitting text through a new `systemMessage` signal.
  Nothing arriving over the socket is ever executed; unparseable input is logged and dropped.

Both halves matter: deleting the `luaL_dostring` call alone would have left the sandbox open
for the next mistake.

**Verified.** `make check` section [64], 11 checks.

The first version of this test was **vacuous** — it built its own Lua state and called
`sandboxLua()` directly, so it passed even with the call commented out of the constructor.
It was rewritten to drive the engine's own state through `loadLuaScript()` + `getLuaString()`.
Commenting out `sandboxLua(m_L)` now produces exactly the 6 expected failures (`os.execute`,
`io`, `package`, `debug`, `dofile`, `require` all reachable again) → 968 passed, 6 failed.
With the fix in place: 974 passed, 0 failed. `grep luaL_dostring` finds only the explanatory
comment.

**Found while doing this:** `src/network_manager/NetworkManager.cpp` is a complete, correct
client for exactly this protocol (JSON parse, typed dispatch, `chatReceived`/`playerJoined`
signals) — and nothing uses it. `gameStateManager` opens its own socket instead. Recorded as
slice 3.4.

---

### 0.2 Remove the undefined `readyBodyForResurrection()` — ✅

**Why.** Declared at `gameStateManager.h:235`, never defined. The first caller would have
been a linker error, and the natural first caller is a death-recovery slice.

**Changed.** Deleted the declaration. It had no callers anywhere in the tree and there is no
body storage in `gameStateManager` to implement it against — `DeathRecovery` owns bodies as
`BodyLocation` values. Implementing it would have meant inventing a lookup that nothing needs.

**Verified.** `grep -rn readyBodyForResurrection` finds nothing outside `trash/` (a dead
directory not in the build). Suite green.

---

### 0.3 Collapse the duplicate teleporter tile set — ✅

**Why.** `DungeonDialog` held both `m_teleportPositions` and `m_teleporterPositions`. The
minimap iterated the first; the generator and the wireframe renderer used the second. The
first was therefore always empty, so **teleporters never appeared on the automap** — a live,
player-visible bug. This was `docs/FIXLIST.md` #15, never fixed.

**Changed.** Deleted `m_teleportPositions`; `DungeonMinimap.cpp` now iterates
`m_teleporterPositions`, the set `generateSpecialTiles()` fills.

**Verified.** `grep` shows exactly one teleporter set, written and read by the same code path.

---

### 0.4 Remove the wrong `PartyHP` — ✅

**Why.** `m_gameStateData["PartyHP"] = QVariantList({50, 40, 30})` at two sites — three
values for a four-member party, and none of them the HP the members actually had. Anything
reading it got a short, wrong list. `docs/FIXLIST.md` #16.

**Changed.** Deleted both assignments. Nothing read the key (the only other mention is a
comment in `GeneralStore.cpp`); HP lives per-character in the `Party` map, which both sites
already write.

**Verified.** `grep -rn PartyHP` returns only the stale comment. Suite green.

---

### 0.5 Purge the duplicate aging path — ✅

**Why.** `processAgingConsequences()` existed **twice**: a live version with a hardcoded
`age > 70` and a flat 10% chance, and a commented-out `m_PC`-era version. Meanwhile
`AgingRules` — race-aware thresholds, death at the race's max age — had been written, tested
(section [28]) and never called. There were two aging rules in the codebase and the tested
one was dead.

**Changed.** Deleted the commented copy. The live one now delegates to
`AgingRules::applyYearOfAging()` per living member, logging the events it returns.

**Verified.** `make check` section [65], 6 checks. Reverting to the old hardcoded rule
produces exactly 3 failures (`a Human at max age (100) dies of old age`, `death by old age
leaves 0 HP`, `an Elf at 100 suffers no decay`) → 980 passed, 3 failed. With the fix:
983 passed, 0 failed.

The race-awareness check is the one that pins the real defect: an Elf at 100 is 180 years
short of decay, but the old rule decayed **every** race past 70.

---

### 0.6 Fix the broken version string — ✅

**Why.** `version.h` was generated from `version.h.in`, which declared:

```
static const char* const FULL_VERSION = $$VERSION_INT
$$VERSION_HASH;
```

`$$VERSION_INT` expands to `"v642"` and `$$VERSION_HASH` to `"1ed6b1a"` — adjacent string
literals, which C++ concatenates. `FULL_VERSION` was `"v6421ed6b1a"`. `UpdateManager`'s
`parseVersionNumber()` does `toInt()` on it, gets `-1`, and silently disables the update
check. Separately, `gameStateManager` prefixed it again (`QString("v%1")`), producing
`"vv642…"` for Lua.

**Changed.** `version.h.in` now emits `FULL_VERSION` alone (the hash stays in `GIT_HASH`,
where it belongs). Removed the extra `"v"` prefix in `gameStateManager.cpp`.

**Verified.** `make check` section [65], 3 checks: the string is non-empty, parses as an
integer after stripping an optional `v`, and carries at most one `v`. Generated
`version.h` now reads `static const char* const FULL_VERSION = "v642";`.

---

## Summary of Phase 0

| Slice | What | Verified by |
|---|---|---|
| 0.1 | Lua RCE closed (sandbox + JSON dispatch) | [64] 11 checks; non-vacuous (968/6 without the fix) |
| 0.2 | Undefined `readyBodyForResurrection()` removed | grep; suite green |
| 0.3 | Teleporters now visible on the automap | single source, written and read together |
| 0.4 | Wrong `PartyHP` removed | grep; nothing read it |
| 0.5 | Aging consolidated into the tested `AgingRules` | [65] 6 checks; non-vacuous (980/3 without the fix) |
| 0.6 | Version string parses again | [65] 3 checks |

**Net test delta:** +29 checks (954 → 983), 0 failures.

---

# v0.0 — 2026-10-07 (game systems)

The release that built the game's systems: eight phases covering items, equipment, combat,
progression, dungeon depth, death, the endgame, the town, and balance.

Version history in `src/core/ReleaseInfo.cpp`.

**Known at release** — and what the next release addresses: large parts of Phases 3–8
shipped as tested logic that nothing calls. The audit in `docs/ROADMAP_V1.1.md` found six
such systems (`Endgame`, `MonsterBalance`, `GoldSinks`, `AgingRules`, `ItemProgression`,
`SpellMechanics`), each with zero callers outside its own translation unit. Phase 0 and
Phase 1 of the current roadmap exist to close that gap.

---

## Earlier development

Pre-1.0 work is in the git history and in `docs/`:

- `docs/ROADMAP_V1.md` — the eight-phase plan to v1.0, with per-slice *Why* / *Done* / *Verify*.
- `docs/FIXLIST.md` — 21 defects found by audit, with locations and fixes.
- `docs/GAME_COMPLETENESS_ANALYSIS.md` — what the game has and what it was missing.
- `docs/COMPLETE_REFACTOR_REPORT.md` — the refactor that preceded the roadmap.

Notable commits:

| Commit | What |
|---|---|
| `1ed6b1a` | fix: add missing QObject includes to 42 headers |
| `b28c1a2` | docs: rewrite README as a game presentation and build guide |
| `186a3b2` | feat: complete the bestiary, fix the character-sheet crash |
| `5f1bfac` | fix: ship resources/ to build/bin and commit the city icons |
| `1b780d6` | fix: repair all 7 skipped tests and the status-flag collision |
| `3543ead` | Phase 8: balance and release |
| `04ba939` | Phase 7: town and quality of life |
| `0dc6a83` | Phase 5-6: death and consequences, win condition and endgame |
| `35cc244` | v0.1: security and latent breakage (Phase 0) |
