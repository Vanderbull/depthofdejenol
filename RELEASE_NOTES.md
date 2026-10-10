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

# v1.0.0 — in progress

**Phase 1 — connect the orphaned systems.** Wires the tested-but-unreachable classes into
the game so a player can start a new game, progress through the dungeon, defeat the final
boss, and see the victory screen.

**Test suite at start of v1.0.0:** 983 passed, 0 failed.

---

## Slice 1.1 — Victory sequence ✅

**Why.** `Endgame::victoryTitle()`, `victoryParagraphs()` and `isVictory()` had zero
callers. The final boss existed, the quest chain completed, and the player got nothing.

**Changed.**
- New `src/victory_dialog/VictoryDialog.h/.cpp` — full-screen dialog with gold title,
  body paragraphs, fade-in animation, and a "New Game Plus" button (final victory only).
- `DungeonDialog::handleVictory()` checks `m_combatIsBoss && level == 15` (the Prince of
  Devils) and shows the victory dialog.
- A `GameRecord` is built from party state (persistence is slice 1.2).
- Registered in `blacklands.pro`.

**Verified.** `make check` section [66] — 9 checks. 992 passed, 0 failed, 25/25 stable.

---

## Slice 1.2 — Hall of Records ✅

**Why.** The dialog existed and was reachable from the main menu, but it read
`GuildLeaders` — it never touched `Endgame::ranked()` / `GameRecord`. The Hall showed
guild leaders, not records.

**Changed.**
- `gameStateManager::addGameRecord(const GameRecord&)` — public method that appends to
  `m_hallofrecordsData` and emits `gameValueChanged("HallOfRecords", ...)`.
- `getGameValue("HallOfRecords")` now returns `m_hallofrecordsData` (not `m_gameStateData`).
- `packStateForSaving()` persists `HallOfRecords` as a `QVariantList` of `QVariantMap`.
- `unpackStateAfterLoading()` restores `m_hallofrecordsData` from the saved list.
- `HallOfRecordsDialog` now builds four ranked sections (Highest Level, Most Gold,
  Deepest Floor, Fastest Completion) using `Endgame::ranked()`, displayed alongside the
  existing guild-leaders section.
- `DungeonDialog::handleVictory()` calls `gsm->addGameRecord(rec)` on final victory.

**Verified.** `make check` section [67] — 11 checks (addGameRecord append, round-trip,
ranked sorting for all 4 categories, win-outranks-non-win). 1004 passed, 0 failed,
25/25 stable runs.

---

## Slice 1.3 — New Game Plus ✅

**Why.** `ngPlusMonsterMultiplier()` / `ngPlusRewardMultiplier()` / `ngPlusBanner()` had no
caller. NG+ was designed and unreachable.

**Changed.**
- `EncounterBuilder::buildEncounter()` takes `int ngPlusLevel = 0` and scales monster HP,
  ATT, and DEF by `Endgame::ngPlusMonsterMultiplier()`.
- `VictoryReward::calculateGold()` takes `int ngPlusLevel = 0` and scales gold by
  `Endgame::ngPlusRewardMultiplier()`.
- `gameStateManager::getNgPlusLevel()` / `setNgPlusLevel()` store the cycle count in game
  state.
- `VictoryDialog` emits `startNewGamePlus`; `DungeonDialog` connects it to
  `gsm->setNgPlusLevel(gsm->getNgPlusLevel() + 1)`.

**Verified.** `make check` section [68] — 14 checks (multipliers, banner, EncounterBuilder
scaling, VictoryReward scaling, NG+ level storage). 1018 passed, 0 failed, 25/25 stable.

---

## Slice 1.4 — MonsterBalance into encounters ✅

**Why.** 8.1 built a per-floor difficulty curve and a readiness check; nothing applied it.
Encounters used raw MDATA5 stats, so the curve was not in the game.

**Changed.**
- `EncounterBuilder::buildEncounter()` takes `int floorLevel = 1` and scales monster HP,
  ATT, and DEF by `MonsterBalance::statMultiplier(floorLevel)` (stacked with NG+ scaling).
- `DungeonDialog` passes the current `DungeonLevel` and `NgPlusLevel` to `buildEncounter`.

**Verified.** `make check` section [69] — 10 checks (stat multiplier, floor 1/10/15 HP and
ATT scaling, NG+ and floor stacking, isPartyReady). 1028 passed, 0 failed, 25/25 stable.

