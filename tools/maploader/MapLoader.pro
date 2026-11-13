# maploader.pro

# This specifies that we're building a console application.
# It does not require a graphical user interface (GUI).
TEMPLATE = app

# The source files for your project
SOURCES += \
    main.cpp \
    MapLoader.cpp

# The header files for your project
HEADERS += \
    MapLoader.h

# Specify the target executable file name
TARGET = maploader_app

# Include the necessary modules.
# QtCore is essential for file I/O (QFile, QDataStream), QString, and QRandomGenerator.
QT += core

# Configuration settings:
# 'console' ensures the application runs in a console window
CONFIG += console c++17

# Specify where to put the object files and the final executable (optional but recommended)
DESTDIR = bin
OBJECTS_DIR = obj
