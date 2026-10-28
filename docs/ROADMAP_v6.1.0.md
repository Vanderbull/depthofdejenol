# v6.1.0 — Skill & Ability System Overhaul

## Status: PLANNING (initial draft)

## Overview
This release adds a dynamic skill system and expands the character's abilities beyond static guild bonuses. The focus is to give the player meaningful choices in skill progression, allowing them to craft unique builds through skill points, skill checks, and ability bonuses.

### Version Metadata
- **Current version:** 6.0.0 (tagged v6.0.0)
- **Target version:** 6.1.0
- **Type:** New features / gameplay mechanics
- **Goal:** Implement a robust skill system, expand combat, magic, crafting, social, survival, companion training, and skill‑based quests.

### Version Bump Tasks
- Update `blacklands.pro`:
  - `SEMVER_MAJOR = "6"`
  - `SEMVER_MINOR = "1"`
  - `SEMVER_PATCH = 0`
  - `SEMVER = "6.1.0"`
- Update `version.h.in` accordingly (uses `$$SEMVER`).
- Verify that `make` generates `version.h` with the new semantic version.
- **Verification:** `grep "SEMANTIC_VERSION" version.h` shows `v6.1.0`.

---

## Slices

### Slice 6.1a — Skill Acquisition & Allocation
**Goal:** Provide the player a pool of skill points that can be spent on a skill tree, plus a UI to allocate.

**Contents:**
- Add a `SkillPoint` field to `Character` (default 0). When a character levels up, reward e.g., 3 skill points.
- Add a skill data table: `SkillDef` with fields `name`, `description`, `prerequisites`, `cost`, `type`, `icon`, etc.
- Add a skill tree UI (e.g., `CharacterSheetDialog` tab) showing available nodes, prerequisites, and cost.
- Implement allocation: Spend skill points on skills (decrement pool, add skill level).
- Verify: (1) Character starts with 0 skill points; (2) after a level‑up a check adds points; (3) spending points updates the pool; (4) cannot exceed the maximum pool.

**Verification:** Unit test `test/equipment_test.cpp` (extend) that checks point allocation and UI.

### Slice 6.1b — Skill Checks in Combat
**Goal:** Combat actions can be influenced by the characters skill level.

**Contents:**
- Extend `CombatState` to include a `skillMod(int skillId)` method that returns skill level modified by base stats (e.g., Dexterity).
- Update `CombatActions` to roll a skill check when performing actions like `DisarmTrap`, `Stealth`, `PickLock`, `DisableTrap`.
- Add a new `SkillCheckResult` enum: Success, PartialSuccess, Failure.
- Add UI signals: `skillCheckResult(QString skillName, SkillCheckResult result)` for UI to display feedback.
- Modify `CombatActions` to add new methods `disarmTrap`, `pickLock`, etc., which roll a skill check and apply the appropriate effect.
- Update `MonsterAI` to use skill checks when deciding to attack, use traps, etc.
- Verify: (1) A character with a high skill level succeeds more often than one with a low level; (2) the skill check rolls are deterministic for a given seed; (3) UI receives correct signals.

**Verification:** Add a unit test in `test/equipment_test.cpp` that creates two characters with different skill levels, runs a disarm trap action, and asserts success rates.

### Slice 6.1c — Combat Skill Feats and Maneuvers
**Goal:** Give characters new combat abilities that require a minimum skill level.

**Contents:**
- Add new `CombatAction` entries: `Parry`, `Dodge`, `DualWield`, `ShieldBash`.
- For each, require a skill check against a target difficulty (e.g., `CombatSkill::Parry`).
- Add a UI button to the combat screen for each action.
- Update `MonsterAI` to use `Parry` and `Dodge` when appropriate.
- Ensure that the new actions follow the existing combat flow (initiative, turn order).
- Verify: (1) `CombatState` correctly toggles `hasActed`; (2) actions succeed only when the required skill check passes; (3) UI buttons are disabled when the character lacks the required skill.

**Verification:** Use the self‑test harness to simulate combat and verify that a low‑skill character fails the maneuver more often than a high‑skill character.

