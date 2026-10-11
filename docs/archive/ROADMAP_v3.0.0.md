# Roadmap v3.0.0 — Tactical Combat and Character Growth

**Status:** Not started
**Version:** v3.0.0 (major — fundamental rework of how combat and growth play)
**Test suite at start:** 1320 + 3 (all passing)

## Goal

v2.0.0 wired the core loop: you can fight, loot, equip, level and descend. But the *fight*
itself is still a single button. `EncounterBuilder` now produces real monster groups
(`numGroups × ingroup` participants), yet the player cannot choose which monster to hit —
`on_combatAttackButton_clicked()` always takes the **first living monster**. Group
encounters are in the data and in the combat state, but not in the interface.

v3.0.0 makes combat **tactical** and growth **chosen**:

- The player picks the target, sees each monster separately, and uses rows to protect casters.
- Level-ups hand the player a stat point instead of rolling a random stat.
- The main quest chain actually drives the game instead of only describing it.
- Saves are versioned and migrated for real, and play time is measured.

This is a major bump because it changes the game's character: fights become decisions, not
a button, and characters become builds, not a random walk.

## Slices

### 3.1 Combat target selection `M` — ✅ DONE

**Why:** `on_combatAttackButton_clicked()` scans participants and takes the first living
monster (`DungeonDialog.cpp:1394`). `EncounterBuilder::buildEncounter()` adds every monster
of a group as its own `CombatParticipant` (`DungeonDialog.cpp:1367`), so a 3-monster fight
gives the player no way to focus one down. `CombatActions::attack(int targetIndex, ...)`
already takes a target — only the UI refuses to pass one.

**Done:**
- `m_combatTargetList` — a `QListWidget` in the combat panel, one row per living monster,
  each carrying its participant index in `Qt::UserRole`
- `selectedMonsterIndex()` / `selectFirstLivingMonster()` on `DungeonDialog`
- `on_combatAttackButton_clicked()` attacks the selected monster, falling back to the first
  living one (so keyboard-only play still works)
- The spell path in `on_spellButton_clicked()` casts at the selected monster too
- `updateCombatUI()` rebuilds the list, preserves the player's choice, and moves the
  selection to a survivor when the chosen monster dies
- `on_combatAttackButton_clicked()` and `updateCombatUI()` made public for tests

**Files:** `src/dungeon_dialog/DungeonDialog.cpp` / `.h`

**Verify:** tests [126]-[128] — in a 3-monster fight, attacking the third damages the third
and leaves the first untouched ✅; killing the selected monster moves the selection to a
survivor ✅; with nothing selected the fallback finds the first living monster ✅.
Non-vacuous: restoring "take the first living monster" fails the targeting test.

---

### 3.2 Per-monster combat display `M` — ✅ DONE

**Why:** the combat panel collapses every monster into one label:
`m_combatMonsterHpLabel->setText("Monster: " + monsterHp.join(", "))` (`DungeonDialog.cpp:1516`).
There is no monster list widget at all (verified: no `monsterList`/`targetList` in the
combat UI). With groups this reads as a wall of numbers. Combat item use is also hardcoded:
`on_combatUseItemButton_clicked()` searches the inventory for a name containing "healing"
or "health" and uses the first match — the player cannot choose a potion.

**Done:**
- Each target row shows the monster's own HP plus its status, via
  `combatStatusText()`: poison/fire/blind/confused/stunned/slowed rounds and "defending"
- The row that is acting this turn is marked with `> ` so the turn order is visible
- `on_combatUseItemButton_clicked()` opens a picker listing the party member's actual
  consumables instead of guessing at the first "healing" match
- `Character::isConsumable()` is the single rule for what counts (potions 23, scrolls 24,
  tomes 25) — the same one `useConsumable()` enforces, so a listed item always works

**Files:** `src/dungeon_dialog/DungeonDialog.cpp` / `.h`, `character.cpp` / `character.h`

**Verify:** tests [129]-[131] — three monsters give three rows with independent HP and
damaging one changes only its row ✅; status reaches the row, not just the helper ✅; only
real consumables are offered ✅. Non-vacuous: dropping the status from the row fails the
row tests; accepting every item as consumable fails the consumable test.

