#ifndef CHARACTERIO_H
#define CHARACTERIO_H

#include <QString>
#include <QVariantMap>

class CharacterIO {
public:
    static QString getFilePath(const QString& characterName);
    static QVariantMap loadFromFile(const QString& characterName);
    static bool saveToFile(const QVariantMap& characterData);
};

#endif
