#include "Arena.h"
#include "src/core/Endgame.h"
#include <algorithm>

// --- Score ---

QVariantMap Arena::Score::toMap() const {
    QVariantMap m;
    m["heroName"] = heroName;
    m["wavesCleared"] = wavesCleared;
    m["totalKills"] = totalKills;
    m["totalDamageTaken"] = totalDamageTaken;
    m["totalRounds"] = totalRounds;
    m["score"] = score;
    return m;
}

void Arena::Score::loadFromMap(const QVariantMap& map) {
    heroName = map.value("heroName").toString();
    wavesCleared = map.value("wavesCleared").toInt();
    totalKills = map.value("totalKills").toInt();
    totalDamageTaken = map.value("totalDamageTaken").toInt();
    totalRounds = map.value("totalRounds").toInt();
    score = map.value("score").toInt();
}

// --- Wave scaling ---

int Arena::monsterCountForWave(int wave) {
    if (wave < 1) wave = 1;
    if (wave > 10) wave = 10;
    return 1 + wave;  // wave 1: 2, wave 10: 11
}

int Arena::monsterLevelForWave(int wave, int ngPlusLevel) {
    if (wave < 1) wave = 1;
    if (wave > 10) wave = 10;
    int ngBonus = ngPlusLevel * 5;
    return 5 + wave * 3 + ngBonus;  // wave 1: 8, wave 10: 35 (+ NG bonus)
}

// --- Scoring ---

int Arena::waveScore(const WaveResult& result) {
    if (!result.cleared) return 0;

    // Base points per wave: 100 * wave (later waves worth more).
    int base = 100 * result.wave;

    // Speed bonus: fewer rounds = more points.
    // Optimal rounds = wave * 2 (kill each monster in ~2 rounds).
    int optimalRounds = result.wave * 2;
    int speedBonus = 0;
    if (result.rounds <= optimalRounds) {
        speedBonus = 200 * result.wave;
    } else if (result.rounds <= optimalRounds * 2) {
        speedBonus = 100 * result.wave;
    }

    // Damage penalty: more damage = fewer points.
    int damagePenalty = result.damageTaken * 2;

    // Kill bonus: 10 points per monster killed.
    int killBonus = result.monstersKilled * 10;

    int score = base + speedBonus + killBonus - damagePenalty;
    if (score < 0) score = 0;
    return score;
}

int Arena::totalScore(const QList<WaveResult>& results) {
    int total = 0;
    for (const WaveResult& r : results) {
        total += waveScore(r);
    }
    return total;
}

// --- Leaderboard ---

QList<Arena::Score> Arena::ranked(QList<Score> scores) {
    std::stable_sort(scores.begin(), scores.end(),
                     [](const Score& a, const Score& b) {
                         return a.score > b.score;
                     });
    return scores;
}

// --- Access ---

bool Arena::isAccessible() {
    return true;
}
