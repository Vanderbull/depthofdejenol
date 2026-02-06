#include "GameStateManager.h"
#include "src/race_data/RaceData.h"
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
        if (val.type() == QVariant::List) {
            m_registry.registerData(it.key(), val.toList());
        } else {
            m_registry.registerData(it.key(), QVariantList() << val);
        }
    }
    return true;
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
    m_proportionalFont = QFont("MS Sans Serif", 8); 
    m_fixedFont = QFont("Courier New", 9);
    // Initialize Global Audio
    m_globalPlayer = new QMediaPlayer(this);
    m_globalAudioOutput = new QAudioOutput(this);
    m_globalPlayer->setAudioOutput(m_globalAudioOutput);
    m_globalAudioOutput->setVolume(0.5f); // Default volume


    struct GameState {
        QList<Character> Party;
    };
    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, &QTimer::timeout, this, &GameStateManager::handleAutosave);
    // Initialize party members
    //QVariantList party;
    for (int i = 0; i < MAX_PARTY_SIZE; ++i) {
        QVariantMap character;
        character["Name"] = "Empty Slot";
        character["Level"] = 0;
        character["Experience"] = 0;
        character["HP"] = 0;
        character["MaxHP"] = 0;
        character["CurrentCharacterGold"] = 1500;
        character["Race"] = "Human"; // Default race
        character["Age"] = 16;       // Default age
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
        party.append(character);
        Character c;
        // 3. Fill the struct using the helper function
        c.loadFromMap(character);
        // 4. Add to your list
        m_PC.append(c);
    }
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

    
    // Initialize race definitions
    m_raceDefinitions = loadRaceData();
    qDebug() << "Loaded" << m_raceDefinitions.size() << "race definitions.";
    

    qDebug() << "GameStateManager initialized.";
    listGameData();
}

void GameStateManager::loadGameData(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open game data file:" << filePath;
        return;
    }
    QString fileContent = file.readAll();
    file.close();
    // Extract JSON part
    int firstBrace = fileContent.indexOf('{');
    int lastBrace = fileContent.lastIndexOf('}');
    
    if (firstBrace == -1 || lastBrace == -1) {
        qWarning() << "MDATA1 file structure invalid.";
        return;
    }

    QString jsonString = fileContent.mid(firstBrace, (lastBrace - firstBrace) + 1);
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject rootObj = doc.object();
    m_gameData.clear();
    // Capture Version info
    if (rootObj.contains("GameVersion")) {
        QVariantMap versionMap;
        versionMap["DataType"] = "VersionInfo";
        versionMap["VersionValue"] = rootObj["GameVersion"].toVariant();
        m_gameData.append(versionMap);
    }
    // Helper to load and tag all categories
    auto processArray = [&](const QString& key, const QString& typeTag) {
        if (rootObj.contains(key) && rootObj[key].isArray()) {
            QJsonArray array = rootObj[key].toArray();
            for (const QJsonValue& value : array) {
                QVariantMap map = value.toObject().toVariantMap();
                map["DataType"] = typeTag; 
                m_gameData.append(map);
            }
        }
    };

    processArray("Guilds", "Guild");
    processArray("ItemTypes", "ItemType");
    processArray("MonsterTypes", "MonsterType");
    processArray("Races", "Race");
    processArray("SubItemTypes", "SubItemType");
    processArray("SubMonsterTypes", "SubMonsterType");

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
            // 1. Handle Nested Lists (Arrays)
            if (val.typeId() == QMetaType::QVariantList || val.typeId() == QMetaType::QStringList) {
                QVariantList list = val.toList();
                qDebug() << "  " << it.key() << ": [Array]";
                for (int j = 0; j < list.size(); ++j) {
                    qDebug() << "     Index" << j << ":" << list[j].toString();
                }
            } 
            // 2. Handle Nested Maps (Objects)
            else if (val.typeId() == QMetaType::QVariantMap) {
                QVariantMap nestedMap = val.toMap();
                qDebug() << "  " << it.key() << ": [Nested Object]";
                QMapIterator<QString, QVariant> mapIt(nestedMap);
                while (mapIt.hasNext()) {
                    mapIt.next();
                    qDebug() << "     " << mapIt.key() << ":" << mapIt.value().toString();
                }
            }
            // 3. Handle Normal Values
            else {
                qDebug() << "  " << it.key() << ":" << val.toString();
            }
        }
    }
    qDebug() << "========================================";
    setGameValue("ResourcesLoaded", true);
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
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open spell data file:" << filePath;
        return;
    }
    m_spellData.clear();
    QTextStream in(&file);
    
    if (in.atEnd()) return;
    QString headerLine = in.readLine();
    QStringList headers = headerLine.split(',');

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList fields = line.split(',');
        
        if (fields.size() == headers.size()) {
            QVariantMap spell;
            for (int i = 0; i < headers.size(); ++i) {
                spell[headers[i].trimmed()] = fields[i].trimmed();
            }
            m_spellData.append(spell);
        }
    }
    file.close();
    qDebug() << "Successfully loaded" << m_spellData.size() << "spells from MDATA2.";
}

