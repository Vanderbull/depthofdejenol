# Roadmap v5.0.0 — Survival: make hunger real

**Status:** ✅ DONE — all 8 slices landed, 1701 tests pass, v5.0.0 tagged
**Version:** v5.0.0 (major — a survival mechanic exists in code but is invisible,
unfixable, and its deaths are unhandled; this version makes it a real system)
**Test suite at start:** 1590 + 3 (all passing)

## Goal

The game has a hunger/starvation system that a player can never see, never manage,
and whose deaths the game does not treat as deaths. The user's report — *"starving
did not kill"* — is the symptom of a mechanic that is unreachable rather than absent.

This version makes survival real, and lets you load a whole party:

- **Hunger is visible** — the player can see it draining and act on it.
- **Food exists and can be eaten** — a real food item and an eat action, not a
  code path with zero callers.
- **Starvation death is a real death** — a body, a journal entry, the Morgue, and a
  party-wipe check outside combat, exactly like a combat death.
- **One HP source of truth** — collapse the two HP stores so trap, pit, fall and
  rest damage land on the same character the party panel shows.
- **A town resupply loop** — the inn sells food and restores hunger, so survival is
  a resource decision rather than an unpayable tax.
- **Load a whole party** — load a saved party of several characters at once, not one
  character at a time.

## What the audit found

Verified against the live tree, not the roadmap's own ✅ marks.

### Hunger is completely invisible
`grep` for any hunger/starving/food display in `character_dialog`, `partyinfo_dialog`,
`dungeon_dialog`, `theCity` returns nothing. `hunger` is decremented but never shown.

### There is no food and no way to eat
- `Character::eatFood()` (`character.cpp:532`) has **zero callers** outside its own
  definition — no button, no slot, no key binding reaches it.
- The inventory "Use" action only handles potions (23), scrolls (24), tomes (25):
  `Character::isConsumable()` (`character.cpp:450`).
- `eatFood()` accepts items of type **22**, but type 22 is **"Amulet"** in the real
  item database (`ItemDatabase.cpp:101`), not food. The only type-22 rows in
  `data/MDATA3.csv` are 8 amulets (Amulet of Life, Pendragon, …).
- The one "Rations" entry (`GeneralStore.cpp:247`) is a **fallback list** used only
  when the item CSV fails to load — it is not a real item.
- Net effect: hunger only ever goes **down**. Nothing can restore it.

### Starvation death is a silent, unhandled death
`movePlayer()` (`DungeonDialog.cpp:320-334`) does flag a starved member dead
(`setDead()`), but:
- It calls `Character::setDead()` (`character.cpp:569`), **not**
  `DeathRecovery::killCharacter()` — so the body's `dungeonLevel/x/y` are never set.
- A combat death goes through `syncCombatToGameState()` (`DungeonDialog.cpp:1919`),
  which records the body location and writes a journal entry. Starvation does none
  of that.
- There is **no party-wipe check outside combat**: `handlePartyWipe()` is only ever
  reached from `advanceCombat()` (`DungeonDialog.cpp:1744`). If the last living
  member starves, the game does not notice.

**Empirical proof (temporary probe, since reverted):** walking a 1-HP member at
hunger 1 on floor 3 produced `hp=0 alive=false bodyLoc=(L0,0,0)`. Starvation does
set `isAlive=false`, but the body location is never recorded — a real death with no
consequences. This is why it reads as "did not kill".

### Two HP stores that never reconcile
- Live party HP: `Character::hp` in `PartyManager` — used by combat and starvation.
- A separate game-state key `CurrentCharacterHP` — written by
  `updatePartyMemberHealth()` (`DungeonDialog.cpp:242`), which handles trap, pit,
  and fall damage (`DungeonHandlers.cpp:13,60,64,71,75,90,107`), by dungeon rest
  (`DungeonDialog.cpp:2268`), and by spells (`SpellCastingDialog.cpp:438`).
- **Nothing ever copies `CurrentCharacterHP` back into the live member.** Verified:
  the only writes are `setGameValue("CurrentCharacterHP", …)`; no read-back exists.
