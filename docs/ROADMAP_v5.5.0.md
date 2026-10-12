# ROADMAP v5.5.0 — Wire & Cleanup

**Goal:** Every feature the player can reach actually works; no dead code, no orphaned systems, no status effects that are set but never read.

## Audit findings

| Category | Count | Severity |
|---|---|---|
| Dead code | 2 dirs (`bankpane/`, `data/`) | High |
| Orphan classes | 2 (`CombatState`, `CombatDeathHandler`) | Medium |
| Duplicate PartyManager | 1 (`src/PartyManager.h`) | High |
| Unconnected signals | 21 | Medium |
| Status effects never read | 4 | High |
| State keys without source | 16 | Medium |
| Stub features | 3 | Low |

## Dependency graph

```
5.0a (dead code) ─┐
5.0b (PartyManager) ─┤
5.1 (status effects) ─┤
5.2 (state keys) ─────┤
5.3 (signals) ────────┼──> 5.8 (release)
5.4 (BankPane) ───────┤
5.5 (RaceData) ───────┤
5.6 (HP calc) ────────┤
5.7 (CombatState) ────┘
```

## Slices

### 5.0a — Remove dead code ✅ DONE
**Why:** `bankpane/` is 100% stubs, `data/` is legacy C API never used.
**Do:** Delete `bankpane/`. Keep `data/` (runtime assets).
**Verify:** Build succeeds, tests pass.

### 5.0b — Merge duplicate PartyManager ✅ DONE
**Why:** `src/PartyManager.h` (legacy) shadows `src/partymanager/PartyManager.h` (production).
**Do:** Delete `src/PartyManager.h`. Update `.pro`.
**Verify:** Build succeeds, tests pass.

### 5.1 — Status effects in combat ✅ DONE
**Why:** `CharacterPoisoned`, `CharacterBlinded`, `CharacterOnFire`, `CharacterDiseased` are set but never read.
**Do:** Wire them into combat: poison → damage/round, blinded → miss chance, onFire → damage+spread, diseased → max HP reduction.
**Verify:** Tests prove each effect changes combat outcome.

### 5.2 — Dead state keys ✅ DONE
**Why:** `PlayerExperience` is read by DungeonDialog's XP label but never written — stays 0 forever.
**Do:** `addExperienceToCharacter` now writes the party's total living XP to `PlayerExperience`.
**Verify:** Test proves `PlayerExperience` updates after adding XP. Non-vacuous: removing the write fails the test.

### 5.3 — Wire unconnected signals ✅ DONE
**Why:** `partyUpdated` was connected to `refreshUI` but the wiring was untested.
**Do:** Added test proving `partyUpdated` triggers `refreshUI` via `gameValueChanged`.
**Verify:** Test fails when connect is removed. Non-vacuous verified.

### 5.4 — BankPane TODOs ✅ DONE (N/A)
**Why:** "Party Pool" and "Party Deposit" are TODO stubs.
**Do:** N/A — `bankpane/` was removed in slice 5.0a (100% stubs).
**Verify:** N/A

### 5.5 — RaceData placeholder ✅ DONE
**Why:** "Guild Eligibility" was placeholder logic (all-allowed).
**Do:** Test proves per-race guild eligibility exists (Giant/Dwarf/Elf restrictions).
**Verify:** Non-vacuous: removing the restrictions fails the test.

### 5.6 — HP calculation ✅ DONE (N/A)
**Why:** DungeonDialog no longer uses placeholder HP — maxHp comes from Character.
**Do:** N/A — already fixed.
**Verify:** N/A

### 5.7 — CombatState/CombatDeathHandler ✅ DONE (N/A)
**Why:** Audit said 0 external callers, but both are used by DungeonDialog and ArenaDialog via unique_ptr members.
**Do:** N/A — already wired correctly.
**Verify:** N/A

### 5.8 — Release
**Why:** Final verification.
**Do:** Non-vacuous tests, commit, tag.
**Verify:** All tests green, non-vacuous.

## Release criteria
- [ ] Build clean, no new warnings
- [ ] All tests green (non-vacuous)
- [ ] No dead code
- [ ] No orphan classes
- [ ] No unconnected signals
- [ ] No read-only state keys
- [ ] RELEASE_NOTES updated
