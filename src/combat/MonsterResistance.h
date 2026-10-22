#ifndef MONSTERRESISTANCE_H
#define MONSTERRESISTANCE_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QPair>

// Monster resistances: what a creature shrugs off, and by how much.
//
// The values live in the curated data/bestiary.json as
// [{"name": "Fire", "percent": 25}, ...] — 308 of the 401 rows carry them.
// They were previously only *displayed* in the Bestiary; combat ignored them
// entirely, so a 100% Mind-resistant Aboleth took full mind damage.
//
// This class is deliberately UI-free so the combat layer can consult it without
// depending on a dialog. The file is loaded once per process.
class MonsterResistance {
public:
    // The 11 resistance names, in the index order used by
    // Character::resistances[12] (index 11 is unused).
    static QStringList names();

    // Index (0..10) for a resistance name, or -1 when the name is unknown.
    static int indexForName(const QString& name);

    // Resistance name for an index, or an empty string when out of range.
    static QString nameForIndex(int index);

    // Percentage (0-100) a monster resists a given resistance name.
    // 0 when the monster is unknown or carries no such resistance.
    static int percentFor(const QString& monsterName, const QString& resistanceName);

    // Every non-zero resistance a monster carries, as (name, percent) pairs.
    static QList<QPair<QString, int> > resistancesFor(const QString& monsterName);

    // Point the loader at a specific file instead of the search list.
    // Returns false when the path cannot be opened. Test hook.
    static bool setSourcePath(const QString& path);

    // Drop the cached table. The next call reloads it; used by tests that
    // point the loader at a different file.
    static void reset();

    static bool isLoaded();
};

#endif // MONSTERRESISTANCE_H