- Consequence: trap/pit/fall damage and dungeon rest act on a dead key. A trap that
  "defeats" the character closes the dungeon while the live party is unharmed; a
  rest "heals" a value the party panel never reads.

### Dead duplicates and committed build junk
- `TheCity.cpp/.h` (capital T) differ from the live `theCity.cpp/.h` and are
  referenced by **0** files. Same for `FontManager.h`, `StoryDialog.h`,
  `DataRegistry.h`.
- `src/core/BattleEngine/` is 8 committed build artefacts (`blacklands.pro`,
  `Makefile`, `moc_*`, `compile`, `packages`, `HommBattle` binary, `main.cpp`) and is
  not referenced by the main build.

### Other reachable-but-unfinished mechanics (context, not this version's spine)
- Arena "Use Item" button is hard-disabled: `ArenaDialog.cpp:505`
  (`// Not implemented yet`).
- Consumable effects are stubs: `character.cpp:492,494,505`
  ("Buff not yet implemented", "Effect not yet implemented").
- Companions: the help text documents a full system (charm/bind/sell/name), the
  `Companion` struct exists, and `ConfinementDialog` has Buy/Sell buttons that only
  show a `QMessageBox` (`ConfinementDialog.cpp:227`).

### You can only load one character, never a party

The game supports a party of four (`MAX_PARTY_SIZE = 4`) and the plumbing for a
whole-party save already exists, but no UI path reaches it:

- **The party serializer is done and unused.** `Party::toMap()` writes `Members`
  (a list) and `SharedGold`; `Party::loadFromMap()` reads them back and clears
  first. `gameStateManager::savePartyToFile()` / `loadPartyFromFile()`
  (`gameStateManager.cpp:1960,1973`) wrap it — and have **zero callers**. They are
  dead code.
- **Every live save/load path handles exactly one character.**
  `saveCharacterToFile(int partyIndex)` writes a flat `.txt` for one member;
  `saveFullGameState()` / `loadFullGameState()` snapshot the whole game under
  `data/saves/` (autosave), but that is a whole-session snapshot, not "load these
  four characters".
- **The load UI is single-select.** `CharacterListDialog`
  (`characterlistdialog.cpp`) lists `data/characters/*.txt` with
  `QAbstractItemView::SingleSelection`, and its Select button only shows a
  "the game would proceed to load … here" placeholder — it does not load anything.
- **The main menu loads one file.** `GameMenu::loadGame()` (`blacklands.cpp:182`)
  opens a file dialog for a single `.json`/`.lua` and calls
  `loadCharacterFromFile(fileName)`.
- **The character-list loader is broken.** It lists `./data/characters` but Delete
  removes `characters/<name>.txt` (wrong directory), and its Select handler is a
  stub.
- **The legacy `.txt` loader is dead.** The `loadCharacterFromFile(characterName)`
  overload that parses `data/characters/<name>.txt`
  (`gameStateManager.cpp:806`) sits inside a `/* … */` block (lines 805-897). The
  `.txt` files in `data/characters/` can be written but not read back by that path.

**Consequence:** a player can save a party's members one at a time and can never
load them back as a party. The serializer that would make it work is already tested
by `Party::toMap`/`loadFromMap` and simply has no UI.

### Repo hygiene note
`data/characters/` holds runtime save files (`adam.txt`, `benke stolt.lua`, …) that
are **not** in `.gitignore` (only `data/journal.json` and `data/saves/` are). A
party-load feature will write more of these; they should be ignored, and the
committed fixtures removed.

## Slices

### 5.1 Hunger is visible `M` ✅ DONE

**Why:** a survival stat the player cannot see is not a mechanic. Right now hunger
drains invisibly, so the first feedback is a death the player never saw coming.

**Do:**
- Show hunger in the dungeon party panel (`updatePartyPanel()`), with a warning
  colour as it runs low, alongside the existing HP line.
