# Roadmap v1.0.0 — Connect the Orphaned Systems

**Status:** Not started
**Version:** v1.0.0 (major — first finishable release)
**Test suite at start:** 983 passed, 0 failed

## Goal

Make the game **finishable**. The v0.0 release built all the systems; v0.1 fixed security
and latent breakage. v1.0.0 wires the tested-but-unreachable systems into the game so a player
can start a new game, progress through the dungeon, defeat the final boss, and see the victory
screen.

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

## Dependency graph

```
1.1 Victory sequence ──┬─ 1.2 Hall of Records records
                       └─ 1.3 New Game Plus entry
1.4 MonsterBalance into encounters ── 1.5 ItemProgression into loot/shops
1.6 AgingRules consolidation (kill the duplicate)
1.7 GoldSinks into the town dialogs
```

**Critical path to "the game can be finished":** `1.1 → 1.2`. Everything else is depth.

## Slices

### 1.1 Victory sequence `M` — ✅ DONE

**Why:** `Endgame::victoryTitle()`, `victoryParagraphs()` and `isVictory()` had **zero
callers**. The final boss existed, the quest chain completed, and the player got nothing —
no screen, no acknowledgement, no end. This was the single biggest gap between "systems
built" and "game finishable".

**Done:**
- New `src/victory_dialog/VictoryDialog.h/.cpp` — full-screen dialog with gold title,
  body paragraphs, fade-in animation, and a "New Game Plus" button (shown only on final
  victory). Mirrors the intro `StoryDialog` pattern.
- `DungeonDialog::handleVictory()` now checks `m_combatIsBoss && level == 15` (the
  Prince of Devils) and shows the `VictoryDialog` with `Endgame::victoryTitle()` and
  `Endgame::victoryParagraphs()`.
- A `GameRecord` is built from the party state (hero name, level, gold, deepest floor,
  won=true) — persistence to the Hall of Records is slice 1.2.
- Registered `VictoryDialog` in `blacklands.pro`.

**Verified:** `make check` section [66] — 9 checks (final boss name, isVictory gate,
victory text non-empty, buildFinalBoss stats). 992 passed, 0 failed, 25/25 stable runs.

---

### 1.2 Hall of Records reads real records `M` — ✅ DONE

**Why:** the dialog existed and was reachable from the main menu, but it read
`GuildLeaders` — it never touched `Endgame::ranked()` / `GameRecord`, which 6.4 built and
tested. The Hall showed guild leaders, not records.

**Done:**
- `gameStateManager::addGameRecord(const GameRecord&)` — public method that appends to
  `m_hallofrecordsData` and emits `gameValueChanged("HallOfRecords", ...)`.
- `getGameValue("HallOfRecords")` now returns `m_hallofrecordsData` (not `m_gameStateData`).
- `packStateForSaving()` persists `HallOfRecords` as a `QVariantList` of `QVariantMap`.
- `unpackStateAfterLoading()` restores `m_hallofrecordsData` from the saved list.
- `HallOfRecordsDialog` now builds four ranked sections (Highest Level, Most Gold,
  Deepest Floor, Fastest Completion) using `Endgame::ranked()`, displayed alongside the
  existing guild-leaders section.
- `DungeonDialog::handleVictory()` calls `gsm->addGameRecord(rec)` on final victory.

**Verified:** `make check` section [67] — 11 checks (addGameRecord append, round-trip,
ranked sorting for all 4 categories, win-outranks-non-win). 1004 passed, 0 failed,
25/25 stable runs.

---

### 1.3 New Game Plus entry `S` — ✅ DONE

**Why:** `ngPlusMonsterMultiplier()` / `ngPlusRewardMultiplier()` / `ngPlusBanner()` had no
caller. NG+ was designed and unreachable.

**Done:**
- `EncounterBuilder::buildEncounter()` takes `int ngPlusLevel = 0` and scales monster HP,
  ATT, and DEF by `Endgame::ngPlusMonsterMultiplier()`.
- `VictoryReward::calculateGold()` takes `int ngPlusLevel = 0` and scales gold by
  `Endgame::ngPlusRewardMultiplier()`.
- `gameStateManager::getNgPlusLevel()` / `setNgPlusLevel()` store the cycle count in game
  state (persisted via `m_gameStateData["NGPlusLevel"]`).
