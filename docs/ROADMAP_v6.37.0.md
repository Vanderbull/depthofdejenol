# Depths of Dejenol v6.37.0 Roadmap

## Version: v6.37.0
## Release Date: 2026-10-23

## Features
- Main menu UI modernization: 
  - New left sidebar with menu items (Continue, New Game, Load Game, Options, Extras, Quit Game).
  - Right content area with Continue button, subtitle, party preview, main actions grid (New Game, Create Character, Assemble Party, Save Game, Load Game, Hall of Records), footer (Options, Multiplayer, Tutorial, Quit), press hints, and partner logos.
  - Modern streamlined look using semi-transparent dark panels and a gold accent for the Continue button.
- All existing buttons and the background image are preserved.
- The layout adapts to window resizing and respects the font chosen in Options.
- Added placeholder partner logos (Monolith and Warner) for brand recognition.

## Changes
- **src/blacklands.h**: 
  - Added new private members for sidebar, content, sidebar buttons, labels, etc.
- **src/blacklands.cpp**: 
  - Rewrote `setupUI()` to build the new layout.
  - Added connections for sidebar buttons.
  - Added placeholder partner logos.
  - Updated `syncFontsToAppFont()` to include new sidebar widgets.
  - Removed static title and version labels.
- **src/MainMenu.qss**: 
  - New styles for sidebar, content, buttons, labels, and placeholders.
- **resources/images/monolith.png**: 
  - Placeholder Monolith logo.
- **resources/images/warner.png**: 
  - Placeholder Warner logo.
- **blacklands.pro**: 
  - Bumped version to v6.37.0 (`SEMVER_MINOR=37`, `SEMVER_PATCH=0`).
- **docs/ROADMAP_v6.37.0.md**: 
  - Added roadmap entry for this version.

## Notes
- All existing functionality preserved.
- Builds pass with `make` and `make check`.
- No credentials or secrets introduced.
