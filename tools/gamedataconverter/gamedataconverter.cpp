#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// --- Standard C++ Headers ---
#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <sstream>

// --- Type Definitions (short=int16_t, long=int32_t, float=float) ---
typedef int16_t short_t;
typedef int32_t long_t;
typedef float float_t;

// --- Data Structures ---
struct Race {
	std::string name;
	short_t minStats[7];
	short_t maxStats[7];
	short_t resistances[12];
	long_t alignment;
	short_t size;
	short_t bonusPoints;
	short_t maxAge;
	float_t expFactor;
};

struct Guild {
	std::string name;
	short_t averageHits;
	short_t MaxLevel;
	short_t MH;
	float_t expFactor;
	short_t u3;
	short_t reqStats[7];
	long_t alignment;
	float_t abilityrates[7];
	short_t u7;
	float_t u8;
	short_t questPercentage;
	float_t spellstuff[38];
	long_t RaceMask;
	short_t u12;
	float_t levelMod;
	float_t u13;
	float_t u14;
	short_t u15;
	float_t u16;
	short_t u17;
	short_t u18;
};

struct SubItemType {
	std::string name;
	short_t itemtype;
};

struct ItemType {
	std::string name;
	short_t IsEquipable;
};

struct SubMonsterType {
	std::string name;
	short_t MonsterType;
};

struct MonsterType {
	std::string name;
	short_t notUsed;
};

struct GameData {
    std::string version;
    std::vector<Race> races;
    std::vector<Guild> guilds;
    std::vector<SubItemType> subItemTypes;
    std::vector<ItemType> itemTypes;
    std::vector<SubMonsterType> subMonsterTypes;
    std::vector<MonsterType> monsterTypes;
    
    // Global counts (for reference)
    short_t numRaces;
    short_t numGuilds;
    short_t numSubItemTypes;
    short_t numItemTypes;
    short_t numMonsterSubTypes;
    short_t numMonsterTypes;
};

// --- RecordReader.h Integration ---
template <size_t RECORD_LENGTH> class RecordReader {
	uint8_t buffer[RECORD_LENGTH];
	std::ifstream file;
	size_t cursor;

public:
	RecordReader(const std::string& filename) : buffer{ 0 } {
		file.open(filename, std::ios::binary);
		if (!file) throw "Unable to open file";
	}
	~RecordReader() { file.close(); }

	void checklength(int length) {
		if (RECORD_LENGTH == 0) { read(length); }
		else { assert(cursor + length <= RECORD_LENGTH); }
	}

	void read(int length = RECORD_LENGTH) {
		cursor = 0;
		if (length == 0) return;
		file.read((char*)buffer, length);
		if (file.fail() && !file.eof()) throw "File Read Error";
	}

	template<typename T>
	T& get(T & var) {
        checklength(sizeof(T));
        uint8_t *ptr = buffer + cursor;
        if constexpr (sizeof(T) == 1) {
            var = *reinterpret_cast<T*>(ptr);
        } else if constexpr (sizeof(T) == 2) {
            uint16_t val = ptr[0] | (ptr[1] << 8);
            var = *reinterpret_cast<T*>(&val);
        } else if constexpr (sizeof(T) == 4) {
            uint32_t val = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
            var = *reinterpret_cast<T*>(&val);
        } else {
            throw "Type size not implemented in get() template";
        }
		cursor += sizeof(T);
		return var;
	}

	std::string& get(std::string & str, uint16_t length = 0) {
		if (!length) {
			length = getWord();
		}
		checklength(length);
		str = std::string(reinterpret_cast<char*>(buffer + cursor), reinterpret_cast<char*>(buffer + cursor + length));
		cursor = cursor + length;
		return str;
	}

	short_t getWord() { short_t var; return get(var); } // Same as short
	long_t getDword() { long_t var; return get(var); } // Same as long
	float_t getFloat() { float_t var; return get(var); } // Same as float
	
	template<typename T>
	void getArray(T * var, size_t len) {
		for (size_t i = 0; i < len; i++) {
			get(var[i]);
		}
	}
};
// --------------------------------------------------------------------------

