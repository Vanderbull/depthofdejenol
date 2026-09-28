# Master project file for all Depth of Dejenol tools editors.
# Build individual editors with: qmake -o <editor>/<editor>.pro && make
# Or build all at once from the editors/ directory.

QT += core widgets
CONFIG += c++20 console
CONFIG -= app_bundle

TARGET = dejenol-editors
TEMPLATE = app

# Source files — each editor's main .cpp plus the shared base
SOURCES += \
    editors/common_csv_editor.cpp \
    editors/item_editor/itemeditor.cpp \
    editors/monster_editor_qt/monstereditor.cpp \
    editors/spell_editor/spelleditor.cpp \
    editors/character_editor/charmereditor.cpp \
    editors/dungeon_editor/dungeoneditor.cpp \
    editors/generalstore_editor/generalstoreeditor.cpp \
    editors/complication_calculator/complicationcalculatormain.cpp \
    editors/complication_effect/complicationeffectmain.cpp \
    editors/map_editor/mapeditormain.cpp \
    editors/gamedata_editor/gamedataeditormain.cpp

# Include path: find the editors/ directory from any subdirectory
INCLUDEPATH += \
    editors \
    editors/item_editor \
    editors/monster_editor_qt \
    editors/spell_editor \
    editors/character_editor \
    editors/dungeon_editor \
    editors/generalstore_editor \
    editors/complication_calculator \
    editors/complication_effect \
    editors/map_editor \
    editors/gamedata_editor

# Each editor binary gets its own target via OBJECTS_DIR override.
# Build individually: qmake -o item_editor/item_editor.pro item_editor/item_editor.pro
# Then: make -C item_editor

# Default build produces a single binary that runs all editors as separate processes.
# For individual editor binaries, use the per-editor .pro files.
