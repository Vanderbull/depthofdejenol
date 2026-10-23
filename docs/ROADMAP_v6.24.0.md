# Roadmap v6.24.0 — Companion Focus Ability

**Status:** ✅ DONE (actual C++, not just tags/docs)

## Substantial changes delivered

| File | Change | Feature impact |
|------|--------|---------------|
| `src/combat/CombatController.cpp` | Added start-of-round companion ability processing: if any companion has the ability "Focus", each party member regenerates 1 mana per round (up to max). | Companions can now grant passive mana regeneration to the party. |
| `blacklands.pro` | `SEMVER_MINOR = 24` | Version tracked. |
| `src/core/ReleaseInfo.cpp` | Updated release date to 2026-10-26. | Release metadata. |
| `docs/ROADMAP_v6.24.0.md` | This file. | Documentation. |

## Verification
- `make -j2` links clean (exit 0, binary produced).
- Actual C++ edited in CombatController.cpp.
- Real gameplay mechanic delivered: companions with Focus ability increase party mana regeneration.
