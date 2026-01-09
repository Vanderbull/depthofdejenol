#ifndef RACEDATA_H
#define RACEDATA_H

#include <QVector>
#include <QString>
#include <QMap>
#include <QStringList>
#include "GameStateManager.h"

struct RaceStat 
{
    int start;
    int min;
    int max;
};

enum AlignmentStatus 
{
    AS_Allowed,
    AS_NotAllowed
};

struct RaceStats 
{
    QString raceName;
    int maxAge;
    int experience;
    RaceStat strength;
    RaceStat intelligence;
    RaceStat wisdom;
    RaceStat constitution;
    RaceStat charisma;
    RaceStat dexterity;
    AlignmentStatus good;
    AlignmentStatus neutral;
    AlignmentStatus evil;
    QMap<QString, AlignmentStatus> guildEligibility; 
};

QVector<QString> loadGuildData();
QVector<RaceStats> loadRaceData();

#endif // RACEDATA_H
