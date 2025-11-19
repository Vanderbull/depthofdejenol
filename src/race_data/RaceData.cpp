#include "RaceData.h"
#include <QDebug>

// Helper to load and return the list of all playable guilds
QVector<QString> loadGuildData() {
    return {
        "Nomad",
        "Warrior",
        "Paladin",
        "Ninja",
        "Villain",
        "Seeker",
        "Thief",
        "Scavenger",
        "Mage",
        "Sorcerer",
        "Wizard",
        "Healer"
    };
}

// Helper to load and return the race data vector
QVector<RaceStats> loadRaceData() {
    QVector<QString> allGuilds = loadGuildData();
    QVector<RaceStats> data = {
        // RaceName, MaxAge, Exp, {Start, Min, Max}, {Int}, {Wis}, {Con}, {Cha}, {Dex}, G, N, E, {}
        {"Human",   100, 3,  {4, 4, 17}, {4, 4, 18}, {4, 4, 18}, {6, 6, 17}, {5, 5, 18}, {6, 6, 18}, AS_Allowed, AS_Allowed, AS_Allowed, {}},
        {"Elf",     400, 7,  {3, 3, 15}, {7, 7, 20}, {7, 7, 20}, {3, 3, 16}, {3, 3, 18}, {3, 3, 18}, AS_Allowed, AS_Allowed, AS_Allowed, {}},
        {"Giant",   225, 7,  {12, 12, 25},{4, 4, 17}, {3, 3, 17}, {9, 9, 19}, {2, 2, 16}, {3, 3, 18}, AS_NotAllowed, AS_Allowed, AS_NotAllowed, {}},
        {"Gnome",   300, 7,  {4, 4, 17}, {7, 7, 19}, {5, 5, 19}, {3, 3, 17}, {9, 9, 22}, {6, 6, 18}, AS_Allowed, AS_Allowed, AS_Allowed, {}},
        {"Dwarf",   275, 5,  {3, 3, 18}, {3, 3, 18}, {7, 7, 19}, {3, 3, 19}, {3, 3, 17}, {5, 5, 18}, AS_NotAllowed, AS_Allowed, AS_NotAllowed, {}},
        {"Ogre",    265, 6,  {9, 9, 20}, {3, 3, 16}, {3, 3, 16}, {9, 9, 21}, {3, 3, 18}, {5, 5, 17}, AS_Allowed, AS_Allowed, AS_Allowed, {}},
        {"Morloch", 175, 4,  {6, 6, 20}, {3, 3, 17}, {5, 5, 19}, {3, 3, 15}, {2, 2, 14}, {5, 5, 20}, AS_Allowed, AS_NotAllowed, AS_Allowed, {}},
        {"Osiri",   325, 8,  {5, 5, 17}, {3, 3, 18}, {3, 3, 17}, {7, 7, 19}, {3, 3, 18}, {10, 10, 22}, AS_NotAllowed, AS_Allowed, AS_NotAllowed, {}},
        {"Troll",   285, 9,  {6, 6, 20}, {3, 3, 18}, {3, 3, 18}, {6, 6, 19}, {3, 3, 17}, {6, 6, 20}, AS_Allowed, AS_Allowed, AS_Allowed, {}}
    };
    // Initialize Guild Eligibility (Placeholder Logic)
    for (RaceStats& race : data) {
        // Default: All races are allowed in all guilds
        for (const QString& guild : allGuilds) {
            race.guildEligibility[guild] = AS_Allowed;
        }
        if (race.raceName == "Giant") {
            race.guildEligibility["Mage"] = AS_NotAllowed;
            race.guildEligibility["Wizard"] = AS_NotAllowed;
            race.guildEligibility["Thief"] = AS_NotAllowed;
        } else if (race.raceName == "Dwarf") {
            race.guildEligibility["Mage"] = AS_NotAllowed;
            race.guildEligibility["Sorcerer"] = AS_NotAllowed;
            race.guildEligibility["Wizard"] = AS_NotAllowed;
        } else if (race.raceName == "Elf") {
            race.guildEligibility["Villain"] = AS_NotAllowed;
        }
    }
    return data;
}