void GameStateManager::loadItemData(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open item data file:" << filePath;
        return;
    }
    m_itemData.clear();
    QTextStream in(&file);
    
    if (in.atEnd()) return;
    QString headerLine = in.readLine();
    QStringList headers = headerLine.split(',');

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        
        QStringList fields = line.split(',');
        // MDATA3 has 35 columns; checking against header size ensures row integrity
        if (fields.size() == headers.size()) {
            QVariantMap item;
            for (int i = 0; i < headers.size(); ++i) {
                item[headers[i].trimmed()] = fields[i].trimmed();
            }
            m_itemData.append(item);
        }
    }
    file.close();
    qDebug() << "Successfully loaded" << m_itemData.size() << "items from MDATA3.";
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

    // If we update a specific "CurrentCharacter" key, we must update the Party[0] slot
    if (key.startsWith("CurrentCharacter") || key == "isAlive" || key == "MaxCharacterHP") {
        QVariantList partyList = m_gameStateData["Party"].toList();
        if (!partyList.isEmpty()) {
            QVariantMap character = partyList[0].toMap();
            
            if (key == "CurrentCharacterName") character["Name"] = value;
            else if (key == "CurrentCharacterLevel") character["Level"] = value;
            else if (key == "CurrentCharacterHP") character["HP"] = value;
            else if (key == "MaxCharacterHP") character["MaxHP"] = value;
            else if (key == "isAlive") character["isAlive"] = value.toInt();
            
            // Check for stats (CurrentCharacterStrength, etc)
            QString statName = key;
            statName.remove("CurrentCharacter");
            if (GameConstants::STAT_NAMES.contains(statName)) {
                character[statName] = value;
            }
            //if (statNames().contains(statName)) {
            //    character[statName] = value;
            //}
            partyList[0] = character;
            m_gameStateData["Party"] = partyList;
        }
    }
    
    emit gameValueChanged(key, value);
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

