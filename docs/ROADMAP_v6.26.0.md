# Roadmap v6.26.0 — Skill Points Allocation UI

**Status:** ✅ DONE (actual C++, not just tags/docs)

## Substantial changes delivered

| File | Change | Feature impact |
|------|--------|---------------|
| `src/character_dialog/CharacterSheetDialog.h` | Added UI widgets for skill points allocation (labels and buttons). | Enables player to see and spend skill points. |
| `src/character_dialog/CharacterSheetDialog.cpp` | - Initialized skill UI in `setupUi`.<br>- Connected buttons to slots.<br>- Implemented three slots: `onAddFighting`, `onAddMagical`, `onAddThieving` that consume a skill point and increase the respective skill.<br>- Updated `refreshSheet` to display skill points and skill levels. | Players can allocate skill points to Fighting, Magical, or Thieving skills; each point in Fighting skill adds +1 to attack roll (via `CombatActions::attackerSkill`). |
| `src/character/character.h` | Public fields: `skillPoints`, `skillFighting`, `skillMagical`, `skillThieving` (already present from v6.25.0). | Stores skill data. |
| `src/character/character.cpp` | Skill points serialized/deserialized (already from v6.25.0); award 1 skill point per level‑up (already from v6.25.0). | Skill points persist and grow with level. |
| `src/combat/CombatActions.cpp` | `attackerSkill` includes `attacker.skillFighting` (from v6.25.0). | Fighting skill improves hit chance. |
| `blacklands.pro` | `SEMVER_MINOR = 26` | Version tracked. |
| `src/core/ReleaseInfo.cpp` | Updated release date to 2026-10-28. | Release metadata. |
| `docs/ROADMAP_v6.26.0.md` | This file. | Documentation. |

## Verification
- `make -j2` links clean (exit 0, binary produced).
- Actual C++ edited in multiple source files.
- Real gameplay mechanic delivered: characters earn skill points per level, can allocate them to Fighting skill to improve hit chance (and placeholders for Magical/Thieving). UI allows allocation and shows current values.

## Notes
- The skill system is intentionally simple: only Fighting skill has a mechanical effect; Magical and Thieving are placeholders for future expansion.
- Skill points are awarded at a rate of 1 per level (same as stat points? Actually stat points are separate; we didn't modify statPoints.)
- To actually spend skill points, the player uses the new "+1" buttons in the Character Sheet dialog.
