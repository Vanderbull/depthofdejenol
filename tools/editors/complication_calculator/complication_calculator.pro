QT += core widgets
CONFIG += c++20
CONFIG -= app_bundle
TARGET = complication-calc-editor
TEMPLATE = app
HEADERS += complicationcalculatoreditor.h
SOURCES += complicationcalculatoreditor.cpp \
           ../../complication_calculator/ComplicationCalculator.cpp \
           main.cpp
INCLUDEPATH += ../../editors
