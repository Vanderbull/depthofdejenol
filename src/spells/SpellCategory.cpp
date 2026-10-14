#include "SpellCategory.h"

QList<SpellCategory::Def> SpellCategory::all() {
    return {
        // Combat damage schools
        {"Fire", Targeting::Area, Resistance::Elemental, true},
        {"Cold", Targeting::Area, Resistance::Elemental, true},
        {"Electrical", Targeting::Single, Resistance::Elemental, true},
        {"Element", Targeting::Area, Resistance::Elemental, true},
        {"Damage", Targeting::Single, Resistance::None, true},
        {"Kill", Targeting::Single, Resistance::Magic, true},
        {"Banish", Targeting::Single, Resistance::Magic, true},

        // Crowd control
        {"Charm", Targeting::Group, Resistance::Charm, true},
        {"Bind", Targeting::Single, Resistance::Charm, false},
        {"Mind", Targeting::Single, Resistance::Mental, true},

        // Healing
        {"Heal", Targeting::Self, Resistance::None, false},
        {"Dispel", Targeting::Single, Resistance::Magic, false},

        // Defensive
        {"Protection", Targeting::Self, Resistance::None, false},
        {"Resistant", Targeting::Self, Resistance::Magic, false},
        {"Visual", Targeting::Self, Resistance::None, false},
        {"Magical", Targeting::Self, Resistance::Magic, false},

        // Utility
        {"Movement", Targeting::Self, Resistance::None, false},
        {"Location", Targeting::Self, Resistance::None, false},
    };
}

const SpellCategory::Def* SpellCategory::find(const QString& name) {
    for (const Def& d : all()) {
        if (d.name == name) return &d;
    }
    return nullptr;
}

QString SpellCategory::targetingName(Targeting t) {
    switch (t) {
    case Targeting::Single: return "Single target";
    case Targeting::Group: return "Group";
    case Targeting::Area: return "Area";
    case Targeting::Self: return "Self";
    }
    return "Unknown";
}

QString SpellCategory::resistanceName(Resistance r) {
    switch (r) {
    case Resistance::None: return "None";
    case Resistance::Magic: return "Magic";
    case Resistance::Charm: return "Charm";
    case Resistance::Mental: return "Mental";
    case Resistance::Elemental: return "Elemental";
    case Resistance::Disease: return "Disease";
    }
    return "Unknown";
}

QStringList SpellCategory::names() {
    QStringList result;
    for (const Def& d : all()) result.append(d.name);
    return result;
}