---

## Slice 1.5 — ItemProgression into loot ✅

**Why.** 8.3 built tiers (Bronze→Mithril) and floor→tier mapping; nothing used it.

**Changed.**
- `VictoryReward::calculateLoot()` gates drops by `ItemProgression::isAvailable()`:
  items whose tier prefix is not available on the current floor are skipped.
- Unprefixed items keep the existing depth filter.

**Verified.** `make check` section [70] — 10 checks (tier curve, availability, Mithril
gating on floor 1 vs floor 15). 1038 passed, 0 failed, 25/25 stable.

---

## Slice 1.6 — GoldSinks into town dialogs ✅

**Why.** 8.4 defined seven sinks with costs; no dialog charged them. Prices were scattered as
literals across Tavern/Morgue/Store instead of one authority.

**Changed.**
- `GeneralStore` identification fee: `50` → `GoldSinks::identificationCost()`.
- `GeneralStore` uncurse fee: `100` → `GoldSinks::uncurseCost()`.
- `GuildsDialog` level-up cost: `50 * currentLevel` → `GoldSinks::guildLevelCost()`.
- `MorgueDialog` rescue cost: `DeathRecovery::rescuePartyCost()` → `GoldSinks::rescueCost()`.
- Added `GoldSinks.h` includes to all three dialogs.

**Verified.** `make check` section [71] — 16 checks (all sink costs, scaling, allSinks,
sinkDescription). 1054 passed, 0 failed, 25/25 stable.

---

## Slice 1.7 — SpellMechanics into combat ✅

**Why.** 8.2 built fire-AoE / cold-slow / lightning-chain / mind-CC and none of it was used;
combat casting did not differentiate schools.

**Changed.**
- `CombatActions::applySpellDamageBySchool()` looks a spell up in the `SpellBook` and applies
  its school's mechanic via `SpellMechanics`: fire splashes (with the per-target bonus), cold
  slows (speed penalty restored on expiry), lightning chains (20% falloff per jump), mind
  stuns (and may confuse).
- `CombatActions::castSpellBySchool()` resolves the spell, charges its own mana cost, rolls
  from its damage range, then applies the school mechanic.
- `CombatParticipant` gained `slowDuration` / `slowAmount` / `stunDuration`; `tickStatusEffects()`
  expires them, and `CombatState::nextTurn()` skips a stunned participant's turn.
- `DungeonDialog`'s spell handler routes through the school-aware path.

**Verified.** `make check` section [72] — 15 checks (fire splash, cold slow + expiry,
lightning chain, mind stun, stun skips a turn, mana charged from the spell, refusal when
short). 1070 passed, 0 failed, 25/25 stable.

---

## Orphaned-system guard ✅

**Why.** Every v1.0.0 slice had the same root cause: a class was written, tested against
directly, and never called by the game. Tests passed; the feature did not exist.

**Changed.**
- `tools/check_orphaned_systems.py` asserts that each of the six previously-orphaned systems
  (`Endgame`, `MonsterBalance`, `GoldSinks`, `AgingRules`, `ItemProgression`,
  `SpellMechanics`) has at least one `Class::` reference outside its own translation unit and
  outside `test/`.
- Wired into `make check` via `blacklands.pro`, so `make check` now runs the self-test suite
  and then the guard.

**Verified.** All six systems pass; the guard is non-vacuous (a synthetic class yields zero
callers, `GoldSinks` yields three). `make check` exits 0.

---

## v1.0.0 version set ✅

**Changed.**
- `blacklands.pro`: `SEMVER_MAJOR = 1`, `SEMVER_MINOR = 0`, `SEMVER_PATCH = 0` →
  `GameConstants::SEMANTIC_VERSION = "1.0.0"`.
- `ReleaseInfo::versionHistory()` gains a 1.0.0 entry as the latest version.
- Suite updated: `[56]` now expects `1.0.0` for version, version string, installer name,
  banner, and the head of the version history.

**Also fixed a flaky test.** `[68]`'s "NG+1 gold reward is higher than base" summed 50 rolls;
gold carries a random 1–10× component, so the ~2.6σ comparison inverted roughly 1 run in 25.
Raised to 2000 rolls (~14σ). 40/40 consecutive runs now stable.

**Verified.** `make check` exits 0; `SEMANTIC_VERSION` reads `1.0.0` in the generated
`version.h`; 1070 passed, 0 failed, 40/40 stable.

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