**Note:** the aggregated `m_combatMonsterHpLabel` is kept — it is the compact "Monster:"
summary and several existing tests read it.

---

### 3.3 Front and back row `L` — ✅ DONE

**Why:** planned in `docs/ROADMAP_v1.1.0.md` (slice 2.2) and never done.
`CombatParticipant` had no row field. `MonsterAI::chooseTarget()` already rolled
"60% front row (first living player), 25% weakest, 15% random" — but "front row" was just
"whoever is first in the list", so the tactic was cosmetic and casters could not be
protected.

**Done:**
- `enum class CombatRow { Front, Back }` plus `row` and `ranged` on `CombatParticipant`
- `CombatState::reachablePlayerIndices(bool ranged)` — the single rule: melee sees the front
  row only, ranged (spells, breath) sees everyone
- `MonsterAI::chooseTarget()` uses it, so a melee monster can no longer pick a back-row
  target even when it is the weakest
- `EncounterBuilder` marks spellcasters and fire-breathers `ranged`; everything else is melee
- Party members default to front (first two) and back (the rest); the new **Row** button
  toggles the acting member via `togglePlayerRow()`
- The party line shows each member's row: `Name [front]: hp/maxHp`

**Files:** `src/combat/CombatState.h` / `.cpp`, `src/combat/MonsterAI.cpp`,
`src/combat/EncounterBuilder.cpp`, `src/dungeon_dialog/DungeonDialog.cpp` / `.h`

**Verify:** tests [132]-[137] — melee reaches the front row only while ranged reaches both
✅; an all-back-row party blocks melee entirely ✅; a melee monster never targets the back
row over 200 decisions ✅; a ranged monster does ✅; the Row button toggles a player and
refuses monsters/invalid indices ✅; the party line shows rows ✅. Non-vacuous: making
reachability ignore the row fails four tests.

**Note:** rows protect players as targets. Monster melee is blocked by an empty front line,
which is the tactical point — put the fighter up front and the caster behind.

---

### 3.4 Stat allocation on level-up `M`

**Why:** `PartyManager::applyLevelUpGains()` grants "+1 to a random stat every 2 levels"
(`PartyManager.cpp:101`). Character creation lets the player spend stat points
(`createcharacterdialog.cpp`), so the game already has the concept — it just takes the
choice away exactly when it matters most. There is no build identity.

**Do:**
- Award stat points on level-up (a pool that accumulates; 1 per level, 2 on even levels).
- Let the player spend them on the character sheet (`CharacterSheetDialog`), showing the
  effective stat and its race cap.
- Keep guild-based automatic bonuses (Warrior +STR, Mage +INT) as a floor, not the only path.

**Files:** `src/partymanager/PartyManager.cpp`, `src/character_dialog/CharacterSheetDialog.cpp`,
`character.cpp` / `character.h`

**Verify:** leveling to 2 adds a spendable point; spending it raises the chosen stat and
decrements the pool; the pool persists through save/load.

---

### 3.5 Main quest chain driver `L` — ✅ DONE

**Why:** `QuestChain` computed steps, completion and rewards, but the only caller was
`QuestChainDialog` — the display. Nothing in the game *advanced* the chain, granted a step's
`rewardGold`/`rewardXp`, or gated victory on it. The "main quest line" was a read-only screen.

**Done:**
- `gameStateManager::setDungeonLevel()` tracks `DeepestFloor` live (never decreases)
- `src/core/QuestChainDriver.h` / `.cpp` — reads live state (DeepestFloor, DungeonLevelRegistry,
  party inventory), grants rewards for newly completed steps, writes journal entries
- `handleVictory()` calls `QuestChainDriver::advance()` after every boss kill
- Victory is now gated on `questResult.chainJustCompleted` instead of hardcoded `level == 15`
- Rewards are tracked via `QuestRewardGiven_<stepId>` game-state keys to prevent double-granting

**Files:** `src/core/QuestChainDriver.h` / `.cpp`, `gameStateManager.cpp` / `.h`,
`src/dungeon_dialog/DungeonDialog.cpp`, `test/selftest.cpp`

**Verify:** tests [148]-[153] — DeepestFloor tracks via setDungeonLevel ✅; advance grants
rewards for new steps ✅; no double-grant on second advance ✅; chain not complete without
Prince ✅; advance reports chain completion after Prince defeat ✅; progressText shows current
step ✅. Non-vacuous: disabling reward granting fails [149] and [150].

