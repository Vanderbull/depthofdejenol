#ifndef SPELLSCALING_H
#define SPELLSCALING_H

#include "src/character/character.h"
#include <QString>

// Spell level-based power scaling.
class SpellScaling {
public:
    // Spell power multiplier based on caster guild level.
    // 1.0 at base level, +10% per level above base.
    static double powerMultiplier(int casterLevel, int spellBaseLevel);

    // Effective damage after scaling.
    static int scaleDamage(int baseDamage, int casterLevel, int spellBaseLevel);

    // Effective healing after scaling.
    static int scaleHealing(int baseHealing, int casterLevel, int spellBaseLevel);

    // Effective duration (turns) after scaling.
    static int scaleDuration(int baseTurns, int casterLevel, int spellBaseLevel);
};

#endif // SPELLSCALING_H
