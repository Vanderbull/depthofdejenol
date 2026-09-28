QT += widgets
CONFIG += c++20
QT_COMPILER_WARNINGS = 1

TARGET = itemeditor
TEMPLATE = app

INCLUDEPATH += ../../editors

HEADERS += ../../editors/common_csv_editor.h
SOURCES += ../common_csv_editor.cpp \
           itemeditor.cpp
