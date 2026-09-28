QT += core widgets
CONFIG += c++20
CONFIG -= app_bundle
TARGET = map-editor-qt
TEMPLATE = app
HEADERS += mapeditorqt.h
SOURCES += mapeditormain.cpp \
           ../../map_editor/mapeditor.cpp
INCLUDEPATH += ../../editors
