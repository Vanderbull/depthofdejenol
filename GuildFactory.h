#ifndef GUILDFACTORY_H
#define GUILDFACTORY_H

#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QVariant>

class GuildFactory {
public:
    // Only the declaration goes here
    //static QVariantList createGuildMasters();
    static QList<QVariantMap> createGuildMasters();
};

#endif // GUILDFACTORY_H
