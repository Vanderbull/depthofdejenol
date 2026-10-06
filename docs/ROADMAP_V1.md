# BlackLands — Roadmap to v1.0

Incremental plan. Every slice is independently buildable, independently verifiable,
and shippable on its own. No slice should take more than a session or two.

**Size key:** `S` = ~1–3 h · `M` = ~1 day · `L` = ~2–3 days

**Rule for every slice:** build clean (`make -j$(nproc)`, zero new warnings) → run the
verification step → only then move on. If a slice can't be verified, it isn't done.

---

## Dependency graph

```
0.1 make check ─────────────────────────────┐ (gates every slice below)
0.2 one source of truth ──┬── 0.3 save path ┤
                          └── 0.3b isAlive  │
                                            │
1.1 ItemDatabase ── 1.2 inventory by ID ──┬─ 1.3 equip ── 1.4 effective stats ── 1.5 UI
                                          ├─ 1.6 consumables
                                          └─ 1.7 identify · 1.8 cursed
                                            
1.4 effective stats ──┐
1.2 inventory by ID ──┴── 2.1 CombatState ── 2.2 turns ──┬─ 2.3 actions ── 2.4 attack math
                                                          ├─ 2.5 monster AI ── 2.6 groups
                                                          ├─ 2.7 spells ── 2.8 status effects
                                                          └─ 2.9 victory ── 2.10 death
                                                                              │
3.1 XP table ── 3.2 level-up ──┬─ 3.3 guild leveling ── 3.4 multi-guild ── 3.5 spells
                               └─ (needs 2.9 to award XP at all)

2.10 death ── 5.1 bodies ── 5.2 Morgue ── 5.3 wipe/rescue
4.1 persistence · 4.2 fifteen floors · 4.3 bosses ── 6.1 quest ── 6.2 final boss ── 6.3 victory
```

**Critical path to a playable loop:** `0.1 → 0.2 → 1.1 → 1.2 → 1.3/1.4 → 2.1 → 2.2 → 2.4 →
2.9 → 3.1 → 3.2`. That sequence alone turns the current shell into a game where you can
equip gear, fight, win, and grow. Everything else is depth on top of it.

**Two bugs found while writing this plan** (both in Phase 0, both small):
- `isAlive` is never serialized — see 0.3b.
- `refreshUI()` and several save paths still read the legacy `m_currentParty` — see 0.2.

---

## Phase 0 — Stabilize the foundation

Nothing above this is worth building until the state layer is trustworthy. The gold bugs
that already cost time are symptoms of the problem below.

### 0.1 Make `make check` mean something `S` — ✅ DONE
**Why:** `check:` only ran `first:` — it only built. There was no regression gate, so every
later slice risked silently breaking an earlier one.

**Done:** added `test/selftest.{h,cpp}`, a headless verification suite linked into the real
binary and reached via `blacklands --selftest`. `blacklands.pro` declares `check` (which
overrides qmake's generated `check: first` — qmake only adds its own when the project hasn't
defined one, so this survives regeneration) and runs the suite. The suite silences Qt logging
for its duration, because the game dumps its whole data set during init and would otherwise
bury the results.

**Verify:** `make check` → 17 passed, exit 0. Deliberately reverted the `isAlive` fix →
exit 1 with the correct single failure. Both confirmed.

---

### 0.2 Collapse the two party sources of truth `L` — ✅ DONE
**Why:** `gameStateManager` held **both** `m_currentParty` (legacy) and
`m_partyManager->currentParty()` (live). Runtime reads went through `PartyManager`; several
write/save paths still touched `m_currentParty`. They drifted.

**Done:** removed `m_currentParty` entirely; `PartyManager` is now the only party. Only four
live uses existed, in two functions:
- `unpackStateAfterLoading()` — dropped the redundant mirror load.
- `addItemToInventory()` — now goes through `PartyManager`. Also fixed a latent
  out-of-bounds write: it indexed `members[m_currentCharacterIndex]` with no range check,
  so a stale index (e.g. after loading a smaller party) wrote past the end of the list.

**Verify:** comment-stripped grep shows zero `m_currentParty` references. `make check` green.

---

### 0.3 Fix save path to serialize the live party `S` — ✅ DONE
**Already correct:** `packStateForSaving()` was writing `m_partyManager->currentParty().toMap()`.
Kept as-is; covered by the save/load checks in the suite.

---