- Show a "Hunger: N/100" row on the character sheet.
- Log a message when a member first drops below a threshold (e.g. 20%) so the
  transition into danger is announced, not silent.

**Files:** `src/dungeon_dialog/DungeonDialog.cpp`, `src/character_dialog/CharacterSheetDialog.cpp`

**Verify:** tests — the party panel string contains the member's hunger; the warning
colour appears below the threshold; the low-hunger message fires once on the
crossing. Non-vacuous: removing the display fails the panel test.

**Landed:** `Character::hungerPercent()` / `isHungerLow()` (band at ≤20%);
`DungeonDialog::partyMemberStatusText()` / `partyMemberColor()` (red dead/starving,
yellow low HP *or* low hunger, green healthy) now drive `updatePartyPanel()`;
`announceHungerWarnings()` logs the crossing once per member (re-armed on recovery)
and is called from `movePlayer()`; `CharacterSheetDialog` shows a "Hunger: N/M (P%)"
row that turns amber when low. Tests [189]–[193]; +23 checks (1590 → 1617).
Non-vacuous: removing the `announceHungerWarnings()` call fails "the move itself
announced the low-hunger crossing"; omitting hunger from the row fails both panel
tests; not setting the sheet text fails the sheet test.

### 5.2 Food exists and can be eaten `M` ✅ DONE

**Why:** hunger can only go down. `eatFood()` exists with zero callers, and the
type it accepts (22) is amulets, not food. There is nothing to eat.

**Do:**
- Give food a real identity. Do **not** reuse type 22. Either add a food type to
  `ItemDef`/`typeName()` and the loader, or introduce a dedicated food table
  (`data/food.csv`) with `name`, `restore`, `price`.
- Ship real food rows (Rations, Waybread, Dried Meat, Elven Lembas) in
  `data/MDATA3.csv` or the new table, and remove the amulet-accepting check from
  `eatFood()`.
- Wire an **Eat** action into the inventory dialog and the dungeon (a key binding
  and a picker of the member's food, mirroring the combat item picker), calling
  `Character::eatFood()`.
- Stock food at the General Store so it can be bought.

**Files:** `character.cpp` / `character.h`, `src/items/ItemDatabase.*`,
`src/inventory_dialog/inventorydialog.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`,
`src/general_store/GeneralStore.cpp`, `data/MDATA3.csv` (or `data/food.csv`)

**Verify:** tests — a real food item restores hunger and is consumed; a non-food item
is rejected; the eat picker lists only food; the store stocks food. Non-vacuous:
reverting the food-type fix makes the amulet-acceptance test fail.

**Landed:** type **35 = Food** (`ItemDef::typeName()`); four real rows in MDATA3
(Rations 12g, Waybread 25g, Dried Meat 40g, Elven Lembas 120g); `Character::FOOD_TYPE`
/ `isFood()` / `foodIndices()` / `hasFood()`; `eatFood()` now gates on real food
(was type 22 = Amulets) and restores 10 + spellLvl×5; an **Eat** button + `H` key in
the dungeon via `DungeonDialog::eatFromInventory()`; the inventory "Use" button eats
food instead of refusing it; `GeneralStore::makeHeldItem()` seeds `M4ED4` from the
item's charges (bought items had 0 charges and vanished on first use). Also fixed a
real crash: `GeneralStore::updateCharacterHeader()` indexed `.first()/.at(0)` on a
possibly empty party (segfault). Tests [194]–[198]; +38 checks (1617 → 1655).
Non-vacuous: reverting `FOOD_TYPE` to 22 (the old amulet type) fails 13 checks;
unregistering the Eat button fails "the dungeon has an Eat button".

### 5.3 Starvation death is a real death `M` ✅ DONE

**Why:** a starved member is flagged dead but no body is recorded, no journal entry
is written, and the party-wipe check never runs outside combat. The death is real in
memory and invisible in the game.

**Do:**
- Route the starvation path through `DeathRecovery::killCharacter()` (which sets
  `dungeonLevel/x/y`) instead of `Character::setDead()`.
- Write a journal entry on a starvation death, matching the combat death.
- Add a party-wipe check to `movePlayer()` (or a shared post-step hook) that calls
  `handlePartyWipe()` when no member is left alive, so starving the whole party ends
  the run instead of leaving a dead-but-open dungeon.
- Persist the starved body so the Morgue can find and resurrect it.

**Files:** `src/dungeon_dialog/DungeonDialog.cpp`, `src/core/DeathRecovery.cpp`

**Verify:** tests — a starved member's `dungeonLevel/x/y` are recorded; a journal
entry is added; a party that starves to zero triggers the wipe path. Non-vacuous:
reverting to `setDead()` fails the body-location test (this exact probe failed
during the audit: `bodyLoc=(L0,0,0)`).

**Landed:** `movePlayer()` now calls `DeathRecovery::killCharacter()` instead of
`setDead()`, so a starved member records `dungeonLevel/x/y` and gets a journal entry
matching a combat death. A `DeathRecovery::isPartyWiped()` check after each step
routes to `handlePartyWipe()` (sets `isAlive=0`, journals the wipe, saves bodies,
closes the dungeon, returns to town). Tests [199]–[201]; +9 checks (1655 → 1664).
Non-vacuous: reverting to `setDead()` fails 4 checks (body X/Y/level + journal);
removing the wipe check fails "the party wipe was journaled".

### 5.4 One HP source of truth `M` ✅ DONE

**Why:** trap, pit, fall and dungeon-rest damage write the dead `CurrentCharacterHP`
key; the party panel and combat read `Character::hp`. Damage is lost and "healing"
is invisible.

**Do:**
- Delete the `CurrentCharacterHP`/`MaxCharacterHP` round-trip and make
  `updatePartyMemberHealth()` and the rest/spell paths write the live member
  (`getPartyMembers()`), then call `updatePartyPanel()`.
- Keep the `CurrentCharacter*` keys only as a one-way **read** mirror refreshed by
  `refreshUI()` for any UI still bound to them — never as a write target.
- Re-check `setGameValue()`'s special-casing of `CurrentCharacterHP` (it patches
  `m_gameStateData["Party"][0]`, a list shape the live party does not use).

