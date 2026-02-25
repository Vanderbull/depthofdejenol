#ifndef CHARACTERIO_H
#define CHARACTERIO_H

#include <QString>
#include <QVariantMap>
#include "character.h"

class CharacterIO {
public:
    // Returns the full path for a character file
    static QString getFilePath(const QString& characterName);

    // Loads a character file into a QVariantMap
    static QVariantMap loadFromFile(const QString& characterName);

    // Saves a character map to a file
    static bool saveToFile(const QVariantMap& characterData);

    // Moves the logic for verifying and repairing files here too
    static bool verifySave(const QString& characterName);
};

#endif
