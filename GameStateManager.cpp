#include "GameStateManager.h"
#include "GameDataLoader.h"
#include "src/race_data/RaceData.h"
#include "RaceFactory.h"
#include "GuildFactory.h"
#include "SaveGameHandler.h"
#include "CharacterIO.h"
#include <QDebug>
#include <QVariantList>
#include <QVariantMap>
#include <QMapIterator>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QMetaType>
#include <QPainter>
GameStateManager* GameStateManager::instance()
{
    // Thread-safe initialization guaranteed by C++11
    static GameStateManager instance;
    return &instance;
}
// --- The Loader Logic ---
bool GameStateManager::loadGameConfig(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open config:" << filePath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    m_registry.clear();

    for (auto it = root.begin(); it != root.end(); ++it) {
        QVariant val = it.value().toVariant();
        if (val.typeId() == QMetaType::QVariantList) {
            m_registry.registerData(it.key(), val.toList());
        } else {
            m_registry.registerData(it.key(), QVariantList() << val);
        }
    }
    return true;
}
void GameStateManager::refreshUI() {
    QVariantList partyList;
    for (const auto& character : m_PC) {
        partyList.append(character.toMap());
    }
    // Update the master list that the UI/QML binds to
    m_gameStateData["Party"] = partyList;
    // If you have specific UI elements bound to "CurrentCharacterHP", 
    // update only those two or three global keys here.
    if (!m_PC.isEmpty()) {
        const Character& active = m_PC[0];
        m_gameStateData["CurrentCharacterHP"] = active.hp;
        m_gameStateData["isAlive"] = active.isAlive ? 1 : 0;
        m_gameStateData["CurrentCharacterAge"] = active.Age;
    }
    emit gameValueChanged("Party", partyList);
}


// Helper to prevent code duplication
QVariantMap GameStateManager::findRaceMap(const QString& raceName) const {
    QVariantList races = m_registry.getData(GameConstants::CAT_RACES);
    for (const QVariant& v : races) {
        QVariantMap map = v.toMap();
        if (map["raceName"].toString() == raceName) return map;
    }
    return QVariantMap();
}

// --- Using the Registry in existing methods ---
int GameStateManager::getRaceMin(const QString& raceName, const QString& statName) const {
    QVariantList races = m_registry.getData(GameConstants::CAT_RACES);
    for (const QVariant& v : races) {
        QVariantMap map = v.toMap();
        if (map["raceName"].toString() == raceName) {
            // Dynamically fetch the stat based on the string name
            // e.g., "Strength" becomes key "str_min"
            QString key = statName.left(3).toLower() + "_min"; 
            return map[key].toInt();
        }
    }
    return 1;
}

// --- Factory Method ---
GameConstants::RaceStats GameStateManager::createRaceFromVariant(const QVariant& data) const {
    QVariantMap map = data.toMap();
    GameConstants::RaceStats s;
    s.raceName = map["raceName"].toString();
    s.maxAge = map["maxAge"].toInt();
    
    // Helper lambda for nested stats
    auto parse = [&](const QString& p) -> GameConstants::RaceStat {
        return { map[p+"_start"].toInt(), map[p+"_min"].toInt(), map[p+"_max"].toInt() };
    };

    s.strength = parse("str");
    s.intelligence = parse("int");
    // ... add other stats (wisdom, dex, etc)
    
    return s;
}

// Add these helper methods
void GameStateManager::incrementStock(const QString& name)
{
    m_confinementStock[name] = m_confinementStock.value(name, 0) + 1;
}

void GameStateManager::decrementStock(const QString& name)
{
    int current = m_confinementStock.value(name, 0);
    if (current > 0) m_confinementStock[name] = current - 1;
}

QMap<QString, int> GameStateManager::getConfinementStock() const
{
    return m_confinementStock;
}

