# Depth of Dejenol — Tools Editors Documentation

## Overview

The `tools/editors/` directory contains 10 standalone Qt6 C++ applications for viewing and editing game data files used by Depth of Dejenol. Each editor is a self-contained executable built from its own `.pro` file. CSV-based editors share a common base class (`CommonCsvEditor`); non-CSV editors are standalone Qt widgets.

## Build System

### Toolchain

- **Qt6 qmake**: `/usr/lib/qt6/bin/qmake`
- **C++ standard**: C++20 (`CONFIG += c++20`)
- **Qt modules**: `core` and `widgets` (some editors also link `gui`)

### Build command pattern

```bash
cd tools/editors/<editor_name>
rm -f Makefile
/usr/lib/qt6/bin/qmake <editor_name>.pro -o Makefile
make -j$(nproc)
```

Each editor directory is self-contained — run qmake and make from within the editor's subdirectory.

### Master project

`tools/editors/editors.pro` is a master aggregating project. It lists all editor source files but produces a single binary that runs editors as separate processes. For individual editor binaries, use the per-editor `.pro` files.

## Shared Base: CommonCsvEditor

### Files

- `tools/editors/common_csv_editor.h` — base class header
- `tools/editors/common_csv_editor.cpp` — base class implementation

### Purpose

`CommonCsvEditor` is a `QMainWindow` subclass that provides a reusable framework for editing CSV game data files. It handles:

- **CSV loading**: Parses semicolon-delimited CSV files with a header row, populating a `QTableWidget`
- **CSV saving**: Escapes fields containing semicolons, quotes, or newlines, then writes back
- **Table UI**: Dark-themed spreadsheet with sorting, editing, and row selection
- **Filter bar**: Text filter that hides non-matching rows
- **Detail pane**: Bottom panel showing HTML-rendered detail for the selected row (subclass override point)
- **JSON export**: Exports the current table data as a JSON array

### Key methods

| Method | Purpose |
|--------|---------|
| `CommonCsvEditor(const QString& csvPath, const QString& windowTitle, QWidget* parent)` | Constructor — loads CSV, sets up UI |
| `loadCsv(const QString& path)` | Load and parse a CSV file into the table |
| `saveCsv(const QString& path)` | Write table contents back to CSV |
| `csvEscape(const QString& s) const` | Escape a field for CSV output (handles `;`, `"`, `\n`) |
| `detailHtml(int row) const` | Virtual — returns HTML for the detail pane. Subclasses override this. |
| `exportToJson(const QString& path)` | Export table data as JSON array |
| `refreshTable()` | Re-read the CSV from disk and rebuild the table |

### Subclassing

CSV editors derive from `CommonCsvEditor` and override `detailHtml(int row)` to provide a rich HTML preview of the selected row. The constructor calls the base constructor with the CSV file path and window title.

## Editors

### 1. Item Editor (`item_editor/`)

- **Binary**: `itemeditor`
- **Target**: `tools/itemconverter/data/MDATA3.csv` (item definitions)
- **Base**: `CommonCsvEditor`
- **Detail pane**: Shows item name, type, stats, and other fields in a formatted HTML table
- **CSV**: Semicolon-delimited with header row

### 2. Character Editor (`character_editor/`)

- **Binary**: `character-editor`
- **Target**: `tools/characterconverter/data/MDATA4.csv` (character/class definitions)
- **Base**: `CommonCsvEditor`
- **Detail pane**: Shows character name, stats, racial modifiers, and class information

### 3. Spell Editor (`spell_editor/`)

- **Binary**: `spell-editor`
- **Target**: `tools/spellconverter/data/MDATA2.csv` (spell definitions)
- **Base**: `CommonCsvEditor`
- **Detail pane**: Shows spell name, school, level, mana cost, and effect description

### 4. Dungeon Editor (`dungeon_editor/`)

