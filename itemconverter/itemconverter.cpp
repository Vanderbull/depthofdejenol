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
#include <algorithm> // for std::min in RecordWriter (if used)

// --- MTypes.h Integration (Relevant structures for Item loading) ---
// Note: Only the Item structure and its related typedefs are included here 
// for brevity and direct relevance to itemconverter.
typedef std::vector<class Spell> Spells; // Forward declaration needed for unused typedefs
typedef std::vector<class Character> Characters;
typedef std::vector<class Monster> Monsters;

struct Item {
	std::string name;
	int16_t ID;
	int16_t att;
	int16_t def;
	int32_t price;
	int16_t floor;
	int16_t rarity;
	int32_t abilities;
	int16_t swings;
	int16_t specialType;
	int16_t spellIndex;
	int16_t spellID;
	int16_t charges;
	int16_t deleted;
	int32_t guilds;
	int16_t levelScale;
	float damageMod;
	int32_t alignmentFlags;
	int16_t nHands;
	int16_t type;
	int32_t resistanceFlags;
	int16_t statsRequired[7];
	int16_t statsMod[7];
	int16_t cursed;
	int16_t spellLvl;
	int16_t classRestricted;
};
typedef std::vector<Item> Items;
// -----------------------------------------------------------------------

// --- RecordReader.h Integration (The binary file reading class) ---
// This is the full RecordReader template class from the provided header.
template <size_t RECORD_LENGTH> class RecordReader {
	uint8_t buffer[RECORD_LENGTH];
	std::ifstream file;
	size_t cursor;

public:
	RecordReader(const std::string& filename) : buffer{ 0 } {
		file.open(filename, std::ios::binary);
		if (!file)
			throw "Unable to open file";
	}
	~RecordReader() {
		file.close();
	}

	void checklength(int length) {
		if (RECORD_LENGTH == 0) {
			read(length);
		}
		else {
			assert(cursor + length <= RECORD_LENGTH);
		}
	}

	void read(int length = RECORD_LENGTH) {
		cursor = 0;
		if (length == 0) {
			return;
		}
		file.read((char*)buffer, length);
		if (file.fail() && !file.eof())
			throw "File Read Error";
	}

	//seek cursor to the n'th record
	void seek(size_t recnum) {
		if (RECORD_LENGTH == 0) {
			file.seekg(recnum);
		}
		else {
			file.seekg((std::ifstream::pos_type)RECORD_LENGTH * (recnum - 1));
		}
	}

	//The 4 basic raw reading functions
	uint8_t& get(uint8_t & var) {
		checklength(1);
		var = buffer[cursor];
		cursor += 1;
		return var;
	}
	uint16_t& get(uint16_t & var) {
		checklength(2);
		var = buffer[cursor] + (buffer[cursor + 1] << 8);
		cursor += 2;
		return var;
	}
	uint32_t & get(uint32_t & var) {
		checklength(4);
		var = 0;
		for (size_t i = 0; i < 4; i++) {
			var = (var << 8) + buffer[cursor + 3 - i];
		}
		cursor += 4;
		return var;
	}
	uint64_t& get(uint64_t & var) {
		checklength(8);
		var = 0;
		for (size_t i = 0; i < 8; i++) {
			var = (var << 8) + buffer[cursor + 7 - i];
		}
		cursor += 8;
		return var;
	}

	//Basic raw read functions for more types
	int8_t& get(int8_t & var) {
		get(*reinterpret_cast<uint8_t*>(&var));
		return var;
	}
	int16_t& get(int16_t & var) {
		get(*reinterpret_cast<uint16_t*>(&var));
		return var;
	}
	int32_t& get(int32_t & var) {
		get(*reinterpret_cast<uint32_t*>(&var));
		return var;
	}
	int64_t& get(int64_t & var) {
		get(*reinterpret_cast<uint64_t*>(&var));
		return var;
	}
	float& get(float& var) {
		get(*reinterpret_cast<uint32_t*>(&var));
		return var;
	}

	//Read string. When length is 0, 16 bit length will be read from the stream first.
	std::string& get(std::string & str, uint16_t length = 0) {
		if (!length) {
			length = getWord();
		}
		checklength(length);
		// Note: The original implementation reads a null-terminated string without including the null byte in the string object
		// The original code: str = std::string(reinterpret_cast<char*>(buffer + cursor), reinterpret_cast<char*>(buffer + cursor + length));
		// Assuming the length read is the length of the string data itself:
		str = std::string(reinterpret_cast<char*>(buffer + cursor), length); 
		cursor = cursor + length;
		return str;
	}

	template<typename T>
	void getArray(T * var, size_t len) {
		for (size_t i = 0; i < len; i++) {
			get(var[i]);
		}
	}

	//
	// Named Get functions that return the values
	//

	std::string getString(uint16_t len = 0) {
		std::string var;
		return get(var, len);
	}

	uint8_t getByte() { //VB3 'String * 1'
		uint8_t var;
		return get(var);
	}
	int16_t getWord() { //VB3 'Integer'
		int16_t var;
		return get(var);
	}
	int32_t getDword() { //VB3 'Long'
		int32_t var;
		return get(var);
	}
	int64_t getIntCurrency() {//VB3 Currency rounded to 64-bit integer
		int64_t var;
		return get(var) / 10000;
	}
	float getFloat() { //VB3 Float
		float f;
		return get(f);
	}
	int64_t getCurrency() {//VB3 Currency is 64-bit integer 100th of a cent.
		int64_t var;
		get(var);
		return var;
	}
};
// --------------------------------------------------------------------------