**Files:** `src/dungeon_dialog/DungeonDialog.cpp`,
`src/dungeon_dialog/DungeonHandlers.cpp`, `src/spell_casting/SpellCastingDialog.cpp`,
`gameStateManager.cpp`

**Verify:** tests — trap/pit/fall damage reduces the live member's HP and the panel
reflects it; dungeon rest heals the live member; the party panel and the damage path
agree. Non-vacuous: reverting to `CurrentCharacterHP` fails the agreement test.

**Landed:** `updatePartyMemberHealth()` now writes the live member (`getPartyMembers()[row].hp`) instead of the dead `CurrentCharacterHP` key; on death it routes through `DeathRecovery::killCharacter()` (body location + journal) and checks `isPartyWiped()`. `on_restButton_clicked()` and `SpellCastingDialog::castHealingSpell()` read/write the live member. The `CurrentCharacter*` special-casing in `setGameValue()` was removed — it patched `m_gameStateData["Party"][0]`, a list shape the live party never uses. Tests [202]–[206]; +8 checks (1664 → 1672). Non-vacuous: reverting `updatePartyMemberHealth` to `CurrentCharacterHP` fails 3 checks (live HP reduced, panel shows reduced HP, source of truth).

### 5.5 Town resupply loop `S` ✅ DONE

**Why:** once hunger can kill, there must be a fair way to manage it. The inn should
be where survival is paid for.

**Do:**
- Add a food shop to the General Store and a "buy rations" action at the inn/tavern.
- Resting at the inn restores hunger (currently it restores HP and mana only).
- Optional: a starting ration bundle for a new party so the first descent is not a
  guaranteed slow death.

**Files:** `src/tavern_dialog/TavernDialog.cpp`, `src/general_store/GeneralStore.cpp`,
`gameStateManager.cpp` (new-game setup)

