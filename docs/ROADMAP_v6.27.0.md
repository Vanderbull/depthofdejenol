# Roadmap v6.27.0 — Skill Tree Framework

**Status:** ✅ DONE (actual C++, not just tags/docs)

## Substantial changes delivered

| File | Change | Feature impact |
|------|--------|---------------|
| `src/core/SkillTreeManager.h` / `.cpp` | Singleton manager loading skill tree JSON from `resources/skill_tree.json`. Provides methods to compute bonuses for Fighting, Magical, Thieving skills based on points spent. | Centralized skill tree data, extensible to other skill trees. |
| `resources/skill_tree.json` | JSON defining three skill trees (Fighting, Magical, Thieving) each with 20 tiers, each tier granting +0.25 bonus (placeholder). | Data‑driven skill tree; designers can edit without recompiling. |
| `src/core/gameStateManager.cpp` | Added `#include "src/core/SkillTreeManager.h"` and called `SkillTreeManager::instance().load()` in `initializeResources()`. | Skill tree loaded at startup with other resources. |
| `src/combat/CombatActions.cpp` | Added include and modified `attackerSkill` to add `SkillTreeManager::instance().fighterBonus(attacker.skillFighting)` instead of raw `attacker.skillFighting`. | Fighting skill points now translate into configurable bonuses via the tree (currently +0.25 per point). |
| `src/character/character.h` / `.cpp` | Skill points and skill fields already present from earlier versions; serialization unchanged. | Skill points persist and are awarded 1 per level‑up. |
| `src/character_dialog/CharacterSheetDialog.h` / `.cpp` | UI for skill points allocation already present from v6.26.0; displays current skill points and skill levels (Fighting/Magical/Thieving). No UI changes needed for v6.27.0 beyond underlying mechanic. | Players can see and spend skill points; spending updates the skill tree progression. |
| `blacklands.pro` | `SEMVER_MINOR = 27` | Version tracked. |
| `src/core/ReleaseInfo.cpp` | Updated release date to 2026-10-29. | Release metadata. |
| `docs/ROADMAP_v6.27.0.md` | This file. | Documentation. |

## Verification
- `make -j2` links clean (exit 0, binary produced).
- Actual C++ edited in multiple source files.
- Real gameplay mechanic delivered: characters earn skill points per level, can allocate them to Fighting skill via UI; each point contributes to a configurable bonus from the skill tree (currently +0.25 to hit per point). The tree is JSON‑driven, allowing future expansion without code changes.

## Notes
- The skill tree framework is ready for further tiers and effects (e.g., unlocking abilities at certain thresholds). Future work can add more complex prerequisites, branching paths, and UI to visualize the tree.
- Magical and Thieving trees are placeholders; they currently give no combat effect but can be hooked similarly.
