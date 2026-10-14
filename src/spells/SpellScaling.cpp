#include "SpellScaling.h"

double SpellScaling::powerMultiplier(int casterLevel, int spellBaseLevel) {
    if (spellBaseLevel <= 0) return 1.0;
    double diff = casterLevel - spellBaseLevel;
    if (diff <= 0) return 1.0;
    return 1.0 + diff * 0.1;  // +10% per level above base
}

int SpellScaling::scaleDamage(int baseDamage, int casterLevel, int spellBaseLevel) {
    return qRound(baseDamage * powerMultiplier(casterLevel, spellBaseLevel));
}

int SpellScaling::scaleHealing(int baseHealing, int casterLevel, int spellBaseLevel) {
    return qRound(baseHealing * powerMultiplier(casterLevel, spellBaseLevel));
}

int SpellScaling::scaleDuration(int baseTurns, int casterLevel, int spellBaseLevel) {
    if (spellBaseLevel <= 0) return baseTurns;
    int diff = casterLevel - spellBaseLevel;
    if (diff <= 0) return baseTurns;
    return baseTurns + (diff / 3);  // +1 turn per 3 levels above base
}
