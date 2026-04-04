#-------------------------------------------------- Project 
# Configuration
#--------------------------------------------------
QT += core gui widgets multimedia network
TARGET = blacklands

# Qt 6 & Silent mode (only outputs warnings/errors)
CONFIG += c++20 silent
QMAKE_PARALLEL_BUILD = 1

# Define a central directory for all build-related files
BUILD_DIR = build
DESTDIR = $$BUILD_DIR/bin

# Differentiate between debug and release objects
CONFIG(debug, debug|release): OBJECTS_DIR = $$BUILD_DIR/obj/debug
CONFIG(release, debug|release): OBJECTS_DIR = $$BUILD_DIR/obj/release

MOC_DIR = $$BUILD_DIR/moc
RCC_DIR = $$BUILD_DIR/rcc
UI_DIR = $$BUILD_DIR/ui

#--------------------------------------------------
# Compiler & Search Paths
#--------------------------------------------------
QMAKE_RESOURCE_FLAGS += --root /

INCLUDEPATH += _PRO_FILE_PWD_/include

#--------------------------------------------------
# Source Files
#--------------------------------------------------
SOURCES += \
    gameStateManager.cpp \
    audioManager.cpp \
    blacklands.cpp \
    theCity.cpp \
    src/network_manager/NetworkManager.cpp \
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
    src/event/EventManager.cpp \
    src/update/UpdateManager.cpp \
    src/update/UpdateDialog.cpp

#--------------------------------------------------
# Header Files
#--------------------------------------------------
HEADERS += \
    src/core/GameConstants.h \
    gameStateManager.h \
    audioManager.h \
    blacklands.h \
    theCity.h \
    storyDialog.h \
    src/network_manager/NetworkManager.h \
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
    src/core/game_resources.h \
    src/dungeon_dialog/DungeonHandlers.h \
    src/event/EventManager.h \
    src/dungeon_dialog/MinimapDialog.h \
    src/update/UpdateManager.h \
    src/update/UpdateDialog.h

HEADERS += src/spell_casting/SpellCastingDialog.h
SOURCES += src/spell_casting/SpellCastingDialog.cpp

HEADERS += fontManager.h
SOURCES += fontManager.cpp

HEADERS += src/partymanager/PartyManager.h
SOURCES += src/partymanager/PartyManager.cpp

HEADERS += character.h
SOURCES += character.cpp

#--------------------------------------------------
# Post-Link Operations
#--------------------------------------------------
# Copy data folder from project root to build/bin/
QMAKE_POST_LINK += $$escape_expand(\\n\\t) $(COPY_DIR) $$quote($$PWD/data) $$quote($$DESTDIR)

# Copy final binary back to root for easy execution
QMAKE_POST_LINK += $$escape_expand(\\n\\t) $(COPY_FILE) $$quote($$DESTDIR/$$TARGET) $$quote($$PWD/$$TARGET)

DISTFILES += .gitignore

#--------------------------------------------------
# 3rd Party: Lua 5.5.0 Scripting Engine
#--------------------------------------------------
# Points to the directory where lua.h and .c files live
INCLUDEPATH += $$PWD/3rdparty/lua

# Core Lua Sources (Excluding lua.c and luac.c to avoid main() conflicts)
SOURCES += \
    $$PWD/3rdparty/lua/lapi.c \
    $$PWD/3rdparty/lua/lcode.c \
    $$PWD/3rdparty/lua/lctype.c \
    $$PWD/3rdparty/lua/ldebug.c \
    $$PWD/3rdparty/lua/ldo.c \
    $$PWD/3rdparty/lua/ldump.c \
    $$PWD/3rdparty/lua/lfunc.c \
    $$PWD/3rdparty/lua/lgc.c \
    $$PWD/3rdparty/lua/llex.c \
    $$PWD/3rdparty/lua/lmem.c \
    $$PWD/3rdparty/lua/lobject.c \
    $$PWD/3rdparty/lua/lopcodes.c \
    $$PWD/3rdparty/lua/lparser.c \
    $$PWD/3rdparty/lua/lstate.c \
    $$PWD/3rdparty/lua/lstring.c \
    $$PWD/3rdparty/lua/ltable.c \
    $$PWD/3rdparty/lua/ltm.c \
    $$PWD/3rdparty/lua/lundump.c \
    $$PWD/3rdparty/lua/lvm.c \
    $$PWD/3rdparty/lua/lzio.c \
    $$PWD/3rdparty/lua/lauxlib.c \
    $$PWD/3rdparty/lua/lbaselib.c \
    $$PWD/3rdparty/lua/lcorolib.c \
    $$PWD/3rdparty/lua/ldblib.c \
    $$PWD/3rdparty/lua/liolib.c \
    $$PWD/3rdparty/lua/lmathlib.c \
    $$PWD/3rdparty/lua/loadlib.c \
    $$PWD/3rdparty/lua/loslib.c \
    $$PWD/3rdparty/lua/lstrlib.c \
    $$PWD/3rdparty/lua/ltablib.c \
    $$PWD/3rdparty/lua/lutf8lib.c \
    $$PWD/3rdparty/lua/linit.c

# Ensure the C compiler is used for these files
QMAKE_CFLAGS += -std=c11
DEFINES += LUA_COMPAT_5_3

# --- Suppression of the 'tmpnam' warning ONLY on Linux ---
linux {
    DEFINES += LUA_USE_LINUX
}

# --- If you eventually port to Windows, you can add specific needs here ---
win32 {
    # Windows-specific Lua defines would go here if needed
    # DEFINES += LUA_USE_WINDOWS
}

# 1. Capture the data from the system
VERSION_HASH = $$system(git rev-parse --short HEAD)
VERSION_DATE = $$system(date +%Y-%m-%d_%H:%M:%S)
COMMIT_COUNT = $$system(git rev-list --count HEAD)

# 2. Calculate the version integer (Commit 1 = v0)
VERSION_INT = $$num_add($$COMMIT_COUNT, -1)
lessThan(VERSION_INT, 0): VERSION_INT = 0

# 3. Tell qmake to use the template
QMAKE_SUBSTITUTES += version.h.in

# 4. VERY IMPORTANT: Add literal quotes to the variables
# This ensures they appear as "9629655" in version.h instead of 9629655
VERSION_HASH = \"$$VERSION_HASH\"
VERSION_DATE = \"$$VERSION_DATE\"
VERSION_INT  = \"v$$VERSION_INT\"
