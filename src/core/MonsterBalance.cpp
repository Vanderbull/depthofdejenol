#include "MonsterBalance.h"

int MonsterBalance::recommendedLevel(int floor) {
    if (floor <= 0) return 1;
    // Floor 1 → level 1, floor 15 → level 15.
    return floor;
}

double MonsterBalance::statMultiplier(int floor) {
    if (floor <= 0) return 1.0;
    // +10% per floor: floor 1 = 1.0, floor 15 = 2.4.
    return 1.0 + 0.1 * (floor - 1);
}

int MonsterBalance::scaledHp(int floor, int baseHp) {
    if (baseHp <= 0) return 0;
    return static_cast<int>(baseHp * statMultiplier(floor));
}

int MonsterBalance::scaledAttack(int floor, int baseAttack) {
    if (baseAttack <= 0) return 0;
    return static_cast<int>(baseAttack * statMultiplier(floor));
}

int MonsterBalance::scaledDefense(int floor, int baseDefense) {
    if (baseDefense <= 0) return 0;
    return static_cast<int>(baseDefense * statMultiplier(floor));
}

int MonsterBalance::scaledXp(int floor, int baseXp) {
    if (baseXp <= 0) return 0;
    // XP scales faster than stats: +15% per floor.
    if (floor <= 0) return baseXp;
    return static_cast<int>(baseXp * (1.0 + 0.15 * (floor - 1)));
}

int MonsterBalance::scaledGold(int floor, int baseGold) {
    if (baseGold <= 0) return 0;
    // Gold scales with stats.
    return static_cast<int>(baseGold * statMultiplier(floor));
}

bool MonsterBalance::isPartyReady(int partySize, int avgLevel, int floor) {
    if (partySize <= 0 || avgLevel <= 0 || floor <= 0) return false;
    int recommended = recommendedLevel(floor);
    // A party of 4+ at the recommended level is ready.
    // A smaller party needs to be over-leveled.
    if (partySize >= 4) return avgLevel >= recommended;
    if (partySize >= 2) return avgLevel >= recommended + 2;
    return avgLevel >= recommended + 5;
}

QString MonsterBalance::difficultyLabel(int floor) {
    if (floor <= 0) return "Unknown";
    if (floor <= 3) return "Easy";
    if (floor <= 6) return "Moderate";
    if (floor <= 10) return "Hard";
    if (floor <= 13) return "Very Hard";
    return "Deadly";
}

QList<QVariantMap> MonsterBalance::floorTable() {
    QList<QVariantMap> table;
    for (int floor = 1; floor <= 15; ++floor) {
        QVariantMap row;
        row["floor"] = floor;
        row["recommendedLevel"] = recommendedLevel(floor);
        row["statMultiplier"] = statMultiplier(floor);
        row["difficulty"] = difficultyLabel(floor);
        table.append(row);
    }
    return table;
}
