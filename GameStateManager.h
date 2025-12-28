// GameStateManager.h
#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include <QObject>
#include <QVariantMap>
#include <QtGlobal>

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
    // Improved thread-safe static accessor
    static GameStateManager* instance();
    static const int MAX_PARTY_SIZE = 4;
    
signals:
    void gameValueChanged(const QString& key, const QVariant& value);

private:
    QVariantMap m_gameStateData;
    QMap<QString, int> m_confinementStock;
    QList<QVariantMap> m_monsterData; // In-memory storage for MDATA5
    QList<QVariantMap> m_itemData;    // In-memory storage for MDATA3
    QList<QVariantMap> m_spellData;    // In-memory storage for MDATA2
    QList<QVariantMap> m_gameData;    // In-memory storage for MDATA1

public:
    QVariantMap getCharacterData(int index) const;
    void updateCharacterData(int index, const QVariantMap& data);

    // Game Data Method
    void loadGameData(const QString& filePath); 

    const QList<QVariantMap>& gameData() const { return m_gameData; }
    int gameCount() const { return m_gameData.size(); }
    QVariantMap getGame(int index) const {
        return (index >= 0 && index < m_gameData.size()) ? m_gameData[index] : QVariantMap();
    }

    // Spell Data Method
    void loadSpellData(const QString& filePath); 

    const QList<QVariantMap>& spellData() const { return m_spellData; }
    int spellCount() const { return m_spellData.size(); }
    QVariantMap getSpell(int index) const {
        return (index >= 0 && index < m_spellData.size()) ? m_spellData[index] : QVariantMap();
    }


    // Item Data Method
    void loadItemData(const QString& filePath);

    const QList<QVariantMap>& itemData() const { return m_itemData; }
    int itemCount() const { return m_itemData.size(); }
    QVariantMap getItem(int index) const {
        return (index >= 0 && index < m_itemData.size()) ? m_itemData[index] : QVariantMap();
    }

    // Monster Data Methods
    void loadMonsterData(const QString& filePath);
    const QList<QVariantMap>& monsterData() const { return m_monsterData; }
    int monsterCount() const { return m_monsterData.size(); }
    QVariantMap getMonster(int index) const {
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
    static QStringList statNames() {
        return {"Strength", "Intelligence", "Wisdom", "Constitution", "Charisma", "Dexterity"};
    }
    static int defaultStatPoints() { return 5; }
    static QStringList alignmentNames() {
        return {"Good", "Neutral", "Evil"};
    }
    static QStringList sexOptions() {
        return {"Male", "Female"};
    }
    static int defaultAlignmentIndex() {
        return 1; // Default to "Neutral"
    }
};

#endif // GAMESTATEMANAGER_H