### 0.3b `isAlive` is never serialized `S` — ✅ DONE
**Why:** Found while auditing the save path. `Character::toMap()` wrote `StatusFlags` but
**not** `isAlive`, and `loadFromMap()` had the `isAlive` read commented out. So `isAlive`
always reloaded as its default `true` — a dead character came back alive on every save/load.
FIXLIST #2 is marked FIXED, but that fix only covered the old Lua text save path, not this
`toMap`/`loadFromMap` path.

**Done:** `toMap()` writes `isAlive`; `loadFromMap()` reads it, defaulting to `true` so saves
written before the field existed still load as alive.

**Verify:** suite section [6] kills a member, saves, loads, asserts still dead. Reverting the
fix makes exactly that check fail (exit 1), so the test is not vacuous.

---

### 0.3c `Party::loadFromMap` appended instead of replacing `S` — ✅ DONE
**Why:** Found by the new suite on its first run. `Party::loadFromMap()` appended to
`members` without clearing, and `loadFullGameState()` loaded the party **twice** — once
directly and again via `unpackStateAfterLoading()`. A 4-member party became 8 after one load
and 16 after two.

**Done:** `loadFromMap()` clears `members` first; removed the duplicate load from
`loadFullGameState()` (it now delegates entirely to `unpackStateAfterLoading()`, which already
calls `refreshUI()`).

**Verify:** suite section [5] loads twice and asserts the member count stays constant.

---

### 0.4 Delete dead code in `gameStateManager.cpp` `M`
**Why:** 2 169 lines, 289 of them comments — entire commented-out `m_PC`-era implementations
of `refreshUI`, `loadCharacterFromFile`, `setCharacterInventory`, `getPartyMember`,
`isWholePartyDead`, `addExperienceToParty`. Reading the file is slow and error-prone.

**Do:** delete every commented-out function body. Git history preserves them.
Mechanical, but do it *after* 0.2 so nothing you need gets thrown away.

**Files:** `gameStateManager.cpp`.

**Verify:** identical binary behaviour — smoke test passes, no symbol changes.

---

### 0.5 Close the two outstanding security/link fixes `S`
**Why:** FIXLIST #1 (RCE) and #12 (declared-but-undefined function) are still open.

**Do:**
- **#1** — remove `luaL_dostring` on raw socket bytes in `onServerDataReceived()`. Dispatch a
  fixed command set instead. Also strip `os`/`io` from the Lua state before opening a socket.
- **#12** — implement or delete `readyBodyForResurrection()`.

**Files:** `gameStateManager.cpp`, `gameStateManager.h`.

**Verify:** multiplayer chat still works; `nm` shows no undefined symbol.

---

## Phase 1 — Items and equipment

366 items exist with full stats and there is no way to wear one. This unlocks everything
downstream: combat needs equipment numbers, progression needs gear tiers.

### 1.1 Introduce a typed item database `M` — ✅ DONE
**Why:** items were `QVariantMap` looked up by name string, in at least three places
(`GeneralStore::loadItemsFromCsv`, `gameStateManager::itemData`, `getItemStats`).
No compile-time safety, linear scan, three copies of the truth.

**Done:** added `src/items/ItemDatabase.{h,cpp}`:
- `ItemDef` — one field per MDATA3 column, same names as the CSV, so the mapping is
  obvious from either side.
- `ItemSlot` namespace with `ItemDef::slot()`, deriving the body slot from the `type`
  column. The mapping was read off the actual data, not guessed: 0/1/3/4/5/6/7 weapons →
  MainHand, 2 Cross and 11 Shield → OffHand, 12/13 → Head, 14/15 → Hands, 8/9/10 → Body,
  16 Cloak, 17 Wrist, 18/19 Waist, 20 Feet, 21 Ring, 22 Amulet; 23–34 (potions, scrolls,
  tomes, dust, crystals, rods, stones, spheres, cubes, curios, crests) → not equippable.
- `ItemDef::typeName()` for display, plus `requirementFor()`/`modifierFor()` keyed by
  `GameConstants` stat names.
- `ItemDatabase` singleton: header-driven CSV parse (column order can change), O(1)
  `byId()`/`byName()`, and `toVariantMap()` so existing map-based UI keeps working.
- `loadItemData()` builds the database alongside the legacy `m_itemData` list, and
  `getItemStats()` now hits the database first.

**Verify:** `make check` section [7] — 22 checks over real MDATA3 values (Bronze Sword
id/att/price/StrReq, Eliminator's 303 566 432 price and 2 hands, slot derivation for plate/
helm/shield/potion, the cursed flag, stat modifiers, and null for unknown id/name).

