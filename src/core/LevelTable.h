#ifndef LEVELTABLE_H
#define LEVELTABLE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

// Data-driven XP table. Loads from data/levels.json.
// xpForLevel(n) returns the XP required to go from level n to level n+1.
class LevelTable {
public:
    static LevelTable& instance();

    // Load the table from JSON. Returns true on success.
    bool load(const QString& filePath);

    // XP required to go from level n to n+1.
    // Returns 100 * n^1.5 as fallback if not loaded.
    int xpForLevel(int n) const;

    // Total XP required to reach level n from level 1.
    int totalXpForLevel(int n) const;

    // Get the current level given total XP.
    int levelForXp(int totalXp) const;

    // Get the XP progress within the current level.
    // Returns (xpIntoLevel, xpToNext).
    QPair<int, int> xpProgress(int totalXp) const;

    // Maximum level in the table.
    int maxLevel() const;

    // Check if the table is loaded.
    bool isLoaded() const { return m_loaded; }

public:
    LevelTable() = default;

    bool m_loaded = false;
    QVector<int> m_xpTable;  // xpTable[n] = XP to go from level n to n+1
};

#endif // LEVELTABLE_H
