#ifndef SPELLTARGETING_H
#define SPELLTARGETING_H

#include "SpellCategory.h"
#include <QString>

// Spell targeting logic — how many monsters a spell can hit.
class SpellTargeting {
public:
    // Number of targets for a spell based on its category and caster level.
    // Single: always 1.
    // Group: 4 (as per MORDOR manual: "will cause 4 Monsters in 1 Group").
    // Area: depends on caster level — 1 + (casterLevel / 3).
    // Self: 0 (not applicable, targets the caster).
    static int targetCount(const QString& category, int casterLevel);

    // Max targets cap (MORDOR manual says "4 Monsters in 1 Group" for most).
    static int maxTargets();

    // True if the spell hits multiple targets.
    static bool isMultiTarget(const QString& category);
};

#endif // SPELLTARGETING_H
