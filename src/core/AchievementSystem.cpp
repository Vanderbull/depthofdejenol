#include "AchievementSystem.h"
#include "src/core/gameStateManager.h"
#include "src/core/Endgame.h"
#include <QVariantList>
#include <QVariantMap>

// --- Achievement ---

QVariantMap AchievementSystem::Achievement::toMap() const {
    QVariantMap m;
    m["id"] = id;
    m["name"] = name;
    m["description"] = description;
    m["goldReward"] = goldReward;
    return m;
}

void AchievementSystem::Achievement::loadFromMap(const QVariantMap& map) {
    id = map.value("id").toString();
    name = map.value("name").toString();
    description = map.value("description").toString();
    goldReward = map.value("goldReward").toInt();
}

// --- All achievements ---

QList<AchievementSystem::Achievement> AchievementSystem::all() {
    QList<Achievement> list;

    auto add = [&list](const QString& id, const QString& name, const QString& desc, int gold) {
        Achievement a;
        a.id = id;
        a.name = name;
        a.description = desc;
        a.goldReward = gold;
        list.append(a);
    };

    add("first_blood", "First Blood", "Kill your first monster", 100);
    add("slayer", "Slayer", "Kill 100 monsters", 500);
    add("floor_5", "Delver", "Reach dungeon floor 5", 250);
    add("floor_10", "Deep Delver", "Reach dungeon floor 10", 500);
    add("floor_15", "Abyss Walker", "Reach dungeon floor 15", 1000);
    add("boss_5", "Grotto Vanquisher", "Defeat the floor 5 boss", 500);
    add("boss_10", "Bone Breaker", "Defeat the floor 10 boss", 1000);
    add("boss_15", "Devil's Bane", "Defeat the Prince of Devils", 5000);
    add("ng_plus_1", "New Game", "Complete your first NG+ run", 1000);
    add("ng_plus_3", "Veteran", "Reach NG+ level 3", 3000);
    add("gold_10000", "Treasurer", "Earn 10,000 gold", 500);
    add("gold_50000", "Dragon Hoard", "Earn 50,000 gold", 2000);
    add("die_1", "Mortal", "Die for the first time", 100);
    add("die_10", "Persistent", "Die 10 times", 500);
    add("level_10", "Seasoned", "Reach level 10", 500);
    add("level_20", "Hero", "Reach level 20", 1000);
    add("level_30", "Legend", "Reach level 30", 2000);
    add("arena_1", "Contestant", "Clear 1 Arena wave", 250);
    add("arena_5", "Fighter", "Clear 5 Arena waves", 500);
    add("arena_10", "Arena Champion", "Clear all 10 Arena waves", 2000);
    add("abyss_1", "Abyssal Explorer", "Reach Abyss floor 1", 1000);
    add("abyss_5", "Abyssal Warrior", "Defeat the floor 5 Abyss boss", 3000);
    add("abyss_10", "Heart Destroyer", "Defeat the Abyssal Heart", 10000);

    return list;
}

// --- Unlock state ---

bool AchievementSystem::isUnlocked(const QString& id) {
    QStringList unlocked = unlockedIds();
    return unlocked.contains(id);
}

bool AchievementSystem::unlock(const QString& id) {
    // Validate the achievement exists
    bool found = false;
    for (const Achievement& a : all()) {
        if (a.id == id) { found = true; break; }
    }
    if (!found) return false;

    // Already unlocked?
    if (isUnlocked(id)) return false;

    // Unlock it
    QVariantList unlockedList = gameStateManager::instance()->getGameValue("UnlockedAchievements").toList();
    unlockedList.append(id);
    gameStateManager::instance()->setGameValue("UnlockedAchievements", unlockedList);

    // Grant gold reward
    const Achievement* ach = nullptr;
    for (const Achievement& a : all()) {
        if (a.id == id) { ach = &a; break; }
    }
    if (ach && ach->goldReward > 0) {
        gameStateManager::instance()->addPartyGold(ach->goldReward);
    }

    return true;
}

// --- Check conditions ---