- **Binary**: `dungeon-editor`
- **Target**: `tools/dungeoncconverter/data/MDATA11.csv` (dungeon/room definitions)
- **Base**: `CommonCsvEditor`
- **Detail pane**: Shows dungeon name, room layout, monsters, treasures, and special properties

### 5. Monster Editor (`monster_editor_qt/`)

- **Binary**: `monster-editor`
- **Target**: `tools/monsterconverter/data/MDATA5.csv` (401 monster definitions)
- **Base**: `CommonCsvEditor`
- **Detail pane**: Shows monster name, type, stats (Str/Dex/Int/Cha/Con/Wis), HP, attacks, and special abilities

### 6. General Store Editor (`generalstore_editor/`)

- **Binary**: `generalstoreeditor`
- **Target**: `tools/generalstoreconverter/data/MDATA_Store.csv` (shop inventory)
- **Base**: `CommonCsvEditor`
- **Detail pane**: Shows item name, category, price in GP, weight, and rarity with color-coded rarity labels (Common/Uncommon/Rare/Epic/Legendary)

### 7. Complication Calculator (`complication_calculator/`)

- **Binary**: `complication-calc-editor`
- **Target**: Wraps `tools/complication_calculator/ComplicationCalculator.h` logic
- **Type**: Standalone Qt widget (not CSV-based)
- **UI**: Input fields for character age, race max age, and race selection. Displays calculated complication risk percentage with color-coded risk level (teal/blue/purple/red) and a progress-bar style indicator.
- **Formula**: `(Age × 2 / Race Max Age) × 100`
- **Includes**: `ComplicationCalculator.cpp` from the tools directory

### 8. Complication Effect (`complication_effect/`)

- **Binary**: `complication-effect-editor`
- **Target**: Wraps `tools/complication_effect/ComplicationEffect.h` logic
- **Type**: Standalone Qt widget (not CSV-based)
- **UI**: Input spin boxes for 6 stats (Str/Dex/Int/Cha/Con/Wis), max hits, and age. Racial minimum spin boxes. Apply button runs the complication effect calculation (subtract 5 from each stat, floor at racial minimum, multiply max hits by 0.8333, add 15 years to age). Results displayed in a before/after table.
- **Includes**: `ComplicationEffect.cpp` from the tools directory

### 9. Game Data Editor (`gamedata_editor/`)

- **Binary**: `game-data-editor`
- **Target**: `MDATA1.js` (game metadata JSON/JS file)
- **Type**: Standalone Qt widget (JSON-based, not CSV)
- **UI**: Four tabs — Races (9 entries, 30 columns), Guilds (9 entries, 14 columns), Item Types (37 entries, 2 columns), Monster Types (2 entries, 2 columns). Each tab has a `QTableWidget` for editing and a `QTextBrowser` preview showing the JSON for that section. Loads MDATA1.js by default, with File > Load/Save menu options.
- **Data handling**: Reads JS format (`const gameData = {...};`) by extracting the JSON object between braces. Writes standard JSON.
- **Includes**: No external logic files — pure Qt widget editing the JSON directly

### 10. Map Editor (`map_editor/`)

- **Binary**: `map-editor-qt`
- **Target**: Wraps `tools/map_editor/mapeditor.h` + `mapeditor.cpp` (tile-based map data)
- **Type**: Standalone Qt canvas application (not CSV-based)
- **UI**: 
  - Custom `MapCanvas` widget that paints a tile grid (empty/wall/floor/water) with zoom (1×–8×)
  - Left-click to paint tiles, toggle Walls mode to add/remove walls
  - Tile palette (Empty/Wall/Floor/Water) as toggle buttons in the toolbar
  - Map size controls (width/height spin boxes), New/Load/Save file menu
  - Legend panel showing tile colors, controls help text
  - Load/Save maps as JSON files
- **Includes**: `mapeditor.cpp` from `tools/map_editor/`

## File Format Details

### CSV Format

