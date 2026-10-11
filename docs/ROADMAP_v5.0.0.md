# Roadmap v5.0.0 — Survival: make hunger real

**Status:** Not started
**Version:** v5.0.0 (major — a survival mechanic exists in code but is invisible,
unfixable, and its deaths are unhandled; this version makes it a real system)
**Test suite at start:** 1590 + 3 (all passing)

## Goal

The game has a hunger/starvation system that a player can never see, never manage,
and whose deaths the game does not treat as deaths. The user's report — *"starving
did not kill"* — is the symptom of a mechanic that is unreachable rather than absent.

This version makes survival real:

- **Hunger is visible** — the player can see it draining and act on it.
- **Food exists and can be eaten** — a real food item and an eat action, not a
  code path with zero callers.
- **Starvation death is a real death** — a body, a journal entry, the Morgue, and a
  party-wipe check outside combat, exactly like a combat death.
- **One HP source of truth** — collapse the two HP stores so trap, pit, fall and
  rest damage land on the same character the party panel shows.
- **A town resupply loop** — the inn sells food and restores hunger, so survival is
  a resource decision rather than an unpayable tax.

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

## Slices

### 5.1 Hunger is visible `M`

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

### 5.2 Food exists and can be eaten `M`

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

### 5.3 Starvation death is a real death `M`

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

### 5.4 One HP source of truth `M`

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

### 5.5 Town resupply loop `S`

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

### 5.6 Repo hygiene: dead duplicates and build junk `S`

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

### 5.7 Release `S`

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

## Dependency Graph

```
5.1 Hunger visible ── 5.2 Food ── 5.5 Town resupply (needs food to sell)
5.3 Starvation death is real (independent, but needs 5.1 to be observable)
5.4 One HP source of truth (independent)
5.6 Repo hygiene (independent)
5.7 Release (after all others)
```

**Critical path:** `5.1 → 5.2 → 5.5` — visibility, then a way to eat, then a way to
buy food. A survival mechanic is only fair once all three exist.

**Ordering note:** land 5.4 (HP source of truth) early — it touches the same damage
and rest paths that 5.3 and 5.5 modify.

## Release Criteria

- [ ] Hunger is visible in the dungeon and on the character sheet
- [ ] Real food items exist and can be bought and eaten
- [ ] A starvation death records a body, a journal entry, and triggers a party wipe
- [ ] One HP source of truth: trap/rest damage and the party panel agree
- [ ] The inn restores hunger and sells food
- [ ] Dead duplicate files and committed build junk are gone
- [ ] `SEMANTIC_VERSION` is `5.0.0`
- [ ] `RELEASE_NOTES.md` is updated
- [ ] All tests pass (target: 1650+)
- [ ] `make check` is green

## Slice Sizes

| Slice | Size | Depends on |
|-------|------|------------|
| 5.1 Hunger is visible | M | — |
| 5.2 Food exists and can be eaten | M | 5.1 |
| 5.3 Starvation death is a real death | M | — |
| 5.4 One HP source of truth | M | — |
| 5.5 Town resupply loop | S | 5.2 |
| 5.6 Repo hygiene | S | — |
| 5.7 Release | S | 5.1–5.6 |

---

**Target:** 7 slices. The spine is the survival loop: see hunger → eat food → die
properly when you can't. Everything else is either the same class of defect (silent
state, unreachable mechanics) or hygiene.