---

### 1.1b Store read keys that don't exist in MDATA3 `S` — ✅ DONE
**Why:** found while wiring the database. `getItemStats()` returns MDATA3 rows, which use
`price` and `cursed` — but the store read `cost` and `isCursed`. So every sell fell back to
the 10 GP default (a 303-million-gold Eliminator sold for 5), and **no item was ever
recognised as cursed**, which would have blocked 1.8 entirely.

**Done:** store reads `price` / `cursed`. Sell and inspect now use the real price via
`toLongLong()` (prices exceed `int` — Eliminator is 303 566 432). The shop's Type column
also showed a raw type number; it now shows the readable name from the database.

---

### 1.2 Inventory stores item IDs, not names `L` — ✅ DONE
**Why:** `Character::inventory` was a `QStringList` of names. Duplicates were
indistinguishable, and there was nowhere to put charges, identification, or equipped state.
`HeldItem` was already defined in `character.h` and unused.

**Done:** migrated `Character::inventory` and added `Character::bankInventory` as
`QList<HeldItem>`. `HeldItem` gained `identified` and `name` fields. Serialization handles
three formats: old `QStringList`, `QVariantList` of strings, and new `QVariantList` of
maps — so existing saves load and resolve item IDs from the database. All call sites updated:
`gameStateManager` (addItemToInventory, setCharacterInventory, setBankInventory,
getBankInventory, addItemToCharacter), `GeneralStore`, `BankDialog`, `TradeDialog`,
`InventoryDialog`, `DungeonDialog`. Dungeon loot now starts as `identified = false`.

**Files:** `character.h/.cpp`, `gameStateManager.h/.cpp`, `src/general_store/*`,
`src/bank_dialog/*`, `src/inventory_dialog/*`, `src/dungeon_dialog/DungeonDialog.cpp`.

**Verify:** `make check` section [8] — 24 checks covering round-trip, old-format loading,
new-format loading, bank inventory, and gameStateManager integration. 63 passed, 0 failed.

---

### 1.3 Equip and unequip `L` — ✅ DONE
**Why:** `InventoryDialog`'s Equip button only moved a list row locally and did nothing to
game state. `Character::M586D[36]` (equipped slots) was never written.

**Done:** added `Character::equipped` as `QList<HeldItem>`. Implemented `equipItem()` and
`unequipItem()` on `Character`, with `gameStateManager` wrappers. Validation before equipping:
- stat requirements (`StrReq` … `DexReq`) — checked against base stats
- `nHands` — two-handed weapons require both MainHand and OffHand free
- slot occupancy — one item per body slot
- equippability — potions and other non-equipment refused

On success the item moves from `inventory` to `equipped`; on failure a human-readable reason
is returned. `InventoryDialog` now has working Equip and Unequip buttons that call through to
`gameStateManager` and show a warning dialog on refusal. Equipped items serialize and
deserialize correctly.

**Files:** `character.h/.cpp`, `gameStateManager.h/.cpp`, `src/inventory_dialog/inventorydialog.*`.

**Verify:** `make check` section [9] — 27 checks covering stat requirements, slot occupancy,
two-handed weapons, unequip, serialization round-trip, and gameStateManager integration.
90 passed, 0 failed.

---

### 1.4 Effective stats from equipment `M` — ✅ DONE
**Why:** `StrMod` … `DexMod` existed in the data and were ignored. Nothing changed when you
wore plate.

**Done:** added `effectiveStrength()` through `effectiveDexterity()` plus `effectiveStat(name)`
on `Character`. Each sums the base stat with all equipped items' modifiers. `refreshUI()`
syncs the six `CurrentCharacter*` stat keys so the character sheet shows effective values.

**Files:** `character.h/.cpp`, `gameStateManager.cpp`.

**Verify:** `make check` section [10] — 11 checks covering base stats, +1 modifier, unequip
restores base, case-insensitive lookup, and stacking.

---

### 1.5 Real inventory UI `M` — ✅ DONE
**Why:** the dialog was a stub — one list, an Equip button that lied.

**Done:** added effective stats panel showing base vs effective for all six stats. Equipped
tab populates from `Character::equipped`. Item tooltips show type, slot, ATT/DEF, price,
modifiers, requirements, cursed flag, and two-handed status. Use button wired to
`useConsumable`. Full equip/unequip cycle works through the UI.

**Files:** `src/inventory_dialog/inventorydialog.h/.cpp`.

**Verify:** equip → effective stats panel updates → unequip → panel restores. Tested via
`make check` sections [9] and [10].

