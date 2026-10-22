#include "MonsterResistance.h"
#include "src/core/ResistanceNames.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QHash>

namespace {

// monster name -> (normalised resistance name -> percent)
QHash<QString, QHash<QString, int> > g_table;
bool g_loaded = false;

QString normalize(const QString& name)
{
    return name.trimmed().toLower();
}

// Same search order the spell data and the Bestiary use: cwd, then next to the
// binary, then one level up (build/bin -> project root).
QStringList defaultCandidates()
{
    return QStringList()
        << QStringLiteral("data/bestiary.json")
        << QCoreApplication::applicationDirPath() + QStringLiteral("/data/bestiary.json")
        << QCoreApplication::applicationDirPath() + QStringLiteral("/../data/bestiary.json")
        << QStringLiteral("../data/bestiary.json");
}

// Parse one bestiary.json document into the table. Keeps the strongest entry
// when a monster lists the same resistance twice.
void parseDocument(const QJsonDocument& doc)
{
    if (!doc.isObject()) return;

    const QJsonArray arr = doc.object().value(QStringLiteral("monsters")).toArray();
    for (const QJsonValue& v : arr) {
        const QJsonObject o = v.toObject();
        const QString monster = o.value(QStringLiteral("name")).toString();
        if (monster.isEmpty()) continue;

        QHash<QString, int>& row = g_table[monster];
        const QJsonArray resArr = o.value(QStringLiteral("resistances")).toArray();
        for (const QJsonValue& rv : resArr) {
            const QJsonObject ro = rv.toObject();
            const QString resName = normalize(ro.value(QStringLiteral("name")).toString());
            const int pct = ro.value(QStringLiteral("percent")).toInt();
            if (!resName.isEmpty() && pct > 0) {
                row[resName] = qMax(row.value(resName), pct);
            }
        }
    }
    g_loaded = true;
}

void ensureLoaded()
{
    if (g_loaded) return;
    for (const QString& path : defaultCandidates()) {
        QFile file(path);
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) continue;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (doc.isObject()) {
            parseDocument(doc);
            return;
        }
    }
}

} // namespace

QStringList MonsterResistance::names()
{
    return ResistanceNames::all();
}

int MonsterResistance::indexForName(const QString& name)
{
    const QStringList all = ResistanceNames::all();
    const QString probe = normalize(name);
    for (int i = 0; i < all.size(); ++i) {
        if (normalize(all.at(i)) == probe) return i;
    }
    return -1;
}

QString MonsterResistance::nameForIndex(int index)
{
    const QStringList all = ResistanceNames::all();
    if (index < 0 || index >= all.size()) return QString();
    return all.at(index);
}

bool MonsterResistance::isLoaded()
{
    return g_loaded;
}

void MonsterResistance::reset()
{
    g_table.clear();
    g_loaded = false;
}

bool MonsterResistance::setSourcePath(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    g_table.clear();
    g_loaded = false;
    parseDocument(doc);
    return g_loaded;
}

int MonsterResistance::percentFor(const QString& monsterName, const QString& resistanceName)
{
    ensureLoaded();
    return g_table.value(monsterName).value(normalize(resistanceName), 0);
}

QList<QPair<QString, int> > MonsterResistance::resistancesFor(const QString& monsterName)
{
    ensureLoaded();
    QList<QPair<QString, int> > out;
    const QHash<QString, int>& row = g_table.value(monsterName);
    // Walk the canonical index order so the panel reads top to bottom.
    const QStringList all = ResistanceNames::all();
    for (const QString& name : all) {
        const int pct = row.value(normalize(name), 0);
        if (pct > 0) out << qMakePair(name, pct);
    }
    return out;
}
