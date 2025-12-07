// GameStateManager.cpp
#include "GameStateManager.h"
#include <QDebug> // For logging/debugging
#include <QVariantList> // Added for guild leader initialization
#include <QVariantMap> // Added for guild leader initialization
#include <QVariant> // Ensure QVariant::fromValue is available if not in the others
#include <QMapIterator> // Added for iterating and printing the entire map
#include <QDateTime> // NEW: Added for logging guild actions with a timestamp

// Static pointer to hold the single instance
GameStateManager* gsm_instance = nullptr;

// Public static accessor function
GameStateManager* GameStateManager::instance()
{
    if (!gsm_instance) {
        gsm_instance = new GameStateManager();
    }
    return gsm_instance;
}

// Private constructor
GameStateManager::GameStateManager(QObject *parent) 
    : QObject(parent) 
{

    m_gameStateData["CurrentCharacterLevel"] = 1;
    // --- ADDED: Initialize Character Experience state ---
    // Use qulonglong for the experience amount to support large values
    m_gameStateData["CurrentCharacterExperience"] = QVariant::fromValue((qulonglong)0); 
    
    // Initialize default state data
    m_gameStateData["ResourcesLoaded"] = false;
    m_gameStateData["GameVersion"] = "1.1.7.6.450";
    m_gameStateData["PlayerScore"] = 0;
    
    // User-requested initial states (based on final values in the provided log)
    m_gameStateData["DungeonLevel"] = 1;
    m_gameStateData["DungeonX"] = 17;
    m_gameStateData["DungeonY"] = 12;
    // Note: QVariant::fromValue is used for qulonglong to ensure correct storage type
    m_gameStateData["PlayerGold"] = QVariant::fromValue((qulonglong)0);

    // --- NEW: Initialize Banked Gold state ---
    // Use the same type as PlayerGold for consistency.
    m_gameStateData["BankedGold"] = QVariant::fromValue((qulonglong)0);
    
    // Initialize Character Creation Defaults
    m_gameStateData["CurrentCharacterName"] = "";
    m_gameStateData["CurrentCharacterRace"] = "";
    m_gameStateData["CurrentCharacterSex"] = "";
    m_gameStateData["CurrentCharacterAlignment"] = "";
    m_gameStateData["CurrentCharacterGuild"] = "";
    
    // Initialize Stats (using 0 as a default value)
    m_gameStateData["CurrentCharacterStrength"] = 0;
    m_gameStateData["CurrentCharacterIntelligence"] = 0;
    m_gameStateData["CurrentCharacterWisdom"] = 0;
    m_gameStateData["CurrentCharacterConstitution"] = 0;
    m_gameStateData["CurrentCharacterCharisma"] = 0;
    m_gameStateData["CurrentCharacterDexterity"] = 0;

    // Initialize remaining stat points
    m_gameStateData["CurrentCharacterStatPointsLeft"] = 0;

    // --- NEW: Initialize Character Status States ---
    m_gameStateData["CharacterPoisoned"] = false;
    m_gameStateData["CharacterBlinded"] = false;
    // ----------------------------------------------

    // --- ADDED: Initialize the Guild Action Log ---
    m_gameStateData["GuildActionLog"] = QVariantList();

    // -------------------------------------------------------------------------
    // Initialize Guild Leaders for Hall of Records (UPDATED FROM IMAGE)
    // Stored as a QVariantList of QVariantMap objects
    // -------------------------------------------------------------------------
    QVariantList guildLeadersList;

    // Record 1: Strongest
    QVariantMap record1;
    record1["Achievement"] = "Strongest";
    record1["Name"] = "Goch";
    record1["Date"] = "4/15/2001";
    record1["RecordValue"] = 25;
    record1["RecordUnit"] = "Strength";
    guildLeadersList.append(record1);

    // Record 2: Smartest
    QVariantMap record2;
    record2["Achievement"] = "Smartest";
    record2["Name"] = "Tuadar";
    record2["Date"] = "5/9/2001";
    record2["RecordValue"] = 25;
    record2["RecordUnit"] = "Intelligence";
    guildLeadersList.append(record2);

    // Record 3: Wisest
    QVariantMap record3;
    record3["Achievement"] = "Wisest";
    record3["Name"] = "Tuadar";
    record3["Date"] = "11/24/2000";
    record3["RecordValue"] = 25;
    record3["RecordUnit"] = "Wisdom";
    guildLeadersList.append(record3);

    // Record 4: Healthiest
    QVariantMap record4;
    record4["Achievement"] = "Healthiest";
    record4["Name"] = "Spore";
    record4["Date"] = "5/17/2002";
    record4["RecordValue"] = 22;
    record4["RecordUnit"] = "Constitution";
    guildLeadersList.append(record4);
    
    // Record 5: Most Attractive
    QVariantMap record5;
    record5["Achievement"] = "Most Attractive";
    record5["Name"] = "Tuadar";
    record5["Date"] = "4/17/2001";
    record5["RecordValue"] = 23;
    record5["RecordUnit"] = "Charisma";
    guildLeadersList.append(record5);

    // Record 6: Quickest
    QVariantMap record6;
    record6["Achievement"] = "Quickest";
    record6["Name"] = "Healer";
    record6["Date"] = "5/17/2002";
    record6["RecordValue"] = 27;
    record6["RecordUnit"] = "Dexterity";
    guildLeadersList.append(record6);
    
    // Record 7: Deadliest Creature Defeated
    QVariantMap record7;
    record7["Achievement"] = "Deadliest Creature Defeated";
    record7["Name"] = "Healer";
    record7["Date"] = "5/17/2002";
    record7["RecordValue"] = "Giant Leech";
    record7["RecordUnit"] = "Creature";
    guildLeadersList.append(record7);

    // Record 8: Most Experienced Explorer
    QVariantMap record8;
    record8["Achievement"] = "Most Experienced Explorer";
    record8["Name"] = "Crashland";
    record8["Date"] = "5/17/2002";
    // Using QVariant::fromValue for the large total experience number.
    record8["RecordValue"] = QVariant::fromValue((qulonglong)1322451);
    record8["RecordUnit"] = "Total Experience";
    guildLeadersList.append(record8);

    // Record 9: Wealthiest Explorer
    QVariantMap record9;
    record9["Achievement"] = "Wealthiest Explorer";
    record9["Name"] = "Tuadar";
    record9["Date"] = "5/14/2001";
    // Using QVariant::fromValue for the large gold amount.
    record9["RecordValue"] = QVariant::fromValue((qulonglong)1022531528);
    record9["RecordUnit"] = "Gold in the Bank";
    guildLeadersList.append(record9);
    
    // Record 10: Master of Fighting (Original RecordHolder)
    QVariantMap record10;
    record10["Achievement"] = "Master of Fighting";
    record10["Name"] = "Morgul";
    record10["Date"] = "Original RecordHolder";
    record10["RecordValue"] = 17;
    record10["RecordUnit"] = "Fighting Skill";
    guildLeadersList.append(record10);
    
    // Record 11: Master of Magic
    QVariantMap record11;
    record11["Achievement"] = "Master of Magic";
    record11["Name"] = "Spore";
    record11["Date"] = "5/16/2002";
    record11["RecordValue"] = 17;
    record11["RecordUnit"] = "Spell Knowledge & Power";
    guildLeadersList.append(record11);
    
    // Record 12: Master of Thieving
    QVariantMap record12;
    record12["Achievement"] = "Master of Thieving";
    record12["Name"] = "Healer";
    record12["Date"] = "5/17/2002";
    record12["RecordValue"] = 22;
    record12["RecordUnit"] = "Thieving Skill";
    guildLeadersList.append(record12);


    // Store the complete list in the Game State Manager
    m_gameStateData["GuildLeaders"] = guildLeadersList;

    // -------------------------------------------------------------------------
    
    qDebug() << "GameStateManager initialized with default and user-specified states.";
}

