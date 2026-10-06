#ifndef RELEASEINFO_H
#define RELEASEINFO_H

#include <QString>
#include <QStringList>
#include <QList>

// Release packaging: version, release notes, installer metadata.
class ReleaseInfo {
public:
    // Current version.
    static QString version();

    // Version as a string for display.
    static QString versionString();

    // Release date.
    static QString releaseDate();

    // Release notes for the current version.
    static QStringList releaseNotes();

    // All changes since the previous version.
    static QStringList changes();

    // System requirements.
    static QString systemRequirements();

    // Installer metadata.
    static QString installerName();

    // Is this a release build?
    static bool isRelease();

    // A short banner for the main window.
    static QString banner();

    // All version history.
    static QList<QPair<QString, QString>> versionHistory();
};

#endif // RELEASEINFO_H
