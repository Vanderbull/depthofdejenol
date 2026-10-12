# Release Notes

**Latest:** v5.8.0 — Wire & Cleanup Round 4

## v5.8.0 — Wire & Cleanup Round 4

### Slice 8.0a — Remove dead SenderWindow
- SenderWindow had 0 external callers — fully dead code
- Removed from src/ and blacklands.pro

### Slice 8.0b — Release
- Version bumped to 5.8.0
- ReleaseInfo updated
- Clean build, 1752 tests pass, game runs clean

## v5.7.0 — Wire & Cleanup Round 3

### Slice 7.0a — Remove dead ShortcutHelp
- ShortcutHelp was never used by production code, only by its own test
- After removing the test, the class is fully dead

### Slice 7.0b — QObject includes (N/A)
- All Q_OBJECT headers already include `<QObject>`

## v5.6.0 — Wire & Cleanup Round 2

### Slice 6.0a — Remove dead DungeonMap
- `src/dungeonmap/dungeonmap.{cpp,h}` removed — never included or instantiated
- DungeonMinimap and AutomapDialog handle their own rendering

### Slice 6.0b — Helplesson dwarf image
- Label was "Dwarf Image\nPlaceholder" — no image resource ever loaded
- Label hidden instead of showing placeholder
- Non-vacuous: restoring the placeholder fails the test

### Slice 6.1 — Options backup timestamp
- `lastBackupLabel` was never updated after backup/restore
- Now shows actual date after both operations
- Non-vacuous: removing the update fails the test

### Slice 6.2 — Release
- Version bumped to 5.6.0
- ReleaseInfo updated
- backup/ untracked from git
- Clean build, 1762 tests pass, game runs clean

### Slice 5.1 — Arena status effects
- ArenaDialog::advanceCombat now calls tickStatusEffects at round end
- Also ticks on player turn so poison/fire/blind work in the arena
- Test proves poison deals damage via advanceCombat

### Slice 5.2 — PlayerExperience wiring
- DungeonDialog reads "PlayerExperience" for the XP label, but it was never written
- addExperienceToCharacter now writes the party's total living XP to "PlayerExperience"
- Test proves the label updates after gaining XP

### Slice 5.3 — partyUpdated signal wiring
- PartyManager::partyUpdated is connected to gameStateManager::refreshUI
- Test proves the signal fires and triggers UI refresh
- Non-vacuous: removing the connect fails the test

### Slice 5.5 — RaceData guild eligibility
- RaceData has per-race guild eligibility (Giant/Dwarf/Elf restrictions)
- Test proves the eligibility map is not all-allowed
- Non-vacuous: removing the restrictions fails the test

### Slice 5.6 — HP calculation (N/A)
- DungeonDialog no longer uses placeholder HP — maxHp comes from Character

### Slice 5.7 — CombatState/CombatDeathHandler (N/A)
- Audit said 0 external callers, but both are used by DungeonDialog and
  ArenaDialog via unique_ptr members — already wired correctly

## v5.4.0 — Main Menu & Save System Redesign

### Slice 5.1 — Main menu redesign
- New Game now requires at least one character (opens Create Character first)
- Load Game loads complete game state (not just one character)
- Added Save Game button (named save slots)
- Assemble Party now modifies the current game state and saves it
- Create Character unchanged (saves JSON to data/characters/, adds to party)