GameStateManager::GameStateManager(QObject *parent)
    : QObject(parent)
{
    setupTimers();
    setupFonts();

    //m_proportionalFont = QFont("MS Sans Serif", 8); 
    //m_fixedFont = QFont("Courier New", 9);

    struct GameState {
        QList<Character> Party;
    };
    //m_autosaveTimer = new QTimer(this);
    //connect(m_autosaveTimer, &QTimer::timeout, this, &GameStateManager::handleAutosave);
    // Initialize party members
    initializeDefaultParty();

    // You can also initialize a list for the whole party if needed
    m_gameStateData["PartyHP"] = QVariantList({50, 40, 30});
    m_gameStateData["Party"] = party;
    // --- End party ---
    // --- Current Character ---
    m_gameStateData["inCity"] = false;
    m_gameStateData["CurrentCharacterSex"] = GameConstants::SEX_OPTIONS.at(0);
    //m_gameStateData["CurrentCharacterSex"] = GameStateManager::sexOptions().at(0);
    m_gameStateData["CurrentCharacterAlignment"] = GameConstants::ALIGNMENT_NAMES.at(GameConstants::DEFAULT_ALIGNMENT_INDEX);
    //m_gameStateData["CurrentCharacterAlignment"] = GameStateManager::alignmentNames().at(GameStateManager::defaultAlignmentIndex());
    m_gameStateData["CurrentCharacterStatPointsLeft"] = GameConstants::DEFAULT_STAT_POINTS;
    //m_gameStateData["CurrentCharacterStatPointsLeft"] = GameStateManager::defaultStatPoints();
    m_gameStateData["CurrentCharacterHP"] = 50;
    m_gameStateData["CurrentCharacterLevel"] = 1;
    m_gameStateData["CurrentCharacterAge"] = 16; // Starting age for all races
    m_gameStateData["CurrentCharacterExperience"] = QVariant::fromValue((qulonglong)0);
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
    m_gameStateData["CurrentCharacterGold"] = QVariant::fromValue((qulonglong)1500);
    m_gameStateData["CurrentMana"] = 50;
    m_gameStateData["MaxMana"] = 50;

    // --- End Current Character ---

    m_gameStateData["ActiveCharacterIndex"] = 0;
    m_gameStateData["BankInventory"] = QStringList();
    m_gameStateData["GhostHoundPending"] = false;
    m_gameStateData["MaxCharacterHP"] = 50;
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
    loadGameData("tools/gamedataconverter/data/MDATA1.js");
    loadSpellData("tools/spellconverter/data/MDATA2.csv");
    loadMonsterData("tools/monsterconverter/MDATA5.csv");
    performSanityCheck();
    loadItemData("tools/itemconverter/data/MDATA3.csv");
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
    // Initialize default state data
    m_gameStateData["ResourcesLoaded"] = false;
    m_gameStateData["GameVersion"] = "1.1.7.6.450";
    m_gameStateData["PlayerScore"] = 0;
    m_gameStateData["DungeonLevel"] = 1;
    m_gameStateData["DungeonX"] = 17;
    m_gameStateData["DungeonY"] = 12;
    m_gameStateData["BankedGold"] = QVariant::fromValue((qulonglong)0);
    // Status States
    m_gameStateData["CharacterPoisoned"] = false;
    m_gameStateData["CharacterBlinded"] = false;
    m_gameStateData["CharacterDiseased"] = false;
    m_gameStateData["isAlive"] = 1;
    m_gameStateData["GuildActionLog"] = QVariantList();
    // Initialize Guild Leaders (Hall of Records)
    QVariantList guildLeadersList;

    m_gameStateData["GuildLeaders"] = QVariant::fromValue(GuildFactory::createGuildMasters());

    m_gameStateData["GuildLeaders"] = guildLeadersList;

    
    // Initialize race definitions
    m_raceDefinitions = loadRaceData();
    qDebug() << "Loaded" << m_raceDefinitions.size() << "race definitions.";
    

    qDebug() << "GameStateManager initialized.";
    listGameData();
}

void GameStateManager::loadGameData(const QString& filePath)
{
    QString version;
    m_gameData = GameDataLoader::parseMDATAJson(filePath, version);
    setGameValue("GameVersion", version);
    setGameValue("ResourcesLoaded", true);
    dumpGameDataToConsole(); 
}