All CSV files use semicolon (`;`) as the field delimiter with a header row. Fields containing semicolons, double quotes, or newlines are escaped using double-quote quoting with embedded quotes doubled (standard CSV escaping).

Example row:
```
ItemName;ItemID;Category;Price;Weight;Rarity;...
```

### MDATA1.js Format

The game data file is a JavaScript constant assignment:
```javascript
const gameData = {
  "Races": [...],
  "Guilds": [...],
  "ItemTypes": [...],
  "MonsterTypes": [...]
};
```

The game data editor strips the `const gameData =` prefix and trailing `;` to extract the JSON object, then parses it with `QJsonDocument`.

## .pro File Structure

Each editor's `.pro` file follows this pattern:

```pro
QT += core widgets
CONFIG += c++20
CONFIG -= app_bundle
TARGET = <binary-name>
TEMPLATE = app

HEADERS += <local headers> \
           ../../editors/common_csv_editor.h

SOURCES += <local source> \
           ../../editors/common_csv_editor.cpp \
           main.cpp

INCLUDEPATH += ../../editors
```

Key points:
- `INCLUDEPATH += ../../editors` resolves from `tools/editors/<name>/` to `tools/editors/`, finding `common_csv_editor.h` and `.cpp`
- `HEADERS` must list `common_csv_editor.h` so qmake's moc scans it for `Q_OBJECT`
- `SOURCES` lists the editor's `.cpp`, the shared `common_csv_editor.cpp`, and `main.cpp`
- Non-CSV editors (complication_calculator, complication_effect, gamedata_editor, map_editor) include their respective logic `.cpp` files from `tools/` instead of `common_csv_editor.cpp`

## Styling

All editors use a consistent dark theme:
- Background: `#1e1e1e` (main), `#252526` (panels/groups)
- Text: `#d4d4d4` (primary), `#888` (secondary)
- Accent: `#569cd6` (headers, active elements)
- Buttons: `#0e639c` primary, `#4a4a4a` secondary
- Grid lines: `#3a3a3c`

The style is applied via `setStyleSheet()` on the main window with Qt style sheet syntax.

## Dependencies

Each editor links against:
- `libQt6Widgets.so`
- `libQt6Gui.so`
- `libQt6Core.so`
- `libGLX.so` / `libOpenGL.so` (for Qt GUI)

No other external libraries are required.

## Data Flow

1. **CSV editors**: CSV file → `CommonCsvEditor::loadCsv()` → `QTableWidget` → user edits → `saveCsv()` / `exportToJson()`
2. **Complication calculator**: Spin box values → `calculateComplicationChance()` → percentage display
3. **Complication effect**: Spin box values → `ComplicationEffect::applyEffects()` → before/after table
4. **Game data editor**: MDATA1.js → `QJsonDocument::fromJson()` → `QJsonObject` → tab tables → user edits → `QJsonDocument::toJson()` → file
5. **Map editor**: Map JSON → `MapEditor::loadMap()` → `MapEditor` data → `MapCanvas::paintEvent()` → user paints → `MapEditor::saveMap()`

## Notes

- The `common_csv_editor.h` and `.cpp` files live in the parent `tools/editors/` directory, not in a subdirectory. This avoids relative path complications.
- The `common_csv_editor/` subdirectory exists but is empty — it was used during early development attempts and later abandoned.
- Each editor is a standalone application; they do not share a process or communicate with each other.
- The editors do not modify the original game executable — they edit data files that the game reads at runtime.

---

# Depth of Dejenol — Project Overview

## What It Is

Depth of Dejenol is a Qt6 C++ desktop role-playing game. The codebase is organized as a collection of tools and game modules, each with its own `.pro` file for qmake builds. The project uses binary data files (`.MDR` format) that are converted to CSV or JSON for tooling, and the game itself reads binary data at runtime.

## Directory Structure

