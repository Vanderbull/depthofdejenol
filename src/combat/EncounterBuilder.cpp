#include "EncounterBuilder.h"
#include <QVariantMap>
#include <QList>
#include <QString>

QList<CombatParticipant> EncounterBuilder::buildEncounter(
    const QString& monsterName,
    const QList<QVariantMap>& monsterData)
{
    QList<CombatParticipant> result;

    // Find the monster in the data
    QVariantMap monsterInfo;
    for (const QVariantMap& m : monsterData) {
        if (m["name"].toString() == monsterName) {
            monsterInfo = m;
            break;
        }
    }

    if (monsterInfo.isEmpty()) {
        // Fallback: create a generic monster
        CombatParticipant p;
        p.name = monsterName;
        p.isPlayer = false;
        p.hp = 20;
        p.maxHp = 20;
        p.att = 8;
        p.def = 5;
        p.speed = 5;
        p.dex = 5;
        p.level = 1;
        p.damageMod = 100;
        p.swings = 1;
        p.levelScale = 0;
        result.append(p);
        return result;
    }

    // Get group size from numGroups and hits
    int numGroups = monsterInfo["numGroups"].toInt();
    int hits = monsterInfo["hits"].toInt();

    // Total monsters = numGroups * hits (but cap at reasonable number)
    int totalMonsters = numGroups * hits;
    if (totalMonsters < 1) totalMonsters = 1;
    if (totalMonsters > 10) totalMonsters = 10;  // Cap at 10 for sanity

    // Get base stats
    int baseHp = monsterInfo["StatCon"].toInt() * 2 + 10;  // HP from constitution
    if (baseHp < 5) baseHp = 20;
    int baseAtt = monsterInfo["att"].toInt();
    int baseDef = monsterInfo["def"].toInt();
    int baseSpeed = monsterInfo["StatDex"].toInt();
    int baseDex = monsterInfo["StatDex"].toInt();
    int baseLevel = monsterInfo["levelFound"].toInt();
    int baseDamageMod = monsterInfo["damageMod"].toInt();
    int baseSwings = monsterInfo["hits"].toInt();
    if (baseSwings < 1) baseSwings = 1;

    // Create individual monsters with slight variation
    for (int i = 0; i < totalMonsters; ++i) {
        CombatParticipant p;
        p.name = getMonsterName(monsterName, i + 1);
        p.isPlayer = false;

        // Slight HP variation (±10%)
        int hpVariation = baseHp / 10;
        p.hp = baseHp + (i % 3 - 1) * hpVariation;  // -1, 0, +1 variation
        if (p.hp < 1) p.hp = 1;
        p.maxHp = p.hp;

        p.att = baseAtt;
        p.def = baseDef;
        p.speed = baseSpeed;
        p.dex = baseDex;
        p.level = baseLevel;
        p.damageMod = baseDamageMod;
        p.swings = baseSwings;
        p.levelScale = 0;  // Monsters don't scale with level by default

        result.append(p);
    }

    return result;
}

int EncounterBuilder::getGroupSize(const QString& monsterName,
                                   const QList<QVariantMap>& monsterData)
{
    for (const QVariantMap& m : monsterData) {
        if (m["name"].toString() == monsterName) {
            int numGroups = m["numGroups"].toInt();
            int hits = m["hits"].toInt();
            int total = numGroups * hits;
            if (total < 1) total = 1;
            if (total > 10) total = 10;
            return total;
        }
    }
    return 1;  // Default: 1 monster
}

QString EncounterBuilder::getMonsterName(const QString& baseName, int index)
{
    if (index <= 1) return baseName;
    return QString("%1 %2").arg(baseName).arg(index);
}