// --- Load Function (MDATA1.MDR) ---
GameData loadGameData(std::string filename) {
	// Record length is 260 bytes
	RecordReader<260> rr(filename); 
	GameData gd;

	// rec1: vbstring fileversion
	rr.read();
	rr.get(gd.version);
	assert(gd.version == "1.1"); // Assuming consistency

	// rec2-rec7: Global Counts (Control Data)
	rr.read(); rr.get(gd.numRaces);
	rr.read(); rr.get(gd.numGuilds);
	rr.read(); rr.get(gd.numSubItemTypes);
	rr.read(); rr.get(gd.numItemTypes);
	rr.read(); rr.get(gd.numMonsterSubTypes);
	rr.read(); rr.get(gd.numMonsterTypes);
    
    // --- Load Races Block ---
    gd.races.reserve(gd.numRaces);
	for (size_t i = 0; i < gd.numRaces; i++) {
		rr.read();
		Race r;
		rr.get(r.name);
		rr.getArray(r.minStats, 7);
		rr.getArray(r.maxStats, 7);
		rr.getArray(r.resistances, 12);
		rr.get(r.alignment);
		rr.get(r.size);
		rr.get(r.bonusPoints);
		rr.get(r.maxAge);
		rr.get(r.expFactor);
		gd.races.push_back(r);
	}

    // --- Load Guilds Block ---
    gd.guilds.reserve(gd.numGuilds);
	for (size_t i = 0; i < gd.numGuilds; i++) {
		rr.read();
		Guild g;
		rr.get(g.name);
		rr.get(g.averageHits);
		rr.get(g.MaxLevel);
		rr.get(g.MH);
		rr.get(g.expFactor);
		rr.get(g.u3);
		rr.getArray(g.reqStats, 7);
		rr.get(g.alignment);
		rr.getArray(g.abilityrates, 7); // 7 floats
		rr.get(g.u7);
		rr.get(g.u8);
		rr.get(g.questPercentage);
		rr.getArray(g.spellstuff, 38); // 38 floats
		rr.get(g.RaceMask);
		rr.get(g.u12);
		rr.get(g.levelMod);
		rr.get(g.u13);
		rr.get(g.u14);
		rr.get(g.u15);
		rr.get(g.u16);
		rr.get(g.u17);
		rr.get(g.u18);
		gd.guilds.push_back(g);
	}

    // --- Load SubItemTypes Block ---
    gd.subItemTypes.reserve(gd.numSubItemTypes);
	for (size_t i = 0; i < gd.numSubItemTypes; i++) {
		rr.read();
		SubItemType si;
		rr.get(si.name);
		rr.get(si.itemtype);
		gd.subItemTypes.push_back(si);
	}

    // --- Load ItemTypes Block ---
    gd.itemTypes.reserve(gd.numItemTypes);
	for (size_t i = 0; i < gd.numItemTypes; i++) {
		rr.read();
		ItemType it;
		rr.get(it.name);
		rr.get(it.IsEquipable);
		gd.itemTypes.push_back(it);
	}

    // --- Load SubMonsterTypes Block ---
    gd.subMonsterTypes.reserve(gd.numMonsterSubTypes);
	for (size_t i = 0; i < gd.numMonsterSubTypes; i++) {
		rr.read();
		SubMonsterType sm;
		rr.get(sm.name);
		rr.get(sm.MonsterType);
		gd.subMonsterTypes.push_back(sm);
	}

    // --- Load MonsterTypes Block ---
    gd.monsterTypes.reserve(gd.numMonsterTypes);
	for (size_t i = 0; i < gd.numMonsterTypes; i++) {
		rr.read();
		MonsterType mt;
		rr.get(mt.name);
		rr.get(mt.notUsed);
		gd.monsterTypes.push_back(mt);
	}

	return gd;
}
// --------------------------------------------------------------------------

// Helper function to convert Race struct to QJsonObject
QJsonObject raceToJson(const Race& r) {
    QJsonObject obj;
    obj["name"] = QString::fromStdString(r.name);
    // Write arrays
    QStringList statNames = {"Str", "Int", "Wis", "Con", "Cha", "Dex", "U7"};
    QStringList resNames = {"Fire", "Cold", "Electric", "Mind", "Poison", "Disease", "Magic", "Physical", "Weapon", "Spell", "Special", "U12"};
    
    QJsonObject minStats;
    for (int i = 0; i < 7; ++i) minStats[statNames[i]] = r.minStats[i];
    obj["minStats"] = minStats;

    QJsonObject maxStats;
    for (int i = 0; i < 7; ++i) maxStats[statNames[i]] = r.maxStats[i];
    obj["maxStats"] = maxStats;

    QJsonObject resistances;
    for (int i = 0; i < 12; ++i) resistances[resNames[i]] = r.resistances[i];
    obj["resistances"] = resistances;

    obj["alignment"] = (qlonglong)r.alignment;
    obj["size"] = r.size;
    obj["bonusPoints"] = r.bonusPoints;
    obj["maxAge"] = r.maxAge;
    obj["expFactor"] = r.expFactor;
    return obj;
}

// Helper function to convert Guild struct to QJsonObject
QJsonObject guildToJson(const Guild& g) {
    QJsonObject obj;
    obj["name"] = QString::fromStdString(g.name);
    obj["averageHits"] = g.averageHits;
    obj["MaxLevel"] = g.MaxLevel;
    obj["MH"] = g.MH;
    obj["expFactor"] = g.expFactor;
    obj["u3"] = g.u3;
    
    QStringList statNames = {"Str", "Int", "Wis", "Con", "Cha", "Dex", "U7"};
    QJsonObject reqStats;
    for (int i = 0; i < 7; ++i) reqStats[statNames[i]] = g.reqStats[i];
    obj["reqStats"] = reqStats;

    obj["alignment"] = (qlonglong)g.alignment;
    
    QJsonArray abilityrates;
    for (int i = 0; i < 7; ++i) abilityrates.append(g.abilityrates[i]);
    obj["abilityrates"] = abilityrates;

    obj["u7"] = g.u7;
    obj["u8"] = g.u8;
    obj["questPercentage"] = g.questPercentage;

    QJsonArray spellstuff;
    for (int i = 0; i < 38; ++i) spellstuff.append(g.spellstuff[i]);
    obj["spellstuff"] = spellstuff;
    
    obj["RaceMask"] = (qlonglong)g.RaceMask;
    obj["u12"] = g.u12;
    obj["levelMod"] = g.levelMod;
    obj["u13"] = g.u13;
    obj["u14"] = g.u14;
    obj["u15"] = g.u15;
    obj["u16"] = g.u16;
    obj["u17"] = g.u17;
    obj["u18"] = g.u18;
    return obj;
}