### Slice 6.1d — Magic Proficiency & Spell Slots
**Goal:** Introduce school proficiency and spell slots so magic becomes strategic.

**Contents:**
- Extend `Character` with a `Map<QString, int>` mapping school name to a proficiency level.
- Add `SpellSlot` struct: `school`, `level` (1-3), `count`. Store a vector for each level.
- Add UI: When casting spells, the DC (difficulty class) is adjusted by the character’s proficiency + intelligence + spell slot.
- Implement a skill‑level prerequisite: to learn a spell, a character must have a minimum proficiency in that school.
- Add spell slot UI in the spell casting dialog: show slots, use them, and restore them on rest.
- Update `SpellBook::newlyLearned` to also check proficiency.
- Verify: (1) Characters with higher proficiency can cast higher‑level spells more often; (2) slots are consumed and restored; (3) UI updates accordingly.

**Verification:** Test that a character with level 3 proficiency in Fire can use a firebolt and not run out of MP as fast as a low‑proficiency character.

### Slice 6.1e — Crafting & Artisan Skills
**Goal:** Give players craftable magical and mundane items with a skill check.

**Contents:**
- Add new skill `Alchemy`, `Smithing`, `Enchanting`, `Tannery`, `Tailoring`.
- Define a `craftingRecipe` table with `itemName`, `skill`, `levelRequirement`, `ingredients[]`, `successRate`.
- Add UI: `GeneralStore` and a new `CraftingDialog` to combine ingredients using an artisan skill.
- Implement `craftItem()` in `GeneralStore` that checks skill level, randomness (success chance), consumes ingredients, and produces result.
- Add a skill UI for artisans (maybe a separate tab).
- Verify: (1) A character with the required skill level can craft; (2) a low‑skill attempt may fail and leave ingredients unused; (3) UI reflects the success/failure.

**Verification:** Extend the self‑test to attempt multiple craftings and assert that the success rate matches the defined probability.

### Slice 6.1f — Social & Negotiation Skills
**Goal:** Allow players to influence NPCs through social skills.

**Contents:**
- Add skills `Persuasion`, `Intimidation`, `Deception`, `Diplomacy`, `Religion`.
- Add a UI in NPC dialogs: a set of buttons or a quick‑talk panel with skill checks.
- When talking to an NPC, perform a skill check against a difficulty (e.g., `Reputation` or `NPCDifficulty`).
- Success/failure influences NPC reactions, trade options, and quest availability.
- Update `NPCDialog` to show the NPC's mood and possible outcomes.
- Verify: (1) High skill leads to more positive outcomes; (2) outcomes are recorded in journal; (3) UI reflects NPC mood.

**Verification:** Simulate repeated interactions and measure the outcome distribution.

### Slice 6.1g — Survival & Exploration Skills
**Goal:** Add mechanics for tracking, trapping, stealing, and survival.

**Contents:**
- Add skills `Stealth`, `Detection`, `Survival`, `Search`, `DisarmTrap`, `PickLock`, `Forage`.
- Define skill checks for:
  - `Stealth`: move without being seen.
  - `Detection`: notice hidden doors, traps, creatures.
  - `DisarmTrap`: attempt to disable a trap.
  - `PickLock`: open a locked chest/door.
  - `Forage`: find food from the environment.
- Add a “Skill Check” UI in the dungeon dialog (maybe a right‑click menu or a key).
- Implement the logic in `DungeonHandlers` or a new `SkillCheckManager`.
- Update the automap and bestiary to reveal hidden information based on `Detection` skill.
- Verify: (1) Skill checks succeed at higher rates with higher skill; (2) hidden features become visible after a successful check; (3) UI shows the results.

**Verification:** Unit test that a low‑skill character fails more often than a high‑skill character.

### Slice 6.1h — Companion Training & Command Skills
**Goal:** Allow players to train their companions and give them special abilities.

**Contents:**
- Add a companion skill system: each companion has a set of trainable skills.
- Add UI in the companion dialog: show skill levels, assign actions.
- Implement a `CompanionSkill` table.
- Skills: `Guard`, `Attack`, `Heal`, `Fetch`, `Follow`.
- Add a `companion.command()` call that uses skill check.
- Verify: (1) Companion actions succeed based on skill; (2) UI updates companion skills; (3) companions can learn new skills via training.

