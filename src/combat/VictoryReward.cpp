#include "VictoryReward.h"
#include "src/items/ItemDatabase.h"
#include "src/items/ItemProgression.h"
#include "src/core/SoundEffects.h"
#include "src/core/Endgame.h"
#include <QRandomGenerator>

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
    return level * 100;
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
