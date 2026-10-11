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

### 3.1 Combat target selection `M`

**Why:** `on_combatAttackButton_clicked()` scans participants and takes the first living
monster (`DungeonDialog.cpp:1394`). `EncounterBuilder::buildEncounter()` adds every monster
of a group as its own `CombatParticipant` (`DungeonDialog.cpp:1367`), so a 3-monster fight
gives the player no way to focus one down. `CombatActions::attack(int targetIndex, ...)`
already takes a target — only the UI refuses to pass one.

**Do:**
- Add a monster selection list to the combat panel; each row is one living monster.
- `on_combatAttackButton_clicked()` attacks the *selected* monster, falling back to the
  first living one when nothing is selected (so keyboard-only play still works).
- The same selection feeds spell and item targeting.

**Files:** `src/dungeon_dialog/DungeonDialog.cpp` / `.h`, `src/combat/CombatActions.cpp`

**Verify:** in a 3-monster fight, attacking the third monster damages the third, not the
first; killing the selected monster moves selection to the next living one.

---

### 3.2 Per-monster combat display `M`

**Why:** the combat panel collapses every monster into one label:
`m_combatMonsterHpLabel->setText("Monster: " + monsterHp.join(", "))` (`DungeonDialog.cpp:1516`).
There is no monster list widget at all (verified: no `monsterList`/`targetList` in the
combat UI). With groups this reads as a wall of numbers. Combat item use is also hardcoded:
`on_combatUseItemButton_clicked()` searches the inventory for a name containing "healing"
or "health" and uses the first match — the player cannot choose a potion.

**Do:**
- Replace the joined label with a per-monster row: name, HP (bar or `cur/max`), and status
  icons (poison, fire, blind, stunned).
- Highlight the row that currently acts and the one the player has selected.
- Let combat item use pick from the party member's usable items instead of the first
  "healing" match.

**Files:** `src/dungeon_dialog/DungeonDialog.cpp` / `.h`

**Verify:** a 3-monster fight shows 3 distinct rows with independent HP; damaging one
changes only its row; the item button offers the actual usable items.

---

### 3.3 Front and back row `L`

**Why:** planned in `docs/ROADMAP_v1.1.0.md` (slice 2.2) and never done.
`CombatParticipant` has no row field (verified). `MonsterAI::chooseTarget()` already rolls
"60% front row (first living player), 25% weakest, 15% random" (`MonsterAI.cpp:52`) — but
"front row" is just "whoever is first in the list", so the tactic is cosmetic. Casters
cannot be protected.

**Do:**
- Add `row` (front/back) to `CombatParticipant`.
- Let the player set a party member's row in combat (or from the party panel).
- Melee reaches only the front row; ranged attacks and spells reach both. Monster melee
  follows the same rule.
- `MonsterAI::chooseTarget()` targets the front row by default and only reaches the back
  row with ranged/spell attacks.

**Files:** `src/combat/CombatState.h`, `src/combat/CombatActions.cpp`,
`src/combat/MonsterAI.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`

**Verify:** a back-row member takes no melee damage while a front-row member stands;
putting everyone in the back row makes melee unable to reach anyone; spells still reach both.

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

### 3.5 Main quest chain driver `L`

**Why:** `QuestChain` computes steps, completion and rewards
(`QuestChain::isStepComplete/nextStepIndex/isChainComplete`), but the only caller is
`QuestChainDialog` — the display. Nothing in the game *advances* the chain, grants a step's
`rewardGold`/`rewardXp`, or gates victory on it (verified: no `QuestChain::` call outside
the dialog and `src/core/QuestChain.cpp`). The "main quest line" is a read-only screen.

**Do:**
- On boss defeat (`handleVictory()` already sets `bossDefeated` on the level snapshot),
  advance the chain and grant the step's reward.
- Track `deepestFloorReached` and `itemsHeld` from live game state, not from the dialog.
- On chain completion, trigger the victory sequence (the Prince of Devils step) instead of
  the current hardcoded `level == 15` check.
- Record progress in the journal (the journal wiring from 2.12 is the natural place).

**Files:** `src/core/QuestChain.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`,
`gameStateManager.cpp`, `src/journal_dialog/JournalDialog.cpp`

**Verify:** defeating the floor-3 boss completes step 1 and grants its reward; the chain
cannot complete before its boss steps are done; killing the Prince completes the chain.

---

