#ifndef SPELLRESISTANCE_H
#define SPELLRESISTANCE_H

#include "SpellCategory.h"
#include <QString>

// Spell resistance logic — how monsters resist spells.
class SpellResistance {
public:
    // Resistance roll: returns true if the spell is resisted.
    // chance = base 20% + 5% per level difference (caster vs monster).
    static bool isResisted(const QString& category, int casterLevel,
                           int monsterLevel);

    // Resistance percentage (0-100).
    static int resistanceChance(const QString& category, int casterLevel,
                                 int monsterLevel);

    // True if this spell category is subject to magic resistance.
    static bool hasResistance(const QString& category);
};

#endif // SPELLRESISTANCE_H
