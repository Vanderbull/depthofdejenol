QT += core widgets
CONFIG += c++20 console
CONFIG -= app_bundle
TARGET = character-editor
TEMPLATE = app
HEADERS += ../../editors/common_csv_editor.h
SOURCES += ../../editors/common_csv_editor.cpp charmereditor.cpp
INCLUDEPATH += ../../editors