// -------------------------------------------------------------------------
// EXISTING METHOD IMPLEMENTATION: addCharacterExperience(qulonglong amount)
// -------------------------------------------------------------------------
void GameStateManager::addCharacterExperience(qulonglong amount)
{
    // 1. Get the current experience value. It's stored as qulonglong.
    QVariant currentExpVariant = getGameValue("CurrentCharacterExperience");
    qulonglong currentExp = currentExpVariant.isValid() 
                            ? currentExpVariant.value<qulonglong>() 
                            : 0;

    // 2. Calculate the new experience
    qulonglong newExp = currentExp + amount;

    // 3. Store the new value back into the state manager
    QVariant newExpVariant = QVariant::fromValue(newExp);
    m_gameStateData["CurrentCharacterExperience"] = newExpVariant;
    
    qDebug() << "Experience added:" << amount 
             << " | New Total Experience:" << newExp;
}

// -------------------------------------------------------------------------
// EXISTING METHOD IMPLEMENTATION: logGuildAction(const QString& actionDescription)
// -------------------------------------------------------------------------
void GameStateManager::logGuildAction(const QString& actionDescription)
{
    // 1. Get the current log list
    QVariantList logList = m_gameStateData.value("GuildActionLog").toList();

    // 2. Format the new entry with a timestamp
    QString timestampedAction = QDateTime::currentDateTime().toString("HH:mm:ss") + " - " + actionDescription;

    // 3. Append the new action to the list
    logList.append(timestampedAction);

    // 4. Update the game state
    m_gameStateData["GuildActionLog"] = logList;

    qDebug() << "Guild Action Logged:" << timestampedAction;
}

