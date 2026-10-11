#include "AbyssThemes.h"

QList<FloorTheme> AbyssThemes::all() {
    QList<FloorTheme> themes;

    auto make = [](int level, const QString& name, const QString& desc,
                   int monsters, int treasure, int traps) {
        FloorTheme t;
        t.level = level;
        t.name = name;
        t.description = desc;
        t.monsterCount = monsters;
        t.treasureCount = treasure;
        t.trapCount = traps;
        return t;
    };

    // 1-4: the outer rim. Harder than main dungeon floor 15.
    FloorTheme f1 = make(1, "The Outer Rim", "Where the light dies.", 80, 35, 25);
    f1.fogChance = 18; f1.pitChance = 12;
    themes.append(f1);

    FloorTheme f2 = make(2, "Whispering Dark", "Voices that are almost words.", 85, 38, 27);
    f2.fogChance = 20; f2.antimagicChance = 15;
    themes.append(f2);

    FloorTheme f3 = make(3, "The Drowning Deep", "Water that rises and does not fall.", 90, 40, 29);
    f3.waterChance = 20; f3.pitChance = 14;
    themes.append(f3);

    FloorTheme f4 = make(4, "Halls of the Fallen", "Armies that never left.", 95, 42, 31);
    f4.fogChance = 22; f4.antimagicChance = 18;
    themes.append(f4);

    // 5: first Abyss boss.
    FloorTheme f5 = make(5, "The Maw", "A mouth that opens downward.", 100, 45, 33);
    f5.fogChance = 25; f5.antimagicChance = 20; f5.pitChance = 15;
    f5.isBossFloor = true; f5.bossName = "Abyssal Maw";
    themes.append(f5);

    // 6-9: the inner abyss.
    FloorTheme f6 = make(6, "The Screaming Vats", "Something is being made here.", 105, 48, 35);
    f6.antimagicChance = 22; f6.pitChance = 16;
    themes.append(f6);

    FloorTheme f7 = make(7, "The Bone Cathedral", "A church with no god.", 110, 50, 37);
    f7.fogChance = 28; f7.antimagicChance = 25;
    themes.append(f7);

    FloorTheme f8 = make(8, "The Frozen Wound", "Cold that burns.", 115, 52, 39);
    f8.fogChance = 30; f8.pitChance = 18;
    themes.append(f8);

    FloorTheme f9 = make(9, "The Throne of Echoes", "Every word you ever said, repeated.", 120, 55, 41);
    f9.fogChance = 32; f9.antimagicChance = 28;
    themes.append(f9);

    // 10: final Abyss boss.
    FloorTheme f10 = make(10, "The Abyssal Heart", "The center of everything.", 130, 60, 45);
    f10.fogChance = 35; f10.antimagicChance = 30; f10.pitChance = 20;
    f10.isBossFloor = true; f10.bossName = "The Abyssal Heart";
    themes.append(f10);

    return themes;
}

FloorTheme AbyssThemes::forLevel(int level) {
    if (level < 1) level = 1;
    if (level > MAX_DEPTH) level = MAX_DEPTH;

    QList<FloorTheme> themes = all();
    for (const FloorTheme& t : themes) {
        if (t.level == level) {
            FloorTheme result = t;
            // Monster level bonus tracks depth: floor 1 = +15 (past main dungeon floor 15).
            result.monsterLevelBonus = 15 + level - 1;
            return result;
        }
    }
    return themes.first();
}

bool AbyssThemes::isBossFloor(int level) {
    return level == 5 || level == 10;
}

QString AbyssThemes::bossNameForLevel(int level) {
    if (!isBossFloor(level)) return QString();
    return forLevel(level).bossName;
}
