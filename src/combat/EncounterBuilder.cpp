#include "EncounterBuilder.h"
#include "src/core/Endgame.h"
#include "src/core/MonsterBalance.h"
#include <QVariantMap>
#include <QList>
#include <QString>

namespace {

// The monster tables in this repo come in two shapes: the older MDATA5-style
// rows used by the self-tests ("numGroups", "levelFound", "StatDex") and the
// curated data/bestiary.json rows ("groups", "level", stats.Dexterity). Read
// whichever is present so both sources work.
int readInt(const QVariantMap& m, const QString& a, const QString& b, int fallback = 0)
{
    if (m.contains(a)) return m.value(a).toInt();
    if (m.contains(b)) return m.value(b).toInt();
    return fallback;
}

int readStat(const QVariantMap& m, const QString& legacyKey, const QString& statName)
{
    if (m.contains(legacyKey)) return m.value(legacyKey).toInt();
    const QVariantMap stats = m.value("stats").toMap();
    if (stats.contains(statName)) return stats.value(statName).toInt();
    return 0;
}

} // namespace

QList<CombatParticipant> EncounterBuilder::buildEncounter(
    const QString& monsterName,
    const QList<QVariantMap>& monsterData,
    int ngPlusLevel,
    int floorLevel)
{
    QList<CombatParticipant> result;
    const double ngMult = Endgame::ngPlusMonsterMultiplier(ngPlusLevel);
    const double floorMult = MonsterBalance::statMultiplier(floorLevel);

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

    // Three monster-table shapes exist in this repo, and "hits" means different
    // things in each:
    //   MDATA5 rows   (gameStateManager): hits = HP, numGroups = groups,
    //                                     ingroup = monsters per group
    //   bestiary rows (data/bestiary.json): hits = HP, groups = group count
    //   legacy test rows: hits = monsters per group, HP from Constitution
    // `ingroup` only exists in the MDATA5 shape, which is what tells them apart.
    const bool mdataShape   = monsterInfo.contains("ingroup");
    const bool bestiaryShape = !mdataShape &&
                               monsterInfo.contains("level") && monsterInfo.contains("groups");

    int numGroups = readInt(monsterInfo, "numGroups", "groups", 1);
    int perGroup;
    if (mdataShape)          perGroup = qMax(1, monsterInfo.value("ingroup").toInt());
    else if (bestiaryShape)  perGroup = 1;
    else                     perGroup = qMax(1, monsterInfo.value("hits").toInt());

    // Total monsters in the encounter.
    int totalMonsters = numGroups * perGroup;
    if (totalMonsters < 1) totalMonsters = 1;
    if (totalMonsters > 10) totalMonsters = 10;  // Cap at 10 for sanity

    // Base HP. MDATA5 and bestiary rows carry it directly in "hits"; legacy
    // rows build it from Constitution.
    int baseHp;
    if (mdataShape || bestiaryShape) {
        baseHp = monsterInfo.value("hits").toInt();
    } else {
        baseHp = readStat(monsterInfo, "StatCon", "Constitution") * 2 + 10;
    }
    if (baseHp < 5) baseHp = 20;
    int baseAtt = monsterInfo["att"].toInt();
    int baseDef = monsterInfo["def"].toInt();
    int baseSpeed = readStat(monsterInfo, "StatDex", "Dexterity");
    int baseDex = baseSpeed;
    int baseLevel = readInt(monsterInfo, "levelFound", "level", 1);
    if (baseLevel < 1) baseLevel = 1;
    int baseDamageMod = monsterInfo["damageMod"].toInt();
    if (baseDamageMod <= 0) baseDamageMod = 100;
    // Attacks per round. Only an explicit "swings" column sets this; "hits" is
    // HP in the real tables, so it must never be read as a swing count.
    int baseSwings = monsterInfo.value("swings", 1).toInt();
    if (baseSwings < 1) baseSwings = 1;
    if (baseSwings > 4) baseSwings = 4;  // keep multi-attack sane

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

        // New Game Plus scaling: monsters get stronger each cycle.
        // MonsterBalance scaling: monsters get stronger on deeper floors.
        const double totalMult = ngMult * floorMult;
        p.hp = qMax(1, qRound(p.hp * totalMult));
        p.maxHp = p.hp;
        p.att = qMax(1, qRound(baseAtt * totalMult));
        p.def = qMax(1, qRound(baseDef * totalMult));
        p.speed = baseSpeed;
        p.dex = baseDex;
        p.level = baseLevel;
        p.damageMod = baseDamageMod;
        p.swings = baseSwings;
        p.levelScale = 0;  // Monsters don't scale with level by default

        // Set abilities based on monster name and bestiary category.
        QString lowerName = monsterName.toLower();
        QString category = monsterInfo.value("category").toString().toLower();
        if (lowerName.contains("spider") || lowerName.contains("snake") ||
            lowerName.contains("scorpion") || lowerName.contains("centipede") ||
            category == "insects" || category == "reptiles") {
            p.canPoison = true;
        }
        if (lowerName.contains("dragon") || lowerName.contains("demon") ||
            lowerName.contains("devil") || lowerName.contains("fire") ||
            category == "dragons" || category == "demons" || category == "devils") {
            p.canBreathFire = true;
        }
        if (lowerName.contains("troll") || lowerName.contains("ooze") ||
            lowerName.contains("slime") || category == "slimes") {
            p.canRegenerate = true;
            p.regenerateAmount = 2 + baseLevel / 2;
        }
        if (lowerName.contains("mage") || lowerName.contains("wizard") ||
            lowerName.contains("sorcerer") || lowerName.contains("enchanter") ||
            category == "mages") {
            p.canCastSpells = true;
        }

        result.append(p);
    }

    return result;
}

int EncounterBuilder::getGroupSize(const QString& monsterName,
                                   const QList<QVariantMap>& monsterData)
{
    for (const QVariantMap& m : monsterData) {
        if (m["name"].toString() == monsterName) {
            const bool bestiaryShape = m.contains("level") || m.contains("groups");
            int numGroups = readInt(m, "numGroups", "groups", 1);
            int perGroup = bestiaryShape ? 1 : m["hits"].toInt();
            int total = numGroups * perGroup;
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
