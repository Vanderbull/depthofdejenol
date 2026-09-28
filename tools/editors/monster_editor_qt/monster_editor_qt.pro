QT += core widgets
CONFIG += c++20 console
CONFIG -= app_bundle
TARGET = monster-editor
TEMPLATE = app
HEADERS += ../../editors/common_csv_editor.h
SOURCES += ../../editors/common_csv_editor.cpp monstereditor.cpp
INCLUDEPATH += ../../editors