bool GameStateManager::loadCharacterFromFile(const QString& characterName) 
{
    QString cleanName = characterName;
    if (cleanName.endsWith(".txt")) cleanName.chop(4);
    
    QString filename = QString("data/characters/%1.txt").arg(cleanName);
    QFile file(filename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open character file:" << filename;
        return false;
    }

    QVariantMap characterMap;
    characterMap["Name"] = cleanName; // Default name from filename
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("---") || line.contains("VERSION")) continue;

        QStringList parts = line.split(": ");
        if (parts.size() < 2) continue;

        QString key = parts.at(0).trimmed();
        QString value = parts.at(1).trimmed();

        // Map file keys to QVariantMap keys
        if (key == "Name") characterMap["Name"] = value;
        else if (key == "Race") characterMap["Race"] = value; // New
        else if (key == "Age") characterMap["Age"] = value.toInt(); // New
        else if (key == "HP") characterMap["HP"] = value.toInt();
        else if (key == "MaxHP") characterMap["MaxHP"] = value.toInt();
        else if (key == "Level") characterMap["Level"] = value.toInt();
        else if (key == "CurrentCharacterGold") characterMap["CurrentCharacterGold"] = QVariant::fromValue(value.toULongLong());
        else if (key == "isAlive") characterMap["isAlive"] = value.toInt(); // 0 for dead, 1 for alive
        else if (key == "DungeonLevel") characterMap["DungeonLevel"] = value.toInt();
        else if (key == "DungeonX") characterMap["DungeonX"] = value.toInt();
        else if (key == "DungeonY") characterMap["DungeonY"] = value.toInt();
        // Check if the key is a stat (Str, Int, etc)
        else if (GameConstants::STAT_NAMES.contains(key)) {
            characterMap[key] = value.toInt();
        }
        //else if (statNames().contains(key)) characterMap[key] = value.toInt();
    }
    file.close();

    // --- DATA SYNCHRONIZATION ---
    QVariantList partyList = m_gameStateData["Party"].toList();
    if (!partyList.isEmpty()) {
        // 1. Update the UI Map
        partyList[0] = characterMap;
        setGameValue("Party", partyList);
        
        // 2. Update individual global values for UI bindings
        setGameValue("CurrentCharacterName", characterMap["Name"]);
        setGameValue("isAlive", characterMap["isAlive"].toInt());
        
        // 3. CRITICAL FIX: Sync the internal Character struct list
        // This ensures gsm->getPC().at(0).name matches the 'selected' string in MorgueDialog
        if (!m_PC.isEmpty()) {
            m_PC[0].loadFromMap(characterMap);
        }
        
        qDebug() << "Successfully loaded and synced character to Altar:" << characterMap["Name"];
        return true;
    }
    
    return false;
}

