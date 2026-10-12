#pragma once

#include <QString> // Use QString for string handling
#include "MTypes.h"
// The original <vector> and <string> includes are removed as they are no longer
// explicitly needed for the function signatures below.

extern GameData loadGameInfo(const QString& filename = "MDATA1.MDR"); //Not Implemented
extern Spells loadSpells(const QString& filename = "MDATA2.MDR");
extern Items loadItems(const QString& filename = "MDATA3.MDR");
extern Characters loadCharacters(const QString& filename = "MDATA4.MDR"); //Not Implemented
extern Monsters loadMonsters(const QString& filename = "MDATA5.MDR");
//General Store
//Guildmasters
extern Automap loadAutomap(const QString& filename = "MDATA8.MDR");
extern GuildLogs loadGuildLogs(const QString& filename = "MDATA9.MDR");
//Dungeon state
extern Dungeon loadDungeon(const QString& filename = "MDATA11.MDR");
//Parties
extern Library loadLibrary(const QString& filename = "MDATA13.MDR");
extern HallRecords loadHallRecords(const QString& filename = "MDATA14.MDR");
//Confinement
