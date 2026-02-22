#ifndef RACEFACTORY_H
#define RACEFACTORY_H

#include "include/GameConstants.h"
#include <QVector>
#include <QString>

class RaceFactory {
public:
    // Returns the full list of race definitions for the game
    static QVector<GameConstants::RaceStats> createStandardRaces();
};

#endif
