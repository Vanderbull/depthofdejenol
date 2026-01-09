# Define a central directory for all build-related files.
# The actual path will depend on where qmake is run.
# Here, it creates a 'build' folder relative to the project root.
BUILD_DIR = build

# Specifies the directory for compiled object files (*.o).
# Differentiate between debug and release objects
CONFIG(debug, debug|release): OBJECTS_DIR = $$BUILD_DIR/obj/debug
CONFIG(release, debug|release): OBJECTS_DIR = $$BUILD_DIR/obj/release

# Specifies the directory for MOC (Meta-Object Compiler) files.
MOC_DIR = $$BUILD_DIR/moc

# Paralleling
QMAKE_PARALLEL_BUILD = 1

# Specifies the directory where the final executable or library will be placed.
DESTDIR = $$BUILD_DIR/bin

# Specifies the directory for compiled object files (*.o).
#OBJECTS_DIR = $$BUILD_DIR/obj

# Specifies the directory for MOC (Meta-Object Compiler) files.
#MOC_DIR = $$BUILD_DIR/moc

# Specifies the directory for RCC (Resource Compiler) files.
RCC_DIR = $$BUILD_DIR/rcc

# Specifies the directory for UIC (User Interface Compiler) files (for .ui files).
UI_DIR = $$BUILD_DIR/ui

# 1. Define a variable (group) for the files to copy.
# $$PWD points to the directory containing the .pro file (the root).
#asset_copy.files = $$PWD/assets

# 2. Set the destination path.
# $$DESTDIR points to the final build/bin/ folder.
#asset_copy.path = $$DESTDIR

# 3. Register the group to be installed/copied during the build process.
#INSTALLS += asset_copy

#--------------------------------------------------
QMAKE_RESOURCE_FLAGS += --root /

QT += core gui widgets multimedia
TARGET = game_menu

# Add the 'include' directory so the compiler can find the header files
INCLUDEPATH += \
    _PRO_FILE_PWD_/include
#    $$PWD/include

QMAKE_FILE_GROUP = Everything
SOURCES += GameStateManager.cpp \
    game_menu.cpp \
    TheCity.cpp \
    tools/monster_editor/MonsterEditorDialog.cpp \
    tools/spellbook_editor/SpellbookEditorDialog.cpp \
    src/hall_of_records/hallofrecordsdialog.cpp \
    src/create_character/createcharacterdialog.cpp \
    src/about_dialog/AboutDialog.cpp \
    src/character_dialog/CharacterDialog.cpp \
    src/message_window/MessageWindow.cpp \
    src/sender_window/SenderWindow.cpp \
    src/library_dialog/library_dialog.cpp \
    src/automap/automap_dialog.cpp \
    src/game_controller/game_controller.cpp \
    src/characterlist_dialog/characterlistdialog.cpp \
    src/helplesson/helplesson.cpp \
    src/mordorstatistics/mordorstatistics.cpp \
    src/loadingscreen/LoadingScreen.cpp \
    src/guilds_dialog/GuildsDialog.cpp \
    src/general_store/GeneralStore.cpp \
    src/morgue_dialog/MorgueDialog.cpp \
    src/seer_dialog/SeerDialog.cpp \
    src/confinement_dialog/ConfinementDialog.cpp \
    src/bank_dialog/BankDialog.cpp \
    src/race_data/RaceData.cpp \
    src/inventory_dialog/inventorydialog.cpp \
    src/options_dialog/optionsdialog.cpp \
    src/dungeon_dialog/DungeonDialog.cpp \
    src/partyinfo_dialog/partyinfodialog.cpp \
    src/dungeonmap/dungeonmap.cpp \ 
    src/bank_dialog/TradeDialog.cpp \
    src/game_resources.cpp \
    src/dungeon_dialog/DungeonMinimap.cpp \
    src/dungeon_dialog/DungeonHandlers.cpp \
    src/event/EventManager.cpp

HEADERS += GameStateManager.h \
    game_menu.h \
    TheCity.h \
    StoryDialog.h \
    tools/monster_editor/MonsterEditorDialog.h \
    tools/spellbook_editor/SpellbookEditorDialog.h \
    src/hall_of_records/hallofrecordsdialog.h \
    src/create_character/createcharacterdialog.h \
    src/about_dialog/AboutDialog.h \
    src/character_dialog/CharacterDialog.h \
    src/message_window/MessageWindow.h \
    src/sender_window/SenderWindow.h \
    src/library_dialog/library_dialog.h \
    src/automap/automap_dialog.h \
    src/game_controller/game_controller.h \
    src/characterlist_dialog/characterlistdialog.h \
    src/helplesson/helplesson.h \
    src/mordorstatistics/mordorstatistics.h \
    src/loadingscreen/LoadingScreen.h \
    src/guilds_dialog/GuildsDialog.h \
    src/general_store/GeneralStore.h \
    src/morgue_dialog/MorgueDialog.h \
    src/seer_dialog/SeerDialog.h \
    src/confinement_dialog/ConfinementDialog.h \
    src/bank_dialog/BankDialog.h \
    src/race_data/RaceData.h \
    src/inventory_dialog/inventorydialog.h \
    src/options_dialog/optionsdialog.h \
    src/dungeon_dialog/DungeonDialog.h \
    src/partyinfo_dialog/partyinfodialog.h \
    src/dungeonmap/dungeonmap.h \
    src/bank_dialog/TradeDialog.h \
    include/game_resources.h \
    src/dungeon_dialog/DungeonHandlers.h \
    src/event/EventManager.h \
    src/dungeon_dialog/MinimapDialog.h
    
CXXFLAGS += -std=c++17

DISTFILES += \
    .gitignore
#QMAKE_EXTRA_OBJECTS += qrc_resources.o
# Lägg till din byggkatalog som en sökväg för bibliotek
#LIBS += -L.

# Add object file as a "library"
#LIBS += qrc_resources.o

# Force linker to handle .o file as a normal object file
#QMAKE_LFLAGS += -Wl,-whole-archive qrc_resources.o -Wl,-no-whole-archive

# This forces the copy of the assets folder from $$PWD (Project Root)
# to $$DESTDIR (build/bin/) after the executable is linked.
#QMAKE_POST_LINK += $(COPY_DIR) $$quote($$PWD/data) $$quote($$DESTDIR)
# Use $(COPY_FILE) to copy the compiled executable from its location ($$DESTDIR/$$TARGET)
# to the project root ($$PWD/).
#QMAKE_POST_LINK += $(COPY_FILE) $$quote($$DESTDIR/$$TARGET) $$quote($$PWD/$$TARGET)
# --- Copying the 'assets' folder (The likely source of the initial confusion) ---
# NOTE: Using a single destination argument for COPY_DIR to avoid confusion.
QMAKE_POST_LINK += $$escape_expand(\\n\\t) $(COPY_DIR) $$quote($$PWD/data) $$quote($$DESTDIR)

# --- Copying the binary back to the root ---
# Ensure this command is entirely separate and well-formed.
QMAKE_POST_LINK += $$escape_expand(\\n\\t) $(COPY_FILE) $$quote($$DESTDIR/$$TARGET) $$quote($$PWD/$$TARGET)


