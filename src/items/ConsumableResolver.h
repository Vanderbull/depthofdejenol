#ifndef CONSUMABLERESOLVER_H
#define CONSUMABLERESOLVER_H

#include "ItemDatabase.h"
#include <QString>
#include <QVariantMap>

// Resolves what a potion, scroll or tome actually does.
//
// The item data already carries the answer. MDATA3 items spell their effect
// through `spellID`, which indexes the MDATA2 spell table (105 rows — verified:
// every row's ID equals its row index, so the index is authoritative). Items
// that declare no spell carry their effect in their own stat modifiers.
//
// The old implementation instead matched on the item's *name*
// (`nameLower.contains("healing")`), which silently broke for any item whose
// name did not contain the right keyword — and it guessed the effect from the
// name, so "Scroll of Negrun" (a charm) could never have worked.
class ConsumableResolver {
public:
    // The effect is read from the spell's school (MDATA2 "Class"), never from
    // its damage range. That distinction matters: the charm school carries a
    // damage range (it is the charm's power), so treating "has damage" as
    // "deals damage" would turn every charm scroll into a fireball.
    enum class Effect {
        Damage,        // schools 0-6: fire, cold, electrical, mind, damage, element, death
        Charm,         // school 7: charms monsters
        Bind,          // school 8: restrains a creature
        Banish,        // school 11: banishes a creature
        Dispel,        // school 12: dispels a creature or effect
        Heal,          // school 9 with a damage range
        CurePoison,    // school 9 without one
        Resist,        // school 13: temporary resistance
        Protection,    // school 17: ward
        Levitate,      // school 10, movement
        Teleport,      // school 10, specialEffect 7
        Invisibility,  // school 14
        SeeInvisible,  // school 14
        Reveal,        // schools 15/16: detect, locate, open
        StatBoost,     // no spell: the item's own stat modifiers
        Unknown        // genuinely unimplemented — say so, do not fake it
    };

    struct Resolved {
        Effect effect = Effect::Unknown;
        QString spellName;   // MDATA2 name, or the item name when it has no spell
        int amountMin = 0;   // guaranteed minimum (heal/charm power)
        int amountMax = 0;   // maximum (heal/charm power)
        int school = 0;      // MDATA2 "Class"
        bool isValid() const { return effect != Effect::Unknown; }
    };

    // Pure: no I/O and no singleton, so the mapping is directly testable.
    // `spellRow` is the MDATA2 row for the item's spellID; pass an empty map
    // when the item declares no spell (spellID 0).
    static Resolved resolve(const ItemDef& item, const QVariantMap& spellRow);

    static QString effectName(Effect effect);
};

#endif // CONSUMABLERESOLVER_H
