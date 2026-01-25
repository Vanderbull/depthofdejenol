#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include "character.h"
#include <QObject>
#include <QVariantMap>
#include <QtGlobal>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

class GameStateManager : public QObject
{
    Q_OBJECT

private:
    // Private constructor
    explicit GameStateManager(QObject *parent = nullptr);
    // Prevent copy and assignment
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;

public:
    // --- Global Audio Methods ---
    void playMusic(const QString& fileName, bool loop = true);
    void stopMusic();
    void setVolume(float volume); // 0.0 to 1.0

    // Improved thread-safe static accessor
    static GameStateManager* instance();
    static const int MAX_PARTY_SIZE = 4;
    struct PlacedItem 
    {
        int level;
        int x;
        int y;
        QString itemName;
    };

    void addPlacedItem(int level, int x, int y, const QString& name) 
    {
        m_placedItems.append({level, x, y, name});
    }

    QList<PlacedItem> getPlacedItems() const { return m_placedItems; }
    
signals:
    void gameValueChanged(const QString& key, const QVariant& value);

private:
    QMediaPlayer* m_globalPlayer = nullptr;
    QAudioOutput* m_globalAudioOutput = nullptr;

    QVariantList party;
    QList<Character> m_PC;
    QList<PlacedItem> m_placedItems;
    QVariantMap m_gameStateData;
    QMap<QString, int> m_confinementStock;
    QList<QVariantMap> m_confinementcreaturesData; // In-memory storage for MDATA15
    QList<QVariantMap> m_hallofrecordsData; // In-memory storage for MDATA14
    QList<QVariantMap> m_libraryentriesData; // In-memory storage for MDATA13
    QList<QVariantMap> m_existingpartiesData; // In-memory storage for MDATA12
    QList<QVariantMap> m_dungeonlayoutData; // In-memory storage for MDATA11
    QList<QVariantMap> m_dungeonstuffData; // In-memory storage for MDATA10
    QList<QVariantMap> m_guildlogsData; // In-memory storage for MDATA9
    QList<QVariantMap> m_automapData; // In-memory storage for MDATA8
    QList<QVariantMap> m_guildmastersData; // In-memory storage for MDATA7
    QList<QVariantMap> m_generalstoreData; // In-memory storage for MDATA6
    QList<QVariantMap> m_monsterData; // In-memory storage for MDATA5
    QList<QVariantMap> m_characterData; // In-memory storage for MDATA4
    QList<QVariantMap> m_itemData;    // In-memory storage for MDATA3
    QList<QVariantMap> m_spellData;    // In-memory storage for MDATA2
    QList<QVariantMap> m_gameData;    // In-memory storage for MDATA1
    QTimer *m_autosaveTimer = nullptr;
    

public:
    /**
     * @brief Increments the age of all party members.
     * Called during time skips, long rests, or yearly transitions.
     */
    void incrementPartyAge(int years = 1);

    /**
     * @brief Checks if any party member has died of old age and handles the state change.
     */
    void processAgingConsequences();

    // --- Age Management ---
    /**
     * @brief Returns the maximum age for a given race name.
     */
    int getMaxAgeForRace(const QString& raceName) const;

    /**
     * @brief Checks if a specific character in the party has exceeded their natural lifespan.
     */
    bool isCharacterPastMaxAge(int index) const;

    /**
     * @brief Static helper to get the full mapping of ages.
     */
    static QMap<QString, int> getRaceAgeLimits();

    // Add to the public section of GameStateManager.h
    float getVolume() const {
        return m_globalAudioOutput ? m_globalAudioOutput->volume() : 0.5f;
    }


    bool isActiveCharacterInCity() const {
        if (m_PC.isEmpty()) return false;
        return m_PC[0].DungeonLevel == 0;
    }
    // Getter to provide access to the party list
    QList<Character>& getPC() { return m_PC; }
    const QList<Character>& getPC() const { return m_PC; }

    /**
     * @brief Directly sets the gold for a specific character in the party.
     * @param index The party slot (0 to MAX_PARTY_SIZE - 1)
     * @param newGold the exact amount of gold to set
     */
    void setCharacterGold(int index, qulonglong newGold) {
        if (index >= 0 && index < m_PC.size()) {
            m_PC[index].Gold = newGold;
            
            // If this is the active character (usually index 0), 
            // sync it to the global game state for UI updates.
            if (index == 0) {
                setGameValue("Gold", newGold);
            }
            qDebug() << "Gold for PC" << index << "set to:" << newGold;
        }
    }

    // Logic to "Ready" a body into the active party for resurrection
    bool readyBodyForResurrection(const QString& characterName);

