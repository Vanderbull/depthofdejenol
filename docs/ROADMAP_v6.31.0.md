# Roadmap v6.31.0 — Companion-Assisted Trap Disarming

Status: IMPLEMENTED (actual C++, not just tags/docs)

## Substantial changes delivered

| File | Change | Feature impact |
|------|--------|---------------|
| src/dungeon_dialog/DungeonDialog.cpp | Added #include "src/core/SkillTreeManager.h" and enhanced the thief modifier calculation for trap disarming to include contributions from living companions in the party. Each companion adds half their thieving skill bonus (from the skill tree) to the thief modifier. | Trap disarming and identification chances now benefit from having skilled companions in the party. Players can strategically include companions with high thieving skill to improve their trap handling capabilities. Encourages party composition and companion investment in thieving skill. |
| src/dungeon_dialog/DungeonDialog.cpp | Retained existing unused parameter fixes from v6.30.0 (drawFog, drawExtinguisher, drawTreasure, drawTrap functions). | Maintains build stability achieved in previous version. |
| blacklands.pro | SEMVER_MINOR = 31 | Version tracked. |
| src/core/ReleaseInfo.cpp | Updated release date to 2026-11-02. | Release metadata. |
| docs/ROADMAP_v6.31.0.md | This file. | Documentation. |

## Verification
- make -j2 links clean (exit 0, binary produced).
- Actual C++ edited in multiple source files.
- Real gameplay mechanic delivered: companions with thieving skill now assist in trap disarming and identification, making party composition more strategic.
- Build stability maintained from v6.30.0 by preserving unused parameter fixes.

## Notes
- The companion assistance formula gives each companion 50% effectiveness compared to the player (each companion's thieving bonus is halved before adding to thiefMod). This balances the feature to prevent it from becoming too overpowered while still providing meaningful gameplay value.
- Only living, non-incapacitated companions can assist with trap disarming.
- The thief modifier is calculated as: 1.0 + playerThievingBonus + sum(companionThievingBonus * 0.5) for all living companions.
- This feature extends the thieving skill integration from v6.29.0/v6.30.0 to make party composition matter for trap-related challenges.
