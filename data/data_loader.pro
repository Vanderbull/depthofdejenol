# Define the application type
QT -= gui
CONFIG += console
CONFIG -= app_bundle

# Set the target application name
TARGET = game_data_loader

# Specify the Qt modules to use
# Core is necessary for QString, QFile, QDataStream, etc.
QT += core

# Specify all header files
HEADERS += \
    MTypes.h \
    MLoader.h \
    RecordReader.h

# Specify all source files
SOURCES += \
    main.cpp \
    MLoader.cpp

# Add the directory containing the header/source files
# Assuming all files are in the same directory.
