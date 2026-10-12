#include "VictoryReward.h"
#include "src/items/ItemDatabase.h"
#include "src/items/ItemProgression.h"
#include "src/core/SoundEffects.h"
#include "src/core/Endgame.h"
#include <QRandomGenerator>
#include <cmath>

namespace {

// Monster rows exist in two shapes in this repo: the older MDATA5-style rows
// used by the self-tests ("levelFound") and the curated data/bestiary.json
// rows ("level"). Read whichever is present.
int monsterLevel(const QVariantMap& m)
{
    if (m.contains("levelFound")) return m.value("levelFound").toInt();
    return m.value("level", 1).toInt();
}

const QVariantMap* findMonster(const QString& monsterName,
                               const QList<QVariantMap>& monsterData)
{
    for (const QVariantMap& m : monsterData) {
        if (m["name"].toString() == monsterName) return &m;
    }
    return nullptr;
}

} // namespace

int VictoryReward::calculateXp(const QString& monsterName,
                               const QList<QVariantMap>& monsterData)
{
    int level = getMonsterLevel(monsterName, monsterData);
    // Use the same XP curve as LevelTable: round(30 * level^1.2)
    // This keeps monster XP proportional to the level curve.
    return qRound(30.0 * std::pow(level, 1.2));
}

int VictoryReward::calculateGold(const QString& monsterName,
                                 const QList<QVariantMap>& monsterData,
                                 int ngPlusLevel)
{
    int goldFactor = getGoldFactor(monsterName, monsterData);
    if (goldFactor <= 0) return 0;
    const double ngMult = Endgame::ngPlusRewardMultiplier(ngPlusLevel);
    return qRound(goldFactor * (1 + QRandomGenerator::global()->bounded(10)) * ngMult);
}

QStringList VictoryReward::calculateLoot(const QString& monsterName,
                                         const QList<QVariantMap>& monsterData,
                                         int currentDungeonDepth)
{
    QStringList loot;
    QList<int> dropTable = getDropTable(monsterName, monsterData);

    // data/bestiary.json (the live 401-row monster table) carries no Item0-9
    // fields, so the explicit drop table is empty for every real monster and
    // nothing ever dropped. Derive a table from the monster's own level so a
    // kill can yield a floor-appropriate item.
    if (dropTable.isEmpty()) {
        dropTable = deriveDropTable(monsterName, monsterData, currentDungeonDepth);
    }

    for (int itemId : dropTable) {
        if (itemId <= 0) continue;

        // Look up item in item data
        const ItemDef* item = ItemDatabase::instance().byId(itemId);
        if (!item) continue;

        // Filter by dungeon depth
        if (item->floor > currentDungeonDepth) continue;

        // Filter by ItemProgression tier: items from a tier that has not
        // unlocked yet (e.g. Mithril on floor 1) do not drop.
        // Only apply tier-gating when the item name actually carries a tier
        // prefix; unprefixed items keep the depth filter above.
        bool hasTierPrefix = false;
        for (ItemProgression::Tier t : ItemProgression::allTiers()) {
            if (item->name.startsWith(ItemProgression::tierName(t), Qt::CaseInsensitive)) {
                hasTierPrefix = true;
                if (!ItemProgression::isAvailable(t, currentDungeonDepth)) hasTierPrefix = false;
                break;
            }
        }
        if (!hasTierPrefix && item->name.startsWith("Bronze", Qt::CaseInsensitive)) {
            // Bronze items are always available from floor 1.
            hasTierPrefix = true;
        }
        if (!hasTierPrefix) continue;

        // Roll for drop chance: base 10% + rarity bonus
        // Higher rarity = lower drop chance
        int dropChance = 10 + (5 - qMin(item->rarity, 5)) * 2;
        if (QRandomGenerator::global()->bounded(100) < dropChance) {
            loot.append(item->name);
        }
    }

    return loot;
}

int VictoryReward::getMonsterLevel(const QString& monsterName,
                                   const QList<QVariantMap>& monsterData)
{
    if (const QVariantMap* m = findMonster(monsterName, monsterData)) {
        return monsterLevel(*m);
    }
    return 1;
}

int VictoryReward::getGoldFactor(const QString& monsterName,
                                 const QList<QVariantMap>& monsterData)
{
    if (const QVariantMap* m = findMonster(monsterName, monsterData)) {
        return m->value("goldFactor").toInt();
    }
    return 0;
}

QList<int> VictoryReward::getDropTable(const QString& monsterName,
                                       const QList<QVariantMap>& monsterData)
{
    QList<int> dropTable;
    if (const QVariantMap* m = findMonster(monsterName, monsterData)) {
        // Item0 through Item9
        for (int i = 0; i <= 9; ++i) {
            QString key = QString("Item%1").arg(i);
            int itemId = m->value(key).toInt();
            if (itemId > 0) {
                dropTable.append(itemId);
            }
        }
    }
    return dropTable;
}

QList<int> VictoryReward::deriveDropTable(const QString& monsterName,
                                          const QList<QVariantMap>& monsterData,
                                          int currentDungeonDepth)
{
    // The monster's own level drives which tier of loot it can carry: a
    // level-N monster maps to the tier of a floor-N chest, clamped to the
    // current dungeon depth so nothing from below the party's floor appears.
    const int monsterLvl = getMonsterLevel(monsterName, monsterData);
    int lootFloor = qMin(monsterLvl, currentDungeonDepth);
    if (lootFloor < 1) lootFloor = 1;

    const ItemProgression::Tier tier = ItemProgression::tierForFloor(lootFloor);
    const QString prefix = ItemProgression::tierName(tier);

    QList<int> table;
    for (const ItemDef& item : ItemDatabase::instance().all()) {
        if (item.id <= 0) continue;
        if (item.floor > currentDungeonDepth) continue;
        // Match the tier by name prefix (Bronze Sword, Iron Axe, ...).
        if (!item.name.startsWith(prefix, Qt::CaseInsensitive)) continue;
        table.append(item.id);
    }

    // Keep the table bounded so a single kill rolls a handful of candidates,
    // not the whole tier — pick a stable slice keyed on the monster name.
    if (table.size() > 8) {
        const int offset = qAbs(qHash(monsterName)) % table.size();
        QList<int> slice;
        for (int i = 0; i < 8; ++i) {
            slice.append(table.at((offset + i) % table.size()));
        }
        return slice;
    }
    return table;
}
