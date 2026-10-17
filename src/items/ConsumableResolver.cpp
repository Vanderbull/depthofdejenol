#include "ConsumableResolver.h"

// MDATA2 "Class" column — the spell's school. These are the only signals the
// table gives about what a spell does, so every one of the 18 schools is listed
// here explicitly rather than guessed from the damage range.
//
// Verified against data/spell_effects.csv: classes 0-6 are the attack schools
// (Blue Flame, Ice Spray, Shock, Sleep, Cause Wounds, Poison, Nerve Blank) and
// every one of their spells carries a damage range; classes 8 and 13-17 carry
// none. Class 7 (Charm Insect, Dominate Demon, ...) *does* carry a range, but
// it is the charm's power — the manual describes these as "will cause 4
// Monsters in 1 Group to be Charmed", not as attacks.
namespace {
constexpr int kClassFire         = 0;
constexpr int kClassCold         = 1;
constexpr int kClassElectrical   = 2;
constexpr int kClassMind         = 3;
constexpr int kClassDamage       = 4;
constexpr int kClassElement      = 5;
constexpr int kClassDeath        = 6;
constexpr int kClassCharm        = 7;
constexpr int kClassBind         = 8;
constexpr int kClassHeal         = 9;
constexpr int kClassMovement     = 10;
constexpr int kClassBanish       = 11;
constexpr int kClassDispel       = 12;
constexpr int kClassResistant    = 13;
constexpr int kClassVisual       = 14;
constexpr int kClassSpecial      = 15;
constexpr int kClassLocation     = 16;
constexpr int kClassProtection   = 17;

// MDATA2 "specialEffect" code 7 is the teleport dialog (per
// tools/spellbook_editor/spelleffect.txt: "7 :: Dialog: Teleport").
constexpr int kSpecialTeleport   = 7;
} // namespace

QString ConsumableResolver::effectName(Effect effect) {
    switch (effect) {
    case Effect::Damage:       return QStringLiteral("offensive spell");
    case Effect::Charm:        return QStringLiteral("charm");
    case Effect::Bind:         return QStringLiteral("binding");
    case Effect::Banish:       return QStringLiteral("banishment");
    case Effect::Dispel:       return QStringLiteral("dispelling");
    case Effect::Heal:         return QStringLiteral("healing");
    case Effect::CurePoison:   return QStringLiteral("cure poison");
    case Effect::Resist:       return QStringLiteral("resistance");
    case Effect::Protection:   return QStringLiteral("protection");
    case Effect::Levitate:     return QStringLiteral("levitation");
    case Effect::Teleport:     return QStringLiteral("teleport");
    case Effect::Invisibility: return QStringLiteral("invisibility");
    case Effect::SeeInvisible: return QStringLiteral("see invisible");
    case Effect::Reveal:       return QStringLiteral("reveal");
    case Effect::StatBoost:    return QStringLiteral("stat boost");
    case Effect::Unknown:      break;
    }
    return QStringLiteral("unknown");
}

ConsumableResolver::Resolved
ConsumableResolver::resolve(const ItemDef& item, const QVariantMap& spellRow) {
    Resolved r;
    r.spellName = item.name;

    // No spell row: the item's own stat modifiers are the effect. This is what
    // the Potion/Tome of Might, Agility, Fitness, ... rows declare.
    if (spellRow.isEmpty()) {
        const int mods[6] = { item.strMod, item.intMod, item.wisMod,
                              item.conMod, item.chaMod, item.dexMod };
        for (int m : mods) {
            if (m != 0) { r.effect = Effect::StatBoost; return r; }
        }
        r.effect = Effect::Unknown;
        return r;
    }

    r.spellName  = spellRow.value(QStringLiteral("name")).toString();
    r.school     = spellRow.value(QStringLiteral("Class")).toInt();
    r.amountMin  = spellRow.value(QStringLiteral("damage1")).toInt();
    r.amountMax  = spellRow.value(QStringLiteral("damage2")).toInt();
    const int special = spellRow.value(QStringLiteral("specialEffect")).toInt();

    switch (r.school) {
    // The attack schools. Every spell in them carries a damage range.
    case kClassFire:
    case kClassCold:
    case kClassElectrical:
    case kClassMind:
    case kClassDamage:
    case kClassElement:
    case kClassDeath:
        r.effect = Effect::Damage;
        return r;

    // Charm: the range is the charm's power, not HP damage. The manual
    // describes these as turning monsters friendly, not wounding them.
    case kClassCharm:
        r.effect = Effect::Charm;
        return r;

    case kClassBind:
        r.effect = Effect::Bind;
        return r;

    case kClassBanish:
        r.effect = Effect::Banish;
        return r;

    case kClassDispel:
        r.effect = Effect::Dispel;
        return r;

    case kClassHeal:
        // The Heal school mixes restores and cures; the damage range tells them
        // apart. "Minor Heal" (0-5) heals, "Cure Poison" (0-0) does not.
        //
        // A 0 lower bound means "no fixed floor", not "may heal nothing": every
        // healing spell in the table is 0-N, so without this a potion heals 0
        // HP whenever the roll lands on the floor. Raise the floor to 1 so
        // amountMin is a *guaranteed* minimum and no charge is wasted.
        //
        // The game has no disease status, so "Cure Disease" (Potion of Health)
        // falls through to the same cure as "Cure Poison" (Potion of Curing) —
        // clearing the poison flag is the only cure that can actually land.
        if (r.amountMax > 0) {
            r.amountMin = qMax(r.amountMin, 1);
            r.effect = Effect::Heal;
        } else {
            r.effect = Effect::CurePoison;
        }
        return r;

    case kClassResistant:
        r.effect = Effect::Resist;
        return r;

    case kClassProtection:
        r.effect = Effect::Protection;
        return r;

    case kClassMovement:
        // "Teleport" carries effect code 7; the rest of the school (Feather
        // Essence, Mass Levitate) is levitation.
        r.effect = (special == kSpecialTeleport) ? Effect::Teleport : Effect::Levitate;
        return r;

    case kClassVisual: {
        // Sight Veil = invisibility, See Invisible = true sight.
        const QString n = r.spellName.toLower();
        r.effect = n.contains(QStringLiteral("see")) ? Effect::SeeInvisible
                                                      : Effect::Invisibility;
        return r;
    }

    // "Charm of Opening", "Fate", "Depth Perception", "Find Direction" —
    // utility spells that reveal or manipulate the world rather than a target.
    case kClassSpecial:
    case kClassLocation:
        r.effect = Effect::Reveal;
        return r;

    default:
        break;
    }

    // An unrecognised school is reported as unknown rather than guessed at.
    r.effect = Effect::Unknown;
    return r;
}