---

### 3.6 Play-time tracking and a real Hall of Records `S` — ✅ DONE

**Why:** `DungeonDialog.cpp:1657` recorded `rec.completionTimeSeconds = 0; // TODO: track
actual play time`. `Endgame::outranks()` treats `completionTimeSeconds <= 0` as "not
finished", so the fastest-completion record could never be set. Play time was not measured
anywhere.

**Done:**
- `gameStateManager::playTimeMs()` / `playTimeSeconds()` — live millisecond resolution
  (`QElapsedTimer`), so sub-second precision survives
- `stopPlayTime()` now called from `packStateForSaving()` so the persisted value is exact
- `PlayTimeSeconds` persisted in save and restored on load
- `completionTimeSeconds` on victory uses `gsm->playTimeSeconds()` instead of hardcoded 0
- `GameRecord::formattedTime()` formats as HH:MM:SS; `Endgame::outranks()` sorts
  fastest-completion correctly
- Hall of Records displays `formattedTime()` for the Fastest Completion category

**Files:** `gameStateManager.cpp` / `.h`, `src/dungeon_dialog/DungeonDialog.cpp`,
`src/core/Endgame.cpp`, `src/hall_of_records/hallofrecordsdialog.cpp`

**Verify:** tests [154]-[158] — playTimeMs non-zero after 50ms ✅; timer resets ✅;
play time survives save/load ✅; completionTimeSeconds populated ✅; faster run sorts
ahead ✅. Non-vacuous: forcing playTimeMs to 0 fails [154] and [155].

---

### 3.7 Save versioning and migration `M` — ✅ DONE

**Why:** `VERSIONING.md` documented a migration policy but the code only ever *wrote*
`SaveVersion` — nothing read it on load. v3.0.0 adds fields (row, stat points, play time),
which is exactly when a silent mismatch starts corrupting saves.

**Done:**
- `src/core/savegameVersion.h` / `.cpp` — `CURRENT_SAVE_VERSION` (2), `MAX_LOADABLE_VERSION`,
  `versionOf()`, `isLoadable()`, `migrate()`
- `loadFullGameState()` reads the version, refuses newer saves with a `saveLoadRefused` signal
  and a Swedish user-facing message, then migrates older saves in order
- `migrateFrom1()` fills `StatPoints`, `PlayTimeSeconds`, `row` with safe defaults
  (matching `Character::toMap()`'s lowercase `"row"` so it survives a round-trip)
- `VERSIONING.md` created — documents the policy, the version table, and how to add a version
- `packStateForSaving()` writes `CURRENT_SAVE_VERSION` instead of hardcoded `1`

**Files:** `src/core/savegameVersion.h` / `.cpp`, `gameStateManager.cpp` / `.h`,
`VERSIONING.md`, `test/selftest.cpp`

**Verify:** tests [138]-[141] + [139b] — SaveVersion written on save ✅; v1 save migrates
to current with defaults ✅; legacy save (no SaveVersion) migrates ✅; future-version save
is refused with a message ✅; in-memory version is current after migration ✅. Non-vacuous:
disabling migration fails [139b]; bumping MAX_LOADABLE_VERSION fails [141].

---

### 3.8 Combat visual feedback `S` — ✅ DONE

**Why:** there was no on-hit feedback (no flash/shake/particle code outside Qt's own tool
Makefiles). Combat was readable only from the log. This is not a graphics overhaul — the
wireframe renderer stays — it is the minimum needed to feel a hit land.

**Done:**
- `DungeonDialog::flashCombatFx(bool damage)` — flash combat panel red on damage, green on heal
- `combatFxEnabled()` — reads `CombatFxEnabled` from game state (default on)
- `combatGroupStyleSheet()` — test hook to read the flash state
- QTimer (200 ms) clears the flash automatically
- Hooked into `advanceCombat()`: monster AI hit → red flash

**Files:** `src/dungeon_dialog/DungeonDialog.cpp` / `.h`

**Verify:** tests [159]-[161] — combatFxEnabled defaults to true ✅; flashCombatFx sets a
stylesheet ✅; stylesheet contains background-color ✅; stylesheet cleared after timer ✅;
flashCombatFx is a no-op when disabled ✅. Non-vacuous: disabling fx fails [160].

---

### 3.9 Release hygiene: stop tracking build artifacts `S` — ✅ DONE

**Why:** `FIXLIST.md` #20 is marked FIXED but the artifacts were still tracked (verified:
`git ls-files` listed `Makefile`, `blacklands` and `qrc_resources.cpp`). Every commit in
v2.0.0 carried binary churn — a rebuilt `blacklands` and a regenerated `Makefile` — which
makes diffs unreadable and invites merge conflicts. A release version should not ship them.

