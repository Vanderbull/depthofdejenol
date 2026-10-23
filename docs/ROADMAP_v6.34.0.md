# Depths of Dejenol v6.34.0 Roadmap

## Version: v6.34.0
## Release Date: 2026-11-05

## Features
- Enhanced aging rules: mental stats (INT, WIS, CHA) now also subject to decay past the decay threshold.
- Physical stats (STR, CON, DEX) decay chance remains 10% per year after threshold.
- Mental stats decay chance is 5% per year after threshold.
- This adds more realism to long-lived characters, especially elves and gnomes, who may now see mental stat decline alongside physical decline in very old age.

## Changes
- src/core/AgingRules.cpp:
  - Modified applyYearOfAging to include a 5% chance for each of INT, WIS, CHA to decay by 1 point when past decay threshold.
  - Kept existing 10% chance for STR, CON, DEX decay.
  - Updated log messages to reflect which stats are affected.
- src/core/ReleaseInfo.cpp: Updated release date to 2026-11-05.
- blacklands.pro: Increased version to 6.34.0.
