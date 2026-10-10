#include "DungeonLevelState.h"
#include <QVariantMap>
#include <QVariantList>

// --- LevelSnapshot ---

namespace {

QVariantList positionsToList(const QSet<QPair<int, int>>& set) {
    QVariantList list;
    for (const auto& p : set) {
        QVariantMap m;
        m["x"] = p.first;
        m["y"] = p.second;
        list.append(m);
    }
    return list;
}

QSet<QPair<int, int>> listToPositions(const QVariantList& list) {
    QSet<QPair<int, int>> set;
    for (const QVariant& v : list) {
        QVariantMap m = v.toMap();
        set.insert(qMakePair(m["x"].toInt(), m["y"].toInt()));
    }
    return set;
}

QVariantList monsterMapToList(const QMap<QPair<int, int>, QString>& map) {
    QVariantList list;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        QVariantMap m;
        m["x"] = it.key().first;
        m["y"] = it.key().second;
        m["name"] = it.value();
        list.append(m);
    }
    return list;
}

QMap<QPair<int, int>, QString> listToMonsterMap(const QVariantList& list) {
    QMap<QPair<int, int>, QString> map;
    for (const QVariant& v : list) {
        QVariantMap m = v.toMap();
        map.insert(qMakePair(m["x"].toInt(), m["y"].toInt()), m["name"].toString());
    }
    return map;
}

} // namespace

QVariantMap LevelSnapshot::toMap() const {
    QVariantMap m;
    m["level"] = level;
    m["generated"] = generated;
    m["monsters"] = monsterMapToList(monsterPositions);
    m["treasures"] = monsterMapToList(treasurePositions);
    m["openedChests"] = positionsToList(openedChests);
    m["visitedTiles"] = positionsToList(visitedTiles);
    m["collectedItems"] = positionsToList(collectedItems);
    m["unlockedDoors"] = positionsToList(unlockedDoors);
    m["triggeredTraps"] = positionsToList(triggeredTraps);
    m["trapPositions"] = monsterMapToList(trapPositions);

    QVariantMap up;
    up["x"] = stairsUp.first;
    up["y"] = stairsUp.second;
    m["stairsUp"] = up;

    QVariantMap down;
    down["x"] = stairsDown.first;
    down["y"] = stairsDown.second;
    m["stairsDown"] = down;

    m["bossDefeated"] = bossDefeated;

    // Torch / light state.
    m["torchTurns"] = torchTurnsRemaining;
    m["lightRadius"] = lightRadius;
    QVariantList torches;
    for (const QString& t : collectedTorches) torches.append(t);
    m["collectedTorches"] = torches;
    m["collectedItems"] = positionsToList(collectedItems);
    m["unlockedDoors"] = positionsToList(unlockedDoors);
    m["triggeredTraps"] = positionsToList(triggeredTraps);
    m["trapPositions"] = monsterMapToList(trapPositions);
    return m;
}

void LevelSnapshot::loadFromMap(const QVariantMap& map) {
    level = map.value("level").toInt();
    generated = map.value("generated", false).toBool();
    monsterPositions = listToMonsterMap(map.value("monsters").toList());
    treasurePositions = listToMonsterMap(map.value("treasures").toList());
    openedChests = listToPositions(map.value("openedChests").toList());
    visitedTiles = listToPositions(map.value("visitedTiles").toList());
    collectedItems = listToPositions(map.value("collectedItems").toList());
    unlockedDoors = listToPositions(map.value("unlockedDoors").toList());

    QVariantMap up = map.value("stairsUp").toMap();
    stairsUp = qMakePair(up.value("x", -1).toInt(), up.value("y", -1).toInt());
    QVariantMap down = map.value("stairsDown").toMap();
    stairsDown = qMakePair(down.value("x", -1).toInt(), down.value("y", -1).toInt());

    bossDefeated = map.value("bossDefeated", false).toBool();

    // Torch / light state.
    torchTurnsRemaining = map.value("torchTurns", 0).toInt();
    lightRadius = map.value("lightRadius", 2).toInt();
    QVariantList torches = map.value("collectedTorches").toList();
    for (const QVariant& v : torches) collectedTorches.append(v.toString());
    collectedItems = listToPositions(map.value("collectedItems").toList());
    unlockedDoors = listToPositions(map.value("unlockedDoors").toList());
    triggeredTraps = listToPositions(map.value("triggeredTraps").toList());
    trapPositions = listToMonsterMap(map.value("trapPositions").toList());
}

// --- DungeonLevelRegistry ---

DungeonLevelRegistry& DungeonLevelRegistry::instance() {
    static DungeonLevelRegistry instance;
    return instance;
}

void DungeonLevelRegistry::clear() {
    m_levels.clear();
}

bool DungeonLevelRegistry::hasLevel(int level) const {
    return m_levels.contains(level) && m_levels.value(level).generated;
}

const LevelSnapshot* DungeonLevelRegistry::level(int level) const {
    auto it = m_levels.constFind(level);
    if (it == m_levels.constEnd()) return nullptr;
    return &it.value();
}

LevelSnapshot& DungeonLevelRegistry::levelForEdit(int level) {
    return m_levels[level];
}

void DungeonLevelRegistry::store(const LevelSnapshot& snapshot) {
    m_levels[snapshot.level] = snapshot;
}

int DungeonLevelRegistry::respawnMonsters(int level, double fraction,
                                          int originalMonsterCount,
                                          QRandomGenerator& rng,
                                          const QStringList& monsterPool) {
    auto it = m_levels.find(level);
    if (it == m_levels.end() || !it.value().generated) return 0;
    if (monsterPool.isEmpty()) return 0;  // no pool to draw from

    LevelSnapshot& snap = it.value();
    int current = snap.monsterPositions.size();
    int target = static_cast<int>(originalMonsterCount * fraction);
    int toPlace = qMax(0, target - current);
    if (toPlace == 0) return 0;

    // Free tiles: treasure positions and the stairs are off limits.
    int placed = 0;
    for (int attempt = 0; attempt < toPlace * 20 && placed < toPlace; ++attempt) {
        int x = rng.bounded(30);
        int y = rng.bounded(30);
        QPair<int, int> pos = qMakePair(x, y);

        if (snap.monsterPositions.contains(pos)) continue;
        if (snap.treasurePositions.contains(pos)) continue;
        if (pos == snap.stairsUp || pos == snap.stairsDown) continue;

        const QString monster = monsterPool.at(rng.bounded(monsterPool.size()));
        snap.monsterPositions.insert(pos, monster);
        placed++;
    }
    return placed;
}

QVariantMap DungeonLevelRegistry::toMap() const {
    QVariantMap m;
    QVariantList list;
    for (auto it = m_levels.constBegin(); it != m_levels.constEnd(); ++it) {
        list.append(it.value().toMap());
    }
    m["levels"] = list;
    return m;
}

void DungeonLevelRegistry::loadFromMap(const QVariantMap& map) {
    m_levels.clear();
    for (const QVariant& v : map.value("levels").toList()) {
        LevelSnapshot snap;
        snap.loadFromMap(v.toMap());
        m_levels[snap.level] = snap;
    }
}