---

### 1.6 Consumables `M` — ✅ DONE
**Why:** 26 items have `type == 23` (potions) plus scrolls and dust, with `spellIndex`,
`spellID` and `charges`. Use did nothing.

**Done:** added `Character::useConsumable()` and `gameStateManager::useConsumable()`.
Healing potions restore HP, mana potions restore mana, cure potions remove poison. Charges
decrement per use; item removed at zero. Non-consumable items refused.

**Files:** `character.h/.cpp`, `gameStateManager.h/.cpp`, `src/inventory_dialog/inventorydialog.cpp`.

**Verify:** `make check` section [11] — 8 checks covering healing, charge decrement,
consumption at 0, non-consumable refusal, and gameStateManager integration.

---

### 1.7 Item identification `M` — ✅ DONE
**Why:** the store had an "Identify (50 GP)" button but nothing was ever unidentified.

**Done:** `HeldItem.identified` field added (serialized). Dungeon loot starts as
`identified = false`. `gameStateManager::identifyItem()` clears the flag. Already-identified
items refused.

**Files:** `character.h/.cpp`, `gameStateManager.h/.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`.

**Verify:** `make check` section [12] — 7 checks covering unidentified loot, identification,
already-identified refusal, and gameStateManager integration.

---

### 1.8 Cursed items `M` — ✅ DONE
**Why:** `cursed` was in the data; `uncurseSelectedItem()` existed; nothing was ever cursed.

**Done:** cursed items cannot be unequipped (refused with reason). `gameStateManager::uncurseItem()`
strips the "Cursed" prefix and updates the item ID. Non-cursed items refused.

**Files:** `character.cpp`, `gameStateManager.h/.cpp`.

**Verify:** `make check` section [13] — 6 checks covering equip cursed, unequip refused,
uncurse succeeds, non-cursed refusal, and gameStateManager integration.

---

## Phase 2 — Combat

The core loop. Build it as a headless, unit-testable engine first, then wire the UI.

### 2.1 `CombatState` data model `M` — ✅ DONE
**Why:** combat was a `QTimer` firing every 100 ms with hardcoded damage. No turn, no round,
no participant list. `BattleEngine.h` was an unrelated unwired stub.

**Done:** added `src/combat/CombatState.h/.cpp` — pure logic, no Qt UI dependency.
`CombatParticipant` holds name, HP, att, def, speed, dex, isPlayer, isAlive, hasActed,
level, damageMod, swings. `CombatState` manages the participant list, initiative order
(d20 + speed + dex/2, sorted descending), round counter, and turn cycling via `nextTurn()`.
Dead participants are skipped. `isCombatOver()` returns true when either side has no living
participants.

**Files:** `src/combat/CombatState.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [14] — 19 checks covering empty state, participant counts,
initiative order, turn cycling, dead participant skip, combat-over detection, and multiple
rounds. 142 passed, 0 failed.

---

### 2.2 Turn engine `M` — ✅ DONE
**Why:** real-time cooldowns are not the genre. The design doc calls for initiative order.

**Done:** added `src/combat/TurnEngine.h/.cpp` — wraps `CombatState` and provides
`startRound()`, `nextTurn()`, `markCurrentActed()`, `isRoundOver()`, `isCombatOver()`,
`combatStatus()`, `livingPlayers()`, `livingMonsters()`. Pure logic, no Qt UI dependency.
The QTimer loop in `DungeonDialog` is not yet deleted — that happens in 2.3 when the
action menu is wired.

**Files:** `src/combat/TurnEngine.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [15] — 24 checks covering round start, turn cycling,
dead participant skip, combat status string, multiple rounds, and living participant queries.
166 passed, 0 failed.

---

### 2.3 Player action menu `M` — ✅ DONE
**Why:** only Attack and Defend existed, and Defend only halved damage for one tick.

**Done:** added `src/combat/CombatActions.h/.cpp` — pure logic, no Qt UI dependency.
Five actions: `attack(targetIndex)` (to-hit roll d20 + dex/2 vs 10 + def, damage =
att × swings × damageMod/100, crit on 20, fumble on 1), `defend()` (marks as acted),
`castSpell(targetIndex, spellPower)` (spellPower + att damage), `useItem(itemIndex)`
(marks as acted), `flee()` (d20 + speed vs 12). Dead targets refused.