```
depthofdejenol/
├── blacklands.cpp/h          # Main game window / hub
├── character.cpp/h           # Character data structures and logic
├── audioManager.cpp/h        # Audio playback management
├── fontManager.cpp/h         # Font rendering management
├── theCity.cpp/h             # The city area / overworld
├── gameStateManager.cpp/h    # Game state machine (screens, transitions)
├── data/                     # Binary data loading (MDR format)
│   ├── DataLoader.h/cpp      # Binary record loading framework
│   ├── RecordReader.h        # Template-based binary record parser
│   ├── MTypes.h              # Type definitions for binary data
│   └── main.cpp              # Data loading test/tool
├── maploader/                # Map loading module
│   ├── MapLoader.h/cpp       # Loads and manages game maps
│   └── main.cpp
├── room/                     # Room/dungeon interaction module
│   └── main.cpp
├── bankpane/                 # Bank/character inventory UI
│   ├── BankPane.h/cpp        # Bank UI window
│   └── main.cpp
├── src/                      # Core game engine components
│   ├── PartyManager.h        # Party/adventure group management
│   ├── StaticDataManager.h   # Static game data (races, guilds, etc.)
│   ├── FontRenderer.h        # Custom font rendering
│   └── traps_calculations.h  # Trap damage/effect calculations
├── tools/                    # Development and conversion tools
│   ├── editors/              # Qt6 editors (see Editors section)
│   ├── complication_calculator/  # Complication risk calculation logic
│   ├── complication_effect/       # Complication effect application logic
│   ├── map_editor/                # Map data structures (tile-based)
│   ├── gamedataconverter/         # MDATA1.MDR → JSON converter
│   ├── itemconverter/             # Item data conversion
│   ├── characterconverter/        # Character data conversion
│   ├── spellconverter/            # Spell data conversion
│   ├── dungeoncconverter/         # Dungeon data conversion
│   ├── monsterconverter/          # Monster data conversion
│   ├── generalstoreconverter/     # Store inventory conversion
│   └── monsterconverter/          # Monster data conversion
├── test/                     # Test programs
├── other/                    # Misc UI components
│   └── dungeondialog.cpp/h   # Dungeon encounter dialog
├── version.h                 # Version information
└── stable.h                  # Stable/compat definitions
```

## Core Game Modules

### blacklands (Main Hub)

`blacklands.cpp/h` is the primary game window. It serves as the central hub that players see when entering the game — likely the main navigation screen connecting to other areas (city, dungeons, etc.). It manages the overall game layout and transitions between major game areas.

### character

`character.cpp/h` defines the character data model — stats, abilities, inventory, and progression. This is the core entity that the player controls throughout the game. The character editor (`character-editor`) edits the CSV definitions that feed into this system.

### gameStateManager

`gameStateManager.cpp/h` implements the game's state machine. It handles screen transitions, modal dialogs, and the flow between different game phases (exploration, combat, shop, rest, etc.). This is the orchestration layer that coordinates the other modules.

### theCity / TheCity

`theCity.cpp/h` represents the city overworld area. It handles city navigation, buildings, NPCs, and services (bank, shops, guilds). The bank pane (`bankpane/`) is a sub-component of this — it provides the UI for managing the player's banked items.

### audioManager / AudioManager

Dual-file audio system (`audioManager.cpp/h` and `AudioManager.cpp/h`). Handles sound effects and music playback. May use different backends or cover different audio categories.

### fontManager / FontManager

Dual-file font system. Manages custom font loading and rendering for the game's UI. The `FontRenderer.h` in `src/` is the actual rendering component.

### data (Binary Data Loading)

The `data/` directory contains the framework for reading the game's binary data files (`.MDR` format):

- **RecordReader.h**: A template class that parses fixed-length binary records. It reads bytes from a file stream and reconstructs typed values (16-bit, 32-bit, floats) with little-endian byte ordering. Used as the foundation for all binary data parsing.

