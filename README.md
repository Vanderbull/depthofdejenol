<div align="center">

![Debian](https://img.shields.io/badge/OS-Debian%2013-A81D33?style=for-the-badge&logo=debian&logoColor=white)
![C++](https://img.shields.io/badge/Language-C%2B%2B%2020-blueviolet?style=for-the-badge&logo=c%2B%2B)
![Qt6](https://img.shields.io/badge/Engine-Qt%206-green?style=for-the-badge&logo=qt&logoColor=white)
![Tests](https://img.shields.io/badge/Tests-840%20Passing-brightgreen?style=for-the-badge&logo=github-actions)
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
| **Spells** | 100 spells across multiple guilds |
| **Items** | 366 items with identification and equipment system |
| **Audio** | 16 soundtracks with Qt Multimedia |
| **Dungeon** | Tile-based movement, automap, multiple levels |
| **Characters** | Creation, leveling, guild membership, party management |
| **Facilities** | General Store, Guilds, Bank, Seer, Tavern |
| **Multiplayer** | Basic city server/client with chat |
| **Editors** | Monster Editor, Spellbook Editor, Map Editor |

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

The project ships with a self-contained test suite (840 tests):

```bash
make check
```

Or run directly:

```bash
./blacklands --selftest
```

---

## Project Structure

```
blacklands.pro          # qmake build file
src/                      # Game source code
  core/                   # Game state, constants, data loading
  character_dialog/       # Character sheet, creation
  library_dialog/         # Bestiary, journal
  spell_casting/          # Spellbook, casting
  partymanager/           # Party management
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
