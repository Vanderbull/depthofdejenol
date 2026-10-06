#ifndef SPELLBOOK_H
#define SPELLBOOK_H

#include "character.h"
#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

// A single spell definition parsed from data/spells.json.
struct SpellDef {
    QString name;
    QString category;
    int mana = 0;
    int baseLevel = 1;
    QStringList guilds;             // guilds that teach it
    QMap<QString, int> requiredStats;
    bool isCombatSpell = false;
    QString damage;                 // "min-max" for damage spells
    QString description;
};

// Loads spells.json and answers "which spells does this character know?".
// Spell learning is driven by guild level and stat requirements.
class SpellBook {
public:
    static SpellBook& instance();

    // Load spells from JSON. Returns true on success.
    bool load(const QString& filePath);

    bool isLoaded() const { return m_loaded; }
    int spellCount() const { return m_spells.size(); }

    const QList<SpellDef>& all() const { return m_spells; }

    // Find a spell by name (nullptr if not found).
    const SpellDef* byName(const QString& name) const;

    // Spells a character qualifies for at their current guild level.
    // A spell qualifies when:
    //   - the character's guild level >= spell baseLevel
    //   - the character is a member of one of the spell's guilds
    //   - every required stat is met
    QList<SpellDef> spellsFor(const Character& c) const;

    // Spells newly learned when a character reaches `newGuildLevel` in `guildName`.
    // Call after a guild level-up to find what to announce.
    QList<SpellDef> newlyLearned(const Character& c, const QString& guildName,
                                 int newGuildLevel) const;

    // Check whether a character can cast a specific spell right now.
    bool canCast(const Character& c, const QString& spellName) const;

private:
    SpellBook() = default;

    bool m_loaded = false;
    QList<SpellDef> m_spells;
};

#endif // SPELLBOOK_H
