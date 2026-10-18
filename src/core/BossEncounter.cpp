#include "BossEncounter.h"
#include "AbyssThemes.h"
#include "GameConstants.h"

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
    int hp = GameConstants::BASE_BOSS_HP_PER_TIER * tier + GameConstants::BASE_BOSS_HP_PER_LEVEL * (level - 1);
    int att = GameConstants::BASE_BOSS_ATT_PER_TIER * tier + GameConstants::BASE_BOSS_ATT_PER_LEVEL * level;
    int def = GameConstants::BASE_BOSS_DEF_PER_TIER * tier + GameConstants::BASE_BOSS_DEF_PER_LEVEL * level;

    boss["hp"] = hp;
    boss["maxHp"] = hp;
    boss["att"] = att;
    boss["def"] = def;
    boss["speed"] = 10 + level;
    boss["dex"] = 10 + level;
    boss["level"] = level * 2;
    boss["damageMod"] = GameConstants::BOSS_DAMAGE_MOD;
    boss["swings"] = tier + 1; // floor 5: 2 swings, floor 10: 3, floor 15: 4
    boss["levelScale"] = GameConstants::BOSS_LEVEL_SCALE;

    return boss;
}

int BossEncounter::bossXp(int level) {
    // Far above a normal monster on the same floor (which gives levelFound * 100).
    int tier = level / 5;
    return tier * GameConstants::BOSS_XP_PER_TIER + level * GameConstants::BOSS_XP_PER_LEVEL;
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
    int hp = GameConstants::ABYSS_BASE_HP * tier * depthFactor;
    int att = GameConstants::ABYSS_BASE_ATT * tier * depthFactor;
    int def = GameConstants::ABYSS_BASE_DEF * tier * depthFactor;

    boss["hp"] = hp;
    boss["maxHp"] = hp;
    boss["att"] = att;
    boss["def"] = def;
    boss["speed"] = 20 + level * 2;
    boss["dex"] = 20 + level * 2;
    boss["level"] = 40 + level * 3;
    boss["damageMod"] = GameConstants::ABYSS_DAMAGE_MOD;
    boss["swings"] = tier + 1; // floor 5: 3 swings, floor 10: 4
    boss["levelScale"] = GameConstants::ABYSS_LEVEL_SCALE;

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