bool GameStateManager::saveCharacterToFile(int partyIndex) 
{
    QVariantList party = m_gameStateData["Party"].toList();
    if (partyIndex < 0 || partyIndex >= party.size()) return false;

    QVariantMap character = party[partyIndex].toMap();
    QString characterName = character["Name"].toString();

    // This condition triggers your error message
    if (characterName.isEmpty() || characterName == "Empty Slot") {
        qWarning() << "Save aborted: Slot is empty.";
        return false;
    }

    QDir dir;
    if (!dir.exists("data/characters/")) {
        dir.mkpath("data/characters/");
    }

    QString filename = QString("data/characters/%1.txt").arg(characterName);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "CHARACTER_FILE_VERSION: 1.0\n";
    out << "Name: " << characterName << "\n";
    out << "Race: " << character["Race"].toString() << "\n"; // Updated
    out << "Age: " << character["Age"].toInt() << "\n";      // Updated
    out << "Race: " << character["Race"].toString() << "\n";
    out << "DungeonX: " << getGameValue("DungeonX").toInt() << "\n";
    out << "DungeonY: " << getGameValue("DungeonY").toInt() << "\n";
    out << "DungeonLevel: " << getGameValue("DungeonLevel").toInt() << "\n";
    out << "Guild: " << character["Guild"].toString() << "\n";
    out << "Level: " << character["Level"].toInt() << "\n";
    out << "HP: " << character["HP"].toInt() << "\n";
    out << "MaxHP: " << character["MaxHP"].toInt() << "\n";
    out << "CurrentCharacterGold: " << character["CurrentCharacterGold"].toULongLong() << "\n";
    out << "Experience: " << character["Experience"].toULongLong() << "\n";
    out << "isAlive: " << (character["Dead"].toBool() ? 0 : 1) << "\n";
    out << "isAlive: " << (character["Dead"].toBool() ? 0 : 1) << "\n";
    out << "inCity: " << (getGameValue("inCity").toBool() ? 1 : 0) << "\n"; // Add this line
    
    // Save Stats using the helper list
    for (const QString& stat : GameConstants::STAT_NAMES) {
        out << stat << ": " << character[stat].toInt() << "\n";
    }
    //for (const QString& stat : statNames()) {
    //    out << stat << ": " << character[stat].toInt() << "\n";
    //}
    // Save Inventory as a comma-separated list
    QStringList inv = character["Inventory"].toStringList();
    out << "Inventory: " << inv.join(",") << "\n";
    file.close();
    qDebug() << "Successfully saved character:" << characterName;
    return true;
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

void GameStateManager::listGameData() {
    // Remove the () and the instance() call
    QList<QVariantMap> dataList = m_gameData; 

    qDebug() << "--- START m_gameData LOG ---";
    for (int i = 0; i < dataList.size(); ++i) {
        qDebug() << "Record" << i + 1 << ":" << dataList.at(i);
    }
}

bool GameStateManager::verifySaveGame(const QString& characterName) {
    QString cleanName = characterName;
    if (cleanName.endsWith(".txt")) cleanName.chop(4);
    
    QString filename = QString("data/characters/%1.txt").arg(cleanName);
    QFile file(filename);
    
    if (!file.exists()) {
        qWarning() << "Verification failed: File does not exist -" << filename;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    bool hasVersion = false;
    bool hasLocation = false;
    bool hasName = false;
    
    int foundCoords = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // Check for the header version string
        if (line.startsWith("CHARACTER_FILE_VERSION:")) hasVersion = true;
        
        // Check for the character name match
        if (line.startsWith("Name: ")) {
            if (line.mid(6) == cleanName) hasName = true;
        }

        // Verify that location data exists (required for the city check)
        if (line.startsWith("DungeonX:") || 
            line.startsWith("DungeonY:") || 
            line.startsWith("DungeonLevel:")) {
            foundCoords++;
        }
    }
    file.close();

    hasLocation = (foundCoords == 3);

    if (!hasVersion) qWarning() << "Savegame" << cleanName << "is missing version header.";
    if (!hasLocation) qWarning() << "Savegame" << cleanName << "is missing coordinate data.";
    if (!hasName) qWarning() << "Savegame" << cleanName << "name mismatch inside file.";

    return (hasVersion && hasLocation && hasName);
}

bool GameStateManager::repairSaveGame(const QString& characterName) {
    QString cleanName = characterName;
    if (cleanName.endsWith(".txt")) cleanName.chop(4);
    
    QString path = QString("data/characters/%1.txt").arg(cleanName);
    QFile file(path);
    
    if (!file.exists()) return false;

    // 1. Read existing data into a temporary map
    QMap<QString, QString> dataMap;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.contains(": ")) {
                QStringList parts = line.split(": ");
                dataMap[parts[0]] = parts[1];
            }
        }
        file.close();
    }

    // 2. Identify and fix missing critical data
    bool modified = false;

    // Ensure version exists
    if (!dataMap.contains("CHARACTER_FILE_VERSION")) {
        dataMap["CHARACTER_FILE_VERSION"] = "1.0";
        modified = true;
    }

    if (!dataMap.contains("Race")) { dataMap["Race"] = "Human"; modified = true; }
    if (!dataMap.contains("Age")) { dataMap["Age"] = "16"; modified = true; }
    // Fix missing coordinates (Default to City Entrance: 17, 12, Level 1)
    if (!dataMap.contains("DungeonX")) { dataMap["DungeonX"] = "17"; modified = true; }
    if (!dataMap.contains("DungeonY")) { dataMap["DungeonY"] = "12"; modified = true; }
    if (!dataMap.contains("DungeonLevel")) { dataMap["DungeonLevel"] = "1"; modified = true; }

    // Ensure Name is correct
    if (dataMap["Name"] != cleanName) {
        dataMap["Name"] = cleanName;
        modified = true;
    }

    // 3. Rewrite the file if repairs were made
    if (modified) {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            // Prioritize header and name
            out << "CHARACTER_FILE_VERSION: " << dataMap["CHARACTER_FILE_VERSION"] << "\n";
            out << "Name: " << dataMap["Name"] << "\n";
            
            // Remove handled keys to loop through the rest
            dataMap.remove("CHARACTER_FILE_VERSION");
            dataMap.remove("Name");

            QMapIterator<QString, QString> i(dataMap);
            while (i.hasNext()) {
                i.next();
                out << i.key() << ": " << i.value() << "\n";
            }
            file.close();
            qDebug() << "Successfully repaired savegame for:" << cleanName;
            return true;
        }
    }

    return false;
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
    syncActiveCharacterToParty();
    
    // 2. Save Party Slot 0 (the active player) to the .txt file
    if (saveCharacterToFile(0)) {
        qDebug() << "Autosave successful.";
    } else {
        qWarning() << "Autosave failed!";
    }
}

