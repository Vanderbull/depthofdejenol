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

# Header dependencies are tracked by qmake itself: every header listed in
# HEADERS below becomes a prerequisite of every object that includes it, so
# editing a header rebuilds its dependents. Keep HEADERS in sync with the real
# includes — a header missing from that list is invisible to the build, and the
# result is a binary linking objects compiled against two different
# definitions of the same class. That ODR violation does not crash; it shows up
# as "the value prints correctly but the comparison is false".
DEPENDPATH += $$PWD

INCLUDEPATH += _PRO_FILE_PWD_/include

#--------------------------------------------------
# Source Files
#--------------------------------------------------
SOURCES += \
    src/core/savegameVersion.cpp \
    src/core/gameStateManager.cpp \
    src/partymanager/PartyManager.cpp \
    src/audio/audioManager.cpp \
    src/blacklands.cpp \
    src/theCity.cpp \
    src/network_manager/NetworkManager.cpp \
    src/hall_of_records/hallofrecordsdialog.cpp \
    src/create_character/createcharacterdialog.cpp \
    src/character_dialog/CharacterDialog.cpp \
    src/library_dialog/library_dialog.cpp \
    src/automap/automap_dialog.cpp \
    src/load_party_dialog/LoadPartyDialog.cpp \
    src/guilds_dialog/GuildsDialog.cpp \
    src/general_store/GeneralStore.cpp \
    src/morgue_dialog/MorgueDialog.cpp \
    src/seer_dialog/SeerDialog.cpp \
    src/confinement_dialog/ConfinementDialog.cpp \
    src/bank_dialog/BankDialog.cpp \
    src/assemble_party_dialog/AssemblePartyDialog.cpp \
    src/loadingscreen/LoadingScreen.cpp \
    src/race_data/RaceData.cpp \
    src/inventory_dialog/inventorydialog.cpp \
    src/options_dialog/optionsdialog.cpp \
    src/dungeon_dialog/DungeonDialog.cpp \
    src/partyinfo_dialog/partyinfodialog.cpp \
    src/bank_dialog/TradeDialog.cpp \
    src/game_resources.cpp \
    src/dungeon_dialog/DungeonMinimap.cpp \
    src/dungeon_dialog/DungeonHandlers.cpp \
    src/event/EventManager.cpp \
    src/character/character.cpp

#--------------------------------------------------
# Self-test harness
#--------------------------------------------------
# The headless verification suite lives in test/selftest.cpp and is linked
# into the real binary. It is only reachable via the `blacklands --selftest`
# flag, so the shipped executable is unchanged.
HEADERS += test/selftest.h test/selftest_harness.h test/selftest_includes.h
SOURCES += test/selftest.cpp test/selftest_harness.cpp test/selftest_core.cpp test/selftest_combat.cpp test/selftest_dungeon.cpp test/selftest_progression.cpp test/selftest_world.cpp test/equipment_test.cpp test/selftest_spells.cpp test/selftest_options.cpp test/selftest_difficulty.cpp test/selftest_data.cpp

#--------------------------------------------------
# Item database
#--------------------------------------------------
# Typed, indexed view of MDATA3. Loaded once; consumers look items up by id or
# name instead of scanning QVariantMaps.
HEADERS += src/items/ItemDatabase.h src/items/ConsumableResolver.h
SOURCES += src/items/ItemDatabase.cpp src/items/ConsumableResolver.cpp
HEADERS += src/victory_dialog/VictoryDialog.h
SOURCES += src/victory_dialog/VictoryDialog.cpp

