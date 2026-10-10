# Roadmap v1.1.0 — Combat Reachability and Depth

**Status:** Not started
**Version:** v1.1.0 (minor — new features)
**Test suite at start:** TBD (after v1.0.0)

## Goal

Make combat **reachable and deep**. v1.0.0 makes the game finishable; v1.1.0 makes the
journey better. Combat should start when you encounter a monster (not when you press a button),
and it should have tactical depth (rows, monster spells, school-specific mechanics).

## Slices

### 2.1 Start combat from the encounter, not the button `M`

**Why:** stepping onto a monster only logs a line (`handleEncounters`). Combat begins only
if the player then presses Fight while standing on the tile. `initiateFight()` is an empty
stub. The encounter is announced and then ignored.

**Do:** have `handleEncounters` (or `movePlayer`) start combat when a hostile monster shares
the tile, or prompt. Fill in or delete `initiateFight()`.

**Files:** `src/dungeon_dialog/DungeonHandlers.cpp`, `DungeonDialog.cpp`.

**Verify:** walking onto a hostile monster opens the combat UI without pressing Fight.

---

### 2.2 Front and back row `L`

**Why:** the completeness analysis calls for front/back positioning (warriors up front,
casters behind); `MonsterAI::chooseTarget()` already rolls 60% front / 25% weakest, but
there is no row to mean anything.

**Do:** add a row to `CombatParticipant`, let the player set it, make melee reach only the
front and ranged/spells reach both.

**Files:** `src/combat/CombatState.h`, `CombatActions.cpp`, `MonsterAI.cpp`, combat UI.

**Verify:** a back-row member is not targeted by melee; a front-row one is.

---

### 2.3 Monster spellcasting `M`

**Why:** `MonsterAI::decide()` can return `CastSpell`, but monsters have no spell path wired,
so the branch is effectively dead.

**Do:** give spell-capable monsters a spell (from their data or a per-type table) and execute
the cast through `CombatActions`.

**Files:** `src/combat/MonsterAI.cpp`, `EncounterBuilder.cpp`.

**Verify:** a caster monster casts at least once over N rounds and the party takes spell
damage.

---

### 2.4 Flee re-entry `S`

**Why:** fleeing marks monsters dead (`CombatActions::flee`) so combat ends, but the monster
is removed from the map on victory only — fleeing should leave it present or clear it
consistently.

**Do:** decide the flee semantics (monster stays, party disengages) and make map state agree
with combat state.

**Files:** `src/combat/CombatActions.cpp`, `DungeonDialog.cpp`.

**Verify:** flee → the monster is either still on the tile or gone, never both.

---

## Release criteria

v1.1.0 is ready when:

- [ ] All 4 slices are complete
- [ ] Combat starts automatically on encounter
- [ ] Front/back row affects targeting
- [ ] Monster spells are cast in combat
- [ ] Flee semantics are consistent
- [ ] All tests pass
- [ ] `RELEASE_NOTES.md` is updated
