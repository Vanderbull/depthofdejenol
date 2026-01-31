
<div align="center">
  
![Debian](https://img.shields.io/badge/OS-Debian%2013-A81D33?style=for-the-badge&logo=debian&logoColor=white)
![Coffee](https://img.shields.io/badge/Maintained%20with-Coffee-6F4E37?style=for-the-badge&logo=coffeescript&logoColor=white)
![Quest](https://img.shields.io/badge/Quest-Building%20with%20Bear-orange?style=for-the-badge&logo=adventure-caps&logoColor=white)
![Open Source](https://img.shields.io/badge/Loot-Open%20Source-brightgreen?style=for-the-badge&logo=github&logoColor=white)
[![Build Status](https://img.shields.io/github/actions/workflow/status/Vanderbull/depthofdejenol/build.yaml?branch=main&label=QUEST%20STATUS&logo=github&style=for-the-badge)](https://github.com/Vanderbull/depthofdejenol/actions)
![Language](https://img.shields.io/badge/Skill-C%2B%2B%2020-blueviolet?style=for-the-badge&logo=c%2B%2B)
![Qt6](https://img.shields.io/badge/Engine-Qt%206-green?style=for-the-badge&logo=qt&logoColor=white)
![Graphics](https://img.shields.io/badge/Visuals-Raster%202D-E91E63?style=for-the-badge&logo=image&logoColor=white)
![Spells](https://img.shields.io/badge/Casting-Core%20Logic-9370DB?style=for-the-badge&logo=magic&logoColor=white)
![Recursion](https://img.shields.io/badge/Spell-Recursive%20Loop-4B0082?style=for-the-badge&logo=darkreader&logoColor=white)
![Memory](https://img.shields.io/badge/Potion-Memory%20Safety-EC2F2F?style=for-the-badge&logo=flask&logoColor=white)
![Optimization](https://img.shields.io/badge/Potion-High%20FPS-2ECC71?style=for-the-badge&logo=speedtest&logoColor=white)
![Assets](https://img.shields.io/badge/Potion-Asset%20Mana-3498DB?style=for-the-badge&logo=liquity&logoColor=white)
[![Quest Status](https://img.shields.io/github/actions/workflow/status/Vanderbull/depthofdejenol/build.yaml?label=QUEST%20STATUS&logo=github&style=for-the-badge)](https://github.com/Vanderbull/depthofdejenol/actions)

</div>

<img width="1019" height="216" alt="title_banner" src="https://github.com/user-attachments/assets/555af4ae-3b9a-442d-8d4d-8bd12da5f87f" />
<img width="1404" height="515" alt="races_banner" src="https://github.com/user-attachments/assets/84a36ce2-efab-49fe-8c1d-a92310ec23ba" />

# Black Land: Depth of Dejenol

Title: Black Land (Working Title)

Genre: Party-based Dungeon Crawler / RPG

Platform: PC (Windows/Mac/Linux)

Core Loop: Recruit heroes → Descend into the Depths → Kill monsters/Find loot → Return to the surface to level up/identify items.

# Core Pillars

The Grid: Movement is strictly tile-based and 90-degree turns.

The Grind: Leveling should feel meaningful. The 1% stat increases matter.

The Mystery: Hidden doors, teleporters, and pits must be mapped (or memorized) by the player.

The Library: A massive database of items and monsters that rewards "collecting" knowledge.
    
## Features

- 401 Monsters (with Monster Editor tool and monster conversion utilities)
- 100 Spells (with Spellbook Editor tool)
- 366 Items
- 16 Soundtracks
- Dungeon map and automap support (levels, parties, exploration)
- Character management and dialogs (creation, confinement, party info, morgue)
- In-game facilities: General Store, Guilds, Bank, Seer
- Statistics and records dialog
- Extremly basic city multiplayer server and client functions with chat


For more details, see: [Mdata1.mdr documentation](https://dejenol.com/index.php?title=Mdata1.mdr)

# ⚔️ Black Land

<div align="center">

[![Quest Status](https://img.shields.io/github/actions/workflow/status/Vanderbull/depthofdejenol/build.yaml?label=QUEST%20STATUS&logo=github&style=for-the-badge)](https://github.com/Vanderbull/depthofdejenol/actions)
![Realm](https://img.shields.io/badge/Realm-Debian%2013-red?style=for-the-badge&logo=debian&logoColor=white)

</div>

---

## 📜 The Arcanum (Technical Lore)

Every great adventurer needs to understand the laws of the world they inhabit. In the realm of *Depth of Dejenol*, these laws are written in **C++20** and powered by the **Qt6 Engine**.

### 🧪 Equipped Potions (Resource Management)
* **Potion of Memory Safety (RAII):** We use Smart Pointers ($inline$ $std::unique\_ptr$ $inline$ and $inline$ $std::shared\_ptr$ $inline$) to ensure memory is reclaimed. No manual `delete` calls are allowed.
* **Elixir of High FPS (Optimization):** Utilizing the **Qt RHI**, we draw frames efficiently without the overhead of heavy abstractions.
* **Draft of Resource Loading:** Assets are managed via **Qt Resource Files (.qrc)**, bundling textures into the binary "flask" for instant access.

### 🪄 Active Spells (Code Logic)
* **The Signal-Slot Incantation:** Communication is handled via **Qt's Signal/Slot system**, allowing decoupled "spell casting" between objects.
* **The Bear's Scrying Mirror (LSP):** We use **Bear** to generate a `compile_commands.json`, granting your editor foresight into the code structure.
* **The Ritual of qmake:** The `.pro` blueprint must be read by **qmake6** to prepare the "Scroll of Construction" (Makefile).

---

## 🗺️ Quest Log (Roadmap)

| Quest | Status | Reward |
| :--- | :--- | :--- |
| **The Foundation** | ✅ Complete | Basic Engine & Qt Window |
| **Cleaning the dungeon** | 🏃 In Progress | Nicer looking game | 
| **Dungeon Generation** | 🔒 Locked | Procedural Level Logic |
| **The Final Boss** | 🔒 Locked | Release v1.0 |

---

## 🛡️ The Developer's Character Sheet

| Attribute | Required Level | Tool |
| :--- | :--- | :--- |
| **Intelligence** | C++20 Standard | `g++` / `clang` |
| **Wisdom** | Qt 6.x Framework | `qt6-base-dev` |
| **Perception** | Language Server |

---
