# Extended test harness for the updater patch logic.
#
# Builds on the original test_updater by adding:
#  - extraction path verification (what files the ZIP would lay down) via unzip
#  - malformed manifest cases (missing fields, bad JSON, bad version)
#  - bad checksum case (SHA256 mismatch should fail download)
#  - GitHub release path simulation
#
# It shares the project's UpdateManager so the real download, SHA256
# verification, and version comparison paths get exercised.

TEMPLATE = app
CONFIG += console
CONFIG += c++20
QT += network

INCLUDEPATH += ../src/update
INCLUDEPATH += ..
INCLUDEPATH += ../../

HEADERS += ../src/update/UpdateManager.h

LIBS += -lQt6Core -lQt6Network

SOURCES += test_updater2.cpp \
    ../src/update/UpdateManager.cpp
