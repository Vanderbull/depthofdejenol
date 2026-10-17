<div align="center">

![Debian](https://img.shields.io/badge/OS-Debian%2013-A81D33?style=for-the-badge&logo=debian&logoColor=white)
![C++](https://img.shields.io/badge/Language-C%2B%2B%2020-blueviolet?style=for-the-badge&logo=c%2B%2B)
![Qt6](https://img.shields.io/badge/Engine-Qt%206-green?style=for-the-badge&logo=qt&logoColor=white)
![Tests](https://img.shields.io/badge/Tests-1996%20Passing-brightgreen?style=for-the-badge&logo=github-actions)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

</div>

<img width="1019" height="216" alt="title_banner" src="https://github.com/user-attachments/assets/555af4ae-3b9a-442d-8d4d-8bd12da5f87f" />

# BlackLands

**Genre:** Party-based Dungeon Crawler / RPG

**Platform:** PC (Linux, Windows, macOS)

**Core Loop:** Recruit heroes → Descend into the Depths → Kill monsters & find loot → Return to the surface to level up & identify items.

---

## Features

| Category | Details |
|----------|---------|
| **Monsters** | 401 unique monsters with portraits, categories, abilities, and resistances |
| **Spells** | 96 spells across 17 schools, each with a clear weak → strong progression |
| **Items** | 370 items with identification and equipment system |
| **Audio** | 16 soundtracks with Qt Multimedia |
| **Dungeon** | Tile-based movement, automap, 15 floors, 15 themes, bosses, secret doors |
| **Characters** | Creation, leveling, guild membership, party management (1–4 members) |
| **Town** | General Store, Guilds, Bank, Seer, Tavern, Library, Quest Board, Arena, Morgue |
| **Endgame** | New Game Plus, the Abyss mega-dungeon, an Arena, and 25 achievements |
| **Multiplayer** | Basic city server/client with chat |
| **Editors** | Monster Editor, Spellbook Editor, Map Editor |
| **Options** | Audio, autosave, combat speed, fullscreen, fonts, backup/restore |

---

## Balance

The game is calibrated for a **10–15 hour** full run:

- 15 dungeon floors, each with a recommended party level
- Monster XP scales so a level takes roughly 10–20 fights
- Every spell school has a clear progression from weak to strong
- A full-run difficulty test (`[D1]`) verifies every floor is winnable
- New Game Plus increases monster stats with each cycle

See [RELEASE_NOTES.md](RELEASE_NOTES.md) for the full version history.

---

## Game Data

> **Disclaimer:** The books below list every monster, spell, and item with full
> stats. Reading them will spoil encounters, weaknesses, and loot discoveries.
> The in-game Bestiary, Spellbook, and item descriptions only reveal what you
> have met, learned, or identified. Play first, read later.

| Book | Contents |
|------|----------|
| [docs/MONSTERS.md](docs/MONSTERS.md) | 401 monsters with stats, abilities, and resistances |
| [docs/SPELLS.md](docs/SPELLS.md) | 100 unique spell icons (256x256 AAA quality) across 18 schools |
| [docs/ITEMS.md](docs/ITEMS.md) | 370 items with stats, prices, and where they appear |
| [docs/RACES.md](docs/RACES.md) | 9 races with stat ranges, resistances, and traits |
| [docs/GUILDS.md](docs/GUILDS.md) | 11 guilds with requirements, abilities, and spell access |
| [docs/PROGRESSION.md](docs/PROGRESSION.md) | XP curve for 20 levels with level-up gains |
| [docs/RESEARCH.md](docs/RESEARCH.md) | Laired monster locations and attributes |
| [docs/DUNGEON.md](docs/DUNGEON.md) | 15 dungeon floors, themes, bosses, and special tiles |
| [docs/TOWN.md](docs/TOWN.md) | Town services, gold sinks, and progression |
| [docs/QUESTS.md](docs/QUESTS.md) | Main quest chain, side quests, and rewards |
| [docs/ENDGAME.md](docs/ENDGAME.md) | New Game Plus, Arena, Abyss, and achievements |
| [docs/COMBAT.md](docs/COMBAT.md) | Combat flow, status effects, and monster abilities |
| [docs/LIFECYCLE.md](docs/LIFECYCLE.md) | Aging, marriage, children, and retirement |

---

## Building

### Prerequisites

**Debian / Ubuntu:**

```bash
sudo apt update
sudo apt install build-essential qmake6 qt6-base-dev qt6-multimedia-dev
```

**Fedora:**

```bash
sudo dnf install gcc-c++ make qt6-qtbase-devel qt6-qtmultimedia-devel
```

**macOS (Homebrew):**

```bash
brew install qt@6
```

### Compile

```bash
# Generate the Makefile
qmake6 blacklands.pro

# Build
make -j$(nproc)
```

The binary lands at `build/bin/blacklands`.

### Run

```bash
./build/bin/blacklands
```

> **Note:** The binary expects `data/` and `resources/` alongside it. Both are copied automatically during the build.

---

## Testing

The project ships with a self-contained test suite (1996 tests):

```bash
make check
```

Or run directly:

```bash
./blacklands --selftest
```

The suite covers combat, spells, progression, dungeon generation, world systems,
and a full-run difficulty test that verifies the game is winnable start to finish.

---

## Project Structure

```
blacklands.pro          # qmake build file
src/                      # Game source code
  core/                   # Game state, constants, data loading, balance
  character_dialog/       # Character sheet, creation
  library_dialog/         # Bestiary, journal
  spell_casting/          # Spellbook, casting
  partymanager/           # Party management
  combat/                 # Combat resolution, encounter builder
  dungeon_dialog/         # Dungeon UI
  options_dialog/         # Options (audio, gameplay, display, data)
  ...
data/                     # Game data (JSON, CSV)
resources/                # Images, sounds, fonts
tools/                    # Editor and converter utilities
test/                     # Self-test suite
docs/                     # Documentation
```

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for build setup, code style, and workflow.

---

## License

[MIT](LICENSE) — Copyright (c) 2025 Vanderbull Gaming