void GameStateManager::dumpGameDataToConsole() {
    qDebug() << "========================================";
    qDebug() << "MDATA1 DEEP DATA DUMP";
    qDebug() << "Total Entries:" << m_gameData.size();

    for (int i = 0; i < m_gameData.size(); ++i) {
        const QVariantMap& entry = m_gameData[i];
        QString type = entry.value("DataType").toString();
        qDebug() << QString("--- Entry %1 [%2] ---").arg(i).arg(type);

        QMapIterator<QString, QVariant> it(entry);
        while (it.hasNext()) {
            it.next();
            if (it.key() == "DataType") continue;

            QVariant val = it.value();
            if (val.typeId() == QMetaType::QVariantList || val.typeId() == QMetaType::QStringList) {
                qDebug() << "  " << it.key() << ": [Array]";
                for (const QVariant& item : val.toList()) qDebug() << "     " << item.toString();
            } 
            else if (val.typeId() == QMetaType::QVariantMap) {
                qDebug() << "  " << it.key() << ": [Nested Object]";
                QMapIterator<QString, QVariant> mapIt(val.toMap());
                while (mapIt.hasNext()) { mapIt.next(); qDebug() << "     " << mapIt.key() << ":" << mapIt.value().toString(); }
            } 
            else {
                qDebug() << "  " << it.key() << ":" << val.toString();
            }
        }
    }
    qDebug() << "========================================";
}

void GameStateManager::loadMonsterData(const QString& filePath)
{
    m_monsterData = GameDataLoader::parseCSV(filePath); 
    qDebug() << "Loaded" << m_monsterData.size() << "monsters.";
    dumpMonsterDataReport();
}

void GameStateManager::dumpMonsterDataReport() const
{
    qDebug() << "--- Monster Data Load Report ---";
    qDebug() << "Total Monsters Loaded:" << m_monsterData.size();

    // Print the first 3 entries to verify columns mapped correctly
    for (int i = 0; i < qMin(3, m_monsterData.size()); ++i) {
        const QVariantMap& m = m_monsterData[i];
        qDebug() << "Entry" << i << ":" << m["name"].toString() 
                 << "| HP:" << m["hits"].toInt() 
                 << "| Atk:" << m["att"].toInt();
    }
    qDebug() << "-------------------------------";
}

void GameStateManager::loadSpellData(const QString& filePath)
{
    m_spellData = parseCSV(filePath);
    qDebug() << "Successfully loaded" << m_spellData.size() << "spells.";
}

void GameStateManager::loadItemData(const QString& filePath)
{
    m_itemData = parseCSV(filePath);
    qDebug() << "Successfully loaded" << m_itemData.size() << "items.";
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

    // Delegate the complex party syncing to the new private part
    if (key.startsWith("CurrentCharacter") || key == "isAlive" || key == "MaxCharacterHP") {
        syncGlobalValueToActiveMember(key, value);
    }

    emit gameValueChanged(key, value);
}

void GameStateManager::syncGlobalValueToActiveMember(const QString& key, const QVariant& value) 
{
    QVariantList partyList = m_gameStateData["Party"].toList();
    if (partyList.isEmpty()) return;

    // We assume index 0 is the "Active" member being edited
    QVariantMap character = partyList[0].toMap();

    // Mapping global keys to internal character keys
    static const QMap<QString, QString> specialRemaps = {
        {"CurrentCharacterName", "Name"},
        {"CurrentCharacterLevel", "Level"},
        {"CurrentCharacterHP", "HP"},
        {"MaxCharacterHP", "MaxHP"}
    };

    QString internalKey = specialRemaps.value(key, key);
    internalKey.remove("CurrentCharacter");

    // Update the character map
    character[internalKey] = (key == "isAlive") ? value.toInt() : value;
    
    // Put it back into the list and then back into the state
    partyList[0] = character;
    m_gameStateData["Party"] = partyList;
}

QVariant GameStateManager::getGameValue(const QString& key) const
{
    return m_gameStateData.value(key);
}