// -------------------------------------------------------------------------
// EXISTING METHOD IMPLEMENTATION: printAllGameState()
// -------------------------------------------------------------------------
void GameStateManager::printAllGameState() const
{
    qDebug() << "--- START OF GAME STATE DUMP ---";
    
    // QVariantMap is a QMap<QString, QVariant>, so we can use QMapIterator
    QMapIterator<QString, QVariant> i(m_gameStateData);
    
    while (i.hasNext()) {
        i.next();
        // Print the key, the value, and the underlying data type name
        qDebug() << "Key:" << i.key() 
                 << " | Value:" << i.value() 
                 << " | Type:" << i.value().typeName();
    }
    
    qDebug() << "--- END OF GAME STATE DUMP ---";
}

// Implement public interface methods
void GameStateManager::setGameValue(const QString& key, const QVariant& value)
{
    m_gameStateData[key] = value;
    qDebug() << "State updated:" << key << "=" << value;
}

QVariant GameStateManager::getGameValue(const QString& key) const
{
    return m_gameStateData.value(key);
}

bool GameStateManager::areResourcesLoaded() const
{
    return m_gameStateData.value("ResourcesLoaded").toBool();
}

// -------------------------------------------------------------------------
// NEW METHOD IMPLEMENTATIONS: Character Status Management
// -------------------------------------------------------------------------

void GameStateManager::setCharacterPoisoned(bool isPoisoned)
{
    setGameValue("CharacterPoisoned", isPoisoned);
    qDebug() << "Character status updated: Poisoned =" << isPoisoned;
}

bool GameStateManager::isCharacterPoisoned() const
{
    // The key "CharacterPoisoned" holds a QVariant, .toBool() provides the status.
    return m_gameStateData.value("CharacterPoisoned").toBool();
}

void GameStateManager::setCharacterBlinded(bool isBlinded)
{
    setGameValue("CharacterBlinded", isBlinded);
    qDebug() << "Character status updated: Blinded =" << isBlinded;
}

bool GameStateManager::isCharacterBlinded() const
{
    // The key "CharacterBlinded" holds a QVariant, .toBool() provides the status.
    return m_gameStateData.value("CharacterBlinded").toBool();
}
