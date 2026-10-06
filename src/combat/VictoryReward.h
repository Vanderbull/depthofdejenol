#ifndef VICTORYREWARD_H
#define VICTORYREWARD_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariantMap>

// Victory rewards: XP, gold, and loot from a defeated monster.
// Uses MDATA5 data for goldFactor, Item0-Item9 drop slots, and levelFound for XP.
class VictoryReward {
public:
    // Calculate XP reward for a defeated monster.
    // xp = levelFound * 100
    static int calculateXp(const QString& monsterName,
                           const QList<QVariantMap>& monsterData);

    // Calculate gold reward for a defeated monster.
    // gold = goldFactor * random(1, 10)
    static int calculateGold(const QString& monsterName,
                             const QList<QVariantMap>& monsterData);

    // Calculate loot drops for a defeated monster.
    // Rolls on Item0-Item9, each has a chance to drop.
    // Filters by item's floor vs current dungeon depth.
    // Returns a list of item names.
    static QStringList calculateLoot(const QString& monsterName,
                                     const QList<QVariantMap>& monsterData,
                                     int currentDungeonDepth);

    // Get the monster's level for XP calculation.
    static int getMonsterLevel(const QString& monsterName,
                               const QList<QVariantMap>& monsterData);

    // Get the monster's gold factor.
    static int getGoldFactor(const QString& monsterName,
                             const QList<QVariantMap>& monsterData);

    // Get the monster's drop table (Item0-Item9 as item IDs).
    static QList<int> getDropTable(const QString& monsterName,
                                   const QList<QVariantMap>& monsterData);
};

#endif // VICTORYREWARD_H
