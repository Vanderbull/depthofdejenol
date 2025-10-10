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
    automap_dialog.cpp
SOURCES += marlith_dialog.cpp
SOURCES += inventorydialog.cpp
SOURCES += optionsdialog.cpp
SOURCES += dungeondialog.cpp
HEADERS += gamemenu.h hallofrecordsdialog.h createcharacterdialog.h \
    AboutDialog.h \
    CharacterDialog.h \
    MessageWindow.h \
    MonsterEditorDialog.h \
    SenderWindow.h \
    SpellbookEditorDialog.h \
    library_dialog.h \
    automap_dialog.h
HEADERS += marlith_dialog.h
HEADERS += inventorydialog.h
HEADERS += optionsdialog.h
HEADERS += dungeondialog.h

SOURCES += partyinfodialog.cpp
SOURCES += dungeonmap.cpp

HEADERS += partyinfodialog.h
HEADERS += dungeonmap.h

# Explicitly tell the compiler where to find the headers
INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt6/QtMultimedia
INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt6/QtWidgets
INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt6/QtGui
INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt6/QtCore

# Explicitly tell the linker where to find the libraries
LIBS += -L/usr/lib/x86_64-linux-gnu/qt6/ -lQt6Multimedia -lQt6Widgets -lQt6Gui -lQt6Core

# Use the QMAKE_LFLAGS to force the linker to be explicit
QMAKE_LFLAGS += -L/usr/lib/x86_64-linux-gnu/qt6/ -lQt6Core -lQt6Gui -lQt6Widgets -lQt6Multimedia

CXXFLAGS += -std=c++11

DISTFILES += \
    .gitignore
