QT += core network
QT -= gui  # Server doesn't need a user interface

TARGET = Server
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += main.cpp

# This allows the use of signals/slots inside the main.cpp 
# without a separate header file for the Server class.
CONFIG += moc
