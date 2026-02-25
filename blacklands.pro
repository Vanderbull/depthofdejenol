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
    GameStateManager.cpp \
    AudioManager.cpp \
    blacklands.cpp \
    TheCity.cpp \
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
    include/GameConstants.h \
    GameStateManager.h \
    AudioManager.h \
    blacklands.h \
    TheCity.h \
    StoryDialog.h \
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
    include/game_resources.h \
    src/dungeon_dialog/DungeonHandlers.h \
    src/event/EventManager.h \
    src/dungeon_dialog/MinimapDialog.h \
    src/update/UpdateManager.h \
    src/update/UpdateDialog.h

HEADERS += src/spell_casting/SpellCastingDialog.h
SOURCES += src/spell_casting/SpellCastingDialog.cpp

HEADERS += RaceFactory.h
SOURCES += RaceFactory.cpp

HEADERS += GuildFactory.h
SOURCES += GuildFactory.cpp

HEADERS += CharacterIO.h
SOURCES += CharacterIO.cpp

#--------------------------------------------------
# Post-Link Operations
#--------------------------------------------------
# Copy data folder from project root to build/bin/
QMAKE_POST_LINK += $$escape_expand(\\n\\t) $(COPY_DIR) $$quote($$PWD/data) $$quote($$DESTDIR)

# Copy final binary back to root for easy execution
QMAKE_POST_LINK += $$escape_expand(\\n\\t) $(COPY_FILE) $$quote($$DESTDIR/$$TARGET) $$quote($$PWD/$$TARGET)

DISTFILES += .gitignore
