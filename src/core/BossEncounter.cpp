#include "BossEncounter.h"
#include "AbyssThemes.h"

bool BossEncounter::canDescend(int level, bool bossDefeated) {
    if (!hasBoss(level)) return true;
    return bossDefeated;
}

QString BossEncounter::blockedMessage(int level) {
    if (!hasBoss(level)) return QString();
    return QString("A terrible presence blocks the way down — %1 stands between you and the stairs.")
        .arg(bossName(level));
}

QVariantMap BossEncounter::buildBoss(int level) {
    QVariantMap boss;
    boss["name"] = bossName(level);
    boss["isPlayer"] = false;

    // Base scaled by floor. Floor 5 is a real fight; floor 15 is brutal.
    int tier = level / 5;  // 1, 2, 3
    int hp = 150 * tier + 50 * (level - 1);
    int att = 20 * tier + 2 * level;
    int def = 8 * tier + level;

    boss["hp"] = hp;
    boss["maxHp"] = hp;
    boss["att"] = att;
    boss["def"] = def;
    boss["speed"] = 10 + level;
    boss["dex"] = 10 + level;
    boss["level"] = level * 2;
    boss["damageMod"] = 150;   // bosses hit harder than normal
    boss["swings"] = tier + 1; // floor 5: 2 swings, floor 10: 3, floor 15: 4
    boss["levelScale"] = 5;

    return boss;
}

int BossEncounter::bossXp(int level) {
    // Far above a normal monster on the same floor (which gives levelFound * 100).
    int tier = level / 5;
    return tier * 5000 + level * 250;
}

// --- Abyss bosses (v4.0.0 slice 4.2) ---

QVariantMap BossEncounter::buildAbyssBoss(int level) {
    // Abyss bosses are far deadlier than main dungeon bosses.
    // The Abyss floor number (1-10) is used directly; boss floors are 5 and 10.
    QVariantMap boss;
    boss["name"] = abyssBossName(level);
    boss["isPlayer"] = false;

    // Scale with Abyss depth. Floor 5 is brutal; floor 10 is endgame-hard.
    // Use multiplicative scaling so floor 10 is significantly harder than floor 5.
    int tier = (level >= 10) ? 3 : 2;  // floor 5 = tier 2, floor 10 = tier 3
    int depthFactor = (level >= 10) ? 2 : 1;  // floor 10 gets 2x depth factor
    int hp = 2000 * tier * depthFactor;
    int att = 50 * tier * depthFactor;
    int def = 20 * tier * depthFactor;

    boss["hp"] = hp;
    boss["maxHp"] = hp;
    boss["att"] = att;
    boss["def"] = def;
    boss["speed"] = 20 + level * 2;
    boss["dex"] = 20 + level * 2;
    boss["level"] = 40 + level * 3;
    boss["damageMod"] = 250;   // Abyss bosses hit much harder
    boss["swings"] = tier + 1; // floor 5: 3 swings, floor 10: 4
    boss["levelScale"] = 10;

    return boss;
}

QString BossEncounter::abyssBossName(int level) {
    return AbyssThemes::bossNameForLevel(level);
}

int BossEncounter::abyssBossXp(int level) {
    // Far above main dungeon bosses.
    int tier = (level >= 10) ? 3 : 2;
    int depthFactor = (level >= 10) ? 2 : 1;
    return 10000 * tier * depthFactor;
}
