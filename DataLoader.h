#ifndef DATALOADER_H
#define DATALOADER_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "DataRegistry.h"
#include "GameConstants.h"

class DataLoader {
public:
    // Loads JSON file and populates the registry
    static bool loadToRegistry(const QString& filePath, DataRegistry& registry) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) return false;

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (!doc.isObject()) return false;

        QJsonObject root = doc.object();
        registry.clear();

        for (auto it = root.begin(); it != root.end(); ++it) {
            QVariant val = it.value().toVariant();
            if (val.type() == QVariant::List) {
                registry.registerData(it.key(), val.toList());
            } else {
                // Wrap single objects/maps in a list so getEntry(0) works
                registry.registerData(it.key(), QVariantList() << val);
            }
        }
        return true;
    }

    // Factory: Converts a QVariant entry into a RaceStats struct
    static GameConstants::RaceStats createRace(const QVariant& data) {
        QVariantMap map = data.toMap();
        GameConstants::RaceStats s;

        s.raceName = map["raceName"].toString();
        s.maxAge = map["maxAge"].toInt();
        s.experience = map["experience"].toInt();

        // Lambda to parse nested RaceStat objects
        auto parseStat = [&](const QString& p) -> GameConstants::RaceStat {
            return { map[p+"_start"].toInt(), map[p+"_min"].toInt(), map[p+"_max"].toInt() };
        };

        s.strength = parseStat("str");
        s.intelligence = parseStat("int");
        s.wisdom = parseStat("wis");
        s.constitution = parseStat("con");
        s.charisma = parseStat("cha");
        s.dexterity = parseStat("dex");

        s.good = (GameConstants::AlignmentStatus)map["good"].toInt();
        s.neutral = (GameConstants::AlignmentStatus)map["neutral"].toInt();
        s.evil = (GameConstants::AlignmentStatus)map["evil"].toInt();

        // Parse Guild Eligibility Map
        QVariantMap guilds = map["guildEligibility"].toMap();
        for(auto it = guilds.begin(); it != guilds.end(); ++it) {
            s.guildEligibility[it.key()] = (GameConstants::AlignmentStatus)it.value().toInt();
        }

        return s;
    }
};

#endif