int main(int argc, char *argv[]) {
    // 1. Setup Qt Application and Command Line Parser
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("GameDataConverter");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Converts Dejenol game data (MDATA1.MDR) containing multiple blocks to a single JSON/JS file.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption inFileOption("infile", "Input file to convert (e.g., MDATA1.MDR).", "file");
    parser.addOption(inFileOption);
    QCommandLineOption outFileOption("outfile", "Optional output file name. Defaults to [infile].js.", "file");
    parser.addOption(outFileOption);

    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.count() < 1) {
        qWarning() << "usage:" << QFileInfo(app.applicationFilePath()).fileName() << "infile [outfile]";
        parser.showHelp(1);
        return 1;
    }

    // 2. Determine File Paths
    QFileInfo inFile(args.at(0));
    QString inPath = inFile.absoluteFilePath();
    
    if (inFile.suffix().toLower() != "mdr") {
        qWarning() << "Warning: Input file does not have the extension .mdr or .MDR.";
    }

    QString outPath;
    if (args.count() > 1) {
        outPath = args.at(1);
    } else {
        outPath = inFile.absolutePath() + "/" + inFile.baseName() + ".js";
    }
    
    QFileInfo outFile(outPath);
    QString extension = outFile.suffix().toLower();
    
    // 3. Load Game Data
    GameData gd;
    try {
        gd = loadGameData(inPath.toStdString());
    } catch (const char* msg) {
        qCritical() << "Error loading game data from file:" << msg;
        return 1;
    } catch (...) {
        qCritical() << "An unknown error occurred during file loading.";
        return 1;
    }
    
    // 4. Create JSON/JS Output Structure
    QJsonObject root;
    root["fileVersion"] = QString::fromStdString(gd.version);
    
    // --- Races ---
    QJsonArray racesArray;
    for (const auto& r : gd.races) {
        racesArray.append(raceToJson(r));
    }
    root["Races"] = racesArray;

    // --- Guilds ---
    QJsonArray guildsArray;
    for (const auto& g : gd.guilds) {
        guildsArray.append(guildToJson(g));
    }
    root["Guilds"] = guildsArray;

    // --- Simple Blocks (SubItemTypes, ItemTypes, SubMonsterTypes, MonsterTypes) ---
    QJsonArray subItemTypesArray;
    for (const auto& si : gd.subItemTypes) {
        QJsonObject obj;
        obj["name"] = QString::fromStdString(si.name);
        obj["itemTypeID"] = si.itemtype;
        subItemTypesArray.append(obj);
    }
    root["SubItemTypes"] = subItemTypesArray;

    QJsonArray itemTypesArray;
    for (const auto& it : gd.itemTypes) {
        QJsonObject obj;
        obj["name"] = QString::fromStdString(it.name);
        obj["isEquipable"] = it.IsEquipable;
        itemTypesArray.append(obj);
    }
    root["ItemTypes"] = itemTypesArray;

    QJsonArray subMonsterTypesArray;
    for (const auto& sm : gd.subMonsterTypes) {
        QJsonObject obj;
        obj["name"] = QString::fromStdString(sm.name);
        obj["monsterTypeID"] = sm.MonsterType;
        subMonsterTypesArray.append(obj);
    }
    root["SubMonsterTypes"] = subMonsterTypesArray;

    QJsonArray monsterTypesArray;
    for (const auto& mt : gd.monsterTypes) {
        QJsonObject obj;
        obj["name"] = QString::fromStdString(mt.name);
        obj["notUsed"] = mt.notUsed;
        monsterTypesArray.append(obj);
    }
    root["MonsterTypes"] = monsterTypesArray;

    // 5. Write Output File
    QFile file(outPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Error: Could not open output file for writing:" << outPath;
        return 1;
    }
    
    QTextStream f(&file);
    QJsonDocument doc(root);

    if (extension == "json") {
        f << doc.toJson(QJsonDocument::Indented);
    } else if (extension == "js") {
        // Output as a JavaScript variable (camelCase for JS convention)
        f << "const gameData = ";
        f << doc.toJson(QJsonDocument::Indented);
        f << ";";
    } else {
        qWarning() << "Warning: Output extension" << extension << "is not explicitly handled. Defaulting to JSON style.";
        f << doc.toJson(QJsonDocument::Indented);
    }

    file.close();
    qDebug() << "Successfully converted game data from" << inPath << "to" << outPath;
    
    return 0;
}
