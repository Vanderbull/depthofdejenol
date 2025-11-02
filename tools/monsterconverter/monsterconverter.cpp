#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>

// --- Standard C++ Headers required by MTypes, RecordReader, and MLoader ---
#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>

// --- MTypes.h Integration (Monster Definition) ---
// Assuming the Monster struct based on the fields used in monstersheet.cpp
struct Monster {
	std::string name;
	int16_t att;
	int16_t def;
	int16_t id;
	int16_t hits;
	int16_t numGroups;
	int16_t picID;
	int16_t lockedChance;
	int16_t levelFound;

	int16_t resistances[12]; // ResFire till ResSpecial
	
	int32_t specialPropertyFlags;
	int32_t specialAttackFlags;
	int32_t spellFlags;

	int16_t chance;
	int16_t boxChance[4]; // Placeholder for 4, though sheet only prints 3
	
	int16_t alignment;
	int16_t ingroup;
	int32_t goldFactor;
	int32_t trapFlags;
	int16_t guildlevel;

	int16_t stats[7]; // Placeholder for 7, though sheet only prints 6
	
	int16_t type;
	float damageMod;
	int16_t companionType;
	int16_t companionSpawnMode;
	int16_t companionID;

	int16_t items[11]; // Placeholder for 11, though sheet only prints 10
	
	int16_t subtype;
	int16_t companionSubtype;
	int16_t deleted;
};
typedef std::vector<Monster> Monsters;

// Forward declarations for types used in MLoader.h/MTypes.h
typedef std::vector<class Spell> Spells; 
typedef std::vector<class Item> Items; 
typedef std::vector<class Character> Characters;

// --- RecordReader.h Integration (The binary file reading class) ---
// Note: In a real project, this would be in a separate .h file and included.
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

	std::string getString(uint16_t len = 0) {
		std::string var;
		if (!len) len = getWord();
		checklength(len);
		var = std::string(reinterpret_cast<char*>(buffer + cursor), len);
		cursor += len;
		return var;
	}

	// Basic raw read functions for various types
	template<typename T>
	T& get(T & var) {
        checklength(sizeof(T));
        // Simple little-endian conversion assuming a fixed structure
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
            // Fallback for types not explicitly defined in snippet (e.g. 8-byte)
            throw "Type size not implemented in get() template";
        }
		cursor += sizeof(T);
		return var;
	}
    
	// Named Get functions that return the values
	int16_t getWord() { int16_t var; return get(var); }
	int32_t getDword() { int32_t var; return get(var); }
	float getFloat() { float f; return get(f); }

	template<typename T>
	void getArray(T * var, size_t len) {
		for (size_t i = 0; i < len; i++) {
			get(var[i]);
		}
	}
};
// --------------------------------------------------------------------------

// --- MLoader.cpp Integration (The loadMonsters function) ---
Monsters loadMonsters(std::string filename) {
	Monsters monsters;
	// Monster record size is 160 bytes, inferred from MSaver.cpp
	RecordReader<160> rr(filename); 
	
	// Read Header Records
	rr.read();
	std::string version = rr.getString();
	assert(version == "1.1");
	rr.read();
	rr.getWord(); //uint16_t storeCode (or unused)
	rr.read();
	uint16_t nMonsters = rr.getWord();
	monsters.reserve(nMonsters);
	
	// Read Monster Records
	for (size_t i = 0; i < nMonsters; i++) {
		rr.read();
		Monster m;
		m.name = rr.getString();
		m.att = rr.getWord();
		m.def = rr.getWord();
		m.id = rr.getWord();
		m.hits = rr.getWord();
		m.numGroups = rr.getWord();
		m.picID = rr.getWord();
		m.lockedChance = rr.getWord();
		m.levelFound = rr.getWord();

		// Resistances (12 elements)
		rr.getArray(m.resistances, 12);

		m.specialPropertyFlags = rr.getDword();
		m.specialAttackFlags = rr.getDword();
		m.spellFlags = rr.getDword();

		m.chance = rr.getWord();
		
		// boxChance (4 elements)
		rr.getArray(m.boxChance, 4);

		m.alignment = rr.getWord();
		m.ingroup = rr.getWord();
		m.goldFactor = rr.getDword();
		m.trapFlags = rr.getDword();
		m.guildlevel = rr.getWord();

		// Stats (7 elements)
		rr.getArray(m.stats, 7);

		m.type = rr.getWord();
		m.damageMod = rr.getFloat();
		m.companionType = rr.getWord();
		m.companionSpawnMode = rr.getWord();
		m.companionID = rr.getWord();

		// Items (11 elements)
		rr.getArray(m.items, 11);

		m.subtype = rr.getWord();
		m.companionSubtype = rr.getWord();
		m.deleted = rr.getWord();

		monsters.push_back(m);
	}
	return monsters;
}
// --------------------------------------------------------------------------


// Header string derived directly from the field output order in monstersheet.cpp
const QString MONSTER_HEADERS = "name, att, def, id, hits, numGroups, picID, lockedChance, levelFound, ResFire, ResCold, ResElectric, ResMind, ResPoison, ResDisease, ResMagic, ResPhysical, ResWeapon, ResSpell, ResSpecial, specialPropertyFlags, specialAttackFlags, spellFlags, chance, boxChance0, boxChance1, boxChance2, alignment, ingroup, goldFactor, trapFlags, guildlevel, StatStr, StatInt, StatWis, StatCon, StatCha, StatDex, type, damageMod, companionType, companionSpawnMode, companionID, Item0, Item1, Item2, Item3, Item4, Item5, Item6, Item7, Item8, Item9, subtype, companionSubtype, deleted";

