#include "SpellClassRegistry.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

SpellClassRegistry& SpellClassRegistry::instance() {
    static SpellClassRegistry instance;
    return instance;
}

bool SpellClassRegistry::load(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QByteArray data = file.readAll();
    file.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    // SpellClasses.json is { "classes": [ {class}, ... ] }
    const QJsonArray arr = doc.object().value("classes").toArray();
    if (arr.isEmpty()) {
        return false;
    }

    m_classes.clear();
    for (const QJsonValue& v : arr) {
        const QJsonObject o = v.toObject();
        SpellClassDef def;
        def.id          = o.value("id").toString();
        def.name        = o.value("name").toString();
        def.title       = o.value("title").toString();
        def.description = o.value("description").toString();
        def.guild       = o.value("guild").toString();
        for (const QJsonValue& s : o.value("spellIds").toArray()) {
            def.spellIds.append(s.toString());
        }
        // A class with no id is a data error; skip it rather than expose a
        // nameless entry to the character sheet.
        if (!def.id.isEmpty()) {
            m_classes.append(def);
        }
    }

    m_loaded = !m_classes.isEmpty();
    return m_loaded;
}

const SpellClassDef* SpellClassRegistry::byId(const QString& id) const {
    for (const SpellClassDef& c : m_classes) {
        if (c.id.compare(id, Qt::CaseInsensitive) == 0) return &c;
    }
    return nullptr;
}

QStringList SpellClassRegistry::classIds() const {
    QStringList ids;
    for (const SpellClassDef& c : m_classes) {
        ids.append(c.id);
    }
    return ids;
}

QString SpellClassRegistry::normalizeToClassId(const QString& guildOrClass) const {
    if (guildOrClass.isEmpty()) return QString();

    // Exact class id first.
    if (const SpellClassDef* c = byId(guildOrClass)) {
        return c->id;
    }

    // Otherwise treat it as a spells.json guild name: strip a trailing
    // " Guild" and any trailing "s", then compare case-insensitively.
    QString bare = guildOrClass;
    if (bare.endsWith(" Guild", Qt::CaseInsensitive)) {
        bare.chop(6);
    }
    while (bare.endsWith('s') && !bare.isEmpty()) {
        bare.chop(1);
    }

    for (const SpellClassDef& c : m_classes) {
        QString classBare = c.id;
        while (classBare.endsWith('s') && !classBare.isEmpty()) {
            classBare.chop(1);
        }
        if (classBare.compare(bare, Qt::CaseInsensitive) == 0) {
            return c.id;
        }
    }
    return QString();
}

const SpellClassDef* SpellClassRegistry::byGuildName(const QString& guildName) const {
    const QString id = normalizeToClassId(guildName);
    return id.isEmpty() ? nullptr : byId(id);
}
