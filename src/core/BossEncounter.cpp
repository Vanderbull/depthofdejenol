#include "BossEncounter.h"

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
