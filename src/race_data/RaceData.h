#ifndef RACEDATA_H
#define RACEDATA_H

#include "include/GameConstants.h"
#include <QVector>
#include <QString>
#include <QMap>
#include <QStringList>

// Use types from GameConstants namespace
using RaceStat = GameConstants::RaceStat;
using RaceStats = GameConstants::RaceStats;
using AlignmentStatus = GameConstants::AlignmentStatus;
constexpr auto AS_Allowed = GameConstants::AS_Allowed;
constexpr auto AS_NotAllowed = GameConstants::AS_NotAllowed;
QVector<QString> loadGuildData();
QVector<RaceStats> loadRaceData();

#endif // RACEDATA_H