// --- MLoader.cpp Integration (The loadItems function) ---
Items loadItems(std::string filename) {
	Items items;
	// RecordReader<125> is defined here as it's a fixed size for the Item record.
	RecordReader<125> rr(filename);
	rr.read();
	std::string version = rr.getString();
	assert(version == "1.1");
	rr.read();
	rr.getWord(); //uint16_t storeCode
	rr.read();
	uint16_t nItems = rr.getWord();
	items.reserve(nItems);
	for (size_t i = 0; i < nItems; i++) {
		rr.read();
		Item item;
		item.name = rr.getString();
		item.ID = rr.getWord();
		item.att = rr.getWord();
		item.def = rr.getWord();
		item.price = rr.getDword();
		item.floor = rr.getWord();
		item.rarity = rr.getWord();
		item.abilities = rr.getDword();
		item.swings = rr.getWord();
		item.specialType = rr.getWord();
		item.spellIndex = rr.getWord();
		item.spellID = rr.getWord();
		item.charges = rr.getDword();
		// item.deleted (int16_t) is read here in MTypes but skipped in MLoader.cpp, 
		// but the record size of 125 suggests it is accounted for in the layout. 
		// Assuming the original logic is correct, we skip it as per MLoader.cpp
		// and trust the RecordReader's cursor position.
		
		item.guilds = rr.getDword();
		item.levelScale = rr.getWord();
		item.damageMod = rr.getFloat();
		item.alignmentFlags = rr.getDword();
		item.nHands = rr.getWord();
		item.type = rr.getWord();
		item.resistanceFlags = rr.getDword();
		for (size_t s = 0; s < 7; s++) {
			item.statsRequired[s] = rr.getWord();
		}
		for (size_t s = 0; s < 7; s++) {
			rr.get(item.statsMod[s]);
		}
		item.cursed = rr.getWord();
		item.spellLvl = rr.getWord();
		item.classRestricted = rr.getWord();
		items.push_back(item);
	}
	return items;
}
// --------------------------------------------------------------------------


// The common header string for all formats (Copied from original itemsheet.cpp)
const QString ITEM_HEADERS = "name, ID, att, def, price, floor, rarity, abilities, swings, specialType, spellIndex, spellID, charges, guilds, levelScale, damageMod, alignmentFlags, nHands, type, resistanceFlags, StrReq, IntReq, WisReq, ConReq, ChaReq, DexReq, StrMod, IntMod, WisMod, ConMod, ChaMod, DexMod, cursed, spellLvl, classRestricted";

