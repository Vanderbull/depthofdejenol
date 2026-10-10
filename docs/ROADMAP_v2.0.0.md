# Roadmap v2.0.0 — "The Real Game"

**Status:** Not started
**Version:** v2.0.0 (major — fundamental rework of the game's character)
**Test suite at start:** 1176 passed, 0 failed

## Goal

Make the core loop actually work. v1.0.0 made the game finishable; v1.0.1 fixed the UI.
v2.0.0 transforms it from "walk to floor 15 and press Fight" into a real dungeon crawler:

**Fight → Loot → Equip → Level → Descend Deeper**

Every system below already exists in the codebase — they are tested but unwired. v2.0.0
wires them into the game loop so they matter.

## The Audit: What Exists But Doesn't Matter

| System | File | State | What's Missing |
|---|---|---|---|
| Combat auto-start | `DungeonDialog.cpp:341` | `handleEncounters()` logs a line, combat only starts if you press Fight | `initiateFight()` is a stub |
| Equipment | `inventorydialog.cpp` | Shows equipped items, has unequip | No equip action, no stat requirements, no modifiers applied |
| XP & Leveling | `VictoryReward.cpp`, `LevelTable` | XP calculated on victory, level table loaded | XP never awarded to characters, no level-up flow |
| Loot drops | `VictoryReward::calculateLoot()` | Drops calculated from MDATA5 | Drops never given to player inventory |
| Death flow | `CombatDeathHandler`, `MorgueDialog` | Death detected in combat, morgue can resurrect | Death in combat doesn't reach the morgue |
| Dungeon persistence | `DungeonLevelRegistry` | Level state stored in registry | Levels regenerated on every visit |
| Monster spells | `MonsterAI::decide()` | Can return `CastSpell` | No spell path wired for monsters |
| Status effects | `GameConstants` | Poison, Blind, Confusion flags defined | Not applied in combat (poison DoT, blind penalty exists but blind not set) |
| Spell combat | `CombatActions::castSpellBySchool()` | School mechanics wired | `SpellCastingDialog` result not fed into combat |
| Town wiring | `TavernDialog`, `QuestBoardDialog` | Both exist and are functional | Not reachable from `GameMenu` |
| Item identification | `GeneralStore` | ID cost exists, uncurse exists | Items never spawn as unidentified |
| Bestiary | `BestiaryDialog` | Loads from `bestiary.json` | Never auto-populates from encounters |
| Journal | `JournalDialog` | Full UI with add/filter/clear | Never written to by game events |
| Gold sinks | `GoldSinks` | All costs calculated | No UI charges them (resurrection, ID, uncurse, guild) |
| Aging | `AgingRules` | Stat decay + death by old age | Not applied on rest or time advance |
| Flee semantics | `CombatActions::flee()` | Marks monsters dead | Monsters stay on map after flee |

## Slices

### 2.1 Combat auto-start and flee semantics `M` — ✅ DONE

**Why:** stepping onto a monster only logs a line. Combat begins only if the player then
presses Fight while standing on the tile. `initiateFight()` is an empty stub. The encounter
is announced and then ignored. Fleeing marks monsters dead but they stay on the map.

**Done:**
- Extracted `startCombatAt(pos)` from `on_fightButton_clicked` — shared combat setup path
- `handleEncounters` calls `startCombatAt` for Hostile monsters
- Neutral and friendly monsters do not trigger combat
- Extracted `fleeCombat()` — removes monster from map on successful flee
- Spell damage routes through `applySpellDamageBySchool` (death bookkeeping + school mechanics)
- Tests [73], [74], [75] drive the real paths. Non-vacuous: 2 FAIL + 1 FAIL without fixes.

**Files:** `DungeonDialog.cpp`, `DungeonHandlers.cpp`, `CombatActions.cpp`

**Verify:** walking onto a hostile monster opens combat without pressing Fight ✅; flee →
monster removed from map ✅; spell damage flows through CombatActions ✅.

---

### 2.2 Equipment system `L`

**Why:** 366 items exist in MDATA3 with `type`, `StrReq`, `IntReq`, `WisReq`, `ConReq`,
`ChaReq`, `DexReq`, `StrMod`, `IntMod`, `WisMod`, `ConMod`, `ChaMod`, `DexMod`, `cursed`,
`guilds` bitmask. The inventory shows equipped items and has unequip. But there is no equip
action, no stat requirement enforcement, no stat modifier application.

**Do:**
- Add equip action to `InventoryDialog` (double-click or button)
- Map MDATA3 `type` numbers to slots: Main Hand, Off Hand, Head, Body, Hands, Feet, Cloak,
  Waist, Wrist, Ring
- Enforce stat requirements on equip (block if not met)
- Apply stat modifiers on equip, remove on unequip
- Cursed items cannot be unequipped until uncursed
- Guild bitmask restricts which guilds can use an item

**Files:** `inventorydialog.cpp`, `ItemDatabase.h`, `character.h`

**Verify:** equipping a sword with StrReq 15 blocks a character with Str 12; equipping a
+2 Str gauntlets shows effective Str = base + 2; cursed item cannot be removed.

---

### 2.3 XP and leveling `L`

**Why:** `VictoryReward::calculateXp()` returns `levelFound * 100`. `LevelTable` loads from
`data/levels.json`. But XP is never awarded to characters after combat. There is no level-up
flow. Characters have `level` and `experience` fields that never change.

**Do:**
- Award XP to all living party members on combat victory
- Check XP against `LevelTable::xpForLevel()` after each award
- On level up: increase MaxHP, increase MaxMana (for casters), show level-up message
- Guild-based leveling: different guilds grant different HP/mana/stat bonuses
- Spell learning: characters learn new spells from `spells.json` based on `base_level` and
  `guilds` when they level in a magic guild

**Files:** `CombatActions.cpp` (victory path), `character.h`, `LevelTable.cpp`,
`GuildsDialog.cpp`, `SpellBook.cpp`

**Verify:** killing a level-3 monster awards 300 XP; reaching the threshold increases level
and MaxHP; a Mage guild member learns a new spell on level-up.

---

### 2.4 Loot drops `M`

**Why:** `VictoryReward::calculateLoot()` rolls on Item0-Item9 from MDATA5, filters by
floor, returns item names. But drops are never given to the player. Monsters die and give
nothing but XP and gold.

**Do:**
- On combat victory, roll loot and add items to the lead character's inventory
- Items start as unidentified ("Unknown Sword") until identified at the General Store
- Show loot in the combat result message

**Files:** `CombatActions.cpp` (victory path), `VictoryReward.cpp`, `character.h`

**Verify:** killing a monster that drops "Iron Sword" adds "Unknown Iron Sword" to inventory;
identifying it at the store renames it to "Iron Sword".

---

### 2.5 Death flow `M`

**Why:** `CombatDeathHandler` detects party member death in combat. `MorgueDialog` can
resurrect, hire bodies, and grab bodies. But death in combat doesn't reach the morgue —
dead characters just stay in the party with 0 HP.

**Do:**
- On combat death: mark character `isAlive = false`, remove from active party
- Dead character's body goes to the Morgue (persisted to file)
- Party wipe: all members dead → party left in dungeon, player must form rescue party
- Morgue integration: resurrection costs gold scaled to level (`GoldSinks::resurrectionCost()`)
- Permadeath option: body can only be looted, not resurrected

**Files:** `CombatDeathHandler.cpp`, `MorgueDialog.cpp`, `gameStateManager.cpp`

**Verify:** party member dies in combat → appears in Morgue; resurrecting costs the right
amount; party wipe → all bodies in Morgue.

---

### 2.6 Dungeon persistence `L`

**Why:** `DungeonLevelRegistry` stores level state (monsters, treasures, obstacles, torch
fuel). But levels are regenerated on every visit. When you leave floor 3 and return, it's a
new floor 3.

**Do:**
- Save level state to disk on exit (or use the registry's existing persistence)
- Restore saved state on re-entry
- Monster respawning: some monsters respawn when you leave and return
- Boss monsters do not respawn
- Locked doors and keys persist

**Files:** `DungeonLevelRegistry.cpp`, `DungeonDialog.cpp`, `gameStateManager.cpp`

**Verify:** place an item on floor 3, leave, return → item is still there; kill a monster,
leave, return → normal monsters respawned, boss still dead.

---

### 2.7 Monster spellcasting `M`

**Why:** `MonsterAI::decide()` can return `CastSpell` (40% chance for spellcasters).
But monsters have no spell path wired — the branch is effectively dead. Monsters just
attack every round.

**Do:**
- Give spell-capable monsters a spell (from their data or a per-type table)
- Execute the cast through `CombatActions::castSpellBySchool()`
- Show the spell in the combat log

**Files:** `MonsterAI.cpp`, `EncounterBuilder.cpp`, `CombatActions.cpp`

**Verify:** a caster monster casts at least once over N rounds and the party takes spell
damage.

---

### 2.8 Status effects in combat `M`

**Why:** `GameConstants` defines Poison, Blind, Confusion, On Fire. `CombatActions` handles
Confusion (25% chance to hit ally) and Blind (to-hit penalty). But Poison DoT is not
applied, and no monster or spell inflicts these statuses.

**Do:**
- Poison: lose HP at the start of each turn, lasts N rounds
- Blind: -5 to hit (already coded, but nothing inflicts it)
- Confusion: 25% chance to hit ally (already coded)
- On Fire: damage over time, can be extinguished
- Spells and monster abilities inflict these statuses
- Tavern cures poison and blindness for gold

**Files:** `CombatActions.cpp`, `TurnEngine.cpp`, `TavernDialog.cpp`

**Verify:** poisoned character loses HP each turn; blind character has -5 to hit; tavern
cures both for the right gold amount.

---

### 2.9 Town wiring `M`

**Why:** `TavernDialog` and `QuestBoardDialog` both exist and are functional. But they are
not reachable from `GameMenu`. The city is a set of dialog buttons that don't include them.

**Do:**
- Add Tavern and Quest Board buttons to `GameMenu`
- Tavern: rest (restore HP/mana, advance time, age characters), cure status effects
- Quest Board: fetch quests ("retrieve X from level Y"), kill quests ("slay Z on level W")
- Quest completion: automatic detection (kill count, item in inventory)
- Quest rewards: gold, XP, or items

**Files:** `theCity.cpp` (or wherever GameMenu is), `TavernDialog.cpp`,
`QuestBoardDialog.cpp`

**Verify:** Tavern and Quest Board are reachable from the main menu; resting restores HP
and advances time; accepting and completing a quest gives the reward.

---

### 2.10 Item identification `S`

**Why:** Items found in the dungeon should be unidentified. The General Store has an ID
cost mechanic. But items never spawn as unidentified — they always have their real name.

**Do:**
- Loot drops are unidentified ("Unknown Sword")
- Identify at General Store for `GoldSinks::identificationCost()`
- Uncurse cursed items for `GoldSinks::uncurseCost()`
- Show unidentified items differently in inventory (different color or icon)

**Files:** `CombatActions.cpp` (loot path), `GeneralStore.cpp`, `inventorydialog.cpp`

**Verify:** loot drop shows as "Unknown Iron Sword"; identifying renames it; uncursing
removes the cursed flag.

---

### 2.11 Bestiary auto-population `S`

**Why:** `BestiaryDialog` loads from `bestiary.json` (401 monsters). But it never
auto-populates from encounters — the player sees all monsters from the start, or none.

**Do:**
- Track which monsters the player has encountered
- Show only encountered monsters in the bestiary
- Show monster stats (HP, Att, Def, level found) for encountered monsters
- Show "???" for unencountered monsters

**Files:** `BestiaryDialog.cpp`, `DungeonDialog.cpp` (encounter path)

**Verify:** bestiary is empty at game start; encountering a monster adds it; stats are
shown for encountered monsters.

---

### 2.12 Journal wiring `S`

**Why:** `JournalDialog` has a full UI (add note, filter, clear, exit). But nothing ever
writes to it. The journal is always empty.

**Do:**
- Write to journal on key events: combat victory, level up, quest accepted/completed,
  item identified, death, boss killed
- Auto-entries with timestamps
- Player can add manual notes

**Files:** `JournalDialog.cpp`, `CombatActions.cpp`, `QuestBoardDialog.cpp`

**Verify:** killing a boss adds a journal entry; completing a quest adds an entry; player
can add a manual note.

---

### 2.13 Gold sinks wiring `S`

**Why:** `GoldSinks` calculates costs for resurrection, identification, uncursing, guild
leveling, rest, cure poison, cure blindness. But no UI charges these costs — everything is
free.

**Do:**
- Resurrection at Morgue costs `GoldSinks::resurrectionCost(level, bodyInDungeon)`
- Identification at General Store costs `GoldSinks::identificationCost()`
- Uncurse at General Store costs `GoldSinks::uncurseCost()`
- Guild leveling costs `GoldSinks::guildLevelCost(guild, level)`
- Rest at Tavern costs `GoldSinks::restCostPerHour()` per member
- Cure poison/blindness at Tavern costs the respective amounts

**Files:** `MorgueDialog.cpp`, `GeneralStore.cpp`, `GuildsDialog.cpp`, `TavernDialog.cpp`

**Verify:** resurrecting a level-5 character costs the right amount; identifying an item
costs the right amount; guild leveling deducts gold.

---

### 2.14 Aging consequences `S`

**Why:** `AgingRules` applies stat decay past 70% of max age and death at max age. But it
is never called. Characters never age, never decay, never die of old age.

**Do:**
- Call `AgingRules::applyYearOfAging()` when time advances (rest at tavern, or periodic)
- Stat decay: small chance to lose STR/CON/DEX past threshold
- Death at max age: character dies, body goes to Morgue
- Show aging messages in the adventure log

**Files:** `TavernDialog.cpp` (rest path), `gameStateManager.cpp` (periodic),
`AgingRules.cpp`

**Verify:** resting 100 years past the decay threshold causes stat decay; reaching max age
kills the character.

---

## Dependency Graph

```
2.1 Combat auto-start ──┬─ 2.2 Equipment ── 2.3 XP/Leveling ── 2.4 Loot drops
                        │                                        │
                        │                                        ├─ 2.10 Item ID
                        │                                        ├─ 2.11 Bestiary
                        │                                        └─ 2.12 Journal
                        ├─ 2.7 Monster spells
                        ├─ 2.8 Status effects
                        └─ 2.5 Death flow ── 2.13 Gold sinks
2.6 Dungeon persistence (independent)
2.9 Town wiring ── 2.13 Gold sinks
2.14 Aging (independent)
```

**Critical path:** `2.1 → 2.2 → 2.3 → 2.4` — this is the core loop. Everything else is
depth.

## Release Criteria

v2.0.0 is ready when:

- [ ] Combat starts automatically on encounter (no Fight button press needed)
- [ ] Equipment system works: equip/unequip, stat requirements, modifiers, cursed items
- [ ] XP is awarded on victory and characters level up with stat gains
- [ ] Loot drops are given to the player and start unidentified
- [ ] Death in combat sends the body to the Morgue; resurrection costs gold
- [ ] Dungeon levels persist between visits; bosses don't respawn
- [ ] Monster spellcasters cast spells in combat
- [ ] Status effects (poison, blind, confusion) are applied and matter
- [ ] Tavern and Quest Board are reachable from the main menu
- [ ] Gold sinks are charged (resurrection, ID, uncurse, guild, rest, cure)
- [ ] Aging causes stat decay and death by old age
- [ ] Bestiary auto-populates from encounters
- [ ] Journal records key events
- [ ] All tests pass (target: 1500+)
- [ ] `RELEASE_NOTES.md` is updated
- [ ] Manual playthrough: start → floor 15 → victory → NG+ works end to end

## Estimated Effort

| Slice | Size | Depends On |
|---|---|---|
| 2.1 Combat auto-start | M | — |
| 2.2 Equipment | L | — |
| 2.3 XP/Leveling | L | 2.1 |
| 2.4 Loot drops | M | 2.1, 2.3 |
| 2.5 Death flow | M | 2.1 |
| 2.6 Dungeon persistence | L | — |
| 2.7 Monster spells | M | 2.1 |
| 2.8 Status effects | M | 2.1 |
| 2.9 Town wiring | M | — |
| 2.10 Item ID | S | 2.4 |
| 2.11 Bestiary | S | 2.1 |
| 2.12 Journal | S | 2.1, 2.3, 2.4 |
| 2.13 Gold sinks | S | 2.5, 2.9 |
| 2.14 Aging | S | 2.9 |

**Total:** 15 slices. Critical path is 4 slices (2.1→2.2→2.3→2.4). The rest can be done
in any order after their dependencies.

## What v2.0.0 Is NOT

- **Not a graphics overhaul** — the wireframe renderer stays
- **Not a new dungeon** — the 15 floors stay as they are
- **Not a multiplayer revamp** — the Lua chat server stays as-is
- **Not a balance pass** — monster stats and item prices stay as designed
- **Not new content** — no new monsters, items, spells, or guilds

v2.0.0 is about **wiring existing systems into the game loop** so the game plays like a
real dungeon crawler, not a tech demo.
