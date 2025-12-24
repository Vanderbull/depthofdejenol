// GameStateManager.cpp
#include "GameStateManager.h"
#include <QDebug> 
#include <QVariantList> 
#include <QVariantMap> 
#include <QMapIterator> 
#include <QDateTime> 
#include <QFile>
#include <QTextStream>
#include <QIODevice>


// IMPROVEMENT: Removed the global gsm_instance pointer to use Magic Statics instead.

GameStateManager* GameStateManager::instance()
{
    // Thread-safe initialization guaranteed by C++11
    static GameStateManager instance;
    return &instance;
}

// Add these helper methods
void GameStateManager::incrementStock(const QString& name) {
    m_confinementStock[name] = m_confinementStock.value(name, 0) + 1;
}

void GameStateManager::decrementStock(const QString& name) {
    int current = m_confinementStock.value(name, 0);
    if (current > 0) m_confinementStock[name] = current - 1;
}

QMap<QString, int> GameStateManager::getConfinementStock() const {
    return m_confinementStock;
}

GameStateManager::GameStateManager(QObject *parent)
    : QObject(parent)
{
    // Initialize the Ghost Hound flag to false
    m_gameStateData["GhostHoundPending"] = false;
    m_confinementStock["Ghost hounf"] = 0;
    m_confinementStock["Skeleton"] = 0;
    m_confinementStock["Kobold"] = 1;
    m_confinementStock["Orc"] = 1;
    m_confinementStock["Clean-Up"] = 1;
    m_confinementStock["Black Bear"] = 1;
    m_confinementStock["Giant Owl"] = 1;
    m_confinementStock["Giant Spider"] = 1;
    m_confinementStock["Giant Centipede"] = 1;
    m_confinementStock["Zombie"] = 1;
    m_confinementStock["Footpad"] = 1;
    m_confinementStock["Gredlan Rogue"] = 1;

    // Load monster data from CSV at start
    loadMonsterData("tools/monsterconverter/MDATA5.csv");

    m_gameStateData["CurrentCharacterSex"] = GameStateManager::sexOptions().at(0);
    m_gameStateData["CurrentCharacterAlignment"] = GameStateManager::alignmentNames().at(GameStateManager::defaultAlignmentIndex());
    m_gameStateData["CurrentCharacterStatPointsLeft"] = GameStateManager::defaultStatPoints();
    // Max ages for each race
    m_gameStateData["MaxHumanAge"] = 100;
    m_gameStateData["MaxElfAge"] = 400;
    m_gameStateData["MaxGiantAge"] = 225;
    m_gameStateData["MaxGnomeAge"] = 300;
    m_gameStateData["MaxDwarfAge"] = 275;
    m_gameStateData["MaxOgreAge"] = 265;
    m_gameStateData["MaxMorlochAge"] = 175;
    m_gameStateData["MaxOsiriAge"] = 325;
    m_gameStateData["MaxTrollAge"] = 285;
    //
    m_gameStateData["CurrentCharacterHP"] = 50;
    m_gameStateData["MaxCharacterHP"] = 50;
    // You can also initialize a list for the whole party if needed
    m_gameStateData["PartyHP"] = QVariantList({50, 40, 30});

    // Initialize default state data
    m_gameStateData["CurrentCharacterLevel"] = 1;
    m_gameStateData["CurrentCharacterAge"] = 16; // Starting age for all races
    m_gameStateData["CurrentCharacterExperience"] = QVariant::fromValue((qulonglong)0);
    m_gameStateData["ResourcesLoaded"] = false;
    m_gameStateData["GameVersion"] = "1.1.7.6.450";
    m_gameStateData["PlayerScore"] = 0;
    m_gameStateData["DungeonLevel"] = 1;
    m_gameStateData["DungeonX"] = 17;
    m_gameStateData["DungeonY"] = 12;
    m_gameStateData["CurrentCharacterGold"] = QVariant::fromValue((qulonglong)1500);
    m_gameStateData["BankedGold"] = QVariant::fromValue((qulonglong)0);
    
    // Character Stats
    m_gameStateData["CurrentCharacterName"] = "";
    m_gameStateData["CurrentCharacterRace"] = "";
    m_gameStateData["CurrentCharacterGuild"] = "";
    m_gameStateData["CurrentCharacterStrength"] = 0;
    m_gameStateData["CurrentCharacterIntelligence"] = 0;
    m_gameStateData["CurrentCharacterWisdom"] = 0;
    m_gameStateData["CurrentCharacterConstitution"] = 0;
    m_gameStateData["CurrentCharacterCharisma"] = 0;
    m_gameStateData["CurrentCharacterDexterity"] = 0;
    m_gameStateData["CurrentCharacterStatPointsLeft"] = 0;

    // Status States
    m_gameStateData["CharacterPoisoned"] = false;
    m_gameStateData["CharacterBlinded"] = false;
    m_gameStateData["CharacterDiseased"] = false;
    m_gameStateData["CharacterDied"] = false;

    m_gameStateData["GuildActionLog"] = QVariantList();

    // Initialize Guild Leaders (Hall of Records)
    QVariantList guildLeadersList;

    auto addLeader = [&](QString ach, QString name, QString date, QVariant val, QString unit) {
        QVariantMap record;
        record["Achievement"] = ach;
        record["Name"] = name;
        record["Date"] = date;
        record["RecordValue"] = val;
        record["RecordUnit"] = unit;
        guildLeadersList.append(record);
    };

    addLeader("Strongest", "Goch", "4/15/2001", 25, "Strength");
    addLeader("Smartest", "Tuadar", "5/9/2001", 25, "Intelligence");
    addLeader("Wisest", "Tuadar", "11/24/2000", 25, "Wisdom");
    addLeader("Healthiest", "Spore", "5/17/2002", 22, "Constitution");
    addLeader("Most Attractive", "Tuadar", "4/17/2001", 23, "Charisma");
    addLeader("Quickest", "Healer", "5/17/2002", 27, "Dexterity");
    addLeader("Deadliest Creature Defeated", "Healer", "5/17/2002", "Giant Leech", "Creature");
    addLeader("Most Experienced Explorer", "Crashland", "5/17/2002", QVariant::fromValue((qulonglong)1322451), "Total Experience");
    addLeader("Wealthiest Explorer", "Tuadar", "5/14/2001", QVariant::fromValue((qulonglong)1022531528), "Gold in the Bank");
    addLeader("Master of Fighting", "Morgul", "Original RecordHolder", 17, "Fighting Skill");
    addLeader("Master of Magic", "Spore", "5/16/2002", 17, "Spell Knowledge & Power");
    addLeader("Master of Thieving", "Healer", "5/17/2002", 22, "Thieving Skill");

    m_gameStateData["GuildLeaders"] = guildLeadersList;
    qDebug() << "GameStateManager initialized.";
}

