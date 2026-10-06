#ifndef SPELLMECHANICS_H
#define SPELLMECHANICS_H

#include "SpellBook.h"
#include <QString>
#include <QStringList>
#include <QList>

// Spell differentiation: fire AoE, cold slow, lightning chain, mind crowd control.
class SpellMechanics {
public:
    enum class School { Fire, Cold, Lightning, Mind, Magical, Damage };

    // School for a spell category.
    static School schoolFor(const QString& category);

    // Display name for a school.
    static QString schoolName(School school);

    // --- Fire: area of effect ---
    // Number of targets a fire spell hits.
    static int fireTargets(const SpellDef& spell);

    // Fire spells deal bonus damage per target.
    static int fireBonusPerTarget(const SpellDef& spell);

    // --- Cold: slow ---
    // Turns a cold spell slows the target.
    static int coldSlowTurns(const SpellDef& spell);

    // Cold spells reduce target speed by this percentage.
    static int coldSlowPercent(const SpellDef& spell);

    // --- Lightning: chain ---
    // Number of additional targets a lightning spell chains to.
    static int lightningChainTargets(const SpellDef& spell);

    // Chain damage falls off by this percentage per jump.
    static int lightningChainFalloff(const SpellDef& spell);

    // --- Mind: crowd control ---
    // Turns a mind spell stuns the target.
    static int mindStunTurns(const SpellDef& spell);

    // Mind spells have a chance to confuse the target.
    static int mindConfuseChance(const SpellDef& spell);

    // --- Utility ---
    // Is this spell an area-of-effect spell?
    static bool isAoe(const SpellDef& spell);

    // Is this a crowd-control spell?
    static bool isCrowdControl(const SpellDef& spell);

    // A short description of the spell's special mechanic.
    static QString mechanicDescription(const SpellDef& spell);

    // All spells in a school.
    static QList<SpellDef> spellsInSchool(const QList<SpellDef>& spells, School school);
};

#endif // SPELLMECHANICS_H
