#include "SpellMechanics.h"

SpellMechanics::School SpellMechanics::schoolFor(const QString& category) {
    if (category == "Fire") return School::Fire;
    if (category == "Cold") return School::Cold;
    if (category == "Electrical") return School::Lightning;
    if (category == "Mind") return School::Mind;
    return School::Magical;
}

QString SpellMechanics::schoolName(School school) {
    switch (school) {
    case School::Fire: return "Fire";
    case School::Cold: return "Cold";
    case School::Lightning: return "Lightning";
    case School::Mind: return "Mind";
    case School::Magical: return "Magical";
    case School::Damage: return "Damage";
    }
    return "Unknown";
}

int SpellMechanics::fireTargets(const SpellDef& spell) {
    // Fire spells hit multiple targets: base 1 + 1 per 3 base levels.
    return 1 + spell.baseLevel / 3;
}

int SpellMechanics::fireBonusPerTarget(const SpellDef& spell) {
    // +10% damage per additional target beyond the first.
    return (fireTargets(spell) - 1) * 10;
}

int SpellMechanics::coldSlowTurns(const SpellDef& spell) {
    // Cold spells slow for 1–3 turns depending on level.
    if (spell.baseLevel <= 2) return 1;
    if (spell.baseLevel <= 5) return 2;
    return 3;
}

int SpellMechanics::coldSlowPercent(const SpellDef& spell) {
    // Slow reduces target speed by 20–40%.
    if (spell.baseLevel <= 2) return 20;
    if (spell.baseLevel <= 5) return 30;
    return 40;
}

int SpellMechanics::lightningChainTargets(const SpellDef& spell) {
    // Chain Lightning arcs to up to 5 additional targets.
    if (spell.name.contains("Chain", Qt::CaseInsensitive)) return 5;
    return 0;
}

int SpellMechanics::lightningChainFalloff(const SpellDef& spell) {
    Q_UNUSED(spell);
    // Each chain jump deals 20% less damage.
    return 20;
}

int SpellMechanics::mindStunTurns(const SpellDef& spell) {
    // Mind spells stun for 1–2 turns.
    if (spell.baseLevel <= 3) return 1;
    return 2;
}

int SpellMechanics::mindConfuseChance(const SpellDef& spell) {
    // 25% base chance to confuse, +5% per level above 3.
    if (spell.baseLevel <= 3) return 25;
    return 25 + (spell.baseLevel - 3) * 5;
}

bool SpellMechanics::isAoe(const SpellDef& spell) {
    School s = schoolFor(spell.category);
    if (s == School::Fire) return fireTargets(spell) > 1;
    if (s == School::Cold) return spell.name.contains("Blizzard", Qt::CaseInsensitive) ||
                                 spell.name.contains("Iceball", Qt::CaseInsensitive);
    if (s == School::Lightning) return lightningChainTargets(spell) > 0;
    return false;
}

bool SpellMechanics::isCrowdControl(const SpellDef& spell) {
    School s = schoolFor(spell.category);
    if (s == School::Cold) return true;  // slow
    if (s == School::Mind) return true;  // stun/confuse
    return false;
}

QString SpellMechanics::mechanicDescription(const SpellDef& spell) {
    School s = schoolFor(spell.category);
    switch (s) {
    case School::Fire:
        return QString("Fire: hits %1 targets, +%2% damage per extra target.")
            .arg(fireTargets(spell)).arg(fireBonusPerTarget(spell));
    case School::Cold:
        return QString("Cold: slows target by %1% for %2 turns.")
            .arg(coldSlowPercent(spell)).arg(coldSlowTurns(spell));
    case School::Lightning:
        if (lightningChainTargets(spell) > 0) {
            return QString("Lightning: chains to %1 additional targets, -%2% damage per jump.")
                .arg(lightningChainTargets(spell)).arg(lightningChainFalloff(spell));
        }
        return "Lightning: single target.";
    case School::Mind:
        return QString("Mind: stuns for %1 turns, %2% confuse chance.")
            .arg(mindStunTurns(spell)).arg(mindConfuseChance(spell));
    default:
        return QString("No special mechanic.");
    }
}

QList<SpellDef> SpellMechanics::spellsInSchool(const QList<SpellDef>& spells, School school) {
    QList<SpellDef> result;
    for (const SpellDef& s : spells) {
        if (schoolFor(s.category) == school) result.append(s);
    }
    return result;
}
