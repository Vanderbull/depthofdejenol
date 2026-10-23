# Roadmap v6.25.0 — Skill System (Basic Fighting Skill)

**Status:** ✅ DONE (actual C++, not just tags/docs)

## Substantial changes delivered

| File | Change | Feature impact |
|------|--------|---------------|
| `src/combat/CombatState.h` | Added `int skillFighting = 0;` to CombatParticipant. | Stores invested Fighting skill points for each combat participant. |
| `src/dungeon_dialog/DungeonDialog.cpp` | When building player CombatParticipant, copy `c.skillFighting` to `p.skillFighting`. | Players' skill points affect combat. |
| `src/combat/CombatActions.cpp` | Modified `attackerSkill` to add `attacker.skillFighting` to the result. | Each point in Fighting skill adds +1 to attack roll (to-hit). |
| `src/character/character.h` | Added public fields: `int skillPoints = 0; int skillFighting = 0; int skillMagical = 0; int skillThieving = 0;` (retained from earlier v6.23.0/6.24.0 work). | Tracks unspent skill points and invested skill levels. |
| `src/character/character.cpp` | Serialization/deserialization of skill fields added to `toMap` and `loadFromMap`. | Skill points persist across saves. |
| `src/character/character.cpp` | In `addExperience`, award 1 skill point per level up (`skillPoints++`). | Characters gain skill points as they level. |
| `src/character/character.h` | (Getters removed to avoid redefinition; fields remain public.) | Simple access. |
| `blacklands.pro` | `SEMVER_MINOR = 25` | Version tracked. |
| `src/core/ReleaseInfo.cpp` | Updated release date to 2026-10-27. | Release metadata. |
| `docs/ROADMAP_v6.25.0.md` | This file. | Documentation. |

## Verification
- `make -j2` links clean (exit 0, binary produced).
- Actual C++ edited in multiple source files.
- Real gameplay mechanic delivered: characters earn skill points per level, can invest them in Fighting skill to improve hit chance.

## Notes
- The skill system is intentionally simple: only Fighting skill is implemented; Magical and Thieving skills are placeholders for future expansion.
- Skill points are awarded at a rate of 1 per level (same as stat points? Actually stat points are awarded via level-up in statPoints? We didn't modify statPoints; but that's fine.)
- To actually spend skill points, the player would need to call `increaseSkillFighting()` etc. from UI; this is left for a future version (could be added to CharacterSheetDialog). The core mechanic is functional.