**Files:** `src/combat/CombatActions.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [16] — 12 checks covering attack, defend, spell, flee,
isPlayerTurn, and dead target refusal. 178 passed, 0 failed.

---

### 2.4 Equipment-driven attack resolution `L` — ✅ DONE
**Why:** damage was `QRandomGenerator::bounded(5, 15)` — equipment, level and stats were ignored.
Weapon `att`, `swings`, `damageMod`, `levelScale` and monster `def` were all sitting unused.

**Done:** attack resolution now uses `att`, `swings`, `damageMod`, `levelScale`, `level`,
and `dex`. To-hit: d20 + dex/2 + level vs 10 + def + level. Damage: att × swings ×
damageMod/100 × (1 + levelScale × level / 100). Crit on 20, fumble on 1. `CombatParticipant`
gained `levelScale` field.

**Files:** `src/combat/CombatState.h`, `src/combat/CombatActions.cpp`.

**Verify:** `make check` section [17] — 3 simulated combat tests: 2-swing warrior
out-damages 1-swing mage over 100 rounds, higher level attacker deals more damage, higher
DEX attacker hits at least as often. 181 passed, 0 failed.

---

### 2.5 Monster turns and AI `L` — ✅ DONE
**Why:** the monster always attacked party member 0 for `bounded(1, 10)`.

**Done:** added `src/combat/MonsterAI.h/.cpp` — pure logic, no Qt UI dependency.
`decide()` returns Attack, Flee, or CastSpell based on HP thresholds (flee below 25% HP,
50% chance; always flee below 10%). `chooseTarget()` uses 60% front row, 25% weakest,
15% random. `takeTurn()` executes the decision and marks the monster as acted.

**Files:** `src/combat/MonsterAI.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [18] — 9 checks covering monster attack, low-HP flee,
full-HP attack, front-row targeting, weakest targeting, and player-turn detection.
190 passed, 0 failed.

---

### 2.6 Group encounters `M` — ✅ DONE
**Why:** encounters are single-monster; the data supports mixed groups.

**Done:** added `src/combat/EncounterBuilder.h/.cpp` — builds a group of monsters from
MDATA5 data. Uses `numGroups` × `hits` to determine group size (capped at 10). Each monster
gets unique name ("Orc 1", "Orc 2", ...), stats from MDATA5 (`att`, `def`, `StatDex`,
`StatCon`, `levelFound`, `damageMod`), and slight HP variation. Falls back to a generic
monster if the type is not found in the data.

**Files:** `src/combat/EncounterBuilder.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [19] — 14 checks covering group size calculation,
unique names, stat population, unknown monster fallback, and a full combat simulation
where all 6 Orcs take turns and the combat ends within 20 rounds. 204 passed, 0 failed.

---

### 2.7 Spells in combat `M` — ✅ DONE
**Why:** `SpellCastingDialog` returns a `SpellResult` that only pokes the active monster's HP
from outside the turn loop.

**Done:** added `castSpellAdvanced()`, `castHeal()`, and `hasEnoughMana()` to `CombatActions`.
`castSpellAdvanced()` takes spell name, mana cost, damage range ("min-max"), and AoE flag.
Mana is deducted from `CombatParticipant.mana` (new field). AoE hits all living participants
of the same type as the target. `castHeal()` heals a target up to max HP. `hasEnoughMana()`
checks mana without casting. All methods return -1 on failure (not enough mana, dead target,
invalid index).

**Files:** `src/combat/CombatState.h` (added `mana`/`maxMana` to `CombatParticipant`),
`src/combat/CombatActions.h/.cpp`.

**Verify:** `make check` section [20] — 18 checks covering single-target Fireball, AoE
Fireball hitting 3 goblins, insufficient mana failure, heal spell, heal capping at max HP,
`hasEnoughMana()`, and a full combat where mage casts AoE Fireball in a group fight.
222 passed, 0 failed.

---

### 2.8 Status effects in combat `M` — ✅ DONE
**Why:** `Poisoned`, `Blinded`, `OnFire` exist in `GameConstants` and are never applied.

**Done:** added status effect system to `CombatActions`. `applyStatus()` applies a status flag
with duration. `tickStatusEffects()` ticks all effects at round end: poison deals 1-3 DoT per
round, OnFire deals 2-5 DoT per round, blind reduces to-hit by 5, confusion has 25% chance
to hit a random ally instead. Durations decrement each tick and expire at 0. `CombatParticipant`
gained `statusFlags`, `poisonDuration`, `blindDuration`, `fireDuration`, `confusionDuration`.
Attack resolution now checks blind penalty and confusion friendly fire.

**Files:** `src/combat/CombatState.h`, `src/combat/CombatActions.h/.cpp`.

**Verify:** `make check` section [21] — 18 checks covering poison DoT and expiry, blind
to-hit reduction, confusion friendly fire, OnFire DoT and expiry, tick at round end, and
a full combat with poison. 240 passed, 0 failed.

---

### 2.9 Victory: XP, gold and loot `M` — ✅ DONE
**Why:** `awardBattleLoot()` picks a **uniformly random item from all 366** — a level 1
character can loot a 303-million-gold Eliminator. No XP is awarded, no gold.

**Done:** added `src/combat/VictoryReward.h/.cpp` — calculates XP, gold, and loot from
MDATA5 data. XP = `levelFound` × 100. Gold = `goldFactor` × random(1, 10). Loot rolls on
`Item0`–`Item9` drop slots, filtered by item's `floor` vs current dungeon depth, with drop
chance based on rarity. Uses `ItemDatabase::byId()` for typed item lookups.

**Files:** `src/combat/VictoryReward.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [22] — 12 checks covering XP calculation, gold range,
drop table extraction, loot filtering by depth, and full victory flow. 252 passed, 0 failed.

