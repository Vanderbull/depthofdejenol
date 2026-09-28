QT += core widgets
CONFIG += c++20
CONFIG -= app_bundle
TARGET = complication-effect-editor
TEMPLATE = app
HEADERS += complicationeffecteditor.h
SOURCES += complicationeffecteditor.cpp \
           ../../complication_effect/ComplicationEffect.cpp \
           main.cpp
INCLUDEPATH += ../../editors
