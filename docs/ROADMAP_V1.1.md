# BlackLands — Roadmap (versioned)

> **Version scheme (2026-10-10):** each phase is a release, committed at its boundary.
>
> | Phase | Version | Content |
> |---|---|---|
> | Phase 0 | **v1.0.0** | Security and latent breakage (✅ done) |
> | Phase 1 | **v1.0.1** | Connect the orphaned systems |
> | Phase 2 | **v1.0.2** | Combat reachability and depth |
> | Phase 3 | **v1.0.3** | Debt and hygiene |
>
> This file keeps its original filename (`ROADMAP_V1.1.md`) because the git history and
> release notes reference it; the top of this file is the current plan.

The v1.0 release built the systems. v1.0.1 **connects them to the game**. The audit below
found that large parts of Phases 3–8 of that build exist as pure, tested logic that
**nothing calls** — the tests pass because they test the classes directly, not because the
player can reach them.

**Size key:** `S` = ~1–3 h · `M` = ~1 day · `L` = ~2–3 days

**Rule for every slice:** build clean (`make -j$(nproc)`, zero new warnings) → run the
verification step → only then move on. If a slice can't be verified, it isn't done.

**Rule for every slice:** a slice is not done while its class is only reachable from tests.
See the "orphaned system" check at the end of this document.

---

## The audit: what is built but unreachable

Verified by grepping for callers outside each class's own `.cpp`:

| System | Callers outside its own file | Consequence |
|---|---|---|
| `Endgame` (victory, NG+, records) | **0** | You can kill the Prince of Devils and nothing happens |
| `MonsterBalance` (difficulty curve) | **0** | Floors do not scale the way 8.1 designed |
| `GoldSinks` | **0** | No UI ever charges the gold sinks 8.4 defined |
| `AgingRules` (decay, max age) | **0** | Tested aging is dead; a second, older aging path runs instead |
| `ItemProgression` | **0** | Tier curve from 8.3 never applied to loot or shops |
| `SpellMechanics` | **0** | Fire/Cold/Lightning/Mind differentiation from 8.2 never used in combat |
| `HallOfRecords` (records) | reads `GuildLeaders` only | `Endgame::ranked()` records never displayed |
| `initiateFight()`, `onEventTriggered()` | empty stubs | Combat only starts if you stand exactly on the monster tile and press Fight |