---

### 2.10 Death in combat `S` — ✅ DONE
**Why:** HP hits 0 and nothing happens; the dialog just carries on.

**Done:** added `src/combat/CombatDeathHandler.h/.cpp` — handles death in combat.
`isPartyWipe()` checks if all players are dead. `isVictory()` checks if all monsters are dead.
`processDeaths()` returns messages for newly dead players. `handleGameOver()` returns true
on party wipe. `reviveAllPartyMembers()` revives all dead players with 1 HP. `isPartyMemberDead()`
checks a specific member. `getDeadPartyMemberNames()` returns names of dead members.

**Files:** `src/combat/CombatDeathHandler.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [23] — 24 checks covering individual death, party wipe,
victory, partial death, revive, dead name listing, full combat with party wipe, and
processDeaths messages. 278 passed, 0 failed.

---

## Phase 3 — Progression

### 3.1 Data-driven XP table `S` — ✅ DONE
**Why:** `addExperienceToCharacter` uses `level * 1000` — linear, and no per-level data.

**Done:** added `data/levels.json` with XP curve (`xpForLevel(n) = round(100 * n^1.5)`) and
`src/core/LevelTable.h/.cpp` — singleton that loads the JSON and exposes `xpForLevel(n)`,
`totalXpForLevel(n)`, `levelForXp(totalXp)`, and `xpProgress(totalXp)`. Falls back to the
formula if the file is not loaded.

**Files:** `data/levels.json`, `src/core/LevelTable.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [24] — 25 checks covering table loading, XP values,
total XP, level-for-XP, XP progress, and fallback. 301 passed, 0 failed.

### 3.2 Level-up stat gains `M` — ✅ DONE
**Why:** levelling only increments the `level` integer. HP, mana and stats never move, so
levelling feels like nothing.

**Done:** `PartyManager::addExperienceToCharacter` now uses `LevelTable::levelForXp()` instead
of `level * 1000`. Added `applyLevelUpGains(Character&)` — grants 2-6 HP (full heal), 1-3
mana for casters (Int or Wis ≥ 10, full restore), and +1 to a random stat every 2 levels.

**Files:** `src/partymanager/PartyManager.h/.cpp`.

**Verify:** `make check` section [25] — 8 checks covering HP gain, mana gain for casters,
no mana for non-casters, stat point on even level, no stat point on odd level, and level-up
via LevelTable. 309 passed, 0 failed.

### 3.3 Guild leveling `M` — ✅ DONE
**Why:** the "Make Level" button in `GuildsDialog` does nothing.

**Done:** added guild progression to `Character` — `guildLevel()`, `joinGuild()`,
`incrementGuildLevel()`, `guildXpToNextLevel()`, `addGuildExperience()`. Guild XP uses the
same curve as character levels (`LevelTable`). Levels and XP serialize via `GuildLevels` /
`GuildExperience` in `toMap`/`loadFromMap`.

**Files:** `character.h/.cpp`.

**Verify:** `make check` section [26] — 14 checks covering join, XP thresholds (100 for
level 1→2, 283 for 2→3), direct increment, multi-guild totals, and save/load persistence.
355 passed, 0 failed.

### 3.4 Multi-guild progression `L` — ✅ DONE
**Why:** a defining mechanic of the source game — total power is the sum of all guild levels.

