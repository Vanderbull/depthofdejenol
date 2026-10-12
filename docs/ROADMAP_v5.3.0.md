# Roadmap v5.3.0 — Aging & Lifecycle

**Status:** IN PROGRESS
**Version:** v5.3.0 (minor — character aging, lifecycle events, long-term persistence)

## Goal

Characters currently never age. The game has no lifecycle — no birth, no death from old age, no legacy. This version adds:

1. **Character aging** — characters age over time, affecting stats and appearance
2. **Lifecycle events** — birth, coming of age, marriage, children (optional), retirement, natural death
3. **Legacy system** — descendants inherit from ancestors
4. **Longevity tracking** — genealogy/history per family line

## Slices

### 5.1 Aging mechanics `S`
- Age progression based on play time
- Stat modifiers by age bracket (young = less stats, adult = peak, old = declining)
- Visual aging indicators (hair color, etc.)

### 5.2 Lifecycle events `S`
- Coming-of-age ceremony (18th birthday)
- Marriage system (optional)
- Children/inheritance (optional)
- Retirement (optional)
- Natural death

### 5.3 Legacy & genealogy `S`
- Family tree tracking
- Descendant inheritance (stats, items, gold)
- Genealogy UI in character sheet

### 5.4 Persistence & migration `S`
- Save/load aging data
- Backward-compatible with v5.2.0 saves
- Migration from old save format

### 5.5 UI updates `S`
- Character sheet shows age and lifecycle stage
- Family tree view
- Timeline of major life events

### 5.6 Release `S`
- Version bump to v5.3.0
- Release notes
- Non-vacuous verification for all fixes

## Dependency Graph

```
5.1 Aging mechanics → 5.2 Lifecycle events → 5.3 Legacy & genealogy → 5.4 Persistence → 5.5 UI updates → 5.6 Release
```

## Release Criteria

- [ ] Characters age over time (test [250])
- [ ] Stat modifiers by age bracket work (test [251])
- [ ] Coming-of-age ceremony triggers at 18 (test [252])
- [ ] Marriage system works (test [253])
- [ ] Family tree tracking works (test [254])
- [ ] Descendant inheritance works (test [255])
- [ ] Save/load aging data round-trips (test [256])
- [ ] Backward-compatible with v5.2.0 saves (test [257])
- [ ] All tests pass
- [ ] `make check` is green
- [ ] Non-vacuous verification for every fix
