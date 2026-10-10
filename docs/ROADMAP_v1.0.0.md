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
