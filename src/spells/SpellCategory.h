#ifndef SPELLCATEGORY_H
#define SPELLCATEGORY_H

#include <QString>
#include <QStringList>
#include <QMap>

// Spell category definitions from MORDOR manual.
// Each category has: display name, targeting type, resistance type,
// and whether it's a combat spell.
class SpellCategory {
public:
    enum class Targeting { Single, Group, Area, Self };
    enum class Resistance { None, Magic, Charm, Mental, Elemental, Disease };

    struct Def {
        QString name;
        Targeting targeting;
        Resistance resistance;
        bool isCombat;
    };

    // All categories from MORDOR manual.
    static QList<Def> all();

    // Lookup by name (nullptr if not found).
    static const Def* find(const QString& name);

    // Display name for targeting type.
    static QString targetingName(Targeting t);

    // Display name for resistance type.
    static QString resistanceName(Resistance r);

    // All category names.
    static QStringList names();
};

#endif // SPELLCATEGORY_H
