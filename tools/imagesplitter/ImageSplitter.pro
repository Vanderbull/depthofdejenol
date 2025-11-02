# ----------------------------------------------------------------------
# QT PROJECT FILE: imagesplitter.pro
# ----------------------------------------------------------------------

# Definerar mallen för ett applikationsprojekt
TEMPLATE = app

# Lägger till nödvändiga Qt-moduler
# core: Basfunktionalitet (QObject, QString, etc.)
# gui: Grafisk funktionalitet (QImage, QPixmap, etc.)
# widgets: (Valfritt, men vanligt i Qt-appar) Krävs om du har en huvudfönster.
#          Behåll den om du kör den i en konsolapp för att säkerställa att QImage fungerar korrekt.
QT += core gui widgets

# ----------------------------------------------------------------------
# Käll- och header-filer
# ----------------------------------------------------------------------

# Lägger till källfilerna (.cpp) i projektet
SOURCES += \
    ImageSplitter.cpp \
    main.cpp

# Lägger till header-filerna (.h) i projektet
HEADERS += \
    ImageSplitter.h

# ----------------------------------------------------------------------
# Extra Inställningar
# ----------------------------------------------------------------------

# Definerar målprogrammets namn
TARGET = ImageSplitterApp

# Ställer in konfigurationen för att inkludera C++11 (minimi-standarden för modern Qt)
CONFIG += c++11