Plus the FIXLIST items still open (#1, #4, #10, #12, #15, #16) and one live bug:

- `m_teleportPositions` is **read** by `DungeonMinimap.cpp:220` but **never written** —
  `generateSpecialTiles()` fills `m_teleporterPositions` instead. Teleporters never appear
  on the minimap. (FIXLIST #15, unfixed.)
- `readyBodyForResurrection()` is declared at `gameStateManager.h:231` with no definition
  (FIXLIST #12) — a linker error waiting for its first caller.
- `luaL_dostring(m_L, data.constData())` on raw socket bytes at `gameStateManager.cpp:1737`
  is still a full RCE (FIXLIST #1).
- `processAgingConsequences()` has **two** definitions — one live at line 1094, one
  commented at 1118 — and neither calls `AgingRules`.
- `version.h` builds `FULL_VERSION` by literal concatenation:
  `"v642"` newline `"1ed6b1a"` → `"v6421ed6b1a"`.

---

## Dependency graph

```
0.1 Lua RCE ── (independent, do first — security)
0.2 readyBodyForResurrection ── 5.x body recovery
0.3 teleporter tile set ── (independent, one-line class of bug)

1.1 Victory sequence ──┬─ 1.2 Hall of Records records
                       └─ 1.3 New Game Plus entry
1.4 MonsterBalance into encounters ── 1.5 ItemProgression into loot/shops
1.6 AgingRules consolidation (kill the duplicate)
1.7 GoldSinks into the town dialogs

2.1 Auto-start combat on encounter ── 2.2 front/back row ── 2.3 monster spells
2.4 SpellMechanics into combat ── (needs 2.3 to have a caster path)
```

**Critical path to "the game can be finished":** `1.1 → 1.2`. Everything else is depth.

---

## Phase 0 — Security and latent breakage

Small, independent, and each one is a real defect today.

### 0.1 Remove the Lua RCE `S` — ✅ DONE
**Why:** `onServerDataReceived()` passed raw TCP bytes to `luaL_dostring`. Any process that
could reach `127.0.0.1:12345` got arbitrary Lua, and Lua has `os.execute`. The bundled
server (`src/server/main.cpp:98`) binds `QHostAddress::Any`, so that was the whole LAN, not
just localhost.

**Done:** two changes, because removing the `luaL_dostring` call alone would leave the
sandbox open for the next mistake:
- `sandboxLua(lua_State*)` — called once after `luaL_openlibs()`. Removes `io`, `package`,
  `debug`, `dofile`, `loadfile`, `require`, `load`, and every `os.*` that can execute,
  spawn, read or write (`execute`, `exit`, `remove`, `rename`, `tmpname`, `getenv`,
  `setlocale`). Keeps `os.date`/`os.time`, which `data/scripts/heartbeat.lua` uses.
- `onServerDataReceived()` now parses JSON and dispatches a fixed message set (`chat`,
  `player_join`, `player_leave`), re-emitting text via a new `systemMessage` signal.
  Nothing from the socket is ever executed. Unparseable input is logged and dropped.

**Verify:** `make check` section [64] — 11 checks. The section drives the **engine's own
Lua state** through `loadLuaScript()` + `getLuaString()` rather than building a state of its
own: calling `sandboxLua()` directly passed even with the call commented out of the
constructor, which made the first version of this test vacuous. With the test corrected,
commenting out `sandboxLua(m_L)` produces exactly the 6 expected failures (`os.execute`,
`io`, `package`, `debug`, `dofile`, `require` all reachable again) and 968/6. 974 passed,
0 failed with the fix in place. `grep luaL_dostring` finds only the explanatory comment.

**Found while doing this:** `src/network_manager/NetworkManager.cpp` is a complete, correct
implementation of exactly this (JSON parse, typed dispatch, `chatReceived`/`playerJoined`
signals) — and **nothing uses it**. `gameStateManager` opens its own socket instead. Wiring
the client to `NetworkManager` and deleting the duplicate socket is a follow-up; see 3.4.

---

### 0.2 Implement or delete `readyBodyForResurrection()` `S` — ✅ DONE
**Why:** declared, never defined. The first caller is a linker error, which will surface at
the worst moment — during a death-recovery slice.

**Done:** deleted the declaration. It had no callers and no body storage in `gameStateManager`
to implement it against — `DeathRecovery` owns bodies as `BodyLocation`. Implementing it would
have meant inventing a lookup nothing needs.

**Verify:** `grep -rn readyBodyForResurrection` finds nothing outside `trash/` (dead
directory, not in the build).

---

### 0.3 Collapse the duplicate teleporter tile set `S` — ✅ DONE
**Why:** `m_teleportPositions` was read by the minimap and never written, so teleporters were
invisible on the map. The writer targeted `m_teleporterPositions`.

**Done:** kept `m_teleporterPositions` (filled by `generateSpecialTiles()`, read by the
wireframe renderer), pointed `DungeonMinimap.cpp` at it, deleted `m_teleportPositions`.

**Verify:** `grep` shows one teleporter set, written and read by the same path.

---

### 0.4 Fix `PartyHP` initialization `S` — ✅ DONE
**Why:** `QVariantList({50, 40, 30})` — three values for a four-member party, matching no
member's real HP. (FIXLIST #16.)

**Done:** deleted both assignments. Nothing read the key; HP lives per-character in `Party`.

**Verify:** `grep -rn PartyHP` returns only a stale comment in `GeneralStore.cpp`.

---

### 0.5 Purge the duplicate aging path `M` — ✅ DONE
**Why:** `processAgingConsequences()` existed twice — one live with a hardcoded `age > 70`,
one commented — and neither used `AgingRules`, which was written, tested (section [28]) and
never called.

**Done:** deleted the commented copy; the live one delegates to
`AgingRules::applyYearOfAging()` per living member.

**Verify:** `make check` section [65] — 6 checks. Reverting to the old hardcoded rule gives
exactly 3 failures (Human death at max age, 0 HP on that death, Elf decay at 100) → 980/3.

---

### 0.6 Fix `version.h` `S` — ✅ DONE
**Why:** `version.h.in` put `$$VERSION_INT` and `$$VERSION_HASH` on adjacent lines, so C++
concatenated them into `"v6421ed6b1a"`. `UpdateManager::parseVersionNumber()` does `toInt()`
on that, gets `-1`, and silently disables the update check. `gameStateManager` also added a
second `"v"`.

**Done:** `version.h.in` emits `FULL_VERSION` alone; the hash stays in `GIT_HASH`. Removed the
extra prefix in `gameStateManager.cpp`.

**Verify:** `make check` section [65] — 3 checks (non-empty, parses as int, at most one `v`).
Generated `version.h` reads `FULL_VERSION = "v642"`.

---

## Phase 1 (v1.0.1) — Connect the orphaned systems

The heart of this release. Each slice wires one tested-but-unreachable class into the game.
Landing all seven is what makes the game completable, not just assembled.

### 1.1 Victory sequence `M`
**Why:** `Endgame::victoryTitle()`, `victoryParagraphs()` and `isVictory()` have **zero
callers**. The final boss exists, the quest chain completes, and the player gets nothing —
no screen, no acknowledgement, no end. This is the single biggest gap between "systems
built" and "game finishable".

**Do:** after `handleVictory()` kills the floor-15 boss, check `Endgame::isVictory()`.
On victory: show the title and paragraphs (a `VictoryDialog` mirroring the intro
`StoryDialog`), record the run via the `GameRecord` path, and offer New Game Plus.

**Files:** `src/dungeon_dialog/DungeonDialog.cpp` (`handleVictory`), new
`src/victory_dialog/`, `blacklands.pro`.

**Verify:** suite check that killing the Prince sets a victory flag and that the victory
text is non-empty; a manual run to floor 15 or a forced-boss test path.

---

### 1.2 Hall of Records reads real records `M`
**Why:** the dialog exists and is reachable from the main menu, but it reads
`GuildLeaders` — it never touches `Endgame::ranked()` / `GameRecord`, which 6.4 built and
tested. The Hall shows guild leaders, not records.

**Do:** have the dialog display the four ranked categories from `Endgame::ranked()` (highest
level, most gold, deepest floor, fastest completion), fed from a persisted record list.
Keep the guild-leaders section or split the dialog into two tabs.

**Files:** `src/hall_of_records/hallofrecordsdialog.cpp`, `src/core/Endgame.*`.

**Verify:** suite check that a saved `GameRecord` appears in the dialog's ranked output.

---

### 1.3 New Game Plus entry `S`
**Why:** `ngPlusMonsterMultiplier()` / `ngPlusRewardMultiplier()` / `ngPlusBanner()` have no
caller. NG+ is designed and unreachable.

**Do:** offer NG+ on the victory screen; store the cycle count in game state; apply the two
multipliers in `EncounterBuilder` and `VictoryReward`.

**Files:** victory dialog, `EncounterBuilder.cpp`, `VictoryReward.cpp`, `gameStateManager`.

**Verify:** starting NG+1 raises monster HP and reward gold by the documented multipliers.

---

### 1.4 MonsterBalance into the encounter path `M`
**Why:** 8.1 built a per-floor difficulty curve and a readiness check; nothing applies it.
Encounters use raw MDATA5 stats, so the curve the roadmap describes is not in the game.

**Do:** route `EncounterBuilder` (or `DungeonDialog` when spawning) through
`MonsterBalance`'s scaled stats for the current floor. Optionally warn on entry when the
party is under-levelled for the floor.

**Files:** `src/combat/EncounterBuilder.cpp` or `DungeonDialog.cpp`, `src/core/MonsterBalance.*`.

**Verify:** a floor-10 encounter has higher HP/att than the same monster on floor 1, by the
documented multiplier.

---

### 1.5 ItemProgression into loot and shops `M`
**Why:** 8.3 built tiers (Bronze→Mithril) and floor→tier mapping; nothing uses it, so loot
tiering is whatever `VictoryReward`'s ad-hoc filter does.

**Do:** use `ItemProgression::tierForFloor()` to filter/augment drops and to gate store
stock.

**Files:** `src/combat/VictoryReward.cpp`, `src/general_store/*`, `src/items/ItemProgression.*`.

**Verify:** floor-1 loot is Bronze-tier; floor-15 loot can be Mithril.

---

### 1.6 GoldSinks into the town dialogs `M`
**Why:** 8.4 defined seven sinks with costs; no dialog charges them. Prices are scattered as
literals across Tavern/Morgue/Store instead of one authority.

**Do:** route every gold charge (resurrect, rescue, identify, uncurse, guild level, rest,
cure) through `GoldSinks` so the cost has one definition.

**Files:** `TavernDialog`, `MorgueDialog`, `GeneralStore`, `GuildsDialog`,
`src/core/GoldSinks.*`.

**Verify:** suite check that each dialog's charge equals the `GoldSinks` value.

---

### 1.7 SpellMechanics into combat `M`
**Why:** 8.2 built fire-AoE / cold-slow / lightning-chain / mind-CC and none of it is used;
combat casting does not differentiate schools.

**Do:** route `CombatActions::castSpellAdvanced()` through `SpellMechanics` for the school's
mechanic.

**Files:** `src/combat/CombatActions.cpp`, `src/spell_casting/SpellMechanics.*`.

**Verify:** a fire spell hits multiple targets, a cold spell applies slow, a lightning spell
chains, per the `SpellMechanics` tests.

---

## Phase 2 (v1.0.2) — Combat reachability and depth

### 2.1 Start combat from the encounter, not the button `M`
**Why:** stepping onto a monster only logs a line (`handleEncounters`). Combat begins only
if the player then presses Fight while standing on the tile. `initiateFight()` is an empty
stub. The encounter is announced and then ignored.

**Do:** have `handleEncounters` (or `movePlayer`) start combat when a hostile monster shares
the tile, or prompt. Fill in or delete `initiateFight()`.

**Files:** `src/dungeon_dialog/DungeonHandlers.cpp`, `DungeonDialog.cpp`.

**Verify:** walking onto a hostile monster opens the combat UI without pressing Fight.

---

### 2.2 Front and back row `L`
**Why:** the completeness analysis calls for front/back positioning (warriors up front,
casters behind); `MonsterAI::chooseTarget()` already rolls 60% front / 25% weakest, but
there is no row to mean anything.

**Do:** add a row to `CombatParticipant`, let the player set it, make melee reach only the
front and ranged/spells reach both.

**Files:** `src/combat/CombatState.h`, `CombatActions.cpp`, `MonsterAI.cpp`, combat UI.

**Verify:** a back-row member is not targeted by melee; a front-row one is.

---

### 2.3 Monster spellcasting `M`
**Why:** `MonsterAI::decide()` can return `CastSpell`, but monsters have no spell path wired,
so the branch is effectively dead.

**Do:** give spell-capable monsters a spell (from their data or a per-type table) and execute
the cast through `CombatActions`.

**Files:** `src/combat/MonsterAI.cpp`, `EncounterBuilder.cpp`.

**Verify:** a caster monster casts at least once over N rounds and the party takes spell
damage.

---

### 2.4 Flee re-entry `S`
**Why:** fleeing marks monsters dead (`CombatActions::flee`) so combat ends, but the monster
is removed from the map on victory only — fleeing should leave it present or clear it
consistently.

**Do:** decide the flee semantics (monster stays, party disengages) and make map state agree
with combat state.

**Files:** `src/combat/CombatActions.cpp`, `DungeonDialog.cpp`.

**Verify:** flee → the monster is either still on the tile or gone, never both.

---

## Phase 3 (v1.0.3) — Debt and hygiene

### 3.1 `initializeParty()` dead code and emit spam `S`
FIXLIST #4. Remove the commented lines and hoist the `emit` out of the loop (4 signals → 1).
**Files:** `gameStateManager.cpp`. **Verify:** one signal per init; suite green.

### 3.2 Modern signal/slot `M`
FIXLIST #10. `setupControls()` builds buttons from a `QMap<QString, const char*>` of
`SLOT(...)` strings — a typo there fails silently at runtime. Convert to member pointers or
lambdas. **Files:** `DungeonDialog.cpp`, `blacklands.cpp`. **Verify:** build clean; every
button still fires.

### 3.3 Purge commented-out legacy bodies `M`
FIXLIST #21. `gameStateManager.cpp` carries hundreds of commented `m_PC`-era lines. Delete
them. **Files:** `gameStateManager.cpp`. **Verify:** identical behaviour; suite green.

### 3.4 Collapse the two network clients `M`
**Why:** found during 0.1. `src/network_manager/NetworkManager.cpp` implements the client
correctly (JSON parse, typed dispatch, `chatReceived`/`playerJoined`/`playerLeft` signals)
and **nothing uses it**. `gameStateManager` opens its own `QTcpSocket` to the same host and
port, which is how the Lua-execution path got in. Two clients, one of them dead.

**Do:** wire the game to `NetworkManager`, delete `gameStateManager`'s socket and
`onServerDataReceived()`, and connect `systemMessage`/`chatReceived` to the chat UI.

**Files:** `gameStateManager.h/.cpp`, `src/network_manager/*`.

**Verify:** one socket in the process; chat round-trips through `NetworkManager`.

---

## The "orphaned system" check

The root cause of every Phase 1 slice is the same: a class was written, tested against
directly, and never called. Add a guard so it cannot recur.

**Do:** for every class under `src/core/`, `src/combat/`, `src/items/`, `src/spell_casting/`,
assert in the suite (or a script) that at least one caller exists **outside** the class's own
translation unit and its tests. A class with zero external callers is either dead code to
delete or a system to wire — never quietly both.

**Verify:** the check fails today for `Endgame`, `MonsterBalance`, `GoldSinks`,
`AgingRules`, `ItemProgression`, `SpellMechanics`, and passes once Phase 1 is done.

---

## Suggested first three slices

1. **0.1 — remove the Lua RCE.** Independent, and it is a security hole.
2. **1.1 — victory sequence.** The difference between "systems built" and "game finishable".
3. **2.1 — start combat from the encounter.** The difference between "combat engine exists"
   and "you fight things".

---

*Created: 2026-10-10. Every claim above was verified against the tree by grepping for callers,
not inferred from the v1.0 roadmap's ✅ marks.*
