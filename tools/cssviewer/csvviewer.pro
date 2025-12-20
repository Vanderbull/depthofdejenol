# Include the necessary Qt modules
QT += widgets

# The name of the executable to be created
TARGET = csvviewer

# Specify that this is an application
TEMPLATE = app

# The C++ standard to use (Modern Qt projects usually use C++17)
CONFIG += c++17

# List of header files (MOC will scan these automatically)
HEADERS += \
    csvviewer.h

# List of source files
SOURCES += \
    csvviewer.cpp

# Optional: Set the installation directory (useful for Linux builds)
# target.path = /usr/bin
# INSTALLS += target
