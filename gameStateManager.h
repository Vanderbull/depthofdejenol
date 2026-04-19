#ifndef gameStateManager_H
#define gameStateManager_H

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

// Project Includes
#include "src/core/GameConstants.h"
#include "src/core/game_resources.h"
#include "dataRegistry.h"
#include "audioManager.h"
#include "fontManager.h"
#include "character.h"

#include <QTcpSocket>
// Qt Core
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QVariantMap>
#include <QPoint>
#include <QTimer>
#include <QDebug>

// Qt GUI & Widgets
#include <QFont>
#include <QPixmap>
#include <QPainter>
#include <QApplication>
#include <QWidget>

// Qt JSON/IO
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

// Forward declarations
class PartyManager;


class gameStateManager : public QObject
{
    Q_OBJECT

private:
    QPixmap m_fontSpriteSheet;

    GameConstants::GameMode m_currentMode = GameConstants::GameMode::InCity;
    GameConstants::CityLocation m_currentCityLocation = GameConstants::CityLocation::Street;
    // Use a pointer so the compiler doesn't need to know
    // the exact size of PartyManager yet.
    PartyManager* m_partyManager;
    //int m_currentCharacterIndex = 0; 
    Party m_currentParty;


    QTcpSocket* m_clientSocket;
    int m_tickCounter = 0;

    lua_State* m_L;
    QTimer* m_luaTimer;

    // The recursive engine that converts Lua data types to Qt data types
    QVariant luaToVariant(lua_State* L, int index);

    // The main public/internal call to get data from a Lua file
    QVariantMap loadLuaTable(const QString& filePath, const QString& tableName);
    void loadGameResources();

    QVariantMap loadRawJsonWithWrapper(const QString& filePath);
    void loadCSVData(const QString& filePath, QList<QVariantMap>& targetList);
    //Helper functions
    void initializeGuildLeaders();
    void initializeRaceAges();
    void initializeGameState();
    void initializeConfinementStock();
    void initializeParty();

    const GameConstants::RaceStat& getStatRef(const GameConstants::RaceStats& race, const QString& statName) const;

    explicit gameStateManager(QObject *parent = nullptr);
    gameStateManager(const gameStateManager&) = delete;
    gameStateManager& operator=(const gameStateManager&) = delete;

    dataRegistry m_registry;
    QVariantMap findRaceMap(const QString& raceName) const;
    GameConstants::RaceStats createRaceFromVariant(const QVariant& data) const;
    QString statusKey(GameConstants::EntityStatus effect) const;

public:
    QPixmap getFontSpriteSheet() const { return m_fontSpriteSheet; }

    void addItemToInventory(const QString& itemName);
    //Character getCurrentCharacter() const;
    //bool hasLivingCharacters() const;
    virtual ~gameStateManager();
    QList<QVariantMap> getMonsterData() const { return m_monsterData; }

    void setGameMode(GameConstants::GameMode newMode);
    void enterLocation(GameConstants::CityLocation location);
    // Helper to get the current state for UI (if needed)
    GameConstants::GameMode currentMode() const { return m_currentMode; }

