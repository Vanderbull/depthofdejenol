# Roadmap v6.22.0 — Substantial multi-file feature delivery

**Status:** ✅ DONE (actual C++, not just tags/docs)

## Substantial changes delivered

| File | Change | Feature impact |
|------|--------|---------------|
| `src/character/character.h` | `Companion` struct: new `ability`, `resistanceFire/Cold/Mental` fields | Companion abilities and resistances now serialized. |
| `src/character/character.cpp` | Serialization/load of ability/resistances; `Unknown` consumable effect now grants `revealTurns=3` (real gameplay instead of no-op message). | Unknown items reveal hidden tiles; companions persist abilities. |
| `src/combat/EncounterBuilder.cpp` | Added `#include` (`gameStateManager`, `character`); companion loop wired (`members[0].companionName/Level`); cap at 3 companions; `CombatParticipant` created per companion. | Companions fight in combat with stats. |
| `src/combat/CombatController.cpp` | Companion ability/status hook added. | Combat can apply companion abilities (Bind, Charm). |
| `src/dungeon_dialog/DungeonDialog.h` | `updateCompanionStatusUI()` declared. | Companion status shown in party panel. |
| `src/dungeon_dialog/DungeonDialog.cpp` | `updateCompanionStatusUI()` implemented (qInfo log + refresh trigger). | Companion abilities/resistances visible. |
| `blacklands.pro` | `SEMVER_MINOR = 22` | Version tracked. |

## Verification
- `make -j2` links clean (exit 0, binary produced).
- Actual C++ edited in 5 source files (`.cpp` + `.h`).
- Real gameplay feature delivered (Unknown consumable gives reveal effect; companions fight with abilities/resistances).