bool GameStateManager::areResourcesLoaded() const 
{
    return m_gameStateData.value("ResourcesLoaded").toBool();
}

void GameStateManager::setCharacterPoisoned(bool isP) 
{
    setGameValue("CharacterPoisoned", isP);
}

bool GameStateManager::isCharacterPoisoned() const 
{
    return m_gameStateData.value("CharacterPoisoned").toBool();
}

void GameStateManager::setCharacterBlinded(bool isB) 
{
    setGameValue("CharacterBlinded", isB);
}

bool GameStateManager::isCharacterBlinded() const 
{
    return m_gameStateData.value("CharacterBlinded").toBool();
}

void GameStateManager::setCharacterOnFire(bool isOnFire) 
{
    setGameValue("CharacterOnFire", isOnFire);
}
bool GameStateManager::isCharacterOnFire() const 
{
    return m_gameStateData.value("CharacterOnFire").toBool();
}
// Unit tests
void GameStateManager::performSanityCheck() 
{
    qDebug() << "[Sanity Check] Verifying Monster Data...";   
    if (m_monsterData.isEmpty()) {
        qCritical() << "FAIL: Monster data is empty!";
        return;
    }
    // Test Case: Check if ID 0 is Goblie (based on your CSV)
    QVariantMap first = getMonster(0);
    if (first["name"].toString() == "Goblie") {
        qDebug() << "PASS: Index 0 is Goblie.";
    } else {
        qWarning() << "FAIL: Index 0 expected 'Goblie', got" << first["name"].toString();
    }
    // Test Case: Check field type conversion
    bool ok;
    int hits = first["hits"].toInt(&ok);
    if (ok && hits > 0) {
        qDebug() << "PASS: 'hits' column is a valid integer (" << hits << ")";
    } else {
        qWarning() << "FAIL: 'hits' column is not a valid integer.";
    }
}

void GameStateManager::setBankInventory(const QStringList& items) 
{
    setGameValue("BankInventory", items);
}

QStringList GameStateManager::getBankInventory() const 
{
    return m_gameStateData.value("BankInventory").toStringList();
}

void GameStateManager::setCharacterInventory(int characterIndex, const QStringList& items) 
{
    QVariantList party = m_gameStateData["Party"].toList();
    if (characterIndex >= 0 && characterIndex < party.size()) {
        QVariantMap character = party[characterIndex].toMap();
        character["Inventory"] = items;
        party[characterIndex] = character;
        m_gameStateData["Party"] = party;
        emit gameValueChanged("Party", m_gameStateData["Party"]);
    }
}

void GameStateManager::addItemToCharacter(int characterIndex, const QString& itemName) 
{
    QVariantList party = m_gameStateData["Party"].toList();
    if (characterIndex >= 0 && characterIndex < party.size()) {
        QVariantMap character = party[characterIndex].toMap();
        // Use QVariantList for internal storage to match QVariant compatibility
        QStringList inventory = character["Inventory"].toStringList(); 
        inventory.append(itemName);
        character["Inventory"] = inventory;
        party[characterIndex] = character;
        m_gameStateData["Party"] = party;
        emit gameValueChanged("Party", m_gameStateData["Party"]);
    }
}

void GameStateManager::updateCharacterGold(int characterIndex, qulonglong amount, bool add) 
{
    QVariantList party = m_gameStateData["Party"].toList();
    if (characterIndex >= 0 && characterIndex < party.size()) {
        QVariantMap character = party[characterIndex].toMap();
        qulonglong currentGold = character["CurrentCharacterGold"].toULongLong();
        
        if (add) currentGold += amount;
        else currentGold = (amount > currentGold) ? 0 : currentGold - amount;

        character["CurrentCharacterGold"] = QVariant::fromValue(currentGold);
        party[characterIndex] = character;
        m_gameStateData["Party"] = party;
        emit gameValueChanged("Party", m_gameStateData["Party"]);
    }
}