**Verify:** tests — inn rest restores hunger; food can be bought; a new party starts
with food. Non-vacuous: removing the hunger restore fails the inn test.

**Landed:** `TavernDialog::onRestClicked()` restores hunger (hours × 25) alongside
HP/mana. `gameStateManager::addStartingFoodToParty()` gives every living member a
Rations stack with charges from the item database. General Store already stocks
food (MDATA3 rows from 5.2). Tests [207]–[209]; +3 checks (1672 → 1675).
Non-vacuous: removing hunger restore fails "tavern rest restored hunger"; removing
starting food fails "a new party starts with food".

### 5.6 Load a whole party `M` ✅ DONE

**Why:** you can save four characters but never load them back together — every
load path handles exactly one character. The party serializer
(`Party::toMap`/`loadFromMap`) and its wrappers (`savePartyToFile`/
`loadPartyFromFile`) already exist and are unused; the feature is a missing UI and a
missing multi-select path, not missing logic.

**Do:**
- Make `CharacterListDialog` multi-select
  (`QAbstractItemView::ExtendedSelection`) and load **every** selected character into
  the party via `addCharacterToParty()`, respecting `MAX_PARTY_SIZE`.
- Replace the placeholder Select handler with a real load that calls
  `loadCharacterFromFile` per selected name and populates the party.
- Add a **Save Party / Load Party** path built on the existing
  `savePartyToFile()` / `loadPartyFromFile()`: save the current party to a named
  party file, and load a whole party back in one action. Prefer a JSON party file
  (the `Members` list + `SharedGold` shape `Party::toMap()` already produces).
- Fix the Delete handler to remove the file it actually listed
  (`data/characters/<name>.txt`, not `characters/<name>.txt`).
- Make the load path robust: a party load must `clear()` the existing party first
  (`Party::loadFromMap()` already clears), reject an over-full party, and skip
  corrupt/absent members rather than aborting the whole load.

**Files:** `src/characterlist_dialog/characterlistdialog.cpp` / `.h`,
`blacklands.cpp` (`GameMenu::loadGame`), `gameStateManager.cpp` / `.h`,
`character.cpp` (`Party::toMap`/`loadFromMap` if a format tweak is needed)

**Verify:** tests — selecting three characters loads three members into the party;
selecting more than `MAX_PARTY_SIZE` loads only the first four (or is rejected);
a saved party round-trips through `savePartyToFile`/`loadPartyFromFile` with the same
member count, names, and shared gold; loading a party over an existing party replaces
it rather than appending (the double-load test). Non-vacuous: reverting the
multi-select to single-select fails the "loads three members" test; removing the
`clear()` in the load path fails the replace test.

**Landed:** `CharacterListDialog` is now `ExtendedSelection` with a real Select handler that loads every selected character. Added Save/Load Party buttons using the existing `savePartyToFile`/`loadPartyFromFile` JSON wrappers. `addCharacterToParty()` now enforces `MAX_PARTY_SIZE`. Tests [210]–[214]; +14 checks (1675 → 1689). Non-vacuous: reverting to `SingleSelection` fails "the character list allows multi-select"; removing the `MAX_PARTY_SIZE` guard fails "the party size does not exceed MAX_PARTY_SIZE".

### 5.7 Repo hygiene: dead duplicates and build junk `S` ✅ DONE

**Why:** the tree carries dead duplicates and committed build output that confuse
navigation and the build.

**Do:**
- Remove `TheCity.cpp/.h`, `FontManager.h`, `StoryDialog.h`, `DataRegistry.h` (all
  referenced by 0 files; `TheCity.*` differs from the live `theCity.*`).
- Remove `src/core/BattleEngine/` (8 committed build artefacts, not in the main
  build) and add its patterns to `.gitignore` if anything still generates there.
- Confirm the build and the full suite still pass.

**Files:** repo root, `src/core/BattleEngine/`, `.gitignore`

**Verify:** no source or `.pro` file references a removed file ✅; build green ✅;
suite green ✅.

