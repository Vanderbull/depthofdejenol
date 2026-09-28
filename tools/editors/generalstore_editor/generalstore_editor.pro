QT       += core gui widgets
TARGET   = generalstoreeditor
TEMPLATE = app

INCLUDEPATH += ../../editors

HEADERS  = ../../editors/common_csv_editor.h generalstoreeditor.h
SOURCES  = generalstoreeditor.cpp \
           ../../editors/common_csv_editor.cpp