QT += core widgets
CONFIG += c++20 console
CONFIG -= app_bundle
TARGET = dungeon-editor
TEMPLATE = app
HEADERS += ../../editors/common_csv_editor.h
SOURCES += ../../editors/common_csv_editor.cpp dungeoneditor.cpp
INCLUDEPATH += ../../editors
