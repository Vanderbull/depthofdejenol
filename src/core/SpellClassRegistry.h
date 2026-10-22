#ifndef SPELLCLASSREGISTRY_H
#define SPELLCLASSREGISTRY_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

// A single character class parsed from data/SpellClasses.json.
//
// The manual lists twelve classes (Nomad, Warrior, Paladin, Ninja, Villain,
// Seeker, Thief, Scavenger, Mage, Sorcerer, Wizard, Healer) and each one has its
// own spell list. In the game a class *is* the starting guild, so `id` matches
// GameConstants::GUILD_NAMES and `guild` names the spells.json guild that
// teaches the class its spells. Classes that teach no spells (Nomad, Ninja,
// Scavenger) have an empty spellIds list — that is the truthful state of the
// data, not a placeholder.
//
// spellIds hold spell *names*, which is how spells.json is keyed; resolve them
// through SpellBook::byName().
struct SpellClassDef {
    QString id;              // e.g. "Paladin"
    QString name;            // display name
    QString title;           // flavour title
    QString description;     // one-line description
    QString guild;           // spells.json guild name, "" when the class teaches none
    QStringList spellIds;    // spell names granted by this class
};

// Loads SpellClasses.json and answers "which class is this, and what does it cast?".
class SpellClassRegistry {
public:
    static SpellClassRegistry& instance();

    // Load the class table. Returns true on success.
    bool load(const QString& filePath);

    bool isLoaded() const { return m_loaded; }
    int classCount() const { return m_classes.size(); }

    const QList<SpellClassDef>& all() const { return m_classes; }

    // Look up a class by id ("Paladin"). nullptr if unknown.
    const SpellClassDef* byId(const QString& id) const;

    // Best-effort lookup that also accepts a spells.json guild name
    // ("Paladins Guild", "Paladins", "paladin"). Used when a character's
    // guild is the only thing known about them.
    const SpellClassDef* byGuildName(const QString& guildName) const;

    // Class ids in file order.
    QStringList classIds() const;

    // Normalise any of the above spellings to a class id ("" if unknown).
    QString normalizeToClassId(const QString& guildOrClass) const;

private:
    SpellClassRegistry() = default;

    bool m_loaded = false;
    QList<SpellClassDef> m_classes;
};

#endif // SPELLCLASSREGISTRY_H