bool GameStateManager::loadCharacterFromFile(const QString& characterName) {
    QVariantMap characterMap = CharacterIO::loadFromFile(characterName);
    if (characterMap.isEmpty()) return false;

    // 1. Force the 'isAlive' flag to 1 if it's missing or set to 0 incorrectly
    // Many older save formats use "Dead" (bool) instead of "isAlive" (int)
    int aliveStatus = 1; 
    if (characterMap.contains("isAlive")) {
        aliveStatus = characterMap["isAlive"].toInt();
    } else if (characterMap.contains("Dead")) {
        aliveStatus = characterMap["Dead"].toBool() ? 0 : 1;
    }
    characterMap["isAlive"] = aliveStatus;

    // 2. Update the QVariantList for the QML/UI
    QVariantList partyList = m_gameStateData["Party"].toList();
    if (!partyList.isEmpty()) {
        partyList[0] = characterMap;
        m_gameStateData["Party"] = partyList;
    }

    // 3. Update the logical C++ struct (This is what 'isCharacterPastMaxAge' etc. uses)
    if (!m_PC.isEmpty()) {
        m_PC[0].loadFromMap(characterMap);
        m_PC[0].isAlive = aliveStatus; // Manually override to be safe
    }

    // 4. Update the Global 'isAlive' key used by the UI
    setGameValue("isAlive", aliveStatus);
    setGameValue("CurrentCharacterName", characterMap["Name"]);

    refreshUI();
    return true;
}

bool GameStateManager::saveCharacterToFile(int partyIndex) {
    QVariantList party = m_gameStateData["Party"].toList();
    if (partyIndex < 0 || partyIndex >= party.size()) return false;

    // Delegate the actual file writing to the new class
    return CharacterIO::saveToFile(party[partyIndex].toMap());
}

// Example of how to correctly update HP in the Game State
void GameStateManager::updatePartyMemberHP(int index, int newHP) 
{
    QVariantList party = getGameValue("Party").toList();
    if (index >= 0 && index < party.size()) {
        QVariantMap charMap = party[index].toMap();
        
        // Always store as int
        charMap["HP"] = newHP; 
        
        party[index] = charMap;
        setGameValue("Party", party); // This triggers the UI refresh
    }
}

/*
void GameStateManager::syncActiveCharacterToParty() 
{
    // 1. Get the current party list
    QVariantList party = m_gameStateData["Party"].toList();
    if (party.isEmpty()) return;
    // 2. Get the specific map for the first character slot
    QVariantMap character = party[0].toMap();
    // 3. Sync individual "CurrentCharacter" values into the map
    character["Name"]                 = getGameValue("CurrentCharacterName");
    character["Race"]                 = getGameValue("CurrentCharacterRace");
    character["Guild"]                = getGameValue("CurrentCharacterGuild");
    character["Level"]                = getGameValue("CurrentCharacterLevel");
    character["HP"]                   = getGameValue("CurrentCharacterHP");
    character["MaxHP"]                = getGameValue("MaxCharacterHP");
    character["CurrentCharacterGold"] = getGameValue("CurrentCharacterGold");
    character["Experience"]           = getGameValue("CurrentCharacterExperience");
    character["Strength"]             = getGameValue("CurrentCharacterStrength");
    character["Intelligence"]         = getGameValue("CurrentCharacterIntelligence");
    character["Wisdom"]               = getGameValue("CurrentCharacterWisdom");
    character["Constitution"]         = getGameValue("CurrentCharacterConstitution");
    character["Charisma"]             = getGameValue("CurrentCharacterCharisma");
    character["Dexterity"]            = getGameValue("CurrentCharacterDexterity");
    character["Dead"]                 = (getGameValue("isAlive").toInt() == 0);
    // 4. Put the map back into the list and update the master state
    party[0] = character;
    setGameValue("Party", party);
    qDebug() << "Synced active character" << character["Name"].toString() << "to Party Slot 0.";
}
*/

void GameStateManager::listGameData() {
    // Remove the () and the instance() call
    QList<QVariantMap> dataList = m_gameData; 

    qDebug() << "--- START m_gameData LOG ---";
    for (int i = 0; i < dataList.size(); ++i) {
        qDebug() << "Record" << i + 1 << ":" << dataList.at(i);
    }
}