- **DataLoader.h/cpp**: High-level data loading that uses `RecordReader` to parse complete data files (like `MDATA1.MDR`) into C++ structs.

- **MTypes.h**: Type definitions mapping the game's data types (`short` = int16_t, `long` = int32_t, `float` = float) to standard C++ types for binary compatibility.

- **data/main.cpp**: A test/tool program that exercises the data loading framework.

### maploader

`maploader/MapLoader.h/cpp` handles loading game maps from disk. Maps are tile-based and stored in a format that the map editor (`map-editor-qt`) can also read/write. The map editor wraps this module's data structures.

### room

`room/main.cpp` handles individual room interaction within dungeons. When a player enters a room, this module manages the encounter, traps, treasures, and exits.

### bankpane

`bankpane/BankPane.h/cpp` is a Qt widget that provides the bank UI — depositing, withdrawing, and browsing items stored in the city bank. It's a `QMainWindow` subclass with its own `.pro` file.

### src (Core Engine)

- **PartyManager.h**: Manages the player's adventure party — party members, their stats, and group mechanics.
- **StaticDataManager.h**: Accessor for static game data (races, guilds, item types, monster types) loaded from MDATA1. This is what the game data editor edits.
- **FontRenderer.h**: Low-level font rendering, likely using the font manager's loaded fonts to draw text on the game canvas.
- **traps_calculations.h**: Standalone trap damage and effect calculations. Used by the room module when resolving trap encounters.

### other (Dungeon Dialog)

`other/dungeondialog.cpp/h` provides the UI for dungeon encounters — likely a dialog window that presents the results of a dungeon room interaction (combat, treasure found, traps triggered, etc.).

## Data Pipeline

The project uses a multi-stage data pipeline:

1. **Authoring**: Game data is authored in some original format (likely the `.MDR` binary format).
2. **Conversion**: Converter tools in `tools/` read the binary `.MDR` files and produce human-editable CSV or JSON files.
3. **Editing**: The Qt6 editors in `tools/editors/` allow game designers to view and modify the CSV/JSON data.
4. **Runtime**: The game itself reads binary data at runtime using the `data/` loading framework. The edited CSV/JSON files may be re-converted to binary, or the game may read them directly depending on the data type.

### Converter Tools

Each converter (`itemconverter`, `characterconverter`, `spellconverter`, `dungeoncconverter`, `monsterconverter`, `generalstoreconverter`) reads a specific `.MDR` binary file and writes a CSV file. The `gamedataconverter` reads `MDATA1.MDR` and writes `MDATA1.js` (JSON format).

The `tools/complication_calculator/` and `tools/complication_effect/` directories contain the core logic for complication calculations — these are header + implementation files that the complication editors wrap in a GUI.

## Build Organization

Each module is an independent qmake project:

| Module | .pro file | Type |
|--------|-----------|------|
| Main game | `blacklands.pro` | Application |
| Data loading | `data/data_loader.pro` | Application/tool |
| Map loader | `maploader/MapLoader.pro` | Library/application |
| Room | `room/room.pro` | Application |
| Bank pane | `bankpane/BankPane.pro` | Application |
| Test updater | `test/test_updater.pro` | Test application |
| Editors (each) | `tools/editors/<name>/<name>.pro` | Application |
| Editors master | `tools/editors/editors.pro` | Aggregator |

## Key Design Decisions

- **Binary data at runtime**: The game reads binary `.MDR` files directly for performance, while tooling uses CSV/JSON for editability.
- **Qt6 throughout**: All UI components (game window, editors, tools) use Qt6 with C++20.
- **Modular architecture**: Each game system (city, rooms, bank, audio, fonts) is a separate module with its own interface.
- **State machine**: Game flow is managed by `gameStateManager`, allowing clean transitions between modes.
- **Shared editor base**: CSV editors share `CommonCsvEditor` to avoid duplicating load/save/UI code, while non-CSV editors are standalone.

## Version

Version information is in `version.h` / `Version.h`.