int main(int argc, char *argv[]) {
    // 1. Setup Qt Application and Command Line Parser
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ItemConverter");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Converts Dejenol item data (.MDR) to various formats (.csv, .json, .js).");
    parser.addHelpOption();
    parser.addVersionOption();

    // The original program used positional arguments, which are accessed via positionalArguments() in Qt
    QCommandLineOption inFileOption("infile", "Input file to convert (e.g., MDATA3.MDR).", "file");
    parser.addOption(inFileOption);
    QCommandLineOption outFileOption("outfile", "Optional output file name. Defaults to [infile].csv.", "file");
    parser.addOption(outFileOption);

    // Process the arguments
    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.count() < 1) {
        // Output usage message similar to original
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
        // Default: replace extension with .csv
        outPath = inFile.absolutePath() + "/" + inFile.baseName() + ".csv";
    }
    
    QFileInfo outFile(outPath);
    QString extension = outFile.suffix().toLower();
    
    // 3. Load Items (Using the C++ loadItems function)
    Items items;
    try {
        items = loadItems(inPath.toStdString()); // Call the C++ loader
    } catch (const char* msg) {
        qCritical() << "Error loading items:" << msg;
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

    // Logic for output formatting (CSV, JSON, JS) copied from original itemsheet.cpp
    if (extension == "js") {
        f << "itemVarNames = [\"" << ITEM_HEADERS.split(", ").join("\", \"") << "\"]\n";
        f << "items = [" << "\n";
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
        f << ITEM_HEADERS << "\n";
        ender = "\n";
        ender2 = "\n";
    }

    // 5. Write Item Data
    bool first = true;
    for (const auto& item : items) {
        if (!first) f << ender;
        first = false;
        
        f << starter;
        
        // Convert std::string name to QString for output
        QString itemName = QString::fromStdString(item.name);

        if (quotes) {
            f << "\"" << itemName << "\",";
        } else {
            f << itemName << ",";
        }
        
        // Write all fields using QString::number for conversion, matching original item order
        f << QString::number(item.ID) << ",";
        f << QString::number(item.att) << ",";
        f << QString::number(item.def) << ",";
        f << QString::number(item.price) << ",";
        f << QString::number(item.floor) << ",";
        f << QString::number(item.rarity) << ",";
        f << QString::number(item.abilities) << ",";
        f << QString::number(item.swings) << ",";
        f << QString::number(item.specialType) << ",";
        f << QString::number(item.spellIndex) << ",";
        f << QString::number(item.spellID) << ",";
        f << QString::number(item.charges) << ",";
        // f << item.deleted << ","; // deleted field is in struct but skipped in original output logic
        f << QString::number(item.guilds) << ",";
        f << QString::number(item.levelScale) << ",";
        f << QString::number(item.damageMod) << ",";
        f << QString::number(item.alignmentFlags) << ",";
        f << QString::number(item.nHands) << ",";
        f << QString::number(item.type) << ",";
        f << QString::number(item.resistanceFlags) << ",";
        
        // Write Stats Required (first 6 elements)
        for (int i = 0; i < 6; i++) {
            f << QString::number(item.statsRequired[i]) << ",";
        }
        // Write Stats Modifiers (first 6 elements)
        for (int i = 0; i < 6; i++) {
            f << QString::number(item.statsMod[i]) << ((i == 5) ? "" : ","); // No comma after the last statMod
        }
        
        // Write Final flags/levels (no comma separation needed here as they were handled in the last loop)
        f << "," << QString::number(item.cursed) << ",";
        f << QString::number(item.spellLvl) << ",";
        f << QString::number(item.classRestricted);
    }
    
    // 6. Write Footer and Close File
    f << ender2;
    file.close();

    qDebug() << "Successfully converted items from" << inPath << "to" << outPath;
    
    return 0;
}
