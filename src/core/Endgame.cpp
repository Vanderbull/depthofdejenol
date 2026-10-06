#include "Endgame.h"
#include "BossEncounter.h"
#include <algorithm>

// --- GameRecord ---

QVariantMap GameRecord::toMap() const {
    QVariantMap m;
    m["heroName"] = heroName;
    m["highestLevel"] = highestLevel;
    m["mostGold"] = QVariant::fromValue(mostGold);
    m["deepestFloor"] = deepestFloor;
    m["completionTimeSeconds"] = QVariant::fromValue(completionTimeSeconds);
    m["won"] = won;
    return m;
}

void GameRecord::loadFromMap(const QVariantMap& map) {
    heroName = map.value("heroName").toString();
    highestLevel = map.value("highestLevel").toInt();
    mostGold = map.value("mostGold").toLongLong();
    deepestFloor = map.value("deepestFloor").toInt();
    completionTimeSeconds = map.value("completionTimeSeconds").toLongLong();
    won = map.value("won", false).toBool();
}

QString GameRecord::formattedTime() const {
    if (completionTimeSeconds <= 0) return "--:--:--";
    qint64 total = completionTimeSeconds;
    qint64 hours = total / 3600;
    qint64 minutes = (total % 3600) / 60;
    qint64 seconds = total % 60;
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

// --- Endgame ---

QVariantMap Endgame::buildFinalBoss() {
    // The Prince: an order of magnitude beyond the floor-15 boss.
    QVariantMap boss = BossEncounter::buildBoss(15);
    boss["name"] = finalBossName();
    boss["hp"] = 5000;
    boss["maxHp"] = 5000;
    boss["att"] = 120;
    boss["def"] = 60;
    boss["speed"] = 40;
    boss["dex"] = 35;
    boss["level"] = 40;
    boss["damageMod"] = 200;
    boss["swings"] = 5;
    boss["levelScale"] = 10;
    return boss;
}

QString Endgame::victoryTitle() {
    return "The Gate Closes";
}

QStringList Endgame::victoryParagraphs() {
    QStringList out;
    out << "The Prince of Devils falls. The gate he tore open shudders, cracks, and is still.";
    out << "You climb back through fifteen floors of the dead, past the crypts and the mines, "
           "and step into the sun for the last time.";
    out << "The City remembers what you did. Your name is carved into the Hall of Records, "
           "and the dark below is only dark again.";
    return out;
}

bool Endgame::isVictory(const QList<int>& defeatedBossFloors) {
    return defeatedBossFloors.contains(15);
}

bool Endgame::outranks(const GameRecord& a, const GameRecord& b, Category c) {
    switch (c) {
    case Category::HighestLevel:
        return a.highestLevel > b.highestLevel;
    case Category::MostGold:
        return a.mostGold > b.mostGold;
    case Category::DeepestFloor:
        return a.deepestFloor > b.deepestFloor;
    case Category::FastestCompletion: {
        // Only finished runs count; a win always outranks a non-finish.
        if (a.won != b.won) return a.won;
        if (!a.won) return false;
        // A zero or missing time is not a real completion.
        if (a.completionTimeSeconds <= 0) return false;
        if (b.completionTimeSeconds <= 0) return true;
        return a.completionTimeSeconds < b.completionTimeSeconds;
    }
    }
    return false;
}

QList<GameRecord> Endgame::ranked(QList<GameRecord> records, Category c) {
    std::stable_sort(records.begin(), records.end(),
                     [c](const GameRecord& a, const GameRecord& b) {
                         return outranks(a, b, c);
                     });
    return records;
}

QString Endgame::categoryName(Category c) {
    switch (c) {
    case Category::HighestLevel:     return "Highest Level";
    case Category::MostGold:         return "Most Gold";
    case Category::DeepestFloor:     return "Deepest Floor";
    case Category::FastestCompletion: return "Fastest Completion";
    }
    return "Unknown";
}

double Endgame::ngPlusMonsterMultiplier(int ngPlusLevel) {
    if (ngPlusLevel <= 0) return 1.0;
    // +50% per cycle.
    return 1.0 + 0.5 * ngPlusLevel;
}

double Endgame::ngPlusRewardMultiplier(int ngPlusLevel) {
    if (ngPlusLevel <= 0) return 1.0;
    // +25% per cycle: harder, but worth it.
    return 1.0 + 0.25 * ngPlusLevel;
}

QString Endgame::ngPlusBanner(int ngPlusLevel) {
    if (ngPlusLevel <= 0) return QString();
    return QString("New Game +%1 — monsters are %2% stronger.")
        .arg(ngPlusLevel)
        .arg(static_cast<int>((ngPlusMonsterMultiplier(ngPlusLevel) - 1.0) * 100));
}
