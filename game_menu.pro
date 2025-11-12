QMAKE_RESOURCE_FLAGS += --root /
RESOURCES += resources.qrc

QT += core gui widgets multimedia
TARGET = game_menu
SOURCES += game_menu.cpp \
    hallofrecordsdialog.cpp \
    createcharacterdialog.cpp \
    AboutDialog.cpp \
    CharacterDialog.cpp \
    MessageWindow.cpp \
    MonsterEditorDialog.cpp \
    SenderWindow.cpp \
    SpellbookEditorDialog.cpp \
    library_dialog.cpp \
    automap_dialog.cpp \
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
    AboutDialog.h \
    CharacterDialog.h \
    MessageWindow.h \
    MonsterEditorDialog.h \
    SenderWindow.h \
    SpellbookEditorDialog.h \
    library_dialog.h \
    automap_dialog.h \
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