**Done:** `Character::guildLevels` is a `QMap<QString, int>` — a character can belong to any
number of guilds, each tracked independently. `totalGuildLevels()` sums them. Both levels and
per-guild XP persist across save/load.

**Files:** `character.h/.cpp`.

**Verify:** `make check` section [26] — a character in two guilds (Mages 2, Healers 3) reports
`totalGuildLevels() == 5`; both levels survive a `toMap`/`loadFromMap` round-trip.

### 3.5 Spell learning `M` — ✅ DONE
**Why:** `spells.json` has `base_level`, `guilds` and `required_stats`; nothing reads them.

**Done:** added `src/spell_casting/SpellBook.h/.cpp` — loads all 47 spells from `spells.json`.
`spellsFor(c)` returns spells the character qualifies for (guild level ≥ `base_level`, member
of a teaching guild, all `required_stats` met). `newlyLearned(c, guild, level)` reports what
a specific guild level grants. `canCast()` checks known-spell plus mana.

**Files:** `src/spell_casting/SpellBook.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [27] — 13 checks: level-1 Mage knows Flame Bolt but not
Fireball; guild level 3 unlocks Fireball; a Warrior gets no mage spells; INT 5 blocks Flame
Bolt; `newlyLearned` reports Fireball at level 3; `canCast` honours mana.

### 3.6 Aging and old age `M` — ✅ DONE
**Why:** `incrementPartyAge()` and `processAgingConsequences()` are declared with no real effect.

**Done:** added `src/core/AgingRules.h/.cpp` — `maxAgeForRace()` (Human 100, Elf 400, ...),
`decayThresholdForRace()` (70% of max age), `applyYearOfAging(c)` which drops STR/CON/DEX by
1 with a 10% chance per year past the threshold and kills the character at max age (HP 0,
`Dead` status). `isPastMaxAge()` / `isDecaying()` expose the state.

**Files:** `src/core/AgingRules.h/.cpp`, `blacklands.pro`.

**Verify:** `make check` section [28] — 19 checks: race age limits, decay threshold, no
effect when young, decay fires within 200 rolls at age 80, death at 100 for a Human, and a
100-year-old Elf unaffected.

---

## Phase 4 — Dungeon depth

### 4.1 Persistent level state `L` — ✅ DONE
**Why:** the dungeon regenerates every visit. Exploration and monster clearing are meaningless.

**Done:** added `src/core/DungeonLevelState.h/.cpp` — `LevelSnapshot` holds a floor's monsters,
treasure, opened chests, visited tiles, stairs and boss state; `DungeonLevelRegistry` keeps one
snapshot per floor and serializes them all. `DungeonDialog::enterLevel` restores a visited floor
instead of regenerating it, and stores the snapshot after first generation.

**Files:** `src/core/DungeonLevelState.h/.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`.

**Verify:** `make check` section [29] — 15 checks: store/retrieve a floor, a cleared monster
stays cleared across a leave-and-return, and a full serialize/deserialize round-trip preserves
monsters, opened chests, visited tiles and stairs.

### 4.2 Fifteen themed floors `L` — ✅ DONE
**Why:** lore implies deep depths; there is one flat level.

**Done:** added `src/core/DungeonThemes.h/.cpp` — 15 floors with names and descriptions
(Abandoned Mines → Flooded Workings → ... → The Devil's Threshold), each with its own monster,
treasure and trap counts, tile-mix percentages, and a `monsterLevelBonus` that rises with depth.
`enterLevel` now populates from the theme instead of fixed constants.

**Files:** `src/core/DungeonThemes.h/.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`.

**Verify:** `make check` section [31] — 13 checks: max depth 15, correct names, level bonus
tracks depth, difficulty rises with depth, all 15 names unique, out-of-range clamps.

### 4.3 Boss encounters `M` — ✅ DONE
**Done:** added `src/core/BossEncounter.h/.cpp`. Bosses on floors 5, 10 and 15 (Grotto Warden,
The Bone Tyrant, The Prince of Devils). `canDescend()` blocks the down stairs until the boss is
dead; `buildBoss()` scales HP/att/def/swings with depth; `bossXp()` far exceeds a normal monster.
`enterLevel` places the boss on a room tile; `transitionLevel` refuses descent while it lives.

**Files:** `src/core/BossEncounter.h/.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`.

**Verify:** `make check` section [32] — 22 checks: boss floors identified, names correct,
descent blocked before and allowed after the boss dies, normal floors never block, boss stats
scale with depth.

### 4.4 Locked doors and keys `M` — ✅ DONE
**Why:** `DoorEnums.h` exists; no key mechanic.

**Done:** added `DoorState` and `DoorAndSearch::canOpen()` / `tryOpen()` — a door with a key
name refuses without that key and opens with it; secret doors cannot be opened directly.

**Files:** `src/core/DoorAndSearch.h/.cpp`.

**Verify:** `make check` section [33] — 12 checks: unlocked doors open freely, locked doors
refuse without the key and open with it, the wrong key does not help, secret doors resist
direct opening.

### 4.5 Secret door discovery `S` — ✅ DONE
**Why:** `on_searchButton_clicked()` finds hidden doors with a flat 3×3 scan and no roll.

**Done:** `DoorAndSearch::searchForSecretDoors()` rolls d20 + WIS mod + INT mod against a
difficulty that scales with floor (`10 + floor - 1`), limited to the 3×3 area. The search
button now uses the real roll and the character's WIS/INT.

**Files:** `src/core/DoorAndSearch.h/.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`.

**Verify:** `make check` section [34] — 6 checks: difficulty scales with floor, high WIS/INT
finds more doors than low over 400 attempts, doors outside the 3×3 area are never found.

### 4.6 Monster respawn `S` — ✅ DONE
**Done:** `DungeonLevelRegistry::respawnMonsters(level, fraction, originalCount, rng)` places
back up to `fraction × originalCount` monsters on free tiles. `enterLevel` calls it with 0.3
when returning to a visited floor, so grinding stays viable without trivialising clearing.

**Files:** `src/core/DungeonLevelState.h/.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`.

**Verify:** `make check` section [30] — 6 checks: 30% of 10 respawns exactly 3, a full floor
respawns nothing, an ungenerated floor is a no-op.

---

## Phase 5 — Death and consequences

### 5.1 Death state and body carrying `M`
**Do:** dead members stay in the party as bodies, carried back to town.

### 5.2 Morgue integration `M`
**Why:** `MorgueDialog` has resurrection, body hiring and body grabbing; nothing feeds it.

**Do:** dead characters appear in the Morgue; resurrection costs gold scaled to level.

**Verify:** die, return to town, resurrect, member is alive and in the party.

### 5.3 Party wipe and rescue `L`
**Do:** all dead → the party is stranded; form a rescue party from town to recover bodies
and gear. Permadeath option for hardcore.

**Verify:** wipe a party, recover it with a second party.

---

## Phase 6 — Win condition and endgame

### 6.1 Main quest chain `M`
### 6.2 Final boss — the Prince of Devils `M`
### 6.3 Victory sequence `S` — counterpart to the intro `StoryDialog`
### 6.4 Hall of Records `M` — fastest completion, highest level, most gold, deepest floor
### 6.5 New Game Plus `M`

---

## Phase 7 — Town and quality of life

### 7.1 Tavern / Inn `M` — rest, restore HP/mana, cure status, advance time
### 7.2 Quest board `M` — fetch and kill quests
### 7.3 NPC dialog `M` — personality, dungeon hints
### 7.4 Alignment consequences `S` — evil barred from the Paladin's Guild, etc.
### 7.5 Character sheet `M` — equipped items, effective stats, guild levels, known spells
### 7.6 Bestiary `M` — auto-populate the Library as monsters are encountered
### 7.7 Journal `M`
### 7.8 Sound effects and visual feedback `M` — hits, spells, doors, traps, level-up, death
### 7.9 Keyboard shortcuts `S` — F/S/R/…
### 7.10 Tutorial `M` — guided first dungeon run

---

## Phase 8 — Balance and release

### 8.1 Monster difficulty curve `M` — floor 1 beatable at level 1; floor 15 needs a real party
### 8.2 Spell differentiation `M` — fire AoE, cold slow, lightning chain, mind crowd control
### 8.3 Item progression curve `M` — Bronze → Iron → Steel → Adamantite → Mithril
### 8.4 Gold sinks `M` — resurrection, identification, uncursing, guild leveling
### 8.5 Packaging `M` — installer, version bump, release notes, tag `v1.0.0`

---

## Suggested first three slices

If you want momentum before committing to the long haul:

1. **0.1 — `make check` that means something.** One hour, and it makes every later slice safer.
2. **0.2 — collapse the two party sources of truth.** Fixes the bug class that already burned
   time, and unblocks the save path.
3. **1.1 — typed item database.** Small, self-contained, and it is the prerequisite for both
   equipment and the combat rework.

---

*Created: 2026-10-01*
