#include "LevelTable.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cmath>

LevelTable& LevelTable::instance() {
    static LevelTable instance;
    return instance;
}

bool LevelTable::load(const QString& filePath) {
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
    QJsonArray levels = root.value("levels").toArray();
    if (levels.isEmpty()) {
        return false;
    }

    m_xpTable.clear();
    m_xpTable.append(0);  // Level 0 doesn't exist, placeholder

    for (const QJsonValue& v : levels) {
        QJsonObject obj = v.toObject();
        int xp = obj.value("xpToNext").toInt();
        m_xpTable.append(xp);
    }

    m_loaded = true;
    return true;
}

int LevelTable::xpForLevel(int n) const {
    if (n <= 0) return 100;
    if (n < m_xpTable.size()) {
        return m_xpTable[n];
    }
    // Fallback: 100 * n^1.5
    return static_cast<int>(100.0 * std::pow(n, 1.5));
}

int LevelTable::totalXpForLevel(int n) const {
    if (n <= 1) return 0;
    int total = 0;
    for (int i = 1; i < n && i < m_xpTable.size(); ++i) {
        total += m_xpTable[i];
    }
    // If we need more levels than loaded, use fallback
    for (int i = m_xpTable.size(); i < n; ++i) {
        total += xpForLevel(i);
    }
    return total;
}

int LevelTable::levelForXp(int totalXp) const {
    if (totalXp <= 0) return 1;
    int level = 1;
    int xpAccum = 0;
    for (int i = 1; i < m_xpTable.size(); ++i) {
        xpAccum += m_xpTable[i];
        if (totalXp < xpAccum) {
            return level;
        }
        level++;
    }
    // Beyond loaded table, use fallback
    while (true) {
        int xpNeeded = xpForLevel(level);
        if (totalXp < xpAccum + xpNeeded) {
            return level;
        }
        xpAccum += xpNeeded;
        level++;
    }
}

QPair<int, int> LevelTable::xpProgress(int totalXp) const {
    int level = levelForXp(totalXp);
    int totalForCurrent = totalXpForLevel(level);
    int xpIntoLevel = totalXp - totalForCurrent;
    int xpToNext = xpForLevel(level);
    return qMakePair(xpIntoLevel, xpToNext);
}

int LevelTable::maxLevel() const {
    if (!m_loaded || m_xpTable.isEmpty()) return 20;
    return m_xpTable.size() - 1;
}
