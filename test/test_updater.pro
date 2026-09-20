# Minimal test harness for the updater patch logic.
#
# Standalone console app that:
#  - spins up a tiny HTTP server serving a fake manifest and ZIP
#  - instantiates the real UpdateManager and runs check + download
#  - verifies updateAvailable / noUpdateAvailable / downloadFinished outcomes
#
# It pulls in the project's UpdateManager so the real download, SHA256
# verification, and version comparison paths get exercised.

TEMPLATE = app
CONFIG += console
CONFIG += c++20
QT += network

INCLUDEPATH += ../src/update
INCLUDEPATH += ..
INCLUDEPATH += ../../

HEADERS += ../src/update/UpdateManager.h

SOURCES += test_updater.cpp \
    ../src/update/UpdateManager.cpp
