#ifndef ACHIEVEMENTSYSTEM_H
#define ACHIEVEMENTSYSTEM_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariantMap>

// Achievement system (v4.0.0 slice 4.4).
// Persistent milestones that unlock cosmetics and bragging rights.
// Stored in game state as a list of unlocked achievement IDs.
class AchievementSystem {
public:
    struct Achievement {
        QString id;
        QString name;
        QString description;
        int goldReward = 0;

        QVariantMap toMap() const;
        void loadFromMap(const QVariantMap& map);
    };

    // All available achievements.
    static QList<Achievement> all();

    // Is this achievement unlocked?
    static bool isUnlocked(const QString& id);

    // Unlock an achievement (returns false if already unlocked or unknown).
    static bool unlock(const QString& id);

    // Unlock all achievements whose condition is met.
    // Returns the number of newly unlocked achievements.
    static int checkAndUnlock();

    // Display name for an achievement.
    static QString nameForId(const QString& id);

    // Description for an achievement.
    static QString descriptionForId(const QString& id);

    // Gold reward for an achievement.
    static int goldRewardForId(const QString& id);

    // Total number of unlocked achievements.
    static int unlockedCount();

    // Total number of achievements.
    static int totalCount();

    // List of unlocked achievement IDs.
    static QStringList unlockedIds();

    // Save/load from game state.
    static QVariantMap toMap();
    static void loadFromMap(const QVariantMap& map);

    // Reset all achievements (for testing).
    static void reset();
};

#endif // ACHIEVEMENTSYSTEM_H
