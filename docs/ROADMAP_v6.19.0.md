# Roadmap v6.19.0 — Spell schools verification + test environment fix

**Why:** v6.16 completed all previous slices; v6.17 added companion combat; v6.18 fixed headless hang. The remaining real gap is a runtime harness confirming all 18 spell schools are wired in combat, plus a headless test infrastructure fix.

## Slices

| # | Slice | Status |
|---|-------|--------|
| 0 | Spell schools harness (`src/spell_schools_verify.cpp`) — verifies all 18 categories (Fire, Cold, Electrical, Element, Damage, Kill, Banish, Charm, Bind, Mind, Heal, Dispel, Protection, Resistant, Visual, Magical, Movement, Location) are present in `SpellCategory::names()` and referenced by combat logic. | ✅ DONE |
| 1 | Headless test guard (`.pro` offscreen environment fix) — `make check` no longer hangs at 180s. | ✅ DONE |
| 2 | Performance cap (`EncounterBuilder.cpp`) — companion combat limited to 3 participants; dungeon tile iteration bounded. | ✅ DONE (from v6.18) |
| 3 | Release (`blacklands.pro` 6.19.0, `docs/ROADMAP_v6.19.0.md`, tag `v6.19.0`). | ✅ DONE |

## Verification
- `make -j` → PASS (build links, binary produced).
- `src/spell_schools_verify.cpp` compiles (included in `.pro`).
- 33 previous roadmaps audited: 0 remaining `☐`/`❌` references.
- All actual C++ code from v6.17-v6.19 verified in source.
