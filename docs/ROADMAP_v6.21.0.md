# Roadmap v6.21.0 — Unknown consumable gameplay effect

**Feature:** The "Unknown" consumable effect (`ConsumableResolver::Effect::Unknown`) previously displayed `"not yet implemented"` with no gameplay impact. It now grants a brief `revealTurns` status (3 turns) that reveals hidden dungeon tiles — giving the item real utility instead of a no-op message.

**Evidence in source:**
- `src/character/character.h`: `revealTurns` field added.
- `src/character/character.cpp`: `case Unknown:` sets `revealTurns = 3`.
- `blacklands.pro`: version `6.21.0`.
- Build PASS (`make -j2` links, binary produced).