**Verification:** Unit test that a companion with a high `Attack` skill hits more often in combat.

### Slice 6.1i — Skill‑Based Quest Rewards
**Goal:** Reward players based on the skills they have invested in.

**Contents:**
- Modify the Quest system to reward gold, XP, or item based on the player’s highest relevant skill level.
- Add a `QuestRequirement` that references a skill and a minimum level.
- Update `QuestChainDriver` to check skill level when giving rewards.
- Add a UI notification when a skill‑based quest is completed.
- Verify: (1) Quest rewards vary with skill level; (2) statistics are recorded.

**Verification:** Test with two characters of different skill levels completing the same quest and compare rewards.

### Slice 6.1j — Skill UI & Integration
**Goal:** Provide a cohesive UI for all new skills.

**Contents:**
- Add a new dialog `SkillBookDialog` that lists all skills, their levels, prerequisites, and descriptions.
- Add a tab in `CharacterSheetDialog` for the skill tree and point allocation.
- Add a UI for skill checks (maybe a “Skill Check” button in the combat dialog and dungeon dialog).
- Add a “Skill Log” UI that records skill usage and successes.
- Integrate with the journal and bestiary: show skill‑based entries.
- Verify: (1) UI displays all skills and levels; (2) UI opens/closes correctly; (3) all new features are reachable.

**Verification:** Manual QA: run through creating a character, allocate skills, use skills in combat, check UI responsiveness.

### Slice 6.1k — Final Integration & Polish
**Goal:** Ensure everything works together and the game is stable.

**Contents:**
- Run full self‑test suite.
- Add integration tests for each new feature.
- Fix any issues discovered.
- Update documentation: add a new section in `README.md` about skill system.
- Update `ReleaseInfo.cpp` with the new version entry.
- Update `docs/ROADMAP_v6.1.0.md` with the final state.
- Verify: `make check` passes; `make` builds without errors.

**Verification:** Run `make` and `make check`, confirm 1768+ tests pass and the binary builds.

---

## Version Bump Steps (perform before the slices)

1. Edit `blacklands.pro`:
   - Change `SEMVER_MAJOR = "6"`, `SEMVER_MINOR = "1"`, `SEMVER_PATCH = 0`, `SEMVER = "6.1.0"`.
2. Run `qmake6 blacklands.pro` to regenerate `version.h`.
3. Confirm `version.h` contains the new semantic version.

---

## Verification Criteria

- At the end of each slice, run the existing self‑test suite plus new tests (if applicable).
- All slices combined should pass `make check` and the game should run without fatal errors.
- The final verification step is the `make` and `make check` commands.

---

## Timeline Estimate (approx. per slice)

| Slice | Estimated Work Hours |
|-------|----------------------|
| 6.1a (Allocation) | 6 |
| 6.1b (Combat checks) | 8 |
| 6.1c (Feats) | 6 |
| 6.1d (Spell slots) | 8 |
| 6.1e (Crafting) | 8 |
| 6.1f (Social) | 6 |
| 6.1g (Survival) | 8 |
| 6.1h (Companions) | 6 |
| 6.1i (Quest rewards) | 6 |
| 6.1j (Skill UI) | 8 |
| 6.1k (Integration) | 10 |
| **Total** | **~82 hours** |

---

## Notes & Dependencies

- Slices are ordered by dependency, but some may be parallelized if resources allow.
- The new skill system requires the addition of several data files (skill definitions, companion skills, etc.). These can be stored as JSON or CSV and loaded via the existing `data_registry`.
- The version bump should be done first, and a new git tag v6.1.0 will be placed at the end of the task.

---

## Completion Checklist

- [ ] `blacklands.pro` updated with new version
- [ ] `version.h` regenerated and shows `v6.1.0`
- [ ] All slices implemented and unit‑tested
- [ ] `make` builds without errors
- [ ] `make check` passes all tests
- [ ] Roadmap file updated with final status
- [ ] Git tag `v6.1.0` created and pushed
- [ ] UpdateManager updated to check version from server (if applicable)