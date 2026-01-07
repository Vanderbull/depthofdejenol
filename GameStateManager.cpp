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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
    QVariantList party;
    for (int i = 0; i < MAX_PARTY_SIZE; ++i) {
        QVariantMap character;
        character["Name"] = "Empty Slot";
        character["Level"] = 1;
        character["Experience"] = QVariant::fromValue((qulonglong)0);
        character["HP"] = 50;
        character["MaxHP"] = 50;
        character["Gold"] = QVariant::fromValue((qulonglong)0);
        
        // Stats
        character["Strength"] = 0;
        character["Intelligence"] = 0;
        character["Wisdom"] = 0;
        character["Constitution"] = 0;
        character["Charisma"] = 0;
        character["Dexterity"] = 0;
        
        // Status
        character["Poisoned"] = false;
        character["Blinded"] = false;
        character["Diseased"] = false;
        character["Dead"] = false;

        character["Inventory"] = QStringList();

        party.append(character);
    }

    m_gameStateData["Party"] = party;
    
    // You can still keep a "CurrentActiveIndex" to know which one the UI is showing
    m_gameStateData["ActiveCharacterIndex"] = 0;

    // Initialize Bank Inventory so it isn't null
    m_gameStateData["BankInventory"] = QStringList();

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
    loadGameData("tools/gamedataconverter/data/MDATA1.js");
    loadSpellData("tools/spellconverter/data/MDATA2.csv");
    loadMonsterData("tools/monsterconverter/MDATA5.csv");
    performSanityCheck();
    loadItemData("tools/itemconverter/data/MDATA3.csv");


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
    m_gameStateData["isAlive"] = 0;

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

    // --- ENHANCED TERMINAL DUMP ---
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
                    // Prints each item in the array
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

// Unit tests
void GameStateManager::performSanityCheck() {
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

void GameStateManager::setBankInventory(const QStringList& items) {
    setGameValue("BankInventory", items);
}

QStringList GameStateManager::getBankInventory() const {
    return m_gameStateData.value("BankInventory").toStringList();
}

void GameStateManager::setCharacterInventory(int characterIndex, const QStringList& items) {
    QVariantList party = m_gameStateData["Party"].toList();
    if (characterIndex >= 0 && characterIndex < party.size()) {
        QVariantMap character = party[characterIndex].toMap();
        character["Inventory"] = items;
        party[characterIndex] = character;
        m_gameStateData["Party"] = party;
        emit gameValueChanged("Party", m_gameStateData["Party"]);
    }
}
void GameStateManager::addItemToCharacter(int characterIndex, const QString& itemName) {
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
void GameStateManager::updateCharacterGold(int characterIndex, qulonglong amount, bool add) {
    QVariantList party = m_gameStateData["Party"].toList();
    if (characterIndex >= 0 && characterIndex < party.size()) {
        QVariantMap character = party[characterIndex].toMap();
        qulonglong currentGold = character["Gold"].toULongLong();
        
        if (add) currentGold += amount;
        else currentGold = (amount > currentGold) ? 0 : currentGold - amount;

        character["Gold"] = QVariant::fromValue(currentGold);
        party[characterIndex] = character;
        m_gameStateData["Party"] = party;
        emit gameValueChanged("Party", m_gameStateData["Party"]);
    }
}

bool GameStateManager::loadCharacterFromFile(const QString& characterName) {
// 1. Point to the specific subfolder
    // We remove .txt from the input if it's there, then ensure it's added back
    QString cleanName = characterName;
    if (cleanName.endsWith(".txt")) {
        cleanName.chop(4);
    }
    QString filename = QString("data/characters/%1.txt").arg(cleanName);
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open character file at:" << filename;
        return false;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("---") || line.startsWith("CHARACTER_FILE_VERSION")) {
            continue;
        }

        // Split "Key: Value" pairs
        QStringList parts = line.split(": ");
        if (parts.size() < 2) continue;

        QString key = parts.at(0).trimmed();
        QString value = parts.at(1).trimmed();

        // Map file keys to GameState keys
        if (key == "Name") setGameValue("CurrentCharacterName", value);
        else if (key == "Race") setGameValue("CurrentCharacterRace", value);
        else if (key == "Sex") setGameValue("CurrentCharacterSex", value);
        else if (key == "Alignment") setGameValue("CurrentCharacterAlignment", value);
        else if (key == "Guild") setGameValue("CurrentCharacterGuild", value);
        // Correctly handle the Alive status
        else if (key == "isAlive") {
            // Converts string "0" or "1" from file to int and updates state
            this->setIsAlive(value.toInt()); 
        }
        else if (statNames().contains(key)) {
            // Stat names (Strength, Intelligence, etc.) are saved as-is in the file
            setGameValue(QString("CurrentCharacter%1").arg(key), value.toInt());
        }
    }

    file.close();
    qDebug() << "Successfully loaded character:" << characterName;
    return true;
}
