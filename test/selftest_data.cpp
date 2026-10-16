#include "selftest_harness.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// ---------------------------------------------------------------
// Data file validation tests
// ---------------------------------------------------------------

static bool loadJsonFile(const QString& relativePath, QJsonDocument& doc)
{
    QFile file(relativePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray content = file.readAll();
    file.close();
    QJsonParseError parseError;
    doc = QJsonDocument::fromJson(content, &parseError);
    return parseError.error == QJsonParseError::NoError;
}

void runDataFileTests()
{
    section("Data file validation");

    // All JSON files in data/ must exist and be valid JSON
    QStringList requiredFiles;
    requiredFiles << "data/bestiary.json"
                  << "data/guilds.json"
                  << "data/journal.json"
                  << "data/levels.json"
                  << "data/races.json"
                  << "data/spells.json";

    for (const QString& filePath : requiredFiles) {
        QJsonDocument doc;
        bool loaded = loadJsonFile(filePath, doc);
        check(loaded, QString("JSON file loads: %1").arg(filePath),
              loaded ? "" : "File missing or invalid JSON");
    }

    // bestiary.json must have expected structure
    {
        QJsonDocument doc;
        if (loadJsonFile("data/bestiary.json", doc)) {
            QJsonObject root = doc.object();
            check(root.contains("monsters"), "bestiary.json has 'monsters' key");
            check(root.contains("categories"), "bestiary.json has 'categories' key");
            QJsonArray monsters = root["monsters"].toArray();
            check(monsters.size() > 0, "bestiary.json has at least one monster");
        }
    }

    // guilds.json must have expected structure
    {
        QJsonDocument doc;
        if (loadJsonFile("data/guilds.json", doc)) {
            QJsonObject root = doc.object();
            check(root.contains("guilds"), "guilds.json has 'guilds' key");
            QJsonArray guilds = root["guilds"].toArray();
            check(guilds.size() > 0, "guilds.json has at least one guild");
        }
    }

    // races.json must have expected structure
    {
        QJsonDocument doc;
        if (loadJsonFile("data/races.json", doc)) {
            QJsonObject root = doc.object();
            check(root.contains("races"), "races.json has 'races' key");
            QJsonArray races = root["races"].toArray();
            check(races.size() > 0, "races.json has at least one race");
        }
    }

    // spells.json must have expected structure
    {
        QJsonDocument doc;
        if (loadJsonFile("data/spells.json", doc)) {
            QJsonObject root = doc.object();
            check(root.contains("spells"), "spells.json has 'spells' key");
            // spells can be either an array or an object with spell names as keys
            QJsonValue spells = root["spells"];
            if (spells.isArray()) {
                check(spells.toArray().size() > 0, "spells.json has at least one spell");
            } else if (spells.isObject()) {
                check(spells.toObject().size() > 0, "spells.json has at least one spell");
            } else {
                check(false, "spells.json 'spells' is neither array nor object");
            }
        }
    }

    // levels.json must have expected structure
    {
        QJsonDocument doc;
        if (loadJsonFile("data/levels.json", doc)) {
            QJsonObject root = doc.object();
            check(root.contains("levels"), "levels.json has 'levels' key");
            QJsonArray levels = root["levels"].toArray();
            check(levels.size() > 0, "levels.json has at least one level");
        }
    }

    // journal.json must be a valid array
    {
        QJsonDocument doc;
        if (loadJsonFile("data/journal.json", doc)) {
            check(doc.isArray(), "journal.json is a JSON array");
        }
    }
}
