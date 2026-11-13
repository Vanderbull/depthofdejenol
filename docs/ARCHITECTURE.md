# Architecture Diagram

## Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     Blacklands Game Engine                   │
├─────────────────────────────────────────────────────────────┤
│                                                            │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐  │
│  │  Core Systems │    │  UI Systems  │    │  Combat      │  │
│  │  ┌────────┐  │    │  ┌────────┐  │    │  Systems     │  │
│  │  │ Game   │  │    │  │ Main   │  │    │  ┌────────┐  │  │
│  │  │ State  │  │    │  │ Menu   │  │    │  │ Arena  │  │  │
│  │  │ Manager│  │    │  │ Dialog │  │    │  └────────┘  │  │
│  │  └────────┘  │    └────────┘  │    │  └────────┘  │  │
│  │  ┌────────┐  │    └────────┘  │    └──────────────┘  │
│  │  │ Audio  │  │    │  │ Dungeon│  │                    │
│  │  │ Manager│  │    │  │ Dialog │  │                    │
│  │  └────────┘  │    └────────┘  │                    │
│  │  ┌────────┐  │                              ┌───────▼───────┐│
│  │  │ Network│  │                     ┌────────▼────────┐   │
│  │  │ Manager│  │                    │  Rooms & Levels │   │
│  │  └────────┘  │                    │  ┌─────────────┐   ││
│  └──────────────┘                    │  │ Room        │   ││
│                                       │  │ Level       │   ││
│                                       │  └─────────────┘   ││
│                                       │  ┌─────────────┐   ││
│                                       │  │ Event       │   ││
│                                       │  │ System      │   ││
│                                       │  └─────────────┘   ││
│                                       └─────────────────────┘│
│                                                            │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐  │
│  │  Item        │    │  Combat      │    │  Quest       │  │
│  │  System      │    │  System      │    │  System      │  │
│  │  ┌────────┐  │    │  ┌────────┐  │    │  ┌────────┐  │  │
│  │  │Item    │  │    │  │Combat  │  │    │  │Quest   │  │  │
│  │  │Database│  │    │  │System  │  │    │  │Chain   │  │  │
│  │  │        │  │    │  │System  │  │    │  │Driver  │  │  │
│  │  └────────┘  │    └────────┘  │    └────────┘  │  │
│  └──────────────┘    └──────────────┘    └──────────────┘  │
│                                                            │
└─────────────────────────────────────────────────────────────┘
```

## Core Systems

### Game State Management
- **GameStateManager** - Manages the overall game state (playing, paused, menu, etc.)
- **SaveSystem** - Handles saving/loading game state
- **VersionSystem** - Tracks game version and update state

### Combat System
- **CombatState** - Manages combat state and turn tracking
- **TurnEngine** - Handles turn-based combat logic
- **CombatActions** - Executes combat actions (attack, defend, etc.)
- **MonsterAI** - AI for enemy monsters
- **EncounterBuilder** - Builds random encounters
- **VictoryReward** - Handles victory rewards
- **CombatDeathHandler** - Manages death and respawn

### Item System
- **ItemDatabase** - Manages all item definitions
- **ItemProgression** - Handles item upgrade progression
- **ItemSystem** - General item handling

### Quest System
- **QuestChain** - Individual quest chain definition
- **QuestChainDriver** - Drives quest progression
- **QuestBoard** - UI for quest selection
- **QuestChainDialog** - Quest chain details

### Room and Level System
- **Room** - Represents a single room in the dungeon
- **Level** - A collection of rooms forming a level
- **EventSystem** - Handles room events (traps, puzzles, etc.)
- **DungeonThemes** - Thematic definitions for dungeons
- **AbyssThemes** - Thematic definitions for the Abyss

## UI Systems

### Main Menu
- **GameMenu** - Main menu dialog
- **GameSettings** - Settings configuration

### Character Creation
- **CharacterDialog** - Character creation interface
- **CharacterSheet** - Character stats display

### Dungeon Interface
- **DungeonDialog** - Main dungeon interface
- **DungeonHandlers** - Room navigation and interaction
- **DungeonMinimap** - Mini-map display
- **DungeonLevelState** - Level-specific state

### Combat Interface
- **ArenaDialog** - Area combat interface
- **CombatState** - Combat state management
- **CombatActions** - Action execution

## Architecture Principles

1. **Separation of Concerns** - UI, game logic, and data are clearly separated
2. **State-Driven** - Game flows are driven by state machines
3. **Event-Driven** - Systems communicate through events
4. **Modular** - Each system is self-contained with clear interfaces

## Key Files

| System | Header | Source |
|--------|--------|--------|
| Game State | `src/core/gameStateManager.h` | `src/core/gameStateManager.cpp` |
| Audio | `src/core/audioManager.h` | `src/core/audioManager.cpp` |
| Resolution | `src/core/ResolutionScaler.h` | `src/core/ResolutionScaler.cpp` |
| Network | `src/network_manager/NetworkManager.h` | `src/network_manager/NetworkManager.cpp` |
| Combat | `src/combat/CombatState.h` | `src/combat/CombatState.cpp` |
| Items | `src/items/ItemDatabase.h` | `src/items/ItemDatabase.cpp` |
| Quests | `src/quest_chain_dialog/QuestChainDialog.h` | `src/quest_chain_dialog/QuestChainDialog.cpp` |
