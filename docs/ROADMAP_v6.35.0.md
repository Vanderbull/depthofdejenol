# Depths of Dejenol v6.35.0 Roadmap

## Version: v6.35.0
## Release Date: 2026-11-06

## Features
- Removed the static title text ("THE DEPTHS OF DEJENOL") and version label ("BLACK LANDS — v5.0.0") from the main menu.
- The title and version labels are no longer displayed, cleaning up the main menu UI.
- Updated internal version tracking to v6.35.0 and release date to 2026-11-06.

## Changes
- src/blacklands.h:
  - Removed private QLabel member declarations `m_titleLabel` and `m_versionLabel`.
- src/blacklands.cpp:
  - Removed Title Section in `GameMenu::setupUI()`:
    - Original code that created `m_titleLabel` and added it to the layout.
    - Original code that created `m_versionLabel` and added it to the layout.
    - A comment line now marks the spot where the title and version previously lived.
  - Kept the layout spacing (20px) that used to separate the title/version section, so the menu still has a clean top margin.
  - Updated member variable declarations in the header file accordingly.
- src/core/ReleaseInfo.cpp: Updated `releaseDate()` to return "2026-11-06".
- blacklands.pro: Bumped `SEMVER_MINOR` to 35 for version v6.35.0.
- docs/ROADMAP_v6.35.0.md: Added roadmap entry documenting the version.