    QList<Character>& getPartyMembers();
    Party& getParty();
    const QList<Character>& getPC() const;
    bool isActiveCharacterInCity() const;
    void setCurrentMana(int value);
    int getMaxMana() const;
    int getCurrentMana() const;
    QVariantMap getItemStats(const QString& itemName) const;
    Character& getPartyMember(int index);
    bool isWholePartyDead() const;
    int getMonsterXpReward(const QString& monsterName);
    void addExperienceToParty(int totalXp);
    void addExperienceToCharacter(int index, int amount);
    Character getCurrentCharacter() const;
    bool hasLivingCharacters() const;
    int getCurrentCharacterIndex() const { return m_currentCharacterIndex; }
    void setCurrentCharacterIndex(int index) { m_currentCharacterIndex = index; }
    bool savePartyToFile(const QString& filePath);
    bool loadPartyFromFile(const QString& filePath);
    void addCharacterToParty(const Character& character);
    bool loadLuaScript(const QString& filePath);
    QString getLuaString(const QString& variableName);
    void saveCharacterToLua(const Character& character, const QString& filePath);
    Character loadCharacterFromLua(const QString& filePath);
    static gameStateManager* instance();
    bool loadGameConfig(const QString& filePath);
    void refreshUI();
    // --- Race and Stat Definitions ---
    QVector<QString> getAvailableRaces() const;
    int getRaceMin(const QString& raceName, const QString& statName) const;
    int getRaceMax(const QString& raceName, const QString& statName) const;
    int getRaceStart(const QString& raceName, const QString& statName) const {
        for (const auto& race : m_raceDefinitions) {
            if (race.raceName == raceName) return getStatRef(race, statName).start;
        }
        return 1;
    }
    bool isAlignmentAllowed(const QString& raceName, const QString& alignmentName) const {
        for (const auto& race : m_raceDefinitions) {
            if (race.raceName == raceName) {
                if (alignmentName == "Good")    return race.good == GameConstants::AS_Allowed;
                if (alignmentName == "Neutral") return race.neutral == GameConstants::AS_Allowed;
                if (alignmentName == "Evil")    return race.evil == GameConstants::AS_Allowed;
            }
        }
        return false;
    }
    bool isGuildAllowed(const QString& raceName, const QString& guildName) const {
        for (const auto& race : m_raceDefinitions) {
            if (race.raceName == raceName) {
                return race.guildEligibility.value(guildName, GameConstants::AS_Allowed) == GameConstants::AS_Allowed;
            }
        }
        return true;
    }
    void loadRaceDefinitions();
    const QVector<GameConstants::RaceStats>& getRaceDefinitions() const { return m_raceDefinitions; }
    GameConstants::RaceStats getStatsForRace(const QString& raceName) const;
    // --- Font and Rendering ---
    void loadFontSprite(const QString& path);
    void drawCustomText(QPainter* painter, const QString& text, const QPoint& position);
    void setProportionalFont(const QFont& font);
    void setFixedFont(const QFont& font);
    QFont getProportionalFont() const { return fontManager::instance()->proportionalFont(); }
    QFont getFixedFont() const { return fontManager::instance()->fixedFont(); }
    // --- Party and World Objects ---
    struct PlacedItem {
        int level; int x; int y; QString itemName;
    };
    void addPlacedItem(int level, int x, int y, const QString& name) {
        m_placedItems.append({level, x, y, name});
    }
    QList<PlacedItem> getPlacedItems() const { return m_placedItems; }
    QVector<GameConstants::RaceStats> m_raceDefinitions;
    // --- Character Management ---
    void setCharacterGold(int index, qulonglong newGold);
    void updateCharacterGold(int characterIndex, qulonglong amount, bool add = true);
    void updatePartyMemberHP(int index, int newHP);
    bool readyBodyForResurrection(const QString& characterName);
    // --- Aging and Progression ---
    void incrementPartyAge(int years = 1);
    void processAgingConsequences();
    int getMaxAgeForRace(const QString& raceName) const;
    bool isCharacterPastMaxAge(int index) const;
    static QMap<QString, int> getRaceAgeLimits();
    void addCharacterExperience(qulonglong amount);
    // --- City/Dungeon State ---
    void setInCity(bool inCity) { setGameValue("inCity", inCity); }
    bool isInCity() const { return m_gameStateData.value("inCity", false).toBool(); }
    // --- Data Accessors (M-DATA) ---
    void loadGameData(const QString& filePath);
    void listGameData();
    const QList<QVariantMap>& gameData() const { return m_gameData; }
    void loadSpellData(const QString& filePath);
    const QList<QVariantMap>& spellData() const { return m_spellData; }
    void loadItemData(const QString& filePath);
    const QList<QVariantMap>& itemData() const { return m_itemData; }
    void loadMonsterData(const QString& filePath);
    const QList<QVariantMap>& monsterData() const { return m_monsterData; }
    QVariantMap getGame(int index) const {
        return (index >= 0 && index < m_gameData.size()) ? m_gameData[index] : QVariantMap();
    }
    QVariantMap getSpell(int index) const {
        return (index >= 0 && index < m_spellData.size()) ? m_spellData[index] : QVariantMap();
    }
    QVariantMap getItem(int index) const {
        return (index >= 0 && index < m_itemData.size()) ? m_itemData[index] : QVariantMap();
    }
    QVariantMap getMonster(int index) const {
        return (index >= 0 && index < m_monsterData.size()) ? m_monsterData[index] : QVariantMap();
    }
    // --- Inventory and Stock ---
    void incrementStock(const QString& name);
    void decrementStock(const QString& name);
    QMap<QString, int> getConfinementStock() const;
    void setBankInventory(const QStringList& items);
    QStringList getBankInventory() const;
    void setCharacterInventory(int characterIndex, const QStringList& items);
    void addItemToCharacter(int characterIndex, const QString& itemName);
    // --- Persistence ---
    bool loadCharacterFromFile(const QString& characterName);
    bool saveCharacterToFile(int partyIndex);
    bool verifySaveGame(const QString& characterName);
    bool repairSaveGame(const QString& characterName);
    void startAutosave(int intervalms = 10000);
    void stopAutosave();
    // --- Global Values System ---
    void setGameValue(const QString& key, const QVariant& value);
    QVariant getGameValue(const QString& key) const;
    void logGuildAction(const QString& actionDescription);
    // --- Status Flags ---
    // For World Events: setGlobalStatus("WorldOnFire", true);
    void setGlobalStatus(const QString& key, bool active) {
        setGameValue("status_" + key, active);
    }
    bool hasGlobalStatus(const QString& key) const {
        return getGameValue("status_" + key).toBool();
    }
    void setCharacterStatus(int index, GameConstants::EntityStatus effect, bool active);
    bool hasCharacterStatus(int index, GameConstants::EntityStatus effect) const;
    void setStatus(GameConstants::EntityStatus effect, bool active);
    bool hasStatus(GameConstants::EntityStatus effect) const;
    void clearAllStatuses();
    void setCharacterPoisoned(bool isPoisoned);
    bool isCharacterPoisoned() const;
    void setCharacterBlinded(bool isBlinded);
    bool isCharacterBlinded() const;
    void setCharacterOnFire(bool isOnFire);
    bool isCharacterOnFire() const;
    void setIsAlive(int alive) { setGameValue("isAlive", alive); }
    int getIsAlive() const { return m_gameStateData.value("isAlive").toInt(); }
    void setMaxMana(int maxMana) { setGameValue("MaxMana", maxMana); }
    bool spendMana(int cost) {
        int current = getCurrentMana();
        if (current >= cost) {
            setCurrentMana(current - cost);
            return true;
        }
        return false;
    }
    void restoreMana(int amount) {
        setCurrentMana(getCurrentMana() + amount);
    }
    // --- Diagnostics ---
    void performSanityCheck();
    void printAllGameState() const;
    bool areResourcesLoaded() const;

signals:
    void gameValueChanged(const QString& key, const QVariant& value);
    void fontChanged();

private slots:
    void handleAutosave();
    void onLuaTimerTick();
    void onServerDataReceived();

private:
    int m_currentCharacterIndex = 0;
    QList<PlacedItem> m_placedItems;
    QVariantMap m_gameStateData;
    QMap<QString, int> m_confinementStock;
    QList<QVariantMap> m_gameData;
    QList<QVariantMap> m_spellData;
    QList<QVariantMap> m_itemData;
    QList<QVariantMap> m_characterData;
    QList<QVariantMap> m_monsterData;
    QList<QVariantMap> m_generalstoreData;
    QList<QVariantMap> m_guildmastersData;
    QList<QVariantMap> m_automapData;
    QList<QVariantMap> m_guildlogsData;
    QList<QVariantMap> m_dungeonstuffData;
    QList<QVariantMap> m_dungeonlayoutData;
    QList<QVariantMap> m_existingpartiesData;
    QList<QVariantMap> m_libraryentriesData;
    QList<QVariantMap> m_hallofrecordsData;
    QList<QVariantMap> m_confinementcreaturesData;
    
    QTimer *m_autosaveTimer = nullptr;
};

#endif // gameStateManager_H