int AchievementSystem::checkAndUnlock() {
    gameStateManager* gsm = gameStateManager::instance();
    int count = 0;

    // Combat / progression
    int deepestFloor = gsm->getGameValue("DeepestFloor").toInt();
    int ngPlus = gsm->getNgPlusLevel();
    int gold = gsm->getPartyGold();
    int kills = gsm->getGameValue("TotalKills").toInt();
    int deaths = gsm->getGameValue("TotalDeaths").toInt();
    int arenaWaves = gsm->getGameValue("ArenaWavesCleared").toInt();
    int abyssFloor = gsm->getGameValue("AbyssFloor").toInt();

    // Highest party level
    int highestLevel = 1;
    for (const Character& c : gsm->getPartyMembers()) {
        if (c.level > highestLevel) highestLevel = c.level;
    }

    // Kill milestones
    if (kills >= 1) { if (unlock("first_blood")) count++; }
    if (kills >= 100) { if (unlock("slayer")) count++; }

    // Depth milestones
    if (deepestFloor >= 5) { if (unlock("floor_5")) count++; }
    if (deepestFloor >= 10) { if (unlock("floor_10")) count++; }
    if (deepestFloor >= 15) { if (unlock("floor_15")) count++; }

    // Boss milestones (based on quest chain / defeated bosses)
    // We check the DungeonLevelRegistry for defeated bosses
    // For simplicity, use deepestFloor as proxy
    if (deepestFloor >= 5) { if (unlock("boss_5")) count++; }
    if (deepestFloor >= 10) { if (unlock("boss_10")) count++; }
    if (deepestFloor >= 15) { if (unlock("boss_15")) count++; }

    // NG+ milestones
    if (ngPlus >= 1) { if (unlock("ng_plus_1")) count++; }
    if (ngPlus >= 3) { if (unlock("ng_plus_3")) count++; }

    // Gold milestones
    if (gold >= 10000) { if (unlock("gold_10000")) count++; }
    if (gold >= 50000) { if (unlock("gold_50000")) count++; }

    // Death milestones
    if (deaths >= 1) { if (unlock("die_1")) count++; }
    if (deaths >= 10) { if (unlock("die_10")) count++; }

    // Level milestones
    if (highestLevel >= 10) { if (unlock("level_10")) count++; }
    if (highestLevel >= 20) { if (unlock("level_20")) count++; }
    if (highestLevel >= 30) { if (unlock("level_30")) count++; }

    // Arena milestones
    if (arenaWaves >= 1) { if (unlock("arena_1")) count++; }
    if (arenaWaves >= 5) { if (unlock("arena_5")) count++; }
    if (arenaWaves >= 10) { if (unlock("arena_10")) count++; }

    // Abyss milestones
    if (abyssFloor >= 1) { if (unlock("abyss_1")) count++; }
    if (abyssFloor >= 5) { if (unlock("abyss_5")) count++; }
    if (abyssFloor >= 10) { if (unlock("abyss_10")) count++; }

    return count;
}

// --- Queries ---

QString AchievementSystem::nameForId(const QString& id) {
    for (const Achievement& a : all()) {
        if (a.id == id) return a.name;
    }
    return QString();
}

QString AchievementSystem::descriptionForId(const QString& id) {
    for (const Achievement& a : all()) {
        if (a.id == id) return a.description;
    }
    return QString();
}

int AchievementSystem::goldRewardForId(const QString& id) {
    for (const Achievement& a : all()) {
        if (a.id == id) return a.goldReward;
    }
    return 0;
}

int AchievementSystem::unlockedCount() {
    return unlockedIds().size();
}

int AchievementSystem::totalCount() {
    return all().size();
}

QStringList AchievementSystem::unlockedIds() {
    return gameStateManager::instance()->getGameValue("UnlockedAchievements").toStringList();
}

// --- Save/load ---

QVariantMap AchievementSystem::toMap() {
    QVariantMap m;
    m["UnlockedAchievements"] = unlockedIds();
    return m;
}

void AchievementSystem::loadFromMap(const QVariantMap& map) {
    QVariantList list = map.value("UnlockedAchievements").toList();
    gameStateManager::instance()->setGameValue("UnlockedAchievements", list);
}

// --- Reset ---

void AchievementSystem::reset() {
    gameStateManager::instance()->setGameValue("UnlockedAchievements", QVariantList());
}