bool GameStateManager::verifySaveGame(const QString& characterName) {
    return SaveGameHandler::verifySave(characterName);
}

bool GameStateManager::repairSaveGame(const QString& characterName) {
    return SaveGameHandler::repairSave(characterName);
}
void GameStateManager::startAutosave(int intervalms) {
    if (m_autosaveTimer) {
        m_autosaveTimer->start(intervalms);
        qDebug() << "Autosave started with interval:" << intervalms << "ms";
    }
}

void GameStateManager::stopAutosave() {
    if (m_autosaveTimer) m_autosaveTimer->stop();
}

void GameStateManager::handleAutosave() {
    // Check if a character is actually loaded before saving
    QString currentHero = getGameValue("CurrentCharacterName").toString();
    if (currentHero.isEmpty() || currentHero == "Empty Slot") {
        return; 
    }

    qDebug() << "Autosaving character:" << currentHero;
    
    // 1. Sync the live UI/Game values to the Party structure first
    refreshUI();
    //syncActiveCharacterToParty();
    
    // 2. Save Party Slot 0 (the active player) to the .txt file
    if (saveCharacterToFile(0)) {
        qDebug() << "Autosave successful.";
    } else {
        qWarning() << "Autosave failed!";
    }
}

// 1. Static helper for the raw data
QMap<QString, int> GameStateManager::getRaceAgeLimits()
{
    return {
        {"Human", 100}, {"Elf", 400}, {"Giant", 225},
        {"Gnome", 300}, {"Dwarf", 275}, {"Ogre", 265},
        {"Morloch", 175}, {"Osiri", 325}, {"Troll", 285}
    };
}

int GameStateManager::getMaxAgeForRace(const QString& raceName) const
{
    // Check dynamic state first (e.g., m_gameStateData["MaxHumanAge"])
    QString key = QString("Max%1Age").arg(raceName);
    if (m_gameStateData.contains(key)) {
        return m_gameStateData.value(key).toInt();
    }
    
    // Fallback to the constants you provided
    static const QMap<QString, int> defaultAges = {
        {"Human", 100}, {"Elf", 400}, {"Giant", 225},
        {"Gnome", 300}, {"Dwarf", 275}, {"Ogre", 265},
        {"Morloch", 175}, {"Osiri", 325}, {"Troll", 285}
    };
    return defaultAges.value(raceName, 100); 
}

bool GameStateManager::isCharacterPastMaxAge(int index) const
{
    if (index < 0 || index >= m_PC.size()) return false;

    const Character& c = m_PC.at(index);
    int maxAge = getMaxAgeForRace(c.Race); // Now resolves!
    
    return (c.Age >= maxAge); // Now resolves!
}

void GameStateManager::incrementPartyAge(int years) {
    for (auto& character : m_PC) {
        if (character.name != "Empty Slot") {
            character.Age += years; // Logic only
        }
    }
    processAgingConsequences(); // Logic only
    refreshUI(); // Single sync point for the UI
}
/*
void GameStateManager::incrementPartyAge(int years)
{
    // 1. Update the internal Character structs (m_PC)
    for (int i = 0; i < m_PC.size(); ++i) {
        if (m_PC[i].name != "Empty Slot") {
            m_PC[i].Age += years;
        }
    }

    // 2. Sync changes back to the QVariantMap for the UI
    QVariantList partyList = m_gameStateData["Party"].toList();
    for (int i = 0; i < partyList.size(); ++i) {
        QVariantMap characterMap = partyList[i].toMap();
        if (characterMap["Name"].toString() != "Empty Slot") {
            characterMap["Age"] = m_PC[i].Age;
            partyList[i] = characterMap;
        }
    }
    
    // 3. Update master state and notify UI
    m_gameStateData["Party"] = partyList;
    emit gameValueChanged("Party", partyList);

    // 4. Update the "CurrentCharacterAge" global value if index 0 changed
    if (!m_PC.isEmpty()) {
        setGameValue("CurrentCharacterAge", m_PC[0].Age);
    }

    qDebug() << "The party has aged by" << years << "year(s).";
    
    // 5. Check if anyone died of natural causes
    processAgingConsequences();
}
*/
void GameStateManager::processAgingConsequences()
{
    for (int i = 0; i < m_PC.size(); ++i) {
        if (m_PC[i].name == "Empty Slot") continue;

        if (isCharacterPastMaxAge(i)) {
            // Log the event
            QString deathMsg = QString("%1 has passed away peacefully of old age at %2.")
                               .arg(m_PC[i].name)
                               .arg(m_PC[i].Age);
            logGuildAction(deathMsg);
            qDebug() << deathMsg;

            // Handle death logic
            if (i == 0) {
                // If it's the player, set the global "isAlive" state to 0
                setIsAlive(0); 
            } else {
                // Handle party member death (e.g., mark as dead in struct/map)
                m_PC[i].isAlive = 0;
                updatePartyMemberHP(i, 0);
            }
        }
    }
}

