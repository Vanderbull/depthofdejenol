#ifndef DUNGEONLEVELSTATE_H
#define DUNGEONLEVELSTATE_H

#include <QString>
#include <QMap>
#include <QSet>
#include <QPair>
#include <QVariantMap>
#include <QVariantList>
#include <QRandomGenerator>

// A snapshot of everything that changes on one dungeon floor.
// Serialized so a floor can be left and returned to without regenerating.
struct LevelSnapshot {
    int level = 0;
    bool generated = false;

    // Where things are.
    QMap<QPair<int, int>, QString> monsterPositions;
    QMap<QPair<int, int>, QString> treasurePositions;
    QSet<QPair<int, int>> openedChests;
    QSet<QPair<int, int>> visitedTiles;
    QSet<QPair<int, int>> collectedItems;
    QSet<QPair<int, int>> unlockedDoors;

    // Stairs.
    QPair<int, int> stairsUp = {-1, -1};
    QPair<int, int> stairsDown = {-1, -1};

    // Boss state (only meaningful on boss floors).
    bool bossDefeated = false;

    // Torch / light mechanic.
    int torchTurnsRemaining = 0;   // >0 while the torch burns; 0 = out.
    int lightRadius = 2;           // tiles visible around the player in darkness.
    QStringList collectedTorches;  // names of distinct torch items found on this floor (for persistence).

    // Trapped floor tiles that have already fired (so they don't repeat).
    QSet<QPair<int, int>> triggeredTraps;

    // Trap positions with their type (e.g. "Spike", "Dart").
    QMap<QPair<int, int>, QString> trapPositions;

    QVariantMap toMap() const;
    void loadFromMap(const QVariantMap& map);
};

// Holds one snapshot per dungeon level for the current game.
class DungeonLevelRegistry {
public:
    static DungeonLevelRegistry& instance();

    void clear();

    bool hasLevel(int level) const;
    const LevelSnapshot* level(int level) const;
    LevelSnapshot& levelForEdit(int level);

    // Store a snapshot.
    void store(const LevelSnapshot& snapshot);

    // Number of floors currently held.
    int count() const { return m_levels.size(); }

    // Respawn a fraction of the cleared monsters on a floor.
    // `fraction` is 0.0-1.0; 0.3 means roughly 30% of the floor's original
    // monster count is placed back on free tiles. Returns how many respawned.
    // Uses `originalMonsterCount` as the floor's baseline population.
    // `monsterPool` is the set of monster names to draw from (typically the
    // floor's thematic monsters); when empty the respawn is skipped, because
    // this pure-logic layer has no access to the bestiary.
    int respawnMonsters(int level, double fraction, int originalMonsterCount,
                        QRandomGenerator& rng, const QStringList& monsterPool = {});

    // Serialize every floor (for the save file).
    QVariantMap toMap() const;
    void loadFromMap(const QVariantMap& map);

private:
    DungeonLevelRegistry() = default;

    QMap<int, LevelSnapshot> m_levels;
};

#endif // DUNGEONLEVELSTATE_H
