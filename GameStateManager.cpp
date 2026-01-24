#include "GameStateManager.h"
//#include "character.h"
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

GameStateManager* GameStateManager::instance()
{
    // Thread-safe initialization guaranteed by C++11
    static GameStateManager instance;
    return &instance;
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
    m_gameStateData["CurrentCharacterSex"] = GameStateManager::sexOptions().at(0);
    m_gameStateData["CurrentCharacterAlignment"] = GameStateManager::alignmentNames().at(GameStateManager::defaultAlignmentIndex());
    m_gameStateData["CurrentCharacterStatPointsLeft"] = GameStateManager::defaultStatPoints();
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
            if (val.type() == QVariant::List || val.type() == QVariant::StringList) {
                QVariantList list = val.toList();
                qDebug() << "  " << it.key() << ": [Array]";
                for (int j = 0; j < list.size(); ++j) {
                    qDebug() << "     Index" << j << ":" << list[j].toString();
                }
            } 
            // 2. Handle Nested Maps (Objects)
            else if (val.type() == QVariant::Map) {
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
            if (statNames().contains(statName)) {
                character[statName] = value;
            }
            
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
        else if (key == "HP") characterMap["HP"] = value.toInt();
        else if (key == "MaxHP") characterMap["MaxHP"] = value.toInt();
        else if (key == "Level") characterMap["Level"] = value.toInt();
        else if (key == "CurrentCharacterGold") characterMap["CurrentCharacterGold"] = QVariant::fromValue(value.toULongLong());
        else if (key == "isAlive") characterMap["isAlive"] = value.toInt(); // 0 for dead, 1 for alive
        else if (key == "DungeonLevel") characterMap["DungeonLevel"] = value.toInt();
        else if (key == "DungeonX") characterMap["DungeonX"] = value.toInt();
        else if (key == "DungeonY") characterMap["DungeonY"] = value.toInt();
        // Check if the key is a stat (Str, Int, etc)
        else if (statNames().contains(key)) characterMap[key] = value.toInt();
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
    for (const QString& stat : statNames()) {
        out << stat << ": " << character[stat].toInt() << "\n";
    }
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
