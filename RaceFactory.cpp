#include "RaceFactory.h"

QVector<GameConstants::RaceStats> RaceFactory::createStandardRaces() {
    return {
        // Human
        {"Human", 100, 3, {4,4,17}, {4,4,18}, {4,4,18}, {6,6,17}, {5,5,18}, {6,6,18}, 
         GameConstants::AS_Allowed, GameConstants::AS_Allowed, GameConstants::AS_Allowed, {}},
        // Elf
        {"Elf", 400, 7, {3,3,15}, {7,7,20}, {7,7,20}, {3,3,16}, {3,3,18}, {3,3,18}, 
         GameConstants::AS_Allowed, GameConstants::AS_Allowed, GameConstants::AS_NotAllowed, {}}
    };
}