    void setInCity(bool inCity) 
    {
        setGameValue("inCity", inCity);
    }
    bool isInCity() const 
    {
        return m_gameStateData.value("inCity", false).toBool();
    }
    QVariantMap getCharacterData(int index) const;
    void updateCharacterData(int index, const QVariantMap& data);
    // Game Data Method
    void loadGameData(const QString& filePath); 
    const QList<QVariantMap>& gameData() const { return m_gameData; }
    int gameCount() const { return m_gameData.size(); }
    QVariantMap getGame(int index) const 
    {
        return (index >= 0 && index < m_gameData.size()) ? m_gameData[index] : QVariantMap();
    }
    // Spell Data Method
    void loadSpellData(const QString& filePath); 
    const QList<QVariantMap>& spellData() const { return m_spellData; }
    int spellCount() const { return m_spellData.size(); }
    QVariantMap getSpell(int index) const 
    {
        return (index >= 0 && index < m_spellData.size()) ? m_spellData[index] : QVariantMap();
    }
    // Item Data Method
    void loadItemData(const QString& filePath);
    const QList<QVariantMap>& itemData() const { return m_itemData; }
    int itemCount() const { return m_itemData.size(); }
    QVariantMap getItem(int index) const 
    {
        return (index >= 0 && index < m_itemData.size()) ? m_itemData[index] : QVariantMap();
    }
    // Monster Data Methods
    void loadMonsterData(const QString& filePath);
    const QList<QVariantMap>& monsterData() const { return m_monsterData; }
    int monsterCount() const { return m_monsterData.size(); }
    QVariantMap getMonster(int index) const 
    {
        return (index >= 0 && index < m_monsterData.size()) ? m_monsterData[index] : QVariantMap();
    }
    void performSanityCheck();
    void incrementStock(const QString& name);
    void decrementStock(const QString& name);
    QMap<QString, int> getConfinementStock() const;
    void setGameValue(const QString& key, const QVariant& value);
    QVariant getGameValue(const QString& key) const;
    void addCharacterExperience(qulonglong amount);
    void logGuildAction(const QString& actionDescription);
    void printAllGameState() const;
    bool areResourcesLoaded() const;
    // Character Status Methods
    void setCharacterPoisoned(bool isPoisoned);
    bool isCharacterPoisoned() const;
    void setCharacterBlinded(bool isBlinded);
    bool isCharacterBlinded() const;
    void setCharacterOnFire(bool isOnFire);
    bool isCharacterOnFire() const;
    void setIsAlive(int alive) 
    {
        setGameValue("isAlive", alive);
    }
    int getIsAlive() const 
    {
        return m_gameStateData.value("isAlive").toInt();
    }
    static QStringList statNames() 
    {
        return {"Strength", "Intelligence", "Wisdom", "Constitution", "Charisma", "Dexterity"};
    }
    static int defaultStatPoints() { return 5; }
    static QStringList alignmentNames() 
    {
        return {"Good", "Neutral", "Evil"};
    }
    static QStringList sexOptions() 
    {
        return {"Male", "Female"};
    }
    static int defaultAlignmentIndex() 
    {
        return 1; // Default to "Neutral"
    }
    // In GameStateManager.h public section:
    static QStringList guildNames() 
    {
        return {
            "Nomad", "Paladin", "Warrior", "Villain", 
            "Seeker", "Thief", "Scavenger", "Mage", 
            "Sorcerer", "Wizard", "Healer", "Ninja"
        };
    }
    static QMap<QString, QString> guildMasters() 
    {
        return {
            {"Nomad", "Goch"},
            {"Paladin", "Tuadar"},
            {"Warrior", "Spore"},
            {"Villain", "Darkness"},
            {"Seeker", "Healer"},
            {"Thief", "Nimblefingers"},      
            {"Scavenger", "Morgul"},         
            {"Mage", "Ge'nal"},              
            {"Sorcerer", "Ge'nal"},          
            {"Wizard", "Archmage"},
            {"Healer", "Theshal"},           
            {"Ninja", "Shadowblade"}
        };
    }
    // --- Inventory Management ---
    void setBankInventory(const QStringList& items);
    QStringList getBankInventory() const;
    void setCharacterInventory(int characterIndex, const QStringList& items);
    void addItemToCharacter(int characterIndex, const QString& itemName);
    void updateCharacterGold(int characterIndex, qulonglong amount, bool add = true);
    bool loadCharacterFromFile(const QString& characterName);
    bool saveCharacterToFile(int partyIndex);
    void updatePartyMemberHP(int index, int newHP);
    void syncActiveCharacterToParty();
    void listGameData();
    bool verifySaveGame(const QString& characterName);
    bool repairSaveGame(const QString& characterName);
    void startAutosave(int intervalms = 10000); // 10 seconds default
    void stopAutosave();
private slots:
    void handleAutosave();
};

#endif // GAMESTATEMANAGER_H
