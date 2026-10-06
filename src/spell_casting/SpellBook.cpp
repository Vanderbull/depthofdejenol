#include "SpellBook.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

SpellBook& SpellBook::instance() {
    static SpellBook instance;
    return instance;
}

bool SpellBook::load(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    QJsonObject spellsObj = root.value("spells").toObject();
    if (spellsObj.isEmpty()) {
        return false;
    }

    m_spells.clear();

    // spells.json is { "spells": { "<Category>": [ {spell}, ... ], ... } }
    for (auto catIt = spellsObj.constBegin(); catIt != spellsObj.constEnd(); ++catIt) {
        QString category = catIt.key();
        QJsonArray arr = catIt.value().toArray();

        for (const QJsonValue& v : arr) {
            QJsonObject obj = v.toObject();

            SpellDef def;
            def.name = obj.value("name").toString();
            def.category = category;
            def.mana = obj.value("mana").toInt();
            def.baseLevel = obj.value("base_level").toInt(1);
            def.isCombatSpell = obj.value("is_combat_spell").toBool();
            def.damage = obj.value("damage").toString();
            def.description = obj.value("description").toString();

            for (const QJsonValue& g : obj.value("guilds").toArray()) {
                def.guilds.append(g.toString());
            }

            QJsonObject reqs = obj.value("required_stats").toObject();
            for (auto reqIt = reqs.constBegin(); reqIt != reqs.constEnd(); ++reqIt) {
                def.requiredStats[reqIt.key()] = reqIt.value().toInt();
            }

            if (!def.name.isEmpty()) {
                m_spells.append(def);
            }
        }
    }

    m_loaded = !m_spells.isEmpty();
    return m_loaded;
}

const SpellDef* SpellBook::byName(const QString& name) const {
    for (const SpellDef& s : m_spells) {
        if (s.name == name) return &s;
    }
    return nullptr;
}

QList<SpellDef> SpellBook::spellsFor(const Character& c) const {
    QList<SpellDef> result;

    for (const SpellDef& s : m_spells) {
        // Must belong to one of the character's guilds at a sufficient level.
        bool guildOk = false;
        for (const QString& g : s.guilds) {
            if (c.guildLevel(g) >= s.baseLevel) {
                guildOk = true;
                break;
            }
        }
        if (!guildOk) continue;

        // Every required stat must be met.
        bool statsOk = true;
        for (auto it = s.requiredStats.constBegin(); it != s.requiredStats.constEnd(); ++it) {
            int have = c.effectiveStat(it.key());
            if (have < it.value()) {
                statsOk = false;
                break;
            }
        }
        if (!statsOk) continue;

        result.append(s);
    }

    return result;
}

QList<SpellDef> SpellBook::newlyLearned(const Character& c, const QString& guildName,
                                        int newGuildLevel) const {
    QList<SpellDef> result;

    for (const SpellDef& s : m_spells) {
        if (!s.guilds.contains(guildName)) continue;
        if (s.baseLevel != newGuildLevel) continue;

        // Stat requirements still apply.
        bool statsOk = true;
        for (auto it = s.requiredStats.constBegin(); it != s.requiredStats.constEnd(); ++it) {
            if (c.effectiveStat(it.key()) < it.value()) {
                statsOk = false;
                break;
            }
        }
        if (!statsOk) continue;

        result.append(s);
    }

    return result;
}

bool SpellBook::canCast(const Character& c, const QString& spellName) const {
    const SpellDef* s = byName(spellName);
    if (!s) return false;

    // Must know it (guild + level + stats).
    QList<SpellDef> known = spellsFor(c);
    bool knows = false;
    for (const SpellDef& k : known) {
        if (k.name == spellName) { knows = true; break; }
    }
    if (!knows) return false;

    // Must afford the mana.
    return c.mana >= s->mana;
}
