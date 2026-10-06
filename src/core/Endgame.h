#ifndef ENDGAME_H
#define ENDGAME_H

#include "QuestChain.h"
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariantMap>

// A run's record, kept for the Hall of Records.
struct GameRecord {
    QString heroName;
    int highestLevel = 0;
    qint64 mostGold = 0;
    int deepestFloor = 0;
    qint64 completionTimeSeconds = 0;  // 0 = not finished
    bool won = false;

    QVariantMap toMap() const;
    void loadFromMap(const QVariantMap& map);

    // Completion time as "HH:MM:SS".
    QString formattedTime() const;
};

// Victory sequence, final boss and New Game Plus.
class Endgame {
public:
    // --- 6.2 Final boss ---
    static QString finalBossName() { return "The Prince of Devils"; }

    // The Prince is far above a floor boss.
    static QVariantMap buildFinalBoss();

    // --- 6.3 Victory sequence ---
    // Title and body of the victory text, the counterpart to the intro StoryDialog.
    static QString victoryTitle();
    static QStringList victoryParagraphs();

    // Has the game been won?
    static bool isVictory(const QList<int>& defeatedBossFloors);

    // --- 6.4 Hall of Records ---
    // Compare two records: true when `a` outranks `b` for the given category.
    enum class Category { HighestLevel, MostGold, DeepestFloor, FastestCompletion };
    static bool outranks(const GameRecord& a, const GameRecord& b, Category c);

    // Sort records best-first for a category.
    static QList<GameRecord> ranked(QList<GameRecord> records, Category c);

    // Category display name.
    static QString categoryName(Category c);

    // --- 6.5 New Game Plus ---
    // New Game Plus starts harder: monsters scale by this multiplier.
    static double ngPlusMonsterMultiplier(int ngPlusLevel);

    // Gold and XP rewards scale too, so it stays worth playing.
    static double ngPlusRewardMultiplier(int ngPlusLevel);

    // A short banner for the difficulty.
    static QString ngPlusBanner(int ngPlusLevel);
};

#endif // ENDGAME_H
