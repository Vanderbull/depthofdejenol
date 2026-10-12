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

### 5.2 — Dead state keys
**Why:** 16 keys read but never written (`CurrentCharacterHP`, `HallOfRecords`, etc.).
**Do:** Either write them or remove the reads.
**Verify:** No read-only keys remain.

### 5.3 — Wire unconnected signals
**Why:** 21 signals emitted but never connected.
**Do:** Connect `partyChanged`→UI refresh, `depositGold`/`withdrawGold`→HP/gold, `eventTriggered`→Journal, `chatReceived`→message window.
**Verify:** Each signal has a connected slot.

### 5.4 — BankPane TODOs
**Why:** "Party Pool" and "Party Deposit" are TODO stubs.
**Do:** Implement both.
**Verify:** Tests prove they work.

### 5.5 — RaceData placeholder
**Why:** "Guild Eligibility" is placeholder logic.
**Do:** Implement from MDATA1 data.
**Verify:** Tests prove eligibility rules work.

### 5.6 — HP calculation
**Why:** DungeonDialog uses placeholder HP.
**Do:** Con-based HP calculation.
**Verify:** Tests prove HP scales with Con.

### 5.7 — CombatState/CombatDeathHandler
**Why:** 0 external callers — either dead code or not wired.
**Do:** Wire into Arena/Dungeon or delete.
**Verify:** No orphans remain.

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