int main(int argc, char *argv[]) {
    // 1. Setup Qt Application and Command Line Parser
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("MonsterConverter");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Converts Dejenol monster data (.MDR) to various formats (.csv, .json, .js).");
    parser.addHelpOption();
    parser.addVersionOption();

    // The original program used positional arguments
    QCommandLineOption inFileOption("infile", "Input file to convert (e.g., MDATA5.MDR).", "file");
    parser.addOption(inFileOption);
    QCommandLineOption outFileOption("outfile", "Optional output file name. Defaults to [infile].csv.", "file");
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
    
    // The original monstersheet.cpp used "MDATA5.MDR" as default
    if (inPath.isEmpty()) inPath = "MDATA5.MDR"; 
    
    if (inFile.suffix().toLower() != "mdr") {
        qWarning() << "Error: Input file must have the extension .mdr or .MDR.";
        return 1;
    }

    QString outPath;
    if (args.count() > 1) {
        outPath = args.at(1);
    } else {
        // Default: replace extension with .csv
        outPath = inFile.absolutePath() + "/" + inFile.baseName() + ".csv";
    }
    
    QFileInfo outFile(outPath);
    QString extension = outFile.suffix().toLower();
    
    // 3. Load Monsters (Using the C++ loadMonsters function)
    Monsters monsters;
    try {
        monsters = loadMonsters(inPath.toStdString()); // Call the C++ loader
    } catch (const char* msg) {
        qCritical() << "Error loading monsters from file:" << msg;
        return 1;
    } catch (...) {
        qCritical() << "An unknown error occurred during file loading.";
        return 1;
    }
    
    // 4. Setup Output Stream and Formatting
    QFile file(outPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Error: Could not open output file for writing:" << outPath;
        return 1;
    }

    QTextStream f(&file);
    QString starter = "";
    QString ender = "";
    QString ender2 = "";
    bool quotes = false;

    // Logic for output formatting (CSV, JSON, JS)
    if (extension == "js") {
        f << "monsterVarNames = [\"" << MONSTER_HEADERS.split(", ").join("\", \"") << "\"]\n";
        f << "monsters = [" << "\n";
        starter = "\t[";
        ender = "],\n";
        ender2 = "]\n]";
        quotes = true;
    } else if (extension == "json") {
        f << "[" << "\n";
        starter = "\t[";
        ender = "],\n";
        ender2 = "]\n]";
        quotes = true;
    } else if (extension == "csv") {
        f << MONSTER_HEADERS << "\n";
        ender = "\n";
        ender2 = "\n";
    }

    // 5. Write Monster Data
    bool first = true;
    for (const auto& m : monsters) {
        if (!first) f << ender;
        first = false;
        
        f << starter;
        
        // Convert std::string name to QString for output
        QString monsterName = QString::fromStdString(m.name);

        if (quotes) {
            f << "\"" << monsterName << "\",";
        } else {
            f << monsterName << ",";
        }
        
        // Write all standard fields
        f << QString::number(m.att) << ",";
        f << QString::number(m.def) << ",";
        f << QString::number(m.id) << ",";
        f << QString::number(m.hits) << ",";
        f << QString::number(m.numGroups) << ",";
        f << QString::number(m.picID) << ",";
        f << QString::number(m.lockedChance) << ",";
        f << QString::number(m.levelFound) << ",";
        
        // Write Resistances (12 elements)
        for (int i = 0; i < 12; i++) {
            f << QString::number(m.resistances[i]) << ",";
        }
        
        // Write Flags and Chance
        f << QString::number(m.specialPropertyFlags) << ",";
        f << QString::number(m.specialAttackFlags) << ",";
        f << QString::number(m.spellFlags) << ",";
        f << QString::number(m.chance) << ",";

        // Write Box Chances (3 elements, as per original loop 0 to 2)
        for (int i = 0; i < 3; i++) {
            f << QString::number(m.boxChance[i]) << ",";
        }
        
        // Write Alignment and misc flags
        f << QString::number(m.alignment) << ",";
        f << QString::number(m.ingroup) << ",";
        f << QString::number(m.goldFactor) << ",";
        f << QString::number(m.trapFlags) << ",";
        f << QString::number(m.guildlevel) << ",";

        // Write Stats (6 elements, as per original loop 0 to 5)
        for (int i = 0; i < 6; i++) {
            f << QString::number(m.stats[i]) << ",";
        }
        
        // Write Type and Companions
        f << QString::number(m.type) << ",";
        f << QString::number(m.damageMod) << ",";
        f << QString::number(m.companionType) << ",";
        f << QString::number(m.companionSpawnMode) << ",";
        f << QString::number(m.companionID) << ",";

        // Write Items (10 elements, as per original loop 0 to 9)
        for (int i = 0; i < 10; i++) {
            f << QString::number(m.items[i]) << ",";
        }

        // Write Final flags (subtype, companionSubtype, deleted)
        f << QString::number(m.subtype) << ",";
        f << QString::number(m.companionSubtype) << ",";
        f << QString::number(m.deleted); // No trailing comma
    }
    
    // 6. Write Footer and Close File
    f << ender2;
    file.close();

    qDebug() << "Successfully converted monsters from" << inPath << "to" << outPath;
    
    return 0;
}
