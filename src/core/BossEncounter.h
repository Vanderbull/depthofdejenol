#ifndef BOSSENCOUNTER_H
#define BOSSENCOUNTER_H

#include "DungeonThemes.h"
#include <QString>
#include <QStringList>
#include <QVariantMap>

// Boss encounters on floors 5, 10 and 15. A boss guards the stairs down:
// the party cannot descend until the boss is defeated.
class BossEncounter {
public:
    // Is this floor gated by a boss?
    static bool hasBoss(int level) { return DungeonThemes::isBossFloor(level); }

    // Can the party use the down stairs on this floor?
    // Non-boss floors: always true. Boss floors: only once the boss is dead.
    static bool canDescend(int level, bool bossDefeated);

    // Message shown when a boss blocks the stairs.
    static QString blockedMessage(int level);

    // Build the boss as a CombatParticipant-ready map.
    // Stats scale with the floor so floor 15 is far deadlier than floor 5.
    static QVariantMap buildBoss(int level);

    // The boss's name.
    static QString bossName(int level) { return DungeonThemes::bossNameForLevel(level); }

    // XP awarded for defeating the boss (much larger than a normal monster).
    static int bossXp(int level);

    // --- Abyss bosses (v4.0.0 slice 4.2) ---
    // Build an Abyss boss as a CombatParticipant-ready map.
    // Abyss floors are 1-10, boss floors are 5 and 10.
    static QVariantMap buildAbyssBoss(int level);

    // The Abyss boss's name.
    static QString abyssBossName(int level);

    // XP awarded for defeating an Abyss boss.
    static int abyssBossXp(int level);
};

#endif // BOSSENCOUNTER_H
