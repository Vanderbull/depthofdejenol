#ifndef RACEDATA_H
#define RACEDATA_H

#include <QVector>
#include <QString>
#include <QMap>
#include <QStringList>

// Structure to hold min/max/start values for a stat (e.g., Str, Int)
struct RaceStat {
    int start; // The default value when the race is selected
    int min;
    int max;
};

// Simplified Enum based on the table values ('X' and '-')
enum AlignmentStatus {
    AS_Allowed,   // Represents 'X'
    AS_NotAllowed // Represents '-'
};

// Structure to hold all stats for a single race
struct RaceStats {
    QString raceName;
    int maxAge;
    int experience;
    RaceStat strength;
    RaceStat intelligence;
    RaceStat wisdom;
    RaceStat constitution;
    RaceStat charisma;
    RaceStat dexterity;
    AlignmentStatus good; // G
    AlignmentStatus neutral; // N
    AlignmentStatus evil; // E
    // Guild Eligibility Map: GuildName -> Allowed/NotAllowed
    QMap<QString, AlignmentStatus> guildEligibility; 
};

// Functions to load data (defined in racedata.cpp)
QVector<QString> loadGuildData();
QVector<RaceStats> loadRaceData();

#endif // RACEDATA_H
