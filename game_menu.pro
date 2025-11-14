# Define a central directory for all build-related files.
# The actual path will depend on where qmake is run.
# Here, it creates a 'build' folder relative to the project root.
BUILD_DIR = build

# Specifies the directory where the final executable or library will be placed.
DESTDIR = $$BUILD_DIR/bin

# Specifies the directory for compiled object files (*.o).
OBJECTS_DIR = $$BUILD_DIR/obj

# Specifies the directory for MOC (Meta-Object Compiler) files.
MOC_DIR = $$BUILD_DIR/moc

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
SOURCES += game_menu.cpp \
    hallofrecordsdialog.cpp \
    createcharacterdialog.cpp \
    src/about_dialog/AboutDialog.cpp \
    CharacterDialog.cpp \
    MessageWindow.cpp \
    MonsterEditorDialog.cpp \
    SenderWindow.cpp \
    SpellbookEditorDialog.cpp \
    library_dialog.cpp \
    src/automap/automap_dialog.cpp \
    game_controller.cpp \
    characterlistdialog.cpp \
    helplesson.cpp \
    mordorstatistics.cpp \
    LoadingScreen.cpp \
    GuildsDialog.cpp \
    GeneralStore.cpp \
    TheCity.cpp \
    MorgueDialog.cpp \
    SeerDialog.cpp \
    ConfinementDialog.cpp \
    BankDialog.cpp \
    RaceData.cpp \
    marlith_dialog.cpp \
    inventorydialog.cpp \
    optionsdialog.cpp \
    DungeonDialog.cpp \
    partyinfodialog.cpp \
    dungeonmap.cpp 

SOURCES +=

HEADERS += game_menu.h \
    hallofrecordsdialog.h \
    createcharacterdialog.h \
    src/about_dialog/AboutDialog.h \
    CharacterDialog.h \
    MessageWindow.h \
    MonsterEditorDialog.h \
    SenderWindow.h \
    SpellbookEditorDialog.h \
    library_dialog.h \
    src/automap/automap_dialog.h \
    game_controller.h \
    characterlistdialog.h \
    helplesson.h \
    mordorstatistics.h \
    LoadingScreen.h \
    GuildsDialog.h \
    GeneralStore.h \
    TheCity.h \
    MorgueDialog.h \
    SeerDialog.h \
    ConfinementDialog.h \
    BankDialog.h \
    RaceData.h \
    marlith_dialog.h \
    inventorydialog.h \
    optionsdialog.h \
    DungeonDialog.h \
    partyinfodialog.h \
    dungeonmap.h \

CXXFLAGS += -std=c++11

DISTFILES += \
    .gitignore
#QMAKE_EXTRA_OBJECTS += qrc_resources.o
# Lägg till din byggkatalog som en sökväg för bibliotek
#LIBS += -L.

# Lägg till objektfilen som ett "bibliotek"
#LIBS += qrc_resources.o

# Tvinga länkaren att behandla .o-filen som en vanlig objektfil
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


