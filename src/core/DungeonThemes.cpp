#include "DungeonThemes.h"

QList<FloorTheme> DungeonThemes::all() {
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

    // 1-3: the mines. Easiest, most water, few traps.
    FloorTheme f1 = make(1, "Abandoned Mines", "Timber props and shallow pits.", 12, 6, 2);
    f1.waterChance = 8; f1.pitChance = 3;
    themes.append(f1);

    FloorTheme f2 = make(2, "Flooded Workings", "Cold water seeps through the rock.", 15, 7, 3);
    f2.waterChance = 12; f2.pitChance = 4;
    themes.append(f2);

    FloorTheme f3 = make(3, "Collapsed Tunnels", "Unstable ceilings and sudden drops.", 18, 8, 4);
    f3.pitChance = 6; f3.fogChance = 3;
    themes.append(f3);

    // 4-6: caverns.
    FloorTheme f4 = make(4, "Natural Caverns", "Dripping limestone and blind things.", 22, 9, 5);
    f4.waterChance = 6; f4.fogChance = 5;
    themes.append(f4);

    FloorTheme f5 = make(5, "Sunless Grotto", "A cavern where nothing has seen light.", 25, 10, 6);
    f5.fogChance = 8; f5.pitChance = 5;
    f5.isBossFloor = true; f5.bossName = "Grotto Warden";
    themes.append(f5);

    FloorTheme f6 = make(6, "Crystal Deeps", "Veins of quartz that hum in the dark.", 28, 12, 7);
    f6.antimagicChance = 5;
    themes.append(f6);

    // 7-9: crypts.
    FloorTheme f7 = make(7, "Ancient Crypts", "Stone sarcophagi line the corridors.", 32, 13, 9);
    f7.fogChance = 6;
    themes.append(f7);

    FloorTheme f8 = make(8, "Bone Halls", "The walls are stacked with the long dead.", 36, 14, 10);
    f8.antimagicChance = 6;
    themes.append(f8);

    FloorTheme f9 = make(9, "The Ossuary", "Every surface is bone, white and dry.", 40, 15, 11);
    f9.fogChance = 7; f9.antimagicChance = 7;
    themes.append(f9);

    // 10: boss.
    FloorTheme f10 = make(10, "Throne of Bone", "A vast chamber of fused skeletons.", 45, 18, 12);
    f10.fogChance = 8; f10.antimagicChance = 8; f10.pitChance = 6;
    f10.isBossFloor = true; f10.bossName = "The Bone Tyrant";
    themes.append(f10);

    // 11-14: the descent into fire.
    FloorTheme f11 = make(11, "Black Marble", "Polished floors that reflect no light.", 50, 20, 14);
    f11.antimagicChance = 10;
    themes.append(f11);

    FloorTheme f12 = make(12, "The Furnaces", "Furnace mouths gape in the walls.", 55, 22, 16);
    f12.antimagicChance = 10; f12.pitChance = 7;
    themes.append(f12);

    FloorTheme f13 = make(13, "Ash Wastes", "Warm ash drifts over everything.", 60, 24, 18);
    f13.fogChance = 12; f13.pitChance = 8;
    themes.append(f13);

    FloorTheme f14 = make(14, "The Inferno", "Air that scorches the throat.", 65, 26, 20);
    f14.antimagicChance = 12; f14.pitChance = 9;
    themes.append(f14);

    // 15: final boss.
    FloorTheme f15 = make(15, "The Devil's Threshold", "The last gate before the Prince.", 70, 30, 22);
    f15.fogChance = 15; f15.antimagicChance = 15; f15.pitChance = 10;
    f15.isBossFloor = true; f15.bossName = "The Prince of Devils";
    themes.append(f15);

    return themes;
}

FloorTheme DungeonThemes::forLevel(int level) {
    QList<FloorTheme> themes = all();

    if (level < 1) level = 1;
    if (level > MAX_DEPTH) level = MAX_DEPTH;

    for (const FloorTheme& t : themes) {
        if (t.level == level) {
            FloorTheme result = t;
            // Monster level bonus tracks depth: floor 1 = +0, floor 15 = +14.
            result.monsterLevelBonus = level - 1;
            return result;
        }
    }
    return themes.first();
}

bool DungeonThemes::isBossFloor(int level) {
    return level == 5 || level == 10 || level == 15;
}

QString DungeonThemes::bossNameForLevel(int level) {
    if (!isBossFloor(level)) return QString();
    return forLevel(level).bossName;
}