**Done:**
- `git rm --cached` removed `Makefile`, `blacklands`, `qrc_resources.cpp`, `.qmake.stash`,
  `bankpane/moc_BankPane.cpp`, `bankpane/moc_predefs.h`, `data/saves/autosave.json`
- `.gitignore` extended to cover them
- Verified: `qmake6 -o Makefile blacklands.pro && make` works from a clean checkout,
  `make check` green, `git status` clean after build

**Files:** `.gitignore`, git index

**Verify:** `git status` is clean after a full `make`; `qmake6 -o Makefile blacklands.pro &&
make` works from a fresh checkout.

---

## Dependency Graph

```
3.1 Target selection ──┬─ 3.2 Per-monster display
                       └─ 3.3 Front/back row
3.4 Stat allocation (independent)
3.5 Quest chain driver ── 3.6 Play time (records need a completion)
- [x] Save versioning (must land before 3.4/3.6 persist new fields)
3.8 Visual feedback (independent)
3.9 Repo hygiene (independent)
```

**Critical path:** `3.1 → 3.2` — target selection is the hole that makes group encounters
playable; the display makes the choice visible. Everything else is depth.

**Ordering note:** land 3.7 (save versioning) *before* 3.4 and 3.6 add persisted fields, so
the migration path exists from the first save that contains them.

## Release Criteria

v3.0.0 is ready when:

- [x] The player can choose which monster to attack, and the choice is visible
- [x] Each monster in a group shows its own HP and status
- [x] Front/back row changes who melee can reach
- [ ] Level-ups grant spendable stat points; the pool persists
- [x] The main quest chain advances from play and grants its rewards
- [x] Play time is tracked and the Hall of Records sorts by it
- [x] Saves are versioned, migrated, and newer saves are refused
- [x] Combat gives visual feedback on hits and deaths
- [x] `Makefile`, `blacklands` and `qrc_resources.cpp` are untracked; a clean build works
- [ ] All tests pass (target: 1500+ — carried over from v2.0.0, which ended at 1323)
- [ ] `RELEASE_NOTES.md` and `ReleaseInfo::versionHistory()` are updated
- [ ] Manual playthrough: create party → descend → beat a group fight with targeting →
      complete a quest step → win, with rows and stat points exercised

## Estimated Effort

| Slice | Size | Depends On |
|---|---|---|
| 3.1 Combat target selection | M | — |
| 3.2 Per-monster combat display | M | 3.1 |
| 3.3 Front/back row | L | 3.1 |
| 3.4 Stat allocation on level-up | M | 3.7 |
| 3.5 Main quest chain driver | L | — |
| 3.6 Play time + Hall of Records | S | 3.5, 3.7 |
| 3.7 Save versioning and migration | M | — |
| 3.8 Combat visual feedback | S | 3.2 |
| 3.9 Release hygiene | S | — |

**Total:** 9 slices. Critical path is 2 slices (3.1 → 3.2). 3.7 should land early so 3.4
and 3.6 have a migration path.

## What v3.0.0 Is NOT

- **Not a graphics overhaul** — the wireframe renderer stays; 3.8 is feedback, not art
- **Not a new dungeon** — the 15 floors and their themes stay
- **Not new content** — no new monsters, items, spells, or guilds
- **Not a balance pass** — monster stats and prices stay as designed
- **Not multiplayer work** — the Lua chat server stays as-is
- **Not a rewrite** — every slice wires or extends existing systems, as in v2.0.0

v3.0.0 is about making the fight a decision and the character a build, so the loop v2.0.0
connected actually has depth to play with.
