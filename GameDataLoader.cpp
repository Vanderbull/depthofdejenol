#include "GameDataLoader.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QList<QVariantMap> GameDataLoader::parseCSV(const QString& filePath) {
    QList<QVariantMap> dataList;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return dataList;

    QTextStream in(&file);
    if (in.atEnd()) return dataList;

    QStringList headers = in.readLine().split(',');
    for (int i = 0; i < headers.size(); ++i) headers[i] = headers[i].trimmed();

    while (!in.atEnd()) {
        QStringList fields = in.readLine().split(',');
        if (fields.size() == headers.size()) {
            QVariantMap row;
            for (int i = 0; i < headers.size(); ++i) row[headers[i]] = fields[i].trimmed();
            dataList.append(row);
        }
    }
    return dataList;
}

QList<QVariantMap> GameDataLoader::parseMDATAJson(const QString& filePath, QString& versionOut) {
    QList<QVariantMap> results;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return results;

    QByteArray content = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(content.mid(content.indexOf('{')));
    QJsonObject root = doc.object();

    versionOut = root["GameVersion"].toString();

    // Map your categories to Type tags
    QMap<QString, QString> categories = {
        {"Guilds", "Guild"}, {"Races", "Race"}, {"MonsterTypes", "MonsterType"} 
    };

    for (auto it = categories.begin(); it != categories.end(); ++it) {
        QJsonArray arr = root[it.key()].toArray();
        for (const QJsonValue& val : arr) {
            QVariantMap map = val.toObject().toVariantMap();
            map["DataType"] = it.value();
            results.append(map);
        }
    }
    return results;
}
