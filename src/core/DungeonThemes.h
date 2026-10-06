#ifndef DUNGEONTHEMES_H
#define DUNGEONTHEMES_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QRandomGenerator>

// A dungeon floor's character: what it is called, how hard it is, how it looks.
struct FloorTheme {
    int level = 1;
    QString name;
    QString description;

    // Difficulty scaling.
    int monsterCount = 0;      // monsters placed on the floor
    int monsterLevelBonus = 0; // added to each monster's level
    int treasureCount = 0;
    int trapCount = 0;

    // Tile mix (percentages, should sum to <= 100).
    int waterChance = 0;
    int pitChance = 0;
    int fogChance = 0;
    int antimagicChance = 0;

    // Boss floor?
    bool isBossFloor = false;
    QString bossName;
};

// Static table of the 15 floors.
class DungeonThemes {
public:
    // Theme for a floor. Levels outside 1..15 clamp into range.
    static FloorTheme forLevel(int level);

    static const int MAX_DEPTH = 15;

    // Is this a boss floor (5, 10, 15)?
    static bool isBossFloor(int level);

    // Boss name for a floor, or empty when it is not a boss floor.
    static QString bossNameForLevel(int level);

    // All themes, for UI listing.
    static QList<FloorTheme> all();
};

#endif // DUNGEONTHEMES_H
