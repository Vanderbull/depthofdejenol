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

    // A short UI hint for a floor: "Recommended level 3 — Moderate".
    static QString floorHint(int floor);

    // --- Balance model (v4.0.0 slice 4.5) ---------------------------------
    // A transparent, tunable model of the difficulty curve. It is not a
    // substitute for playtesting, but it turns "is the run winnable?" into a
    // checkable property instead of a claim.

    // Representative monster stats on a floor (a baseline monster scaled by
    // the floor multiplier).
    static int representativeMonsterHp(int floor);
    static int representativeMonsterAtt(int floor);

    // Expected party totals at a given level (4-member party).
    static int partyHpAtLevel(int level);
    static int partyAttAtLevel(int level);

    // Can a party at `partyLevel` beat a floor's representative monster
    // without being wiped? Uses average damage, not dice.
    static bool isFloorWinnable(int floor, int partyLevel);

    // Every floor is beatable by a party at its recommended level.
    static bool isRunWinnable();

    // Estimated minutes to clear a floor (exploration + combat + backtracking).
    static int estimatedMinutesForFloor(int floor);

    // Estimated total minutes for a full 1..15 run.
    static int estimatedRunMinutes();

    // Estimated total hours for a full run, as a one-decimal string.
    static QString estimatedRunHours();

    // Is the estimated run length inside the target band (10-15 hours)?
    static bool isRunLengthInTarget();

    // All floor difficulty data as a list of maps.
    static QList<QVariantMap> floorTable();
};

#endif // MONSTERBALANCE_H
