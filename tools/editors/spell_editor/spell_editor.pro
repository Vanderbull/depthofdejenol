QT += core widgets
CONFIG += c++20 console
CONFIG -= app_bundle
TARGET = spell-editor
TEMPLATE = app
HEADERS += ../../editors/common_csv_editor.h
SOURCES += ../../editors/common_csv_editor.cpp spelleditor.cpp
INCLUDEPATH += ../../editors