### Slice 5.2 — Legacy .lua character migration
- Auto-converts .lua character files to JSON on startup
- Idempotent (won't overwrite existing JSON)
- Test added for migration

### Slice 5.3 — HP repair for legacy saves
- Characters with missing HP in old saves now get default HP (10) instead of loading as dead

### Slice 5.4 — Remove all legacy .txt/.lua character code
- Removed saveCharacterToLua, loadCharacterFromLua, migrateLegacyCharacterFiles
- Removed src/core/savegameUtils (was .txt-only)
- All dialogs (theCity, Seer, Morgue, CreateCharacter) now use JSON exclusively
- 4 seed test characters (Aria, Borin, Cael, Dara) checked into git
- Tests that wiped data/characters/ now stash/restore

### Slice 5.5 — Save/load complete gamestate
- Load Game now uses loadFullGameState (party + dungeon + records)
- Save Game creates named save files in data/saves/
- Assemble Party modifies and saves the current party
- Tests: save, load, assemble, persistence

### Slice 5.6 — Arena fixes
- Arena button was wired to handleLocationClick (no arena case) — now wired to on_arenaButton_clicked
- Combat state converted from global pointers to unique_ptr members (no leaks)
- Spell button now uses castSpellBySchool (Fireball) instead of attack()
- Item button now uses useItem() with healing item detection
- ArenaWavesCleared updated in game state so achievements trigger
- Tests: 1746 passed, 0 failed

## v5.3.0 — Aging & Lifecycle ✅ RELEASED

Characters now age over time with full lifecycle events: coming-of-age, marriage, children, and retirement.

### Slice 5.1 — Aging mechanics
- `AgingRules::applyYearOfAging(c, years)` supports multi-year aging
- `AgingRules::getAgeBracket()` returns Child/Adult/Decaying
- Coming-of-age ceremony at 18
- Race-specific decay threshold and max age (already existed)

### Slice 5.2 — Lifecycle events
- `Character::spouse` and `Character::children` fields
- `isEligibleForMarriage()`, `marry()`, `addChild()`, `canRetire()`
- All lifecycle data serializes via `toMap()`/`loadFromMap()`

### Slice 5.3 — Legacy & genealogy
- `Character::inheritFrom(ancestor)` — descendant inherits half of ancestor's stats and 10% of gold
- Family tree tracking via children list
- Genealogy data in character sheet

### Slice 5.4 — Persistence & migration
- Lifecycle data persists in JSON save/load round-trips
- Backward-compatible with v5.2.0 saves (missing fields default correctly)
- Party save/load preserves all lifecycle data

### Slice 5.5 — UI updates
- Character sheet data includes age, spouse, children
- Family tree view data
- Life event timeline data

### Slice 5.6 — Release
- Version bumped to v5.3.0
- All 6 slices complete

**Verified:** 1841 tests, 0 failed. Non-vacuous: reverting each fix fails its test.

---

**Previous:** v5.2.0 — Savegame revamp ✅ RELEASED

## v5.2.0 — Savegame revamp ✅ RELEASED

The savegame system had three critical bugs: autosave silently failed, party building only kept the last selected character, and `saveCharacterToFile()` didn't save inventory, equipped items, hunger, or guild data.

### Slice 5.1 — Autosave fix
- `handleAutosave()` now calls `saveFullGameState("autosave")` instead of `saveCharacterToFile(0)`
- `handleAutosave()` is now public (was private)
- Fixed: `saveCharacterToFile()` reads from `PartyManager` (single source of truth) instead of stale `m_gameStateData["Party"]`

### Slice 5.2 — Party building from individual characters
- New `addCharacterFromFile()` method — loads a character and ADDS to the party (doesn't replace)
- `CharacterListDialog::onSelectClicked()` now uses `addCharacterFromFile()` instead of `loadCharacterFromFile()`
- `saveCharacterToFile()` now saves: hunger, gold, mana, playTimeSeconds, experience, inventory (JSON), equipped (JSON), bankInventory (JSON)

### Slice 5.3 — CharacterListDialog UI update
- Selection preview shows member names, count, and average level
- Party size indicator (e.g. "Party: 2 / 4")
- Clear selection button

### Slice 5.4 — LoadPartyDialog preview
- Preview shows member names, count, average level
- Deceased members marked with "(deceased)"
- Empty saves show "No party members found"

### Slice 5.5 — JSON-only save format
- `loadCharacterFromFile()` — only `.json` supported; `.lua` and `.txt` rejected
- `addCharacterFromFile()` — only `.json` supported; `.lua` and `.txt` rejected
- `saveCharacterToFile()` — writes `.json` only
- `Character::loadFromMap()` — defaults for missing fields (hunger=100, isAlive=true, etc.)

**Verified:** 1912 tests, 0 failed (+211 checks). Non-vacuous: reverting each fix fails its test.

---

**Previous:** v5.1.0 — Main Menu redesign ✅

## v5.1.0 — Main Menu redesign ✅ RELEASED

The main menu had a critical layout bug: `m_runButton` and `m_newButton` shared
the same grid cell, so only one was visible. The menu also hid unavailable
buttons instead of showing them as disabled, used a raw file picker for loading,
and drew debug text over the background.

### What changed
- **Continue button** — loads autosave directly with a party preview showing
  member names, count, and average level
- **Party preview panel** — styled frame under the Continue button showing the
  saved party at a glance
- **Clear visual hierarchy** — primary (Continue/New Game), secondary (Load
  Game/Character List), footer (Options/Help/About/Exit)
- **Load Party dialog** — lists all saved parties with preview instead of a raw
  file picker
- **Debug text removed** — `paintEvent` no longer draws "ABCDEFGH"
- **Fixed grid collision** — `m_runButton` and `m_newButton` no longer share a
  cell
- **Version label** — shows "BLACK LANDS — v5.0.0" under the title

**Verified:** 1728 tests, 0 failed (+27 checks). Tests [220]–[223] cover
Continue button visibility, click-to-load, party preview text, and hidden
state when no autosave exists.

## v5.0.0 — Survival: make hunger real ✅ RELEASED

A hunger/starvation system existed in code but was invisible, unfixable, and its
deaths were unhandled. This version made survival real and added party loading.

### Slice 5.1 — Hunger is visible
- Party panel shows hunger with a warning colour below 20%
- Character sheet shows "Hunger: N/M (P%)"
- A log message fires once when a member crosses into the low band
- **Verified:** 1617 tests, +23 checks. Non-vacuous: removing the announce call,
  omitting hunger from the row, and not setting the sheet text all fail

### Slice 5.2 — Food exists and can be eaten
- Type 35 = Food (was: type 22 = Amulets — `eatFood()` ate jewellery)
- Four MDATA3 rows: Rations, Waybread, Dried Meat, Elven Lembas
- Eat button + `H` key in the dungeon; inventory Use button eats food
- `GeneralStore::makeHeldItem()` seeds charges (bought items vanished on use)
- Fixed a crash: `GeneralStore::updateCharacterHeader()` indexed `.first()` on
  an empty party
- **Verified:** 1655 tests, +38 checks. Non-vacuous: reverting to type 22 fails
  13 checks; unregistering the Eat button fails the button test

### Slice 5.3 — Starvation death is a real death
- `DeathRecovery::killCharacter()` records body location (was: `setDead()` → body
  at (0,0,0), no journal entry)
- Journal entry on starvation death
- Party-wipe check after each step → `handlePartyWipe()`
- **Verified:** 1664 tests, +9 checks. Non-vacuous: reverting to `setDead()` fails
  4 checks; removing the wipe check fails the journal test

### Slice 5.4 — One HP source of truth
- `updatePartyMemberHealth()` writes the live member, not `CurrentCharacterHP`
- Rest and spell healing write the live member
- Removed `CurrentCharacter*` special-casing in `setGameValue()`
- **Verified:** 1672 tests, +8 checks. Non-vacuous: reverting to
  `CurrentCharacterHP` fails 3 checks

### Slice 5.5 — Town resupply loop
- Tavern rest restores hunger (hours × 25)
- `addStartingFoodToParty()` gives every member a Rations stack
- **Verified:** 1675 tests, +3 checks. Non-vacuous: removing each feature fails
  its test

### Slice 5.6 — Load a whole party
- `CharacterListDialog` is `ExtendedSelection` with a real Select handler
- Save Party / Load Party buttons (JSON via `savePartyToFile`/`loadPartyFromFile`)
- `addCharacterToParty()` enforces `MAX_PARTY_SIZE`
- **Verified:** 1689 tests, +14 checks. Non-vacuous: reverting to `SingleSelection`
  and removing the guard each fail

### Slice 5.7 — Repo hygiene
- Removed `TheCity.*`, `FontManager.h`, `StoryDialog.h`, `DataRegistry.h`
- Removed `src/core/BattleEngine/` (8 build artefacts)
- Added `data/characters/` to `.gitignore`
- **Verified:** 1700 tests, +10 checks. Non-vacuous: restoring a dead file fails

### Slice 5.8 — Release
- `SEMVER_MAJOR` bumped to 5
- v5.0.0 added to `ReleaseInfo::versionHistory()`
- **Verified:** 1701 tests, +1 check. Non-vacuous: reverting to 4 fails 4 checks

**Test progression:** 1590 → 1701 (+111 checks across 8 slices)
**All non-vacuous probes observed failing, then restored.**

Every change to this project is recorded here, newest release first.
Format: what changed, why it mattered, and how it was verified.

## Version scheme (agreed 2026-10-10)

Semantic versioning with game-studio conventions:

- **Major (X.0.0)** — first version that can be played start to finish, or a fundamental rework of the game's character.
- **Minor (X.Y.0)** — new features, mechanics, areas; backwards-compatible.
- **Patch (X.Y.Z)** — bug fixes, balance, text, polish; no new features.

---

# v5.0.0 — Survival: make hunger real 🚧 IN PROGRESS

**Goal:** the hunger/starvation system exists in code but the player can never see
it, never manage it, and its deaths the game does not treat as deaths. The user's
report — *"starving did not kill"* — is the symptom of a mechanic that is
unreachable rather than absent. This version makes survival real, and adds the
ability to load a whole party.

## What landed

### Slice 5.1 — Hunger is visible
- `Character::hungerPercent()` and `isHungerLow()` (warning band at ≤20%)
- `DungeonDialog::partyMemberStatusText()` / `partyMemberColor()` drive the party
  panel: the row now reads "Name (Lv N) — HP — Hunger N/M", coloured red for
  dead/starving, yellow for low HP **or** low hunger, green when healthy
- `announceHungerWarnings()` logs the crossing into the warning band once per
  member (re-armed when hunger recovers), called from `movePlayer()`
- `CharacterSheetDialog` shows a "Hunger: N / M (P%)" row that turns amber when low

**Verified:** tests [189]–[193] (1617 passed, 0 failed; +23 checks).
Non-vacuous, all three reverts observed to fail:
- removing the `announceHungerWarnings()` call from `movePlayer()` →
  "the move itself announced the low-hunger crossing" FAILS
- omitting hunger from the panel row → both panel row tests FAIL
- not setting the sheet's hunger text → "character sheet shows the member's hunger" FAILS

### Slice 5.2 — Food exists and can be eaten
- **Type 35 = Food** is a real item type (was: `eatFood()` gated on type 22, which is
  Amulets — it ate jewellery and no food item existed)
- Four real food rows in MDATA3: Rations (12g), Waybread (25g), Dried Meat (40g),
  Elven Lembas (120g)
- `Character::FOOD_TYPE` / `isFood()` / `foodIndices()` / `hasFood()`; `eatFood()`
  restores 10 + spellLvl×5 hunger
- **Eat** button + `H` key in the dungeon (`DungeonDialog::eatFromInventory()`); the
  inventory "Use" button eats food instead of refusing it
- `GeneralStore::makeHeldItem()` seeds charges so a bought item is a stack, not a
  single portion that vanishes
- **Fixed a real crash:** `GeneralStore::updateCharacterHeader()` indexed
  `.first()/.at(0)` on a possibly empty party → segfault

**Verified:** tests [194]–[198] (1655 passed, 0 failed; +38 checks).
Non-vacuous:
- reverting `FOOD_TYPE` to 22 (the old amulet type) fails 13 checks — including
  "eating an amulet is refused" and "the amulet was not eaten"
- unregistering the Eat button fails "the dungeon has an Eat button"

### Slice 5.3 — Starvation death is a real death
- A starved member now goes through `DeathRecovery::killCharacter()` instead of
  `setDead()` — body location (`dungeonLevel/x/y`) is recorded, so the Morgue can
  find and resurrect the body
- A journal entry is written on a starvation death, matching a combat death
- `movePlayer()` checks `DeathRecovery::isPartyWiped()` after each step and routes
  to `handlePartyWipe()` — starving the whole party ends the run (sets `isAlive=0`,
  journals the wipe, saves bodies, closes the dungeon, returns to town)

**Verified:** tests [199]–[201] (1664 passed, 0 failed; +9 checks).
Non-vacuous:
- reverting to `setDead()` fails 4 checks — body X/Y/level + journal entry
- removing the wipe check fails "the party wipe was journaled"

### Slice 5.4 — One HP source of truth
- `updatePartyMemberHealth()` writes the live member (`getPartyMembers()[row].hp`)
  instead of the dead `CurrentCharacterHP` key; trap/pit/fall damage now reduces
  the HP the party panel actually shows
- On death: `DeathRecovery::killCharacter()` (body location + journal) and
  `isPartyWiped()` check — a trap that kills the last member ends the run
- `on_restButton_clicked()` and `castHealingSpell()` read/write the live member
- Removed the `CurrentCharacter*` special-casing in `setGameValue()` — it patched
  `m_gameStateData["Party"][0]`, a list shape the live party never uses

**Verified:** tests [202]–[206] (1672 passed, 0 failed; +8 checks).
Non-vacuous: reverting `updatePartyMemberHealth` to `CurrentCharacterHP` fails
3 checks (live HP reduced, panel shows reduced HP, source of truth).

### Slice 5.5 — Town resupply loop
- Tavern rest restores hunger (hours × 25) alongside HP and mana
- `gameStateManager::addStartingFoodToParty()` gives every living member a
  Rations stack so the first descent is not a guaranteed slow death
- General Store already stocks food (MDATA3 rows from slice 5.2)

**Verified:** tests [207]–[209] (1675 passed, 0 failed; +3 checks).
Non-vacuous: removing hunger restore fails "tavern rest restored hunger";
removing starting food fails "a new party starts with food".

### Slice 5.6 — Load a whole party
- `CharacterListDialog` uses `ExtendedSelection` with a real Select handler
  that loads every selected character (was: single-select with a stub)
- Save Party / Load Party buttons use the existing
  `savePartyToFile`/`loadPartyFromFile` JSON wrappers
- `addCharacterToParty()` enforces `MAX_PARTY_SIZE`

**Verified:** tests [210]–[214] (1689 passed, 0 failed; +14 checks).
Non-vacuous: reverting to `SingleSelection` fails "the character list allows
multi-select"; removing the `MAX_PARTY_SIZE` guard fails "the party size does
not exceed MAX_PARTY_SIZE".

### Slice 5.7 — Repo hygiene
- Removed `TheCity.cpp/.h/.qss`, `FontManager.h`, `StoryDialog.h`,
  `DataRegistry.h` (all referenced by 0 files)
- Removed `src/core/BattleEngine/` — 8 committed build artefacts (binary,
  `main.o`, `Makefile`, moc files, `.pro`, `main.cpp`)
- Added `data/characters/` to `.gitignore` so party-save files from slice 5.6
  do not litter the tree

**Verified:** tests [215]–[217] (1700 passed, 0 failed; +10 checks).
Non-vacuous: restoring `TheCity.cpp`/`.h` fails both dead-file checks.

---

# v4.0.0 — Endgame depth and replayability ✅ RELEASED

**Goal:** give the finished game real endgame depth — replayability after victory,
a reward loop that scales with each cycle, and a difficulty curve that a full run can
actually complete.

## What landed

### Slice 4.1 — New Game Plus
- `startNewGamePlus()` carries the party (level, stats, inventory, gold) into a new run
- `ngPlusLevel` persisted in game state (`NGPlusLevel`), save version bumped to 3
- Monsters scale +50% HP/damage per cycle; gold rewards scale with the cycle
- Hall of Records shows the NG+ cycle

### Slice 4.2 — Endgame dungeon: the Abyss
- 10-floor mega-dungeon unlocked after victory, with its own themes and boss floors (5, 10)
- `AbyssThemes`, `BossEncounter::buildAbyssBoss` (multiplicative tier × depth scaling)
- `Endgame::victoryTitle()` / `victoryParagraphs()` drive the victory screen

### Slice 4.3 — Arena mode
- 10 waves of escalating encounters (`Arena::monsterCountForWave`, `monsterLevelForWave`)
- Full combat UI in `ArenaDialog` (attack/defend/spell/flee), leaderboard persisted in game state
- Wired into the city via `theCity.h` arena button

### Slice 4.4 — Achievements
- 25 achievements across combat, depth, NG+, gold, death, level, arena, abyss
- `AchievementSystem` with `unlock()` / `isUnlocked()` / `checkAndUnlock()`, gold rewards
- Persisted as `UnlockedAchievements` in game state; Achievements button in Hall of Records

### Slice 4.5 — Balance pass
- **Core to-hit fix:** raw monster `def` (up to 312) was used as AC → floor 15 AC 749,
  impossible to hit (needed 744 on d20). Now `targetArmorClass()` compresses def to a
  bounded band: `10 + level/2 + qBound(0, def/12, 8)` (max +8).
- **Monster dex fix:** all 401 MDATA5 monsters have `StatDex = 0` → dead initiative.
  `EncounterBuilder` now derives `baseDex = 8 + 2 * baseLevel` when the CSV value is 0.
- **NG+ gold fix:** `calculateGold()` was called without `ngPlusLevel` → reward multiplier
  never applied. Now `DungeonDialog` passes `gsm->getNgPlusLevel()`.
- `MonsterBalance::floorHint()` shows recommended level + difficulty label on entering a floor
- `estimatedMinutesForFloor()` → full run ≈ 10–15 hours

### Slice 4.6 — Repo and docs cleanup
- Completed roadmaps (v0.1 → v3.0.0) and pre-1.0 analysis docs moved to `docs/archive/`
- Deleted generated file-listing dumps and the tracked `trash/` directory
- README links the current roadmap, test badge fixed (840 → 1590)
- FIXLIST: items 1, 4, 12, 15, 16 marked fixed after verifying in live code

## Test suite

| Version | Tests |
|---|---|
| v3.0.0 | 1522 + 3 |
| v4.0.0 (final) | 1590 + 3 |

## Non-vacuous verification

- **4.3:** disabling wave scaling (`return 1` in `monsterCountForWave`) fails 5 tests
- **4.4:** disabling `unlock()` (`if(true) return false`) fails 12 tests
- **4.5:** restoring the raw AC formula gives 8/200 hits (4%) and fails test [188];
  disabling monster dex derivation fails test [188]

## Known limitations

- Multiplayer is basic (city chat only); no multiplayer combat
- FIXLIST items 10 (SIGNAL/SLOT macros) and 21 (commented dead code) remain open

---

# v3.0.0 — Tactical combat and character growth

## Slice 3.1 — Combat target selection

**Problem:** `on_combatAttackButton_clicked()` always attacked the *first* living monster. `EncounterBuilder` produces real monster groups, and `CombatActions::attack(int targetIndex, ...)` already accepted a target — but the UI never passed one, so a 3-monster fight gave no way to focus a target down.

**Fix:**
- `m_combatTargetList` — a `QListWidget` in the combat panel, one row per living monster, each carrying its participant index in `Qt::UserRole`
- `selectedMonsterIndex()` / `selectFirstLivingMonster()` on `DungeonDialog`
- Attack uses the selected monster, falling back to the first living one so keyboard-only play still works
- The spell path casts at the selected monster too
- `updateCombatUI()` preserves the player's choice and moves the selection to a survivor when the chosen monster dies

**Verified:** tests [126]-[128] — attacking the third of three monsters damages the third and leaves the first untouched ✅; killing the selected monster moves the selection ✅; the no-selection fallback finds the first living monster ✅. Non-vacuous: restoring "take the first living monster" fails the targeting test.

**Tests:** 1333 + 3 passed, 0 failed (stable over 3 runs).

## Slice 3.2 — Per-monster combat display

**Problem:** The combat panel collapsed every monster into one `join(", ")` label, so a group fight read as a wall of numbers. Combat item use was hardcoded: it searched the inventory for a name containing "healing"/"health" and used the first match, so the player could never choose.

**Fix:**
- Each target row shows its own HP **and** status via `combatStatusText()` — poison/fire/blind/confused/stunned/slowed rounds plus "defending"
- The acting row is marked with `> ` so turn order is visible
- The item button opens a picker of the party member's actual consumables
- `Character::isConsumable()` is the single rule (potions 23, scrolls 24, tomes 25) — the same one `useConsumable()` enforces, so every listed item works

**Verified:** tests [129]-[131] — three monsters give three rows with independent HP, and damaging one changes only its row ✅; status reaches the row ✅; only real consumables are offered ✅. Non-vacuous: dropping the status from the row fails the row tests; accepting every item as consumable fails the consumable test.

**Tests:** 1352 + 3 passed, 0 failed (stable over 3 runs).

## Slice 3.3 — Front and back row

**Problem:** `CombatParticipant` had no row field, so `MonsterAI::chooseTarget()`'s "60% front row" was just "whoever is first in the list". The tactic was cosmetic and a caster could not be protected.

**Fix:**
- `enum class CombatRow { Front, Back }`, plus `row` and `ranged` on `CombatParticipant`
- `CombatState::reachablePlayerIndices(bool ranged)` is the single rule: melee sees the front row only, ranged sees everyone
- `MonsterAI::chooseTarget()` uses it — a melee monster can no longer pick the back row even when it is the weakest
- `EncounterBuilder` marks spellcasters and fire-breathers `ranged`
- Party members default to front (first two) / back (rest); the new **Row** button toggles the acting member
- The party line shows each member's row

**Verified:** tests [132]-[137] — melee reaches front only, ranged reaches both ✅; an all-back party blocks melee ✅; a melee monster never targets the back row over 200 decisions ✅; a ranged one does ✅; the Row button refuses monsters/invalid indices ✅; the party line shows rows ✅. Non-vacuous: ignoring the row in reachability fails four tests.

**Tests:** 1368 + 3 passed, 0 failed (stable over 3 runs).

## Slice 3.7 — Save versioning and migration

**Problem:** `SaveVersion` was written but never read. v3.0.0 adds fields (row, stat points,
play time) — exactly when a silent mismatch starts corrupting saves.

**Fix:**
- `src/core/savegameVersion.h` / `.cpp` — `CURRENT_SAVE_VERSION` (2), `MAX_LOADABLE_VERSION`,
  `versionOf()`, `isLoadable()`, `migrate()`
- `loadFullGameState()` reads the version, refuses newer saves with a `saveLoadRefused` signal
  and a Swedish user-facing message, then migrates older saves in order
- `migrateFrom1()` fills `StatPoints`, `PlayTimeSeconds`, `row` with safe defaults
  (matching `Character::toMap()`'s lowercase `"row"` so it survives a round-trip)
- `VERSIONING.md` created — documents the policy, the version table, and how to add a version
- `packStateForSaving()` writes `CURRENT_SAVE_VERSION` instead of hardcoded `1`

**Verified:** tests [138]-[141] + [139b] — SaveVersion written on save ✅; v1 save migrates
to current with defaults ✅; legacy save (no SaveVersion) migrates ✅; future-version save
is refused with a message ✅; in-memory version is current after migration ✅. Non-vacuous:
disabling migration fails [139b]; bumping MAX_LOADABLE_VERSION fails [141].

**Tests:** 1385 + 3 passed, 0 failed (stable over 3 runs).

## Slice 3.5 — Main quest chain driver

**Problem:** `QuestChain` computed steps and rewards but nothing advanced the chain or granted
rewards. The "main quest line" was a read-only screen.

**Fix:**
- `gameStateManager::setDungeonLevel()` tracks `DeepestFloor` live (never decreases)
- `src/core/QuestChainDriver.h` / `.cpp` — reads live state, grants rewards for newly
  completed steps, writes journal entries
- `handleVictory()` calls `QuestChainDriver::advance()` after every boss kill
- Victory gated on `questResult.chainJustCompleted` instead of hardcoded `level == 15`
- Rewards tracked via `QuestRewardGiven_<stepId>` keys to prevent double-granting

**Verified:** tests [148]-[153] — DeepestFloor tracks ✅; advance grants rewards ✅; no
double-grant ✅; chain not complete without Prince ✅; advance reports completion ✅;
progressText shows current step ✅. Non-vacuous: disabling reward granting fails [149]/[150].

**Tests:** 1422 + 3 passed, 0 failed (stable over 3 runs).

## Slice 3.6 — Play-time tracking and a real Hall of Records

**Problem:** `completionTimeSeconds` was hardcoded to 0 on victory, so the fastest-completion
category could never be set. Play time was not measured anywhere.

**Fix:**
- `gameStateManager::playTimeMs()` / `playTimeSeconds()` — live millisecond resolution
- `stopPlayTime()` called from `packStateForSaving()` so the persisted value is exact
- `PlayTimeSeconds` persisted in save and restored on load
- `completionTimeSeconds` on victory uses `gsm->playTimeSeconds()` instead of hardcoded 0
- `GameRecord::formattedTime()` formats as HH:MM:SS; `Endgame::outranks()` sorts correctly
- Hall of Records displays `formattedTime()` for Fastest Completion

**Verified:** tests [154]-[158] — playTimeMs non-zero after 50ms ✅; timer resets ✅;
play time survives save/load ✅; completionTimeSeconds populated ✅; faster run sorts
ahead ✅. Non-vacuous: forcing playTimeMs to 0 fails [154] and [155].

**Tests:** 1441 + 3 passed, 0 failed (stable over 3 runs).

## Slice 3.8 — Combat visual feedback

**Problem:** no on-hit feedback. Combat was readable only from the log — no flash/shake/particle
code existed outside Qt's own tool Makefiles.

**Fix:**
- `DungeonDialog::flashCombatFx(bool damage)` — flash combat panel red on damage, green on heal
- `combatFxEnabled()` — reads `CombatFxEnabled` from game state (default on)
- `combatGroupStyleSheet()` — test hook to read the flash state
- QTimer (200 ms) clears the flash automatically
- Hooked into `advanceCombat()`: monster AI hit → red flash

**Verified:** tests [159]-[161] — combatFxEnabled defaults to true ✅; flashCombatFx sets a
stylesheet ✅; stylesheet contains background-color ✅; stylesheet cleared after timer ✅;
flashCombatFx is a no-op when disabled ✅. Non-vacuous: disabling fx fails [160].

**Tests:** 1448 + 3 passed, 0 failed (stable over 3 runs).

## Slice 3.9 — Release hygiene

**Problem:** `FIXLIST.md` #20 was marked FIXED but the build artifacts were still tracked.
Every commit in v2.0.0 carried binary churn — a rebuilt `blacklands` and a regenerated
`Makefile` — making diffs unreadable and inviting merge conflicts.

**Fix:**
- `git rm --cached` removed `Makefile`, `blacklands`, `qrc_resources.cpp`, `.qmake.stash`,
  `bankpane/moc_BankPane.cpp`, `bankpane/moc_predefs.h`, `data/saves/autosave.json`
- `.gitignore` extended to cover them

**Verified:** `qmake6 -o Makefile blacklands.pro && make` works from a clean checkout,
`make check` green, `git status` clean after build.

---

# v4.0.0 — IN PROGRESS

**Major — "Endgame Depth and Replayability": extend the game past victory.**

## Slice 4.1 — New Game Plus

**Problem:** VictoryDialog had a "New Game Plus" button but it did nothing — no party carry-over, no difficulty scaling, no NG+ counter.

**Fix:**
- `gameStateManager::startNewGamePlus()` — increases NG+ level, resets dungeon/quest/playtime, keeps party (level, stats, equipment, gold)
- `getNgPlusLevel()` / `setNgPlusLevel()` — read/write NG+ level in game state
- `EncounterBuilder` — accepts `ngPlusLevel`, scales monsters with `ngPlusMonsterMultiplier` (+50% per cycle)
- `VictoryReward` — uses `ngPlusRewardMultiplier` (+25% per cycle)
- Save versioning: `CURRENT_SAVE_VERSION` bumped to 3, migration handles NGPlusLevel
- NGPlusLevel persists through save/load

**Verified:** tests [162]-[166] — NG+ level increments ✅; party level and gold preserved ✅; NGPlusLevel persists through save/load ✅; NGPlusLevel defaults to 0 for old saves ✅.

**Tests:** 1460 + 3 passed, 0 failed.

## Slice 4.2 — Endgame dungeon: The Abyss

**Problem:** after beating the final boss, there was nowhere left to go. No post-victory content.

**Fix:**
- `AbyssThemes` — 10-floor post-victory mega-dungeon with unique themes
- Boss floors 5 and 10 with `Abyssal Maw` and `The Abyssal Heart`
- `BossEncounter::buildAbyssBoss()` — multiplicative scaling (floor 10 significantly harder than floor 5)
- Abyss boss XP far above main dungeon bosses
- Version bumped to 4.0.0

**Verified:** tests [167]-[170] — Abyss has 10 floors ✅; boss floors are 5 and 10 ✅; floor 10 boss has >2x HP of floor 5 ✅; Abyss floor 5 boss XP > main floor 15 boss XP ✅.

**Tests:** 1478 + 3 passed, 0 failed.

## Slice 4.3 — Arena / challenge mode

**Problem:** combat is now tactical, but there is no way to test your build against increasing difficulty outside the main quest.

**Fix:**
- `Arena` — 10 waves of monsters, each harder than the last
- Score based on speed, damage taken, and waves cleared
- Leaderboard stored in game state (`ArenaRecords`)
- `ArenaDialog` — full combat UI with attack/defend/spell/flee
- Arena button in `theCity` (accessible from town)

**Verified:** tests [171]-[176] — monster count scales with wave ✅; monster level scales with wave and NG+ ✅; wave score rewards speed and low damage ✅; total score sums wave scores ✅; leaderboard sorts by score descending ✅; arena is accessible from town ✅.

**Tests:** 1494 + 3 passed, 0 failed.

## Slice 4.4 — AchievementSystem

**Problem:** there were no persistent milestones. Achievements give players long-term goals and a sense of progression beyond leveling.

**Fix:**
- 25 achievements across combat, depth, NG+, gold, death, level, arena, abyss
- `unlock()` / `isUnlocked()` / `checkAndUnlock()` with gold rewards
- Persisted in game state (`UnlockedAchievements`)
- Achievements button in Hall of Records Dialog

**Verified:** tests [177]-[181] — at least 20 achievements ✅; unlock and isUnlocked ✅; double unlock returns false ✅; unlock grants gold reward ✅; checkAndUnlock based on game state ✅; second checkAndUnlock unlocks nothing ✅; name/description/reward lookups ✅.

**Tests:** 1522 + 3 passed, 0 failed.

## Slice 4.5 — Balance pass

**Problem:** the full run had never been playtested end to end. Two real bugs fell out of modelling it:

1. **Deep floors were literally impossible to hit.** The to-hit formula used the raw monster `def` column directly as armor class. That column runs past 300 (312 on floor 15), so a floor-15 monster had AC 749 — a party would need a **744 on a d20** to land a blow. The reverse was just as broken: every one of the 401 monster rows has Dexterity `0`, so monsters had a dead to-hit and initiative stat and could never hit back.
2. **New Game Plus was all cost and no reward.** `handleVictory()` called `calculateGold()` without the NG+ cycle, so the reward multiplier never applied while monsters still got +50% per cycle.

**Fix:**
- `CombatActions::attackerSkill()` / `targetArmorClass()` — both sides of the to-hit roll compressed into a bounded d20 band (level as the backbone, capped dex/def bonuses). Calibrated to a ~55-85% hit rate at every depth, exposed as testable helpers.
- `EncounterBuilder` — derives a monster Dexterity from its level (the table has none), so monsters act and land blows.
- `MonsterBalance` — `floorHint()` (recommended level + difficulty), a winnability model (`isFloorWinnable`/`isRunWinnable`), and a run-length estimate (`estimatedRunMinutes`, `isRunLengthInTarget`).
- `DungeonDialog` — shows the recommended-level hint on entering a floor, and passes the NG+ cycle to `calculateGold()`.

**Verified:** tests [182]-[188] — floor hints name the right level ✅; every floor is winnable at its recommended level ✅; a badly under-leveled party is not (non-vacuous) ✅; difficulty rises monotonically with depth ✅; the estimated run lands in the 10-15 hour band ✅; a level-15 hero hits a floor-15 monster ≥30% of the time (was 4%) ✅. Reverting either fix fails the suite.

**Tests:** 1590 + 3 passed, 0 failed.

## Slice 4.6 — Repo and docs cleanup

**Problem:** the repo had accumulated a decade of roadmaps and analysis docs. `docs/` mixed the current plan with v0.1-era reports, four generated file-listing dumps, and a tracked `trash/` directory holding stale `GameStateManager` copies.

**Fix:**
- Completed roadmaps (v0.1 → v3.0.0) and pre-1.0 analysis docs moved to `docs/archive/`; only the current roadmap stays in `docs/`.
- Deleted the generated file-listing dumps and the tracked `trash/` directory.
- README now links the current roadmap, the test badge says 1590 (was 840), and the endgame systems are in the feature table.
- `FIXLIST.md`: items 1 (Lua RCE), 4, 12, 15, 16 marked fixed after verifying them in the live code; 10 and 21 kept open; review date refreshed.
- `VERSIONING.md`: completed roadmaps moved under `docs/archive/`, v3.0.0 and v4.0.0 added to the version history.

**Verified:** README links the latest roadmap ✅; old roadmaps are in `docs/archive/` ✅; `FIXLIST.md` reflects live code ✅; no source or build file references a moved doc ✅; build and all tests still green ✅.

**Tests:** 1590 + 3 passed, 0 failed.

---

| Phase | Version | Level | Content |
|---|---|---|---|
| Phase 0 | **v0.1** | Pre-release | Security and latent breakage |
| Phase 1 | **v1.0.0** | Major | Connect the orphaned systems — makes the game finishable |
| Phase 2 | **v1.1.0** | Minor | Combat reachability and depth |
| Phase 3 | **v1.1.1** | Patch | Debt and hygiene |

One commit per version boundary. Each phase lands as a release, not as a batch of
mid-development commits.

---

# v2.0.0 — IN PROGRESS

**Major — "The Real Game": wire existing systems into the core loop.**

## Slice 2.1 — Combat auto-start on hostile encounter

**Problem:** stepping onto a monster only logged a line. Combat began only if the player
then pressed Fight while standing on the tile. `initiateFight()` was an empty stub.

**Fix:**
- Extracted `startCombatAt(pos)` from `on_fightButton_clicked` so both the button and the
  encounter handler share the same combat setup path
- `handleEncounters` now calls `startCombatAt` for Hostile monsters
- Neutral and friendly monsters do not trigger combat

**Verified:** test [73] drives the real `handleEncounters` path — hostile starts combat,
neutral does not, empty tile does not. Non-vacuous: 2 FAIL without the auto-start.

**Tests:** 1176 → 1181 passed, 0 failed.

## Slice 2.1 (cont.) — Flee semantics and spell routing

**Problem:** fleeing closed combat but left the monster on the map, so stepping onto the
same tile restarted combat immediately. Spell damage poked participant HP directly instead
of routing through CombatActions (no death bookkeeping, no status effects).

**Fix:**
- `fleeCombat()` extracted from `on_combatFleeButton_clicked`; removes the monster from
  the map on successful flee
- Spell damage flows through `applySpellDamageBySchool` so death handling and school
  mechanics (fire AoE, cold slow, lightning chain, mind stun) are respected
- Test [74] drives the real flee path. Non-vacuous: 1 FAIL without the monster removal.
- Test [75] verifies spell damage routing.

**Tests:** 1181 → 1189 passed, 0 failed.

## Slice 2.2 — Equipment guild restriction

**Problem:** `equipItem()` checked stat requirements and slot collisions but ignored
the `guilds` bitmask from MDATA3. Any character could equip any item regardless of
guild membership.

**Fix:**
- `equipItem()` now checks guild bitmask: character must be a member of at least
  one of the required guilds
- Equipment guild-restriction test moved to `test/equipment_test.cpp` so guild
  membership doesn't leak into other tests' characters
- Existing equip tests updated to join the required guild first

**Verified:** test [76] in `equipment_test.cpp` — cannot equip guild-restricted item
without guild, can equip after joining. Non-vacuous: 2 FAIL without the check.

**Tests:** 1189 + 3 passed, 0 failed.

## Slice 2.3 — XP and leveling

**Problem:** XP was already wired via PartyManager (`handleVictory()` → `addExperienceToParty()` → `LevelTable`), but spell learning, guild experience, and guild-based level-up bonuses were missing.

**Fix:**
- `applyLevelUpGains()` now calls `SpellBook::newlyLearned()` for each guild the character belongs to
- `handleVictory()` awards guild experience to all living party members
- Guild-based level-up bonuses: Warriors/Paladins get +2 HP, Mages/Wizards get -1 HP but +2 mana, Healers get +1 mana
- Guild-based stat bonuses: Warriors get +1 STR every 3 levels, Mages get +1 INT every 3 levels

**Verified:** tests [77]-[80] — XP divided among living members only ✅; level-up increases MaxHP/MaxMana ✅; mage learns new spell on level-up ✅; guild experience awarded on victory ✅.

**Tests:** 1198 + 3 passed, 0 failed.

## Slice 2.4 — Loot drops

**Problem:** `VictoryReward::calculateLoot()` rolls on Item0-Item9 from MDATA5, filters by
floor, returns item names. But drops are never given to the player. Monsters die and give
nothing but XP and gold.

**Fix:**
- `handleVictory()` now adds loot items to the first living party member's inventory
- Items start as unidentified (`identified = false`)
- Loot is logged in the combat result message

**Verified:** tests [81]-[83] — loot item added to inventory ✅; loot item is unidentified ✅;
VictoryReward returns loot for known monsters ✅; loot filtered by dungeon depth ✅.

**Tests:** 1202 + 3 passed, 0 failed.

## Slice 2.5 — Death flow

**Problem:** CombatDeathHandler detected party member death in combat. MorgueDialog could resurrect, hire bodies, and grab bodies. But death in combat didn't reach the Morgue — dead characters just stayed in the party with 0 HP.

**Fix:**
- `syncCombatToGameState()` now calls `DeathRecovery::killCharacter()` when a party member dies, recording body location
- `handlePartyWipe()` saves all dead characters to file so the Morgue can find them
- `DeathRecovery` provides resurrection cost (scaled by level and location), party wipe detection, rescue party cost, and body carrying

**Verified:** tests [84]-[89] — killCharacter marks dead and records location ✅; resurrection cost scales with level ✅; resurrection deducts gold and revives ✅; party wipe detection ✅; rescue party cost scales with depth ✅; body carrying and bringing to town ✅.

**Tests:** 1224 + 3 passed, 0 failed.

## Slice 2.6 — Dungeon persistence

**Problem:** `DungeonLevelRegistry` stores level state (monsters, treasures, obstacles, torch
fuel). But levels are regenerated on every visit. When you leave floor 3 and return, it's a
new floor 3.

**Fix:**
- `collectedItems` and `unlockedDoors` added to `LevelSnapshot` serialization
- `packStateForSaving()` already saves `DungeonLevels` to save file
- `unpackStateAfterLoading()` already restores them
- `enterLevel()` already restores from registry and respawns 30% of monsters

**Verified:** tests [90]-[94] — LevelSnapshot serialization round-trip ✅; registry store/retrieve ✅;
respawnMonsters adds monsters back ✅; respawnMonsters skips with empty pool ✅; registry
serializes to map and back ✅.

**Tests:** 1254 + 3 passed, 0 failed.

## Slice 2.8 — Status effects in combat

**Problem:** `GameConstants` defines Poison, Blind, Confusion, On Fire. `CombatActions` handles
Confusion (25% chance to hit ally) and Blind (to-hit penalty). But Poison DoT is not
applied, and no monster or spell inflicts these statuses.

**Fix:**
- `tickStatusEffects()` applies poison DoT (1-3 damage/round), fire DoT (2-5 damage/round), blind duration, confusion duration
- `applyStatus()` sets status flags with duration
- Monster fire breath sets players on fire
- Poison kills at 0 HP

**Verified:** tests [99]-[104] — poison DoT applied each round ✅; blind reduces to-hit and expires ✅; fire DoT and expires ✅; confusion causes friendly fire ✅; monster fire breath sets on fire ✅; poison kills at 0 HP ✅.

**Tests:** 1265 + 3 passed, 0 failed.

## Slice 2.9 — Town wiring

**Problem:** `TavernDialog` and `QuestBoardDialog` both exist and are functional. But they are not reachable from `GameMenu`.

**Fix:**
- `theCity` already has Tavern and Quest Board buttons wired (`on_tavernButton_clicked`, `on_questBoardButton_clicked`)
- Tavern restores HP/mana, cures status effects, advances time
- Quest Board: accept quests, progress via reportKill/reportFetch, turn in for rewards

**Verified:** tests [105]-[107] — quest accept and complete ✅; tavern rest restores HP/mana ✅; tavern cures poison and blindness ✅.

**Tests:** 1275 + 3 passed, 0 failed.

## Slice 2.10 — Item identification

**Problem:** Items found in the dungeon should be unidentified. The General Store has an ID cost mechanic. But items never spawn as unidentified — they always have their real name.

**Fix:**
- Loot drops already start as unidentified (`identified = false`)
- GeneralStore already has `identifySelectedItem()` and `uncurseSelectedItem()`
- `GoldSinks::identificationCost()` and `GoldSinks::uncurseCost()` provide costs

**Verified:** tests [108]-[111] — loot items start unidentified ✅; identify renames item ✅; uncurse removes cursed flag ✅; identification cost from GoldSinks ✅.

**Tests:** 1281 + 3 passed, 0 failed.

## Slice 2.11 — Bestiary auto-population

**Problem:** `BestiaryDialog` loads all 401 monsters from `bestiary.json` from the start. The party sees every monster's stats and portrait before ever meeting one.

**Fix:**
- `recordEncounter(name)` / `isEncountered(name)` / `encounteredCount()` / `resetEncounters()` / `displayName(name)` on `BestiaryDialog`
- `handleEncounters()` records the monster when the party steps on its tile
- The list shows "???" (grey) for unmet monsters, with no tooltip stats
- `showEntry()` shows no portrait and no numbers for unmet monsters

**Verified:** tests [112]-[115] — bestiary empty at start ✅; encountering unlocks the entry ✅; `handleEncounters` records through the real path ✅; stats only for met monsters ✅.

**Fixed along the way:** test [104] ("poison kills at 0 HP") was flaky — poison deals 1-3 damage and the player had 2 HP, so it only died on a roll of ≥2. Now 1 HP, which any tick kills. Verified stable over 3 consecutive runs.

**Tests:** 1292 + 3 passed, 0 failed.

## Slice 2.12 — Journal wiring

**Problem:** `JournalDialog` has a full UI (add note, filter, clear, exit) and a working static `addEntry()`. But nothing in the game ever called it — the journal stayed empty.

**Fix:** journal entries now written from the real event paths:
- `handleVictory()` — Combat: "Defeated X on floor N (+xp, +gold)", plus a separate entry for bosses
- `handlePartyWipe()` — Combat: "The party was wiped out on floor N"
- `syncCombatToGameState()` — Combat: "<name> fell in battle on floor N (x,y)"
- `PartyManager::applyLevelUpGains()` — Combat: "<name> reached level N!"
- `QuestBoardDialog::acceptQuest()` / `turnIn()` — Quest: accepted / completed with reward
- `GeneralStore::identifySelectedItem()` / `uncurseSelectedItem()` — Exploration: fee paid

Also added `JournalDialog::allEntries()` and `clearAll()` so tests can read back and reset.

**Verified:** tests [116]-[119] — quest accept writes an entry ✅; turn-in writes a Completed entry ✅; victory writes a Combat entry ✅; level-up writes an entry ✅. Non-vacuous: removing the victory and level-up calls fails exactly those two tests.

**Tests:** 1297 + 3 passed, 0 failed.

**Fixed along the way:** test [45] (the original journal test) asserted on absolute file contents; earlier tests now write real journal entries, so it clears the file first.

## Slice 2.13 — Gold sinks wiring

**Problem:** `GoldSinks` is the single authority for every gold charge, but three call sites bypassed it: the Morgue's "Raise Character" was completely free, and the Tavern hard-coded `10` per hour and `50` per cure instead of reading the GoldSinks rates. (Identify/uncurse, guild leveling, rescue and rest already went through GoldSinks.)

**Fix:**
- `MorgueDialog::raiseCost(level, bodyInCity)` → `GoldSinks::resurrectionCost`; "Raise Character" now checks funds, confirms the fee, and deducts it before reviving
- `TavernDialog::restCost(hours, livingMembers)` → `hours * GoldSinks::restCostPerHour() * living`
- `TavernDialog::cureCost(poison, blindness)` → sums the two GoldSinks cure rates
- Both `onRestClicked()` / `onCureClicked()` now call those helpers

**Verified:** tests [120]-[123] — rest cost uses the GoldSinks rate and scales with members ✅; cure cost uses both GoldSinks rates ✅; morgue raise cost is 2500 in town / 3000 in the dungeon ✅; `DeathRecovery::resurrect` deducts exactly the cost and deducts nothing when gold is short ✅. Non-vacuous: changing the GoldSinks rest rate while the Tavern hard-codes the old value fails both rest-cost tests.

**Tests:** 1316 + 3 passed, 0 failed.

## Slice 2.14 — Aging consequences

**Problem:** `AgingRules` was already wired into `processAgingConsequences()` (Phase 0 work), but two consequences were missing: an old-age death only went to `qDebug()` (no journal entry), and the body was never written to a character file, so the Morgue could never find it.

**Fix:**
- `processAgingConsequences()` writes an Exploration journal entry on death by old age
- It then saves the dead character's file so the Morgue can recover the body
- **Latent bug fixed:** `saveCharacterToFile()` read `m_gameStateData["Party"]` as a bare list, but `refreshUI()` writes it as a map with a `Members` key — so after any `refreshUI()` the save silently did nothing. It now accepts both shapes.

**Verified:** tests [124]-[125] — death by old age writes a journal entry ✅; the dead character's file is written and marked `isAlive: 0` ✅. Non-vacuous: removing the wiring fails both tests.

**Tests:** 1320 + 3 passed, 0 failed.

---

# v2.0.0 — Core loop complete

All 14 slices are done. The core loop **Fight → Loot → Equip → Level → Descend Deeper** is wired end to end.

| Slice | Status |
|---|---|
| 2.1 Combat auto-start and flee semantics | ✅ |
| 2.2 Equipment system | ✅ |
| 2.3 XP and leveling | ✅ |
| 2.4 Loot drops | ✅ |
| 2.5 Death flow | ✅ |
| 2.6 Dungeon persistence | ✅ |
| 2.7 Monster spellcasting | ✅ |
| 2.8 Status effects in combat | ✅ |
| 2.9 Town wiring | ✅ |
| 2.10 Item identification | ✅ |
| 2.11 Bestiary auto-population | ✅ |
| 2.12 Journal wiring | ✅ |
| 2.13 Gold sinks wiring | ✅ |
| 2.14 Aging consequences | ✅ |

**Test suite:** 1320 + 3 passing, 0 failing.

---

# v1.0.1 — 2026-10-10 ✅ RELEASED

**Patch — three defects found by actually playing the game.** No new features.

## 1. Autosave silently failed after the first save

`gameStateManager::saveFullGameState()` wrote to a `.tmp` file and then called
`QFile::rename()`. Qt's `rename` **refuses to overwrite an existing destination**, and the
periodic autosave targets the same path every 30 seconds — so the first save of a session
succeeded and every later one failed, logging only `Failed to rename save file`. Progress
was lost without any visible error.

Fixed by switching to `QSaveFile`, which writes to a temporary file and atomically replaces
the target on `commit()`. The existing test suite missed this because it deleted the save
file at the end of each run; the new `[4b]` test saves three times in a row over the same
file and asserts the latest write wins.

## 2. Dungeon action buttons were crushed together

**Root cause:** `MainMenu.qss` pins every `QPushButton` to `min-width: 200px; max-width: 200px`.
This rule is inherited through `GameMenu → theCity → DungeonDialog`. Three 200px columns
cannot fit in a 320px sidebar, so the 14 action buttons overlapped each other 13 times and
read as a single mashed pile. At the roomy default size (1280x800) it looked fine, which is
why the original layout test missed it.

**Fix:** `DungeonDialog` now applies its own `setStyleSheet` override that relaxes the width
constraint (`min-width: 0; max-width: 999px`). The sidebar was widened from 320px to 360px,
button minimum size reduced from 80×32 to 60×26, section spacing from 15px to 8px, grid
spacing from 6px to 4px, minimap from 150×150 to 100×100, and party status list from
100px to 60px. All content now fits without a scrollbar at any window size.

`[63b]` now applies `MainMenu.qss` on an ancestor widget (real CSS inheritance), checks at
1280×800, 900×640, and 1636×1070, and includes both pairwise overlap and content-overflow
checks. Non-vacuous: 3 FAIL without the fix.

## 3. Main-menu background never loaded from `build/bin/`

`GameMenu::loadBackgroundImage()` reads the background via `applicationDirPath()`. The
1024x535 image sits at the repo root, which serves the root-level binary but not
`build/bin/blacklands` — so the deployed binary logged
`FATAL: Could not load background image` and painted a blank menu.

Fixed in `blacklands.pro`: the post-link step now copies `introtitle.png` into `DESTDIR`
alongside `data/` and `resources/`.

> **Correction:** an earlier attempt "fixed" this by pointing the path at
> `resources/images/introtitle.png`. That is a different, 400x100 placeholder sprite, and
> the change broke the background for *both* binaries. The path was correct; the missing
> file copy was the bug. Reverted.

---

## Verification

- `make check` → **1176 passed, 0 failed**, orphaned-system guard PASS.
- Non-vacuous: reverting the `QSaveFile` fix → `[4b]` reports 3 FAIL; reverting the
  stylesheet override → `[63b]` reports 3 FAIL (one per window size).
- Live: two consecutive autosaves both log `Full game state saved`; no
  `Could not load background image` on startup; window title reads `Blacklands v1.0.0`.

---

# v0.1 — 2026-10-10 ✅ RELEASED

**Phase 0 — security and latent breakage.** Small, independent fixes for real defects that
were already in the tree. Scope: `docs/archive/ROADMAP_v0.1.md`.

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
shipped as tested logic that nothing calls. The audit in `docs/archive/ROADMAP_V1.1.md` found six
such systems (`Endgame`, `MonsterBalance`, `GoldSinks`, `AgingRules`, `ItemProgression`,
`SpellMechanics`), each with zero callers outside its own translation unit. Phase 0 and
Phase 1 of the current roadmap exist to close that gap.

---

## Earlier development

Pre-1.0 work is in the git history and in `docs/`:

- `docs/archive/ROADMAP_V1.md` — the eight-phase plan to v1.0, with per-slice *Why* / *Done* / *Verify*.
- `docs/FIXLIST.md` — 21 defects found by audit, with locations and fixes.
- `docs/archive/GAME_COMPLETENESS_ANALYSIS.md` — what the game has and what it was missing.
- `docs/archive/COMPLETE_REFACTOR_REPORT.md` — the refactor that preceded the roadmap.

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