void GameStateManager::setProportionalFont(const QFont& font) {
//    m_proportionalFont = font;
//    QGuiApplication::setFont(font); // Updates all standard widgets automatically
//    emit fontChanged();
m_proportionalFont = font;
    
    // Apply to the entire application instance
    if (qApp) {
        qApp->setFont(font);
    }

    // Force every top-level widget to update immediately
    for (QWidget *widget : QApplication::allWidgets()) {
        widget->setFont(font);
        widget->update();
    }
    
    emit fontChanged();
}

void GameStateManager::setFixedFont(const QFont& font) {
    m_fixedFont = font;
    emit fontChanged();
}
void GameStateManager::loadFontSprite(const QString& path) {
    m_fontSpriteSheet.load(path);
}

void GameStateManager::drawCustomText(QPainter* painter, const QString& text, const QPoint& position) {
    if (m_fontSpriteSheet.isNull()) return;

    // The sequence exactly as it appears in the image, row by row
    const QString layout = "ABCDEFGHI"   // Row 0
                           "HIJKLMM"     // Row 1
                           "NOPQRSTUUV"  // Row 2
                           "UWXYZ"       // Row 3
                           "124578901";  // Row 4

    // Measurements based on the spritesheet proportions
    static constexpr int SPRITE_WIDTH = 80;   
    static constexpr int SPRITE_HEIGHT = 90;  
    static constexpr int KERNING = 48; // Adjust this to tighten/loosen spacing

    QString upperText = text.toUpper();

    for (int i = 0; i < upperText.length(); ++i) {
        QChar c = upperText[i];
        
        if (c == ' ') continue; // Simple space handling

        // Find character in the layout string
        int index = layout.indexOf(c);
        if (index == -1) continue; 

        int row = 0;
        int col = 0;

        // Map index to the specific grid coordinates
        if (index < 9) { 
            row = 0; col = index; 
        } else if (index < 16) { 
            row = 1; col = (index - 9); 
        } else if (index < 26) { 
            row = 2; col = (index - 16); 
        } else if (index < 31) { 
            row = 3; col = (index - 26); 
        } else { 
            row = 4; col = (index - 31); 
        }

        // Source: Where the letter is in the PNG
        QRect sourceRect(col * SPRITE_WIDTH, row * SPRITE_HEIGHT, SPRITE_WIDTH, SPRITE_HEIGHT);
        
        // Target: Where to draw on the UI
        // We use KERNING for the X offset so letters don't have huge gaps
        QRect targetRect(position.x() + (i * KERNING), position.y(), SPRITE_WIDTH, SPRITE_HEIGHT);

        painter->drawPixmap(targetRect, m_fontSpriteSheet, sourceRect);
    }
}

void GameStateManager::loadRaceDefinitions() {
    // Call the external factory to get the data
    m_raceDefinitions = RaceFactory::createStandardRaces();

    // Initialize game state map using the dynamic data
    for(const auto& race : m_raceDefinitions) {
        m_gameStateData[race.raceName + "MaxAge"] = race.maxAge;
    }
}