void GameStateManager::loadMonsterData(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open monster data file:" << filePath;
        return;
    }

    m_monsterData.clear();
    QTextStream in(&file);
    
    // Read header line
    if (in.atEnd()) return;
    QString headerLine = in.readLine();
    QStringList headers = headerLine.split(',');

    // Read data lines
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        
        QStringList fields = line.split(',');
        if (fields.size() != headers.size()) {
            // Basic error handling for malformed lines
            continue; 
        }

        QVariantMap monster;
        for (int i = 0; i < headers.size(); ++i) {
            monster[headers[i].trimmed()] = fields[i].trimmed();
        }
        m_monsterData.append(monster);
    }

    file.close();
    qDebug() << "Successfully loaded" << m_monsterData.size() << "monsters into memory.";
}

void GameStateManager::addCharacterExperience(qulonglong amount)
{
    qulonglong currentExp = getGameValue("CurrentCharacterExperience").value<qulonglong>();
    setGameValue("CurrentCharacterExperience", QVariant::fromValue(currentExp + amount));
    qDebug() << "Experience added:" << amount;
}

void GameStateManager::logGuildAction(const QString& actionDescription)
{
    QVariantList logList = m_gameStateData.value("GuildActionLog").toList();
    QString entry = QDateTime::currentDateTime().toString("HH:mm:ss") + " - " + actionDescription;
    logList.append(entry);
    m_gameStateData["GuildActionLog"] = logList;
}

void GameStateManager::printAllGameState() const
{
    qDebug() << "--- START OF GAME STATE DUMP ---";
    QMapIterator<QString, QVariant> i(m_gameStateData);
    while (i.hasNext()) {
        i.next();
        qDebug() << "Key:" << i.key() << " | Value:" << i.value();
    }
    qDebug() << "--- END OF GAME STATE DUMP ---";
}

void GameStateManager::setGameValue(const QString& key, const QVariant& value)
{
    m_gameStateData[key] = value;
    emit gameValueChanged(key, value);
}
QVariant GameStateManager::getGameValue(const QString& key) const
{
    return m_gameStateData.value(key);
}
bool GameStateManager::areResourcesLoaded() const {
    return m_gameStateData.value("ResourcesLoaded").toBool();
}
void GameStateManager::setCharacterPoisoned(bool isP) {
    setGameValue("CharacterPoisoned", isP);
}
bool GameStateManager::isCharacterPoisoned() const {
    return m_gameStateData.value("CharacterPoisoned").toBool();
}
void GameStateManager::setCharacterBlinded(bool isB) {
    setGameValue("CharacterBlinded", isB);
}
bool GameStateManager::isCharacterBlinded() const {
    return m_gameStateData.value("CharacterBlinded").toBool();
}
void GameStateManager::setCharacterOnFire(bool isOnFire) {
    setGameValue("CharacterOnFire", isOnFire);
}
bool GameStateManager::isCharacterOnFire() const {
    return m_gameStateData.value("CharacterOnFire").toBool();
}