void GameStateManager::playMusic(const QString& fileName, bool loop)
{
    if (!m_globalPlayer) return;

    m_globalPlayer->stop();
    m_globalPlayer->setSource(QUrl::fromLocalFile(fileName));
    
    if (loop) {
        m_globalPlayer->setLoops(QMediaPlayer::Infinite);
    } else {
        m_globalPlayer->setLoops(1);
    }

    m_globalPlayer->play();
    qDebug() << "Global Audio playing:" << fileName;
}

void GameStateManager::stopMusic()
{
    if (m_globalPlayer) m_globalPlayer->stop();
}

void GameStateManager::setVolume(float volume)
{
    if (m_globalAudioOutput) m_globalAudioOutput->setVolume(volume);
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

// GameStateManager.cpp

void GameStateManager::loadRaceDefinitions() {
    m_raceDefinitions = {
        // Human
        {"Human", 100, 3, {4,4,17}, {4,4,18}, {4,4,18}, {6,6,17}, {5,5,18}, {6,6,18}, 
         GameConstants::AS_Allowed, GameConstants::AS_Allowed, GameConstants::AS_Allowed, {}},
        // Elf
        {"Elf", 400, 7, {3,3,15}, {7,7,20}, {7,7,20}, {3,3,16}, {3,3,18}, {3,3,18}, 
         GameConstants::AS_Allowed, GameConstants::AS_Allowed, GameConstants::AS_NotAllowed, {}},
        // ... (Add the rest of the races from your RaceData.cpp here)
    };

    // After loading the list, you can initialize your game state map 
    // using the dynamic data instead of hardcoded numbers:
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
/*
int GameStateManager::getRaceMin(const QString& raceName, const QString& statName) const {
    for (const auto& race : m_raceDefinitions) {
        if (race.raceName == raceName) return getStatRef(race, statName).min;
    }
    return 1;
}

int GameStateManager::getRaceMax(const QString& raceName, const QString& statName) const {
    for (const auto& race : m_raceDefinitions) {
        if (race.raceName == raceName) return getStatRef(race, statName).max;
    }
    return 18;
}

int GameStateManager::getRaceStart(const QString& raceName, const QString& statName) const {
    for (const auto& race : m_raceDefinitions) {
        if (race.raceName == raceName) return getStatRef(race, statName).start;
    }
    return 1;
}

bool GameStateManager::isAlignmentAllowed(const QString& raceName, const QString& alignmentName) const {
    for (const auto& race : m_raceDefinitions) {
        if (race.raceName == raceName) {
            if (alignmentName == "Good")    return race.good == GameConstants::AS_Allowed;
            if (alignmentName == "Neutral") return race.neutral == GameConstants::AS_Allowed;
            if (alignmentName == "Evil")    return race.evil == GameConstants::AS_Allowed;
        }
    }
    return false;
}

bool GameStateManager::isGuildAllowed(const QString& raceName, const QString& guildName) const {
    for (const auto& race : m_raceDefinitions) {
        if (race.raceName == raceName) {
            return race.guildEligibility.value(guildName, GameConstants::AS_Allowed) == GameConstants::AS_Allowed;
        }
    }
    return true;
}
*/
