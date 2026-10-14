#include "SpellResistance.h"
#include <QRandomGenerator>

bool SpellResistance::isResisted(const QString& category, int casterLevel,
                                  int monsterLevel) {
    int chance = resistanceChance(category, casterLevel, monsterLevel);
    if (chance <= 0) return false;
    if (chance >= 100) return true;
    return (QRandomGenerator::global()->bounded(100) < chance);
}

int SpellResistance::resistanceChance(const QString& category, int casterLevel,
                                       int monsterLevel) {
    const SpellCategory::Def* def = SpellCategory::find(category);
    if (!def) return 0;

    if (def->resistance == SpellCategory::Resistance::None) return 0;

    // Base 20% resistance chance.
    int chance = 20;
    // +5% per level the monster is above the caster.
    if (monsterLevel > casterLevel) {
        chance += (monsterLevel - casterLevel) * 5;
    }
    return qBound(0, chance, 100);
}

bool SpellResistance::hasResistance(const QString& category) {
    const SpellCategory::Def* def = SpellCategory::find(category);
    if (!def) return false;
    return def->resistance != SpellCategory::Resistance::None;
}