GameConstants::RaceStats GameStateManager::getStatsForRace(const QString& raceName) const {
    for (const auto& stats : m_raceDefinitions) {
        if (stats.raceName == raceName) return stats;
    }
    return {}; // Return empty if not found
}
const GameConstants::RaceStat& GameStateManager::getStatRef(const GameConstants::RaceStats& race, const QString& statName) const {
    if (statName == "Strength")     return race.strength;
    if (statName == "Intelligence") return race.intelligence;
    if (statName == "Wisdom")       return race.wisdom;
    if (statName == "Constitution") return race.constitution;
    if (statName == "Charisma")     return race.charisma;
    return race.dexterity;
}

QVector<QString> GameStateManager::getAvailableRaces() const {
    QVector<QString> names;
    for (const auto& race : m_raceDefinitions) {
        names.append(race.raceName);
    }
    return names;
}

void GameStateManager::loadGuildLeaders() {
    // Simply fetch from the factory
    m_guildmastersData = GuildFactory::createGuildMasters();
    
    qDebug() << "Hall of Records initialized with" << m_guildmastersData.size() << "masters.";
}

void GameStateManager::initializeDefaultParty() {
    m_PC.clear();
    QVariantList partyList;

    for (int i = 0; i < MAX_PARTY; ++i) {
        QVariantMap character;
        character["Name"] = "Empty Slot";
        character["Level"] = 0;
        character["Experience"] = 0;
        character["HP"] = 0;
        character["MaxHP"] = 0;
        character["CurrentCharacterGold"] = 1500;
        character["Race"] = "Human"; 
        character["Age"] = 16;       
        
        // Stats
        character["Strength"] = 0;
        character["Intelligence"] = 0;
        character["Wisdom"] = 0;
        character["Constitution"] = 0;
        character["Charisma"] = 0;
        character["Dexterity"] = 0;
        
        // Status
        character["Poisoned"] = 0;
        character["Blinded"] = 0;
        character["Diseased"] = 0;
        character["isAlive"] = 0;
        character["Inventory"] = QStringList();

        partyList.append(character);

        Character c;
        c.loadFromMap(character);
        m_PC.append(c);
    }

    m_gameStateData["Party"] = partyList;
    emit gameValueChanged("Party", partyList);
}

QList<QVariantMap> GameStateManager::parseCSV(const QString& filePath) {
    QList<QVariantMap> dataList;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for parsing:" << filePath;
        return dataList;
    }

    QTextStream in(&file);
    if (in.atEnd()) return dataList;

    // 1. Extract Headers
    // Split by comma and trim whitespace to ensure clean keys
    QStringList headers;
    QString headerLine = in.readLine();
    for (const QString& head : headerLine.split(',')) {
        headers << head.trimmed();
    }

    // 2. Parse Rows
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList fields = line.split(',');
        
        // Ensure row integrity by matching header count
        if (fields.size() == headers.size()) {
            QVariantMap row;
            for (int i = 0; i < headers.size(); ++i) {
                row[headers[i]] = fields[i].trimmed();
            }
            dataList.append(row);
        } else {
            qWarning() << "Row length mismatch in" << filePath << "at line:" << line;
        }
    }

    file.close();
    return dataList;
}
// Helper function
void GameStateManager::setupTimers() {
    if (!m_autosaveTimer) {
        m_autosaveTimer = new QTimer(this);
        connect(m_autosaveTimer, &QTimer::timeout, this, &GameStateManager::handleAutosave);
    }
}

void GameStateManager::setupFonts() {
    m_proportionalFont = QFont("MS Sans Serif", 8);
    m_fixedFont = QFont("Courier New", 9);

    if (qApp) {
        qApp->setFont(m_proportionalFont);
        
        // Use a more concise loop style
        const auto widgets = QApplication::allWidgets();
        for (QWidget *w : widgets) {
            if (w) {
                w->setFont(m_proportionalFont);
                w->update();
            }
        }
    }
    emit fontChanged();
}