### 3.6 Play-time tracking and a real Hall of Records `S`

**Why:** `DungeonDialog.cpp:1657` records `rec.completionTimeSeconds = 0; // TODO: track
actual play time`. `Endgame::isBetter()` treats `completionTimeSeconds <= 0` as "not
finished" (`Endgame.cpp:89`), so the fastest-completion record can never be set. Play time
is not measured anywhere (verified: no `QElapsedTimer`/`playTime` in the codebase).

**Do:**
- Start a timer when a run begins; persist elapsed seconds with the save.
- Populate `completionTimeSeconds` on victory.
- Show play time on the character sheet and in the Hall of Records.

**Files:** `gameStateManager.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`,
`src/hall_of_records/hallofrecordsdialog.cpp`, `src/core/Endgame.cpp`

**Verify:** a completed run records a non-zero time; a faster run sorts ahead of a slower
one; the time survives save/load.

---

### 3.7 Save versioning and migration `M`

**Why:** `VERSIONING.md` documents a migration policy ("Check `SaveVersion` against the
current version. If older, run migrations. If newer, refuse to load.") but the code only
ever *writes* it: `m_gameStateData["SaveVersion"] = 1;` (`gameStateManager.cpp:2364`) is the
sole occurrence. Nothing reads it on load. v3.0.0 adds fields (row, stat points, play time),
which is exactly when a silent mismatch starts corrupting saves.

**Do:**
- Read `SaveVersion` on load; treat a missing field as version 0.
- Migrate older saves: fill new fields with safe defaults.
- Refuse to load a save newer than the running build, with a clear message.
- Add a test that loads a hand-written v1 save and gets correct defaults.

**Files:** `gameStateManager.cpp`, `src/core/savegameUtils.*`, `character.cpp`

**Verify:** a save without `StatPoints`/`row`/`playTime` loads with sane defaults; a save
with `SaveVersion` above the current build is refused with a message.

---

### 3.8 Combat visual feedback `S`

**Why:** there is no on-hit feedback (verified: no flash/shake/particle code outside Qt's
own tool Makefiles). Combat is readable only from the log. This is not a graphics overhaul —
the wireframe renderer stays — it is the minimum needed to feel a hit land.

**Do:**
- Flash the combat panel (or the damaged row) red on damage, green on heal.
- A brief highlight when a monster dies or a character levels up.
- Keep it cheap and optional; respect a reduced-motion setting if one exists.

**Files:** `src/dungeon_dialog/DungeonDialog.cpp`, `src/core/SoundEffects.*` (adjacent)

**Verify:** taking damage visibly flashes the target; the effect is absent when the setting
is off.

---

### 3.9 Release hygiene: stop tracking build artifacts `S`

**Why:** `FIXLIST.md` #20 is marked FIXED but the artifacts are still tracked (verified:
`git ls-files` lists `Makefile`, `blacklands` and `qrc_resources.cpp`). Every commit in
v2.0.0 carried binary churn — a rebuilt `blacklands` and a regenerated `Makefile` — which
makes diffs unreadable and invites merge conflicts. A release version should not ship them.

**Do:**
- `git rm --cached Makefile blacklands qrc_resources.cpp`
- Extend `.gitignore` to cover them and any other generated artifact.
- Confirm a clean build still works from a fresh clone (qmake regenerates `Makefile`).

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
3.7 Save versioning (must land before 3.4/3.6 persist new fields)
3.8 Visual feedback (independent)
3.9 Repo hygiene (independent)
```

**Critical path:** `3.1 → 3.2` — target selection is the hole that makes group encounters
playable; the display makes the choice visible. Everything else is depth.

**Ordering note:** land 3.7 (save versioning) *before* 3.4 and 3.6 add persisted fields, so
the migration path exists from the first save that contains them.

## Release Criteria

v3.0.0 is ready when:

- [ ] The player can choose which monster to attack, and the choice is visible
- [ ] Each monster in a group shows its own HP and status
- [ ] Front/back row changes who melee can reach
- [ ] Level-ups grant spendable stat points; the pool persists
- [ ] The main quest chain advances from play and grants its rewards
- [ ] Play time is tracked and the Hall of Records sorts by it
- [ ] Saves are versioned, migrated, and newer saves are refused
- [ ] Combat gives visual feedback on hits and deaths
- [ ] `Makefile`, `blacklands` and `qrc_resources.cpp` are untracked; a clean build works
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
