#ifndef AGINGRULES_H
#define AGINGRULES_H

#include "character.h"
#include <QString>
#include <QStringList>
#include <QMap>

// Aging rules: stat decay past a race threshold, and death at the race's max age.
// Pure logic — the caller applies the returned deltas to a Character.
class AgingRules {
public:
    // Max age for a race. Falls back to Human (100) for an unknown race.
    static int maxAgeForRace(const QString& race);

    // Age at which stat decay begins (about 70% of max age).
    static int decayThresholdForRace(const QString& race);

    // Apply one year of aging to a character in place.
    // Returns a list of human-readable messages describing what happened.
    //  - Past the decay threshold: a small chance to lose STR/CON/DEX.
    //  - At or past max age: the character dies (isAlive = false, Dead status).
    static QStringList applyYearOfAging(Character& c);

    // True if the character has reached their race's max age.
    static bool isPastMaxAge(const Character& c);

    // True if the character is old enough for stat decay.
    static bool isDecaying(const Character& c);
};

#endif // AGINGRULES_H
