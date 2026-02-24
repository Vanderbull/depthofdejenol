#include <QVariantMap>
#include <QList>
#include <QString>

class GameDataLoader {
public:
    // Generic CSV parser - replaces the private parseCSV in GameStateManager
    static QList<QVariantMap> parseCSV(const QString& filePath);

    // Generic JSON parser for your MDATA1-style files
    static QList<QVariantMap> parseMDATAJson(const QString& filePath, QString& versionOut);
};
