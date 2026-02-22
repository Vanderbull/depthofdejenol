#include "CharacterIO.h"
#include "include/GameConstants.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

QString CharacterIO::getFilePath(const QString& characterName) {
    QString cleanName = characterName;
    if (cleanName.endsWith(".txt")) cleanName.chop(4);
    return QString("data/characters/%1.txt").arg(cleanName);
}

QVariantMap CharacterIO::loadFromFile(const QString& characterName) {
    QVariantMap characterMap;
    QString path = getFilePath(characterName);
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open character file:" << path;
        return characterMap;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("---") || line.contains("VERSION")) continue;

        QStringList parts = line.split(": ");
        if (parts.size() < 2) continue;

        QString key = parts.at(0).trimmed();
        QString value = parts.at(1).trimmed();

        if (key == "CurrentCharacterGold") 
            characterMap[key] = QVariant::fromValue(value.toULongLong());
        else if (GameConstants::STAT_NAMES.contains(key) || key == "Age" || key == "HP" || key == "MaxHP" || key == "Level")
            characterMap[key] = value.toInt();
        else 
            characterMap[key] = value;
    }
    return characterMap;
}

bool CharacterIO::saveToFile(const QVariantMap& character) {
    QString name = character["Name"].toString();
    if (name.isEmpty() || name == "Empty Slot") return false;

    QDir().mkpath("data/characters/");
    QFile file(getFilePath(name));
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "CHARACTER_FILE_VERSION: 1.0\n";
    out << "Name: " << name << "\n";
    out << "Race: " << character["Race"].toString() << "\n";
    out << "Level: " << character["Level"].toInt() << "\n";
    out << "HP: " << character["HP"].toInt() << "\n";
    out << "MaxHP: " << character["MaxHP"].toInt() << "\n";
    out << "CurrentCharacterGold: " << character["CurrentCharacterGold"].toULongLong() << "\n";
    
    for (const QString& stat : GameConstants::STAT_NAMES) {
        out << stat << ": " << character[stat].toInt() << "\n";
    }

    QStringList inv = character["Inventory"].toStringList();
    out << "Inventory: " << inv.join(",") << "\n";
    
    return true;
}
