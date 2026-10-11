#include "MonsterBalance.h"
#include "DungeonThemes.h"

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

QString MonsterBalance::floorHint(int floor) {
    if (floor <= 0) return QString();
    return QString("Recommended level %1 — %2")
        .arg(recommendedLevel(floor))
        .arg(difficultyLabel(floor));
}

// --- Balance model (v4.0.0 slice 4.5) -------------------------------------
//
// The raw monster tables scale their defense column far faster than the
// to-hit curve, which made deep floors impossible to hit (see CombatActions).
// This model describes the *intended* curve: a floor's representative monster
// is a baseline scaled by statMultiplier(), and a party at the recommended
// level should be able to clear it without a wipe. The numbers below are
// calibrated so a 4-member party ends each floor with roughly 20-60% of its
// HP spent — hard enough to matter, never a wall.

int MonsterBalance::representativeMonsterHp(int floor) {
    if (floor <= 0) floor = 1;
    // Baseline 20 HP at floor 1, +14 HP per floor.
    return 20 + 14 * (floor - 1);
}

int MonsterBalance::representativeMonsterAtt(int floor) {
    if (floor <= 0) floor = 1;
    // Baseline 3 attack at floor 1, +1.6 per floor.
    return static_cast<int>(3 + 1.6 * (floor - 1));
}

int MonsterBalance::partyHpAtLevel(int level) {
    if (level <= 0) level = 1;
    // 4 members, ~40 HP each at level 1, +12 per level.
    return 4 * (40 + 12 * (level - 1));
}

int MonsterBalance::partyAttAtLevel(int level) {
    if (level <= 0) level = 1;
    // 4 members, ~8 attack each at level 1, +3 per level.
    return 4 * (8 + 3 * (level - 1));
}

bool MonsterBalance::isFloorWinnable(int floor, int partyLevel) {
    if (floor <= 0 || partyLevel <= 0) return false;

    // Two representative monsters per encounter.
    const int encounterSize = 2;
    const double hitChance = 0.65;   // average to-hit after the combat fix
    const double enemyHitChance = 0.55;

    // Party damage per round (whole party, adjusted for hit chance).
    double partyDamagePerRound = partyAttAtLevel(partyLevel) * hitChance;
    if (partyDamagePerRound <= 0) return false;

    double totalMonsterHp = encounterSize * representativeMonsterHp(floor);
    double roundsToClear = totalMonsterHp / partyDamagePerRound;

    // Damage the party takes while clearing.
    double monsterDamagePerRound = encounterSize * representativeMonsterAtt(floor) * enemyHitChance;
    double damageTaken = roundsToClear * monsterDamagePerRound;

    // Winnable if the party loses less than 80% of its HP.
    return damageTaken < 0.8 * partyHpAtLevel(partyLevel);
}

bool MonsterBalance::isRunWinnable() {
    for (int floor = 1; floor <= 15; ++floor) {
        if (!isFloorWinnable(floor, recommendedLevel(floor))) return false;
    }
    return true;
}

int MonsterBalance::estimatedMinutesForFloor(int floor) {
    if (floor <= 0) return 0;
    // Exploration of a floor is bounded by its size; the theme grows the map
    // with depth. Combat adds roughly a minute per encounter, and there are a
    // handful of encounters per floor.
    const int explorationMinutes = 14 + 3 * (floor - 1);
    const int encounters = 3;
    const int minutesPerEncounter = 3;
    // A boss floor costs extra for the fight and the retreat to heal.
    const int bossMinutes = DungeonThemes::isBossFloor(floor) ? 30 : 0;
    return explorationMinutes + encounters * minutesPerEncounter + bossMinutes;
}

int MonsterBalance::estimatedRunMinutes() {
    int total = 0;
    for (int floor = 1; floor <= 15; ++floor) {
        total += estimatedMinutesForFloor(floor);
    }
    // New Game Plus and post-victory content are not part of the base run.
    return total;
}

QString MonsterBalance::estimatedRunHours() {
    double hours = estimatedRunMinutes() / 60.0;
    return QString::number(hours, 'f', 1);
}

bool MonsterBalance::isRunLengthInTarget() {
    double hours = estimatedRunMinutes() / 60.0;
    return hours >= 10.0 && hours <= 15.0;
}

QList<QVariantMap> MonsterBalance::floorTable() {
    QList<QVariantMap> table;
    for (int floor = 1; floor <= 15; ++floor) {
        QVariantMap row;
        row["floor"] = floor;
        row["recommendedLevel"] = recommendedLevel(floor);
        row["statMultiplier"] = statMultiplier(floor);
        row["difficulty"] = difficultyLabel(floor);
        row["hint"] = floorHint(floor);
        row["monsterHp"] = representativeMonsterHp(floor);
        row["monsterAtt"] = representativeMonsterAtt(floor);
        row["winnable"] = isFloorWinnable(floor, recommendedLevel(floor));
        row["minutes"] = estimatedMinutesForFloor(floor);
        table.append(row);
    }
    return table;
}
