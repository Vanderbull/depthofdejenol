#ifndef ABYSS_THEMES_H
#define ABYSS_THEMES_H

#include "DungeonThemes.h"
#include <QString>
#include <QStringList>
#include <QList>

// The Abyss — a post-victory mega-dungeon with 10 floors.
// Accessible only after beating the final boss.
class AbyssThemes {
public:
    static const int MAX_DEPTH = 10;

    // Theme for an Abyss floor (1-10).
    static FloorTheme forLevel(int level);

    // Is this an Abyss boss floor (5 or 10)?
    static bool isBossFloor(int level);

    // Boss name for an Abyss floor.
    static QString bossNameForLevel(int level);

    // All Abyss themes, for UI listing.
    static QList<FloorTheme> all();

    // Display name of the dungeon.
    static QString dungeonName() { return "The Abyss"; }
};

#endif // ABYSS_THEMES_H