#--------------------------------------------------
# Combat engine
#--------------------------------------------------
HEADERS += src/combat/CombatState.h
SOURCES += src/combat/CombatState.cpp
HEADERS += src/combat/TurnEngine.h
SOURCES += src/combat/TurnEngine.cpp
HEADERS += src/combat/CombatActions.h
SOURCES += src/combat/CombatActions.cpp
HEADERS += src/combat/MonsterAI.h
SOURCES += src/combat/MonsterAI.cpp
HEADERS += src/combat/EncounterBuilder.h
SOURCES += src/combat/EncounterBuilder.cpp
HEADERS += src/combat/VictoryReward.h
SOURCES += src/combat/VictoryReward.cpp
HEADERS += src/combat/CombatDeathHandler.h
SOURCES += src/combat/CombatDeathHandler.cpp
HEADERS += src/core/LevelTable.h
SOURCES += src/core/LevelTable.cpp
HEADERS += src/core/AgingRules.h
SOURCES += src/core/AgingRules.cpp
HEADERS += src/core/DungeonLevelState.h
SOURCES += src/core/DungeonLevelState.cpp
HEADERS += src/core/DungeonThemes.h
SOURCES += src/core/DungeonThemes.cpp
HEADERS += src/core/DoorAndSearch.h
SOURCES += src/core/DoorAndSearch.cpp
HEADERS += src/core/BossEncounter.h
SOURCES += src/core/BossEncounter.cpp
HEADERS += src/core/AbyssThemes.h
SOURCES += src/core/AbyssThemes.cpp
HEADERS += src/core/DeathRecovery.h
SOURCES += src/core/DeathRecovery.cpp
HEADERS += src/core/QuestChain.h
SOURCES += src/core/QuestChain.cpp
HEADERS += src/core/QuestChainDriver.h
SOURCES += src/core/QuestChainDriver.cpp
HEADERS += src/core/Endgame.h
SOURCES += src/core/Endgame.cpp
HEADERS += src/core/MonsterBalance.h
SOURCES += src/core/MonsterBalance.cpp
HEADERS += src/spell_casting/SpellMechanics.h
SOURCES += src/spell_casting/SpellMechanics.cpp
HEADERS += src/items/ItemProgression.h
SOURCES += src/items/ItemProgression.cpp
HEADERS += src/core/GoldSinks.h
SOURCES += src/core/GoldSinks.cpp
HEADERS += src/core/GameSettings.h
SOURCES += src/core/GameSettings.cpp
HEADERS += src/core/ReleaseInfo.h
SOURCES += src/core/ReleaseInfo.cpp
HEADERS += src/core/SoundEffects.h
SOURCES += src/core/SoundEffects.cpp
HEADERS += src/core/AlignmentSystem.h
SOURCES += src/core/AlignmentSystem.cpp
HEADERS += src/tutorial/Tutorial.h
SOURCES += src/tutorial/Tutorial.cpp
HEADERS += src/npc_dialog/NPCDialog.h
SOURCES += src/npc_dialog/NPCDialog.cpp
HEADERS += src/core/AchievementSystem.h
SOURCES += src/core/AchievementSystem.cpp
HEADERS += src/tavern_dialog/TavernDialog.h
SOURCES += src/tavern_dialog/TavernDialog.cpp
HEADERS += src/temple_dialog/TempleDialog.h
SOURCES += src/temple_dialog/TempleDialog.cpp
HEADERS += src/character_dialog/CharacterSheetDialog.h
SOURCES += src/character_dialog/CharacterSheetDialog.cpp
HEADERS += src/library_dialog/BestiaryDialog.h
SOURCES += src/library_dialog/BestiaryDialog.cpp
HEADERS += src/journal_dialog/JournalDialog.h
SOURCES += src/journal_dialog/JournalDialog.cpp
HEADERS += src/quest_board/QuestBoardDialog.h
SOURCES += src/quest_board/QuestBoardDialog.cpp
HEADERS += src/quest_chain_dialog/QuestChainDialog.h
SOURCES += src/quest_chain_dialog/QuestChainDialog.cpp
HEADERS += src/spell_casting/SpellBook.h
SOURCES += src/spell_casting/SpellBook.cpp
HEADERS += src/spells/SpellCategory.h
SOURCES += src/spells/SpellCategory.cpp
HEADERS += src/spells/SpellTargeting.h
SOURCES += src/spells/SpellTargeting.cpp
HEADERS += src/spells/SpellResistance.h
SOURCES += src/spells/SpellResistance.cpp src/spells/SpellScaling.cpp
HEADERS += src/spells/SpellScaling.h
HEADERS += src/spells/SpellIcon.h
SOURCES += src/spells/SpellIcon.cpp

