#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>

#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <stdexcept>

// --- Data Structures ---

struct Item {
    std::string name;
    int16_t ID, att, def;
    int32_t price;
    int16_t floor, rarity;
    int32_t abilities;
    int16_t swings, specialType, spellIndex, spellID;
    int32_t charges; // Note: increased to 32bit to match rr.getDword() in your loader
    int32_t guilds;
    int16_t levelScale;
    float damageMod;
    int32_t alignmentFlags;
    int16_t nHands, type;
    int32_t resistanceFlags;
    int16_t statsRequired[7];
    int16_t statsMod[7];
    int16_t cursed, spellLvl, classRestricted;

    // Helper to get all fields as a list of strings for easy export
    QStringList toStringList() const {
        QStringList fields;
        fields << QString::fromStdString(name)
               << QString::number(ID) << QString::number(att) << QString::number(def)
               << QString::number(price) << QString::number(floor) << QString::number(rarity)
               << QString::number(abilities) << QString::number(swings) << QString::number(specialType)
               << QString::number(spellIndex) << QString::number(spellID) << QString::number(charges)
               << QString::number(guilds) << QString::number(levelScale) << QString::number(damageMod)
               << QString::number(alignmentFlags) << QString::number(nHands) << QString::number(type)
               << QString::number(resistanceFlags);
        
        for (int i = 0; i < 6; ++i) fields << QString::number(statsRequired[i]);
        for (int i = 0; i < 6; ++i) fields << QString::number(statsMod[i]);
        
        fields << QString::number(cursed) << QString::number(spellLvl) << QString::number(classRestricted);
        return fields;
    }
};

// --- Improved RecordReader ---

template <size_t RECORD_LENGTH> 
class RecordReader {
    uint8_t buffer[RECORD_LENGTH];
    std::ifstream file;
    size_t cursor = 0;

public:
    RecordReader(const std::string& filename) {
        file.open(filename, std::ios::binary);
        if (!file.is_open()) throw std::runtime_error("Could not open file: " + filename);
    }

    void read(size_t length = RECORD_LENGTH) {
        cursor = 0;
        if (length == 0) return;
        file.read(reinterpret_cast<char*>(buffer), length);
        if (file.fail() && !file.eof()) throw std::runtime_error("Binary read failure");
    }

    int16_t getWord() {
        if (cursor + 2 > RECORD_LENGTH) return 0;
        int16_t var = buffer[cursor] | (buffer[cursor + 1] << 8);
        cursor += 2;
        return var;
    }

    int32_t getDword() {
        if (cursor + 4 > RECORD_LENGTH) return 0;
        uint32_t var = 0;
        for (int i = 0; i < 4; i++) var |= (buffer[cursor + i] << (8 * i));
        cursor += 4;
        return static_cast<int32_t>(var);
    }

    float getFloat() {
        int32_t raw = getDword();
        return *reinterpret_cast<float*>(&raw);
    }

    std::string getString() {
        uint16_t len = static_cast<uint16_t>(getWord());
        if (cursor + len > RECORD_LENGTH) return "";
        std::string str(reinterpret_cast<char*>(buffer + cursor), len);
        cursor += len;
        return str;
    }
};

// --- Loading Logic ---

std::vector<Item> loadItems(const std::string& filename) {
    RecordReader<125> rr(filename);
    
    rr.read();
    if (rr.getString() != "1.1") throw std::runtime_error("Unsupported MDR version");
    
    rr.read(); // storeCode
    rr.read();
    uint16_t nItems = rr.getWord();
    
    std::vector<Item> items;
    items.reserve(nItems);

    for (size_t i = 0; i < nItems; i++) {
        rr.read();
        Item it;
        it.name = rr.getString();
        it.ID = rr.getWord();
        it.att = rr.getWord();
        it.def = rr.getWord();
        it.price = rr.getDword();
        it.floor = rr.getWord();
        it.rarity = rr.getWord();
        it.abilities = rr.getDword();
        it.swings = rr.getWord();
        it.specialType = rr.getWord();
        it.spellIndex = rr.getWord();
        it.spellID = rr.getWord();
        it.charges = rr.getDword();
        // Skip 'deleted' field implicitly via cursor or explicitly if needed
        it.guilds = rr.getDword();
        it.levelScale = rr.getWord();
        it.damageMod = rr.getFloat();
        it.alignmentFlags = rr.getDword();
        it.nHands = rr.getWord();
        it.type = rr.getWord();
        it.resistanceFlags = rr.getDword();
        for (int s = 0; s < 7; s++) it.statsRequired[s] = rr.getWord();
        for (int s = 0; s < 7; s++) it.statsMod[s] = rr.getWord();
        it.cursed = rr.getWord();
        it.spellLvl = rr.getWord();
        it.classRestricted = rr.getWord();
        items.push_back(it);
    }
    return items;
}

// --- Main Application ---

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("infile", "Input .MDR file");
    parser.addPositionalArgument("outfile", "Output file (optional, defaults to .csv)");
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        parser.showHelp(1);
    }

    QFileInfo inFile(args.at(0));
    QString outPath = (args.size() > 1) ? args.at(1) : inFile.absolutePath() + "/" + inFile.baseName() + ".csv";
    QString ext = QFileInfo(outPath).suffix().toLower();

    try {
        auto items = loadItems(inFile.absoluteFilePath().toStdString());
        
        QFile outFile(outPath);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) 
            throw std::runtime_error("Cannot write to output file");

        QTextStream out(&outFile);
        const QString headers = "name,ID,att,def,price,floor,rarity,abilities,swings,specialType,spellIndex,spellID,charges,guilds,levelScale,damageMod,alignmentFlags,nHands,type,resistanceFlags,StrReq,IntReq,WisReq,ConReq,ChaReq,DexReq,StrMod,IntMod,WisMod,ConMod,ChaMod,DexMod,cursed,spellLvl,classRestricted";

        if (ext == "csv") {
            out << headers << "\n";
            for (const auto& it : items) {
                QStringList row = it.toStringList();
                // Simple CSV escaping: if name has comma, wrap in quotes
                if (row[0].contains(',')) row[0] = "\"" + row[0] + "\"";
                out << row.join(',') << "\n";
            }
        } else if (ext == "json" || ext == "js") {
            if (ext == "js") out << "const itemData = ";
            out << "[\n";
            for (size_t i = 0; i < items.size(); ++i) {
                QStringList row = items[i].toStringList();
                // Quote all strings for JSON array format
                for (auto& s : row) s = "\"" + s + "\"";
                out << "  [" << row.join(", ") << "]" << (i == items.size() - 1 ? "" : ",") << "\n";
            }
            out << "]\n";
        }

        qDebug() << "Success:" << items.size() << "items written to" << outPath;

    } catch (const std::exception& e) {
        qCritical() << "Error:" << e.what();
        return 1;
    }

    return 0;
}
