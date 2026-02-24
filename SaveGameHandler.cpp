#include "SaveGameHandler.h"
#include "CharacterIO.h"
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QDebug>

bool SaveGameHandler::verifySave(const QString& characterName) {
    QString path = CharacterIO::getFilePath(characterName);
    QFile file(path);
    
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Verification failed: File missing or unreadable -" << path;
        return false;
    }

    QTextStream in(&file);
    bool hasVersion = false;
    bool hasName = false;
    int foundCoords = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        if (line.startsWith("CHARACTER_FILE_VERSION:")) hasVersion = true;
        if (line.startsWith("Name: ") && line.mid(6) == characterName) hasName = true;
        
        // Check for required location data
        if (line.startsWith("DungeonX:") || 
            line.startsWith("DungeonY:") || 
            line.startsWith("DungeonLevel:")) {
            foundCoords++;
        }
    }
    file.close();

    return (hasVersion && hasName && foundCoords == 3);
}

bool SaveGameHandler::repairSave(const QString& characterName) {
    QString path = CharacterIO::getFilePath(characterName);
    QFile file(path);
    if (!file.exists()) return false;

    QMap<QString, QString> dataMap;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.contains(": ")) {
                QStringList parts = line.split(": ");
                dataMap[parts[0]] = parts[1];
            }
        }
        file.close();
    }

    bool modified = false;

    // Fix missing version
    if (!dataMap.contains("CHARACTER_FILE_VERSION")) {
        dataMap["CHARACTER_FILE_VERSION"] = "1.0";
        modified = true;
    }

    // Fix missing coordinates (Default to City Entrance)
    if (!dataMap.contains("DungeonX")) { dataMap["DungeonX"] = "17"; modified = true; }
    if (!dataMap.contains("DungeonY")) { dataMap["DungeonY"] = "12"; modified = true; }
    if (!dataMap.contains("DungeonLevel")) { dataMap["DungeonLevel"] = "1"; modified = true; }

    // Ensure the internal name matches the filename
    if (dataMap["Name"] != characterName) {
        dataMap["Name"] = characterName;
        modified = true;
    }

    if (modified) {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            // Write core headers first for consistency
            out << "CHARACTER_FILE_VERSION: " << dataMap["CHARACTER_FILE_VERSION"] << "\n";
            out << "Name: " << dataMap["Name"] << "\n";
            
            dataMap.remove("CHARACTER_FILE_VERSION");
            dataMap.remove("Name");

            QMapIterator<QString, QString> i(dataMap);
            while (i.hasNext()) {
                i.next();
                out << i.key() << ": " << i.value() << "\n";
            }
            file.close();
            qDebug() << "Repaired savegame for:" << characterName;
            return true;
        }
    }
    return false;
}
