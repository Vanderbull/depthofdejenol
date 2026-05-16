#include "savegameUtils.h"
#include <QFile>
#include <QTextStream>
#include <QSet>
#include <QList>
#include <QPair>
#include <QMap>
#include <QDebug>

namespace SavegameUtils {

bool verifySaveGame(const QString& characterName) {
    QString cleanName = characterName;
    if (cleanName.endsWith(".txt")) cleanName.chop(4);

    QString filename = QString("data/characters/%1.txt").arg(cleanName);
    QFile file(filename);

    if (!file.exists()) {
        qWarning() << "Verification failed: File does not exist -" << filename;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    bool hasVersion = false;
    bool hasLocation = false;
    bool hasName = false;
    int foundCoords = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.startsWith("CHARACTER_FILE_VERSION:")) hasVersion = true;

        if (line.startsWith("Name: ")) {
            if (line.mid(6) == cleanName) hasName = true;
        }

        if (line.startsWith("DungeonX:") ||
            line.startsWith("DungeonY:") ||
            line.startsWith("DungeonLevel:")) {
            foundCoords++;
        }
    }
    file.close();

    hasLocation = (foundCoords == 3);
    if (!hasVersion) qWarning() << "Savegame" << cleanName << "is missing version header.";
    if (!hasLocation) qWarning() << "Savegame" << cleanName << "is missing coordinate data.";
    if (!hasName) qWarning() << "Savegame" << cleanName << "name mismatch inside file.";

    return (hasVersion && hasLocation && hasName);
}

bool repairSaveGame(const QString& characterName) {
    QString cleanName = characterName;
    if (cleanName.endsWith(".txt")) {
        cleanName.chop(4);
    }
    QString path = QString("data/characters/%1.txt").arg(cleanName);
    QFile file(path);
    if (!file.exists()) {
        return false;
    }

    QList<QPair<QString, QString>> dataList;
    QSet<QString> uniqueKeys;
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;
            int colonIndex = line.indexOf(':');
            if (colonIndex != -1) {
                QString key = line.left(colonIndex).trimmed();
                QString value = line.mid(colonIndex + 1).trimmed();
                if (!uniqueKeys.contains(key)) {
                    dataList.append(qMakePair(key, value));
                    uniqueKeys.insert(key);
                }
            }
        }
        file.close();
    }

    QMap<QString, QString> dataMap;
    for (const auto& pair : dataList) {
        dataMap[pair.first] = pair.second;
    }

    bool modified = false;

    if (!dataMap.contains("CHARACTER_FILE_VERSION")) {
        dataList.prepend(qMakePair(QString("CHARACTER_FILE_VERSION"), QString("1.0")));
        dataMap["CHARACTER_FILE_VERSION"] = "1.0";
        modified = true;
    }
    if (!dataMap.contains("Race")) {
        dataList.append(qMakePair(QString("Race"), QString("Human")));
        dataMap["Race"] = "Human";
        modified = true;
    }
    if (!dataMap.contains("Age")) {
        dataList.append(qMakePair(QString("Age"), QString("16")));
        dataMap["Age"] = "16";
        modified = true;
    }
    if (!dataMap.contains("DungeonX")) {
        dataList.append(qMakePair(QString("DungeonX"), QString("17")));
        dataMap["DungeonX"] = "17";
        modified = true;
    }
    if (!dataMap.contains("DungeonY")) {
        dataList.append(qMakePair(QString("DungeonY"), QString("12")));
        dataMap["DungeonY"] = "12";
        modified = true;
    }
    if (!dataMap.contains("DungeonLevel")) {
        dataList.append(qMakePair(QString("DungeonLevel"), QString("1")));
        dataMap["DungeonLevel"] = "1";
        modified = true;
    }
    if (dataMap.value("Name") != cleanName) {
        bool nameUpdated = false;
        for (auto& pair : dataList) {
            if (pair.first == "Name") {
                pair.second = cleanName;
                nameUpdated = true;
                break;
            }
        }
        if (!nameUpdated) {
            dataList.append(qMakePair(QString("Name"), cleanName));
        }
        dataMap["Name"] = cleanName;
        modified = true;
    }

    if (modified) {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            out << "CHARACTER_FILE_VERSION: " << dataMap["CHARACTER_FILE_VERSION"] << "\n";
            for (const auto& pair : dataList) {
                if (pair.first == "CHARACTER_FILE_VERSION") continue;
                out << pair.first << ": " << pair.second << "\n";
            }
            file.close();
            qDebug() << "Successfully repaired savegame for:" << cleanName;
            return true;
        }
    }
    return false;
}

} // namespace SavegameUtils
