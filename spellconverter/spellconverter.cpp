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

// --- MTypes.h / Spell Definition (Corrected) ---
// Structure corrected to match the field reading sequence in the MLoader.cpp snippet,
// which is the source of truth for the binary file layout.
struct Spell {
	std::string name; // vbstring spellname
	int16_t ID; // Corresponds to MLoader.cpp s.ID
	int16_t category; // Corresponds to MLoader.cpp s.category (class)
	int16_t level; // Corresponds to MLoader.cpp s.level (spellLevel)
	int16_t u4; // Corresponds to MLoader.cpp s.u4
	// Removed m_alwaysZero to fix alignment
	int16_t killEffect; // Corresponds to MLoader.cpp s.killEffect
	int16_t affectMonster; // Corresponds to MLoader.cpp s.affectMonster
	int16_t affectGroup; // Corresponds to MLoader.cpp s.affectGroup
	int16_t damage1; // Corresponds to MLoader.cpp s.damage1
	int16_t damage2; // Corresponds to MLoader.cpp s.damage2
	int16_t specialEffect; // Corresponds to MLoader.cpp s.specialEffect
	int16_t required[7]; // Required Stats (7 elements)
	int16_t resistedBy; // Final field (resistedBy)
};
typedef std::vector<Spell> Spells;

// Forward declarations for types used in MLoader.h/MTypes.h
typedef std::vector<class Item> Items; 
typedef std::vector<class Character> Characters;
typedef std::vector<class Monster> Monsters;

// --- RecordReader.h Integration (The binary file reading class) ---
// (Implementation unchanged, relying on external RecordReader.h)
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

	// The 4 basic raw reading functions (Simplified for typical types)
	template<typename T>
	T& get(T & var) {
        checklength(sizeof(T));
        // Simple little-endian conversion
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

	// Read string (VB string: reads 16-bit length first)
	std::string& get(std::string & str, uint16_t length = 0) {
		if (!length) {
			length = getWord();
		}
		checklength(length);
		// This line is the source of the assertion failure if length is too large
		str = std::string(reinterpret_cast<char*>(buffer + cursor), reinterpret_cast<char*>(buffer + cursor + length));
		cursor = cursor + length;
		return str;
	}

	// Named Get functions
	int16_t getWord() { int16_t var; return get(var); }
	
	template<typename T>
	void getArray(T * var, size_t len) {
		for (size_t i = 0; i < len; i++) {
			get(var[i]);
		}
	}
};
// --------------------------------------------------------------------------

// --- MLoader.cpp Integration (The loadSpells function) ---
Spells loadSpells(std::string filename) {
	RecordReader<75> buff(filename);
	Spells spells;
	
	// rec1: vbstring fileversion
	buff.read();
	std::string version;
    buff.get(version); // Use ref version matching MLoader.cpp
	assert(version == "1.1");
	
	// rec2: short NumSpells
	buff.read();
	uint16_t numSpells;
    buff.get(numSpells);
	
	spells.reserve(numSpells);
	for (size_t i = 0; i < numSpells; i++) {
		Spell s;
		buff.read(); // Read the 75-byte record for the spell
        
		buff.get(s.name); // Read vbstring name
		buff.get(s.ID);
		buff.get(s.category);
		buff.get(s.level);
		buff.get(s.u4);
		// Skip the field that was causing misalignment
		buff.get(s.killEffect);
		buff.get(s.affectMonster);
		buff.get(s.affectGroup);
		buff.get(s.damage1);
		buff.get(s.damage2);
		buff.get(s.specialEffect);
		
        // Read required stats array
		buff.getArray(s.required, 7); 
        
		buff.get(s.resistedBy);
		
		spells.push_back(s);
	}
	return spells;
}
// --------------------------------------------------------------------------


// Header string updated to reflect the fields actually read
const QString SPELL_HEADERS = "name, ID, Class, Level, u4, killEffect, affectMonster, affectGroup, damage1, damage2, specialEffect, ReqStr, ReqInt, ReqWis, ReqCon, ReqCha, ReqDex, ReqU7, resistedBy";

int main(int argc, char *argv[]) {
    // 1. Setup Qt Application and Command Line Parser
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("SpellConverter");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Converts Dejenol spell data (.MDR) to various formats (.csv, .json, .js).");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption inFileOption("infile", "Input file to convert (e.g., MDATA2.MDR).", "file");
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
    
    if (inFile.suffix().toLower() != "mdr") {
        qWarning() << "Error: Input file must have the extension .mdr or .MDR.";
        return 1;
    }

    QString outPath;
    if (args.count() > 1) {
        outPath = args.at(1);
    } else {
        outPath = inFile.absolutePath() + "/" + inFile.baseName() + ".csv";
    }
    
    QFileInfo outFile(outPath);
    QString extension = outFile.suffix().toLower();
    
    // 3. Load Spells (Using the C++ loadSpells function)
    Spells spells;
    try {
        spells = loadSpells(inPath.toStdString()); // Call the C++ loader
    } catch (const char* msg) {
        qCritical() << "Error loading spells from file: " << msg;
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
        f << "spellVarNames = [\"" << SPELL_HEADERS.split(", ").join("\", \"") << "\"]\n";
        f << "spells = [" << "\n";
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
        f << SPELL_HEADERS << "\n";
        ender = "\n";
        ender2 = "\n";
    }

    // 5. Write Spell Data
    bool first = true;
    for (const auto& s : spells) {
        if (!first) f << ender;
        first = false;
        
        f << starter;
        
        QString spellName = QString::fromStdString(s.name);

        if (quotes) {
            f << "\"" << spellName << "\",";
        } else {
            f << spellName << ",";
        }
        
        // Write standard fields
        f << QString::number(s.ID) << ",";
        f << QString::number(s.category) << ",";
        f << QString::number(s.level) << ",";
        f << QString::number(s.u4) << ",";
        // f << QString::number(s.m_alwaysZero) << ","; // Removed
        f << QString::number(s.killEffect) << ",";
        f << QString::number(s.affectMonster) << ",";
        f << QString::number(s.affectGroup) << ",";
        f << QString::number(s.damage1) << ",";
        f << QString::number(s.damage2) << ",";
        f << QString::number(s.specialEffect) << ",";

        // Write Required Stats (7 elements)
        for (int i = 0; i < 7; i++) {
            f << QString::number(s.required[i]) << ",";
        }

        // Write Final field
        f << QString::number(s.resistedBy); // No trailing comma
    }
    
    // 6. Write Footer and Close File
    f << ender2;
    file.close();

    qDebug() << "Successfully converted spells from" << inPath << "to" << outPath;
    
    return 0;
}
