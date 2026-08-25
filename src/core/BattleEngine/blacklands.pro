# Define template and target application name
TEMPLATE = app
TARGET = HommBattle

# Use C++17 standard and enable Qt modules
CONFIG += c++17 qt
QT += core gui quick

# List your C++ Header files (triggers MOC generation for Q_OBJECT classes)
HEADERS += \
    BattleEngine.h

# List your C++ Source files
SOURCES += \
    main.cpp
