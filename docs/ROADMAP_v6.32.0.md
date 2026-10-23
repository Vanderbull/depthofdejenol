# Depths of Dejenol v6.32.0 Roadmap

## Version: v6.32.0
## Release Date: 2026-11-03

## Features
- Added corpse pickup functionality: Players can now pick up dead party members' bodies using the 'P' key (or via the new Pickup button in the UI) to carry them back to town for resurrection.
- Added Q_UNUSED macros to suppress unused parameter warnings in drawFog, drawExtinguisher, drawTreasure, and drawTrap functions.
- Fixed unused parameter in assignZones and unused variable in generateRandomObstacles.
- Ensured BodyLocation is used correctly from the global namespace.

## Changes
- src/dungeon_dialog/DungeonDialog.cpp:
  - Added #include "src/core/DeathRecovery.h"
  - Added key binding for 'P' key to trigger corpse pickup.
  - Added on_pickupButton_clicked() method to handle corpse pickup logic.
  - Added Q_UNUSED macros for unused parameters in drawFog, drawExtinguisher, drawTreasure, drawTrap.
  - Fixed BodyLocation usage (removed DeathRecovery:: qualifier).
  - Fixed missing semicolon in logMessage.
  - Removed unused variable 'extraRooms' in generateRandomObstacles.
  - Added Q_UNUSED(rng) in assignZones.
- src/core/ReleaseInfo.cpp: Updated release date to 2026-11-03.
- blacklands.pro: Increased version to 6.32.0.
