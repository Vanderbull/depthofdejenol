QT += core gui widgets multimedia
TARGET = game_menu
SOURCES += game_menu.cpp hallofrecordsdialog.cpp createcharacterdialog.cpp \
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
    mordorstatistics.cpp
SOURCES += marlith_dialog.cpp
SOURCES += inventorydialog.cpp
SOURCES += optionsdialog.cpp
SOURCES += dungeondialog.cpp
HEADERS += game_menu.h hallofrecordsdialog.h createcharacterdialog.h \
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
    mordorstatistics.h
HEADERS += marlith_dialog.h
HEADERS += inventorydialog.h
HEADERS += optionsdialog.h
HEADERS += dungeondialog.h

SOURCES += partyinfodialog.cpp
SOURCES += dungeonmap.cpp

HEADERS += partyinfodialog.h
HEADERS += dungeonmap.h

CXXFLAGS += -std=c++11

DISTFILES += \
    .gitignore