#--------------------------------------------------
# Header Files
#--------------------------------------------------
HEADERS += \
    src/core/savegameVersion.h \
    src/core/gameStateManager.h \
    src/partymanager/PartyManager.h \
    src/core/GameConstants.h \
    src/audio/audioManager.h \
    src/blacklands.h \
    src/theCity.h \
    src/storyDialog.h \
    src/network_manager/NetworkManager.h \
    src/hall_of_records/hallofrecordsdialog.h \
    src/create_character/createcharacterdialog.h \
    src/character_dialog/CharacterDialog.h \
    src/library_dialog/library_dialog.h \
    src/automap/automap_dialog.h \
    src/load_party_dialog/LoadPartyDialog.h \
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
    src/bank_dialog/TradeDialog.h \
    src/assemble_party_dialog/AssemblePartyDialog.h \
    src/loadingscreen/LoadingScreen.h \
    src/core/game_resources.h \
    src/dungeon_dialog/DungeonHandlers.h \
    src/event/EventManager.h \
    src/dungeon_dialog/MinimapDialog.h \
    src/character/character.h

HEADERS += src/spell_casting/SpellCastingDialog.h
SOURCES += src/spell_casting/SpellCastingDialog.cpp

HEADERS += src/fontManager.h
SOURCES += src/fontManager.cpp
#--------------------------------------------------
# Post-Link Operations
#--------------------------------------------------
# Copy data folder from project root to build/bin/
QMAKE_POST_LINK += $$escape_expand(\\n\\t) $(COPY_DIR) $$quote($$PWD/data) $$quote($$DESTDIR)

# Copy resources folder (all game art) to build/bin/. Without this the binary
# in build/bin finds no artwork and every button/texture renders blank.
QMAKE_POST_LINK += $$escape_expand(\\n\\t) $(COPY_DIR) $$quote($$PWD/resources) $$quote($$DESTDIR)

# Copy the main-menu background to build/bin/. GameMenu::loadBackgroundImage()
# reads it via applicationDirPath(), so the binary in build/bin needs its own
# copy — the one at the repo root only serves the root-level binary.
QMAKE_POST_LINK += $$escape_expand(\\n\\t) $(COPY_FILE) $$quote($$PWD/introtitle.png) $$quote($$DESTDIR/introtitle.png)

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

# Semantic version: v<major>.<minor>.<patch>
# Bump MAJOR for the first finishable release or a fundamental rework.
# Bump MINOR for new features/mechanics. Bump PATCH for fixes/balance/polish.
SEMVER_MAJOR = 6
SEMVER_MINOR = 9
SEMVER_PATCH = 0
SEMVER = \"$${SEMVER_MAJOR}.$${SEMVER_MINOR}.$${SEMVER_PATCH}\"

# 3. Tell qmake to use the template
QMAKE_SUBSTITUTES += version.h.in

# 4. VERY IMPORTANT: Add literal quotes to the variables
# This ensures they appear as \"9629655\" in version.h instead of 9629655
VERSION_HASH = \"$$VERSION_HASH\"
VERSION_DATE = \"$$VERSION_DATE\"
VERSION_INT  = \"v$$VERSION_INT\"

#--------------------------------------------------
# Verification
#--------------------------------------------------
# Declaring `check` here overrides the default generated target (qmake only
# adds `check: first` when the project has not defined it), so `make check`
# builds and then runs the headless self-test suite, followed by the
# orphaned-system guard (a system reachable only from the tests is not wired
# into the game).
QMAKE_EXTRA_TARGETS += check
check.target = check
check.depends = first
check.commands = ./$${TARGET} --selftest $$escape_expand(\\n\\t) python3 $$quote($$PWD/tools/check_orphaned_systems.py)
