#ifndef MONSTERBALANCE_H
#define MONSTERBALANCE_H

#include <QString>
#include <QList>
#include <QVariantMap>

// Monster difficulty curve: floor 1 beatable at level 1; floor 15 needs a real party.
class MonsterBalance {
public:
    // Recommended party level for a floor.
    static int recommendedLevel(int floor);

    // Monster stat multiplier for a floor (1.0 at floor 1).
    static double statMultiplier(int floor);

    // Monster HP for a floor, given a base HP.
    static int scaledHp(int floor, int baseHp);

    // Monster attack for a floor, given a base attack.
    static int scaledAttack(int floor, int baseAttack);

    // Monster defense for a floor, given a base defense.
    static int scaledDefense(int floor, int baseDefense);

    // Monster XP reward for a floor, given a base XP.
    static int scaledXp(int floor, int baseXp);

    // Monster gold reward for a floor, given a base gold.
    static int scaledGold(int floor, int baseGold);

    // Is a party of this size and level ready for this floor?
    static bool isPartyReady(int partySize, int avgLevel, int floor);

    // A difficulty label for a floor.
    static QString difficultyLabel(int floor);

    // All floor difficulty data as a list of maps.
    static QList<QVariantMap> floorTable();
};

#endif // MONSTERBALANCE_H
