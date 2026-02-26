#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

// Project Includes
#include "include/GameConstants.h"
#include "DataRegistry.h"
#include "AudioManager.h"
#include "character.h"

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

class GameStateManager : public QObject
{
    Q_OBJECT

private:
    QVariantMap loadRawJsonWithWrapper(const QString& filePath);
    void loadCSVData(const QString& filePath, QList<QVariantMap>& targetList);
    //Helper functions
    void initializeGuildLeaders();
    void initializeRaceAges();
    void initializeGameState();
    void initializeConfinementStock();
    void initializeParty();

    const GameConstants::RaceStat& getStatRef(const GameConstants::RaceStats& race, const QString& statName) const;
    QPixmap m_fontSpriteSheet;
    static constexpr int FONT_CHAR_WIDTH = 32;
    static constexpr int FONT_CHAR_HEIGHT = 42;

    explicit GameStateManager(QObject *parent = nullptr);
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;

    DataRegistry m_registry; 
    QVariantMap findRaceMap(const QString& raceName) const;
    GameConstants::RaceStats createRaceFromVariant(const QVariant& data) const;

public:
    static GameStateManager* instance();
    bool loadGameConfig(const QString& filePath);
    void refreshUI();

    // --- Race and Stat Definitions ---
    QVector<QString> getAvailableRaces() const;
    int getRaceMin(const QString& raceName, const QString& statName) const;
    int getRaceMax(const QString& raceName, const QString& statName) const;
    
    // RESTORED: These were commented out in your .cpp, so they must be inline or implemented
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
    QFont getProportionalFont() const { return m_proportionalFont; }
    QFont getFixedFont() const { return m_fixedFont; }

    // --- Party and World Objects ---
    static const int MAX_PARTY_SIZE = 4;
    struct PlacedItem {
        int level; int x; int y; QString itemName;
    };

    void addPlacedItem(int level, int x, int y, const QString& name) {
        m_placedItems.append({level, x, y, name});
    }
    QList<PlacedItem> getPlacedItems() const { return m_placedItems; }
    QVector<GameConstants::RaceStats> m_raceDefinitions;

    // --- Character Management ---
    QList<Character>& getPC() { return m_PC; }
    const QList<Character>& getPC() const { return m_PC; }
    void setCharacterGold(int index, qulonglong newGold);
    void updateCharacterGold(int characterIndex, qulonglong amount, bool add = true);
    void updatePartyMemberHP(int index, int newHP);
//    void syncActiveCharacterToParty();
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
    
    bool isActiveCharacterInCity() const {
        if (m_PC.isEmpty()) return false;
        return m_PC[0].DungeonLevel == 0;
    }

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
    void setCharacterPoisoned(bool isPoisoned);
    bool isCharacterPoisoned() const;
    void setCharacterBlinded(bool isBlinded);
    bool isCharacterBlinded() const;
    void setCharacterOnFire(bool isOnFire);
    bool isCharacterOnFire() const;
    void setIsAlive(int alive) { setGameValue("isAlive", alive); }
    int getIsAlive() const { return m_gameStateData.value("isAlive").toInt(); }

    // --- Mana Management ---
    int getCurrentMana() const {
        int idx = m_gameStateData.value("ActiveCharacterIndex", 0).toInt();
        if (idx >= 0 && idx < m_PC.size()) return m_PC[idx].mana;
        return m_gameStateData.value("CurrentMana", 0).toInt();
    }

    int getMaxMana() const {
        int idx = m_gameStateData.value("ActiveCharacterIndex", 0).toInt();
        if (idx >= 0 && idx < m_PC.size()) return m_PC[idx].maxMana;
        return m_gameStateData.value("MaxMana", 0).toInt();
    }

    void setCurrentMana(int mana) {
        int cappedMana = qMax(0, qMin(mana, getMaxMana()));
        int idx = m_gameStateData.value("ActiveCharacterIndex", 0).toInt();
        if (idx >= 0 && idx < m_PC.size()) m_PC[idx].mana = cappedMana;
        else setGameValue("CurrentMana", cappedMana);
    }

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

private:
    QFont m_proportionalFont;
    QFont m_fixedFont;

    QVariantList party; 
    QList<Character> m_PC;
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

#endif // GAMESTATEMANAGER_H