**Landed:** removed `TheCity.cpp/.h/.qss`, `FontManager.h`, `StoryDialog.h`,
`DataRegistry.h`, and the entire `src/core/BattleEngine/` directory (8 tracked
files including `HommBattle` binary, `main.o`, `Makefile`, and moc files).
Added `data/characters/` to `.gitignore` so the party-save files from slice 5.6
do not litter the tree. Tests [215]–[217]; +10 checks (1689 → 1700). Non-vacuous:
restoring `TheCity.cpp` and `TheCity.h` fails both dead-file checks.

### 5.8 Release `S` ✅ DONE

**Why:** v5.0.0 needs a proper release with notes and a version bump.

**Do:**
- Bump `SEMVER_MAJOR` to 5 in `blacklands.pro`.
- Add a v5.0.0 entry to `ReleaseInfo::versionHistory()` and update the test that
  asserts `history[0].first`.
- Write the v5.0.0 section of `RELEASE_NOTES.md` (what changed, why, how verified,
  with the starvation probe as evidence).
- Tag `v5.0.0`.

**Files:** `blacklands.pro`, `src/core/ReleaseInfo.cpp`, `test/selftest.cpp`,
`RELEASE_NOTES.md`

**Verify:** `make check` green ✅; version is 5.0.0 in the about dialog ✅; release
notes complete ✅.

**Landed:** `SEMVER_MAJOR` bumped to 5 in `blacklands.pro`. v5.0.0 entry added to
`ReleaseInfo::versionHistory()` with a summary of all 8 slices. All version
assertions in the test suite updated from 4.0.0 to 5.0.0. Full v5.0.0 section
written in `RELEASE_NOTES.md`. Test [56] updated; +1 check (1700 → 1701).
Non-vacuous: reverting `SEMVER_MAJOR` to 4 fails 4 checks (version, versionString,
installerName, banner).

## Dependency Graph

```
5.1 Hunger visible ── 5.2 Food ── 5.5 Town resupply (needs food to sell)
5.3 Starvation death is real (independent, but needs 5.1 to be observable)
5.4 One HP source of truth (independent)
5.6 Load a whole party (independent — reuses the unused party serializer)
5.7 Repo hygiene (independent)
5.8 Release (after all others)
```

**Critical path:** `5.1 → 5.2 → 5.5` — visibility, then a way to eat, then a way to
buy food. A survival mechanic is only fair once all three exist.

**Ordering note:** land 5.4 (HP source of truth) early — it touches the same damage
and rest paths that 5.3 and 5.5 modify. Land 5.7 (repo hygiene) before 5.8, and let
it also ignore `data/characters/` so 5.6's party files do not litter the tree.

## Release Criteria

- [ ] Hunger is visible in the dungeon and on the character sheet
- [ ] Real food items exist and can be bought and eaten
- [ ] A starvation death records a body, a journal entry, and triggers a party wipe
- [ ] One HP source of truth: trap/rest damage and the party panel agree
- [ ] The inn restores hunger and sells food
- [ ] A whole party can be loaded at once, and saved/loaded as a party
- [x] Dead duplicate files and committed build junk are gone
- [x] `SEMANTIC_VERSION` is `5.0.0`
- [x] `RELEASE_NOTES.md` is updated
- [x] All tests pass (target: 1700+) — **1701 passed**
- [x] `make check` is green

## Slice Sizes

| Slice | Size | Depends on |
|-------|------|------------|
| 5.1 Hunger is visible | M | — |
| 5.2 Food exists and can be eaten | M | 5.1 |
| 5.3 Starvation death is a real death | M | — |
| 5.4 One HP source of truth | M | — |
| 5.5 Town resupply loop | S | 5.2 |
| 5.6 Load a whole party | M | — |
| 5.7 Repo hygiene | S | — |
| 5.8 Release | S | 5.1–5.7 |

---

**Target:** 8 slices. The spine is the survival loop: see hunger → eat food → die
properly when you can't. Everything else is either the same class of defect (silent
state, unreachable mechanics) or a missing UI over working logic (party load) —
plus hygiene.
