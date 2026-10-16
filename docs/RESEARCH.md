# Monster Research Book

> **Disclaimer:** This document lists laired monster data — where they spawn,
> their attributes, and known drops. Reading it will spoil dungeon exploration
> and encounters. The in-game map and exploration reveal these naturally.

## Overview

- **Source:** `data/monster_research_data.json` (partial — the file has a syntax error
  at line 2, so only the readable portion is documented here)
- **Contents:** Laired monster locations, attributes, and notes

![Bestiary](resources/images/bestiary.png)

## Laired Monsters

| Name | Level | Location | Size | Resistances | Notes |
|------|-------|----------|------|-------------|-------|
| Clean-Up | 1 | 20,20 | — | — | A laired monster on level 1 |
| Aboleth | 1 | 27,8 | — | — | Significant landmark on the first level map |
| Arthropod | 2 | 20,14 | — | — | — |
| Dwarf (corrupted) | 1 | — | Small | Mind 80%, Fire 45%, Cold 50%, Electrical 50% | Militaristic or corrupted variant of the Dwarven race |

## Notes

The full research data file contains ~1104 lines of laired monster placements
across all 15 dungeon floors. Due to a JSON syntax error in the source file,
only the first few entries are documented here. The in-game dungeon exploration
reveals laired monsters as you find them.

## What the Research Data Contains

For each laired monster:

- **name** — monster name
- **level** — dungeon floor (1-15)
- **locations** — x,y coordinates on the dungeon map
- **attributes** — size, resistances, lore notes
- **known_drops** — what the monster drops when killed
- **notes** — hints about the monster's role in the dungeon
