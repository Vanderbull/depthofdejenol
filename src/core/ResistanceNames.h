#ifndef RESISTANCENAMES_H
#define RESISTANCENAMES_H

#include <QString>
#include <QStringList>

// The 11 damage-resistance types, and their canonical index order.
//
// Both sides of the game need this mapping: a "Resist Fire" potion has to find
// the right slot in Character::resistances, and a fire spell has to find the
// right slot in a monster's bestiary row. It lives here rather than in either
// layer so neither has to depend on the other.
namespace ResistanceNames {

// Index order: 0 = Fire ... 10 = Special. Character::resistances[12] reserves
// index 11, which is unused.
inline QStringList all()
{
    return QStringList()
        << QStringLiteral("Fire")
        << QStringLiteral("Cold")
        << QStringLiteral("Electric")
        << QStringLiteral("Mind")
        << QStringLiteral("Poison")
        << QStringLiteral("Disease")
        << QStringLiteral("Magic")
        << QStringLiteral("Physical")
        << QStringLiteral("Weapon")
        << QStringLiteral("Spell")
        << QStringLiteral("Special");
}

// Index (0..10) for a resistance name, or -1 when unknown. Case-insensitive.
inline int indexOf(const QString& name)
{
    const QString probe = name.trimmed().toLower();
    const QStringList names = all();
    for (int i = 0; i < names.size(); ++i) {
        if (names.at(i).toLower() == probe) return i;
    }
    return -1;
}

// Resistance name for an index, or an empty string when out of range.
inline QString nameAt(int index)
{
    const QStringList names = all();
    if (index < 0 || index >= names.size()) return QString();
    return names.at(index);
}

// The resistance a "Resist <Type>" or "<Type> Resistance" spell name grants, or
// an empty string when the name does not name one (e.g. "Protection", "Resist
// All").
//
// The spell data uses both word orders — "Resist Fire" and "Mental Resistance"
// are both real rows in MDATA2 — and two of the names differ from the
// bestiary's: the manual says "Mental" and "Electrical" where data/bestiary.json
// says "Mind" and "Electric". Both spellings must land on the canonical slot, or
// the potion silently grants nothing.
inline QString fromSpellName(const QString& spellName)
{
    const QString trimmed = spellName.trimmed();
    if (trimmed.isEmpty()) return QString();

    QString type;
    if (trimmed.startsWith(QStringLiteral("Resist "), Qt::CaseInsensitive)) {
        type = trimmed.mid(7).trimmed();
    } else if (trimmed.endsWith(QStringLiteral(" Resistance"), Qt::CaseInsensitive)) {
        type = trimmed.left(trimmed.length() - 11).trimmed();
    } else {
        return QString();
    }

    // Manual/bestiary naming aliases — same resistance, two spellings.
    if (type.compare(QStringLiteral("Electrical"), Qt::CaseInsensitive) == 0) {
        type = QStringLiteral("Electric");
    } else if (type.compare(QStringLiteral("Mental"), Qt::CaseInsensitive) == 0) {
        type = QStringLiteral("Mind");
    }

    const int idx = indexOf(type);
    return idx >= 0 ? nameAt(idx) : QString();
}

} // namespace ResistanceNames

#endif // RESISTANCENAMES_H
