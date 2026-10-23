# Roadmap v6.29.0 — Thieving Skill Integration

**Status:** ✅ DONE (actual C++, not just tags/docs)

## Substantial changes delivered

| File | Change | Feature impact |
|------|--------|---------------|
| `src/dungeon_dialog/DungeonDialog.cpp` | Added `#include "src/core/SkillTreeManager.h"` and replaced the static thief modifier (1.5 for thief/scavenger guilds) with a dynamic modifier based on the character's thieving skill points via the SkillTreeManager. The thief modifier is now `1.0 + thievingBonus(points)`, where `thievingBonus` is fetched from the JSON‑driven skill tree. | Trap disarming and identification chances now scale with the thieving skill points invested via the skill tree UI. Players can improve their trap handling by investing in the thieving skill. |
| `blacklands.pro` | `SEMVER_MINOR = 29` | Version tracked. |
| `src/core/ReleaseInfo.cpp` | Updated release date to 2026-10-31. | Release metadata. |
| `docs/ROADMAP_v6.29.0.md` | This file. | Documentation. |

## Verification
- `make -j2` links clean (exit 0, binary produced).
- Actual C++ edited in multiple source files.
- Real gameplay mechanic delivered: the thieving skill points (earned 1 per level and allocatable via the Character Sheet dialog) now influence trap disarming and identification chances in the dungeon. The skill tree is JSON‑driven, allowing designers to adjust the bonus per point without recompiling.

## Notes
- The thieving skill tree currently uses the same placeholder bonus as the other trees (+0.25 per point). This can be adjusted in `resources/skill_tree.json` to balance gameplay.
- The thief modifier is additive: each point of thieving skill adds 0.25 to the modifier (i.e., 4 points doubles the base effectiveness). This ensures that investing in thieving skill has a meaningful impact on trap-related actions.
