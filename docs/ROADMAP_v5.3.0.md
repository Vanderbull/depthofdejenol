# Roadmap v5.3.0 — Aging & Lifecycle

**Status:** ✅ RELEASED
**Version:** v5.3.0 (minor — character aging, lifecycle events, long-term persistence)

## Goal

Characters currently never age. The game has no lifecycle — no birth, no death from old age, no legacy. This version adds:

1. **Character aging** — characters age over time, affecting stats and appearance
2. **Lifecycle events** — birth, coming of age, marriage, children (optional), retirement, natural death
3. **Legacy system** — descendants inherit from ancestors
4. **Longevity tracking** — genealogy/history per family line

## Slices

### 5.1 Aging mechanics `S` ✅ DONE
- `AgingRules::applyYearOfAging(c, years=1)` supports multi-year aging
- `AgingRules::getAgeBracket()` returns Child/Adult/Decaying
- Coming-of-age ceremony at 18
- Race-specific decay threshold and max age (already existed)

### 5.2 Lifecycle events `S` ✅ DONE
- `Character::spouse` and `Character::children` fields
- `isEligibleForMarriage()`, `marry()`, `addChild()`, `canRetire()`
- All lifecycle data serializes via `toMap()`/`loadFromMap()`

### 5.3 Legacy & genealogy `S` ✅ DONE
- `Character::inheritFrom(ancestor)` — descendant inherits half of ancestor's stats and 10% of gold
- Family tree tracking via children list
- Genealogy data in character sheet

### 5.4 Persistence & migration `S` ✅ DONE
- Lifecycle data persists in JSON save/load round-trips
- Backward-compatible with v5.2.0 saves (missing fields default correctly)
- Party save/load preserves all lifecycle data

### 5.5 UI updates `S` ✅ DONE
- Character sheet data includes age, spouse, children
- Family tree view data
- Life event timeline data

### 5.6 Release `S` ✅ DONE
- Version bumped to v5.3.0
- All 6 slices complete

## Dependency Graph

```
5.1 Aging mechanics → 5.2 Lifecycle events → 5.3 Legacy & genealogy → 5.4 Persistence → 5.5 UI updates → 5.6 Release
```

## Release Criteria

- [x] Characters age over time (test [250])
- [x] Stat modifiers by age bracket work (test [251])
- [x] Coming-of-age ceremony triggers at 18 (test [253])
- [x] Marriage system works (test [254])
- [x] Family tree tracking works (test [255])
- [x] Descendant inheritance works (test [258])
- [x] Save/load aging data round-trips (test [260])
- [x] Backward-compatible with v5.2.0 saves (test [261])
- [x] All tests pass (1841 passed, 0 failed)
- [x] `make check` is green
- [x] Non-vacuous verification for every fix
