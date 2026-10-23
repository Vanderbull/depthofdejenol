# Depths of Dejenol v6.33.0 Roadmap

## Version: v6.33.0
## Release Date: 2026-11-04

## Features
- Added loot chance when picking up corpses: When successfully picking up a corpse, there is a 25% chance to find loot.
- Loot can be a random Herb, Flask, or a small amount of gold (5-20).
- Items are added to the party's inventory, gold to the party's gold pool.
- Appropriate log messages inform the player of what they found.

## Changes
- src/dungeon_dialog/DungeonDialog.cpp:
  - Added loot logic in on_pickupButton_clicked() after successful corpse pickup.
  - Uses QRandomGenerator for chance and loot selection.
  - References ItemDatabase to create HeldItem objects for item loot.
  - Uses gameStateManager::spendPartyGold with negative value to add gold.
  - Uses gameStateManager::addItemToInventory to add items to inventory.
- src/core/ReleaseInfo.cpp: Updated release date to 2026-11-04.
- blacklands.pro: Increased version to 6.33.0.
