#ifndef ARENA_H
#define ARENA_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariantMap>

// Arena / challenge mode (v4.0.0 slice 4.3).
// Ten waves of monsters, each harder than the last.
// Score is based on speed, damage taken, and waves cleared.
class Arena {
public:
    struct WaveResult {
        int wave = 0;            // 1-based wave number
        int monstersKilled = 0;
        int damageTaken = 0;     // total HP lost by party this wave
        int rounds = 0;          // turns taken
        bool cleared = false;
    };

    struct Score {
        QString heroName;
        int wavesCleared = 0;
        int totalKills = 0;
        int totalDamageTaken = 0;
        int totalRounds = 0;
        int score = 0;

        QVariantMap toMap() const;
        void loadFromMap(const QVariantMap& map);
    };

    // Monster count for a wave (1-10). Wave 1: 2 monsters, wave 10: 12 monsters.
    static int monsterCountForWave(int wave);

    // Monster level for a wave. Scales with wave number.
    static int monsterLevelForWave(int wave, int ngPlusLevel = 0);

    // Score for a single wave. Faster = more points, less damage = more points.
    static int waveScore(const WaveResult& result);

    // Total score across all waves.
    static int totalScore(const QList<WaveResult>& results);

    // Leaderboard: sort scores best-first (highest score).
    static QList<Score> ranked(QList<Score> scores);

    // Display name for the arena.
    static QString arenaName() { return "The Arena"; }

    // Is the arena accessible? (Always true — accessible from town.)
    static bool isAccessible();
};

#endif // ARENA_H