- `VictoryDialog` emits `startNewGamePlus` signal; `DungeonDialog` connects it to
  `gsm->setNgPlusLevel(gsm->getNgPlusLevel() + 1)`.

**Verified:** `make check` section [68] — 14 checks (multipliers, banner, EncounterBuilder
scaling, VictoryReward scaling, NG+ level storage). 1018 passed, 0 failed, 25/25 stable.

---

### 1.4 MonsterBalance into the encounter path `M` — ✅ DONE

**Why:** 8.1 built a per-floor difficulty curve and a readiness check; nothing applied it.
Encounters used raw MDATA5 stats, so the curve the roadmap describes was not in the game.

**Done:**
- `EncounterBuilder::buildEncounter()` takes `int floorLevel = 1` and scales monster HP,
  ATT, and DEF by `MonsterBalance::statMultiplier(floorLevel)` (stacked with NG+ scaling).
- `DungeonDialog` passes the current `DungeonLevel` and `NgPlusLevel` to `buildEncounter`.

**Verified:** `make check` section [69] — 10 checks (stat multiplier, floor 1/10/15 HP and
ATT scaling, NG+ and floor stacking, isPartyReady). 1028 passed, 0 failed, 25/25 stable.

---

### 1.5 ItemProgression into loot and shops `M` — ✅ DONE

**Why:** 8.3 built tiers (Bronze→Mithril) and floor→tier mapping; nothing used it, so loot
tiering was whatever `VictoryReward`'s ad-hoc filter did.

**Done:**
- `VictoryReward::calculateLoot()` now gates drops by `ItemProgression::isAvailable()`:
  items whose tier prefix is not available on the current floor are skipped.
- Unprefixed items keep the existing depth filter.

**Verified:** `make check` section [70] — 10 checks (tier curve, availability, Mithril
gating on floor 1 vs floor 15). 1038 passed, 0 failed, 25/25 stable.

---

### 1.6 GoldSinks into the town dialogs `M` — ✅ DONE

**Why:** 8.4 defined seven sinks with costs; no dialog charged them. Prices were scattered as
literals across Tavern/Morgue/Store instead of one authority.

**Done:**
- `GeneralStore` identification fee: `50` → `GoldSinks::identificationCost()`.
- `GeneralStore` uncurse fee: `100` → `GoldSinks::uncurseCost()`.
- `GuildsDialog` level-up cost: `50 * currentLevel` → `GoldSinks::guildLevelCost()`.
- `MorgueDialog` rescue cost: `DeathRecovery::rescuePartyCost()` → `GoldSinks::rescueCost()`.
- Added `GoldSinks.h` includes to all three dialogs.

**Verified:** `make check` section [71] — 16 checks (all sink costs, scaling, allSinks,
sinkDescription). 1054 passed, 0 failed, 25/25 stable.

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

## The "orphaned system" check

The root cause of every slice is the same: a class was written, tested against directly, and
never called. Add a guard so it cannot recur.

**Do:** for every class under `src/core/`, `src/combat/`, `src/items/`, `src/spell_casting/`,
assert in the suite (or a script) that at least one caller exists **outside** the class's own
translation unit and its tests. A class with zero external callers is either dead code to
delete or a system to wire — never quietly both.

**Verify:** the check fails today for `Endgame`, `MonsterBalance`, `GoldSinks`,
`AgingRules`, `ItemProgression`, `SpellMechanics`, and passes once v1.0.0 is done.

---

## Suggested order

1. **1.1 — victory sequence.** The difference between "systems built" and "game finishable".
2. **1.2 — Hall of Records.** Completes the victory loop.
3. **1.3 — New Game Plus.** Builds on 1.1.
4. **1.4 — MonsterBalance.** Affects all encounters.
5. **1.5 — ItemProgression.** Affects loot and shops.
6. **1.6 — GoldSinks.** Affects all town transactions.
7. **1.7 — SpellMechanics.** Affects combat depth.

---

## Release criteria

v1.0.0 is ready when:

- [ ] All 7 slices are complete
- [ ] The "orphaned system" check passes for all 6 systems
- [ ] A player can start a new game, reach floor 15, kill the Prince of Devils, and see the
      victory screen
- [ ] The Hall of Records shows real records
- [ ] New Game Plus is accessible from the victory screen
- [ ] All tests pass (target: 1000+ checks)
- [ ] `RELEASE_NOTES.md` is updated
- [ ] `ReleaseInfo.cpp` version history is updated
