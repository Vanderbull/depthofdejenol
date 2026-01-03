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

// --- MTypes.h Integration (Character Definition) ---
// Assuming the Character struct based on the fields used in charactersheet.cpp
struct Character {
/*
    Name (30 bytes): "bjonke                        "
    Race (int16): 0
    Alignment (int16): 0
    Sex (int16): 0
    Days Old (float): 5908
    Current SP (int16): 90
    Dungeon Level (int16): 0
    Current X (int16): 7
    Current Y (int16): 8
    Atk (float): 1.09262e+09
    Def (float): 1.09262e+09
    Base Stats (7x int16): "9, 9, 9, 10, 10, 11, 0"
    Modded Stats (7x int16): "0, 0, 0, 0, 0, 0, 0"
*/
	std::string name;
	int16_t race;
	int16_t alignment;
	int16_t sex;
	float daysOld;
	int16_t currentSP;
	int16_t currentDungeonLevel;
	int16_t currentX;
	int16_t currentY;
	float atk;
	float def;
	int16_t baseStats[7];
	int16_t moddedStats[7];
	
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
	int16_t ingroup;
	int32_t goldFactor;
	int32_t trapFlags;
	int16_t guildlevel;
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
//typedef std::vector<Character> Characters;

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

	      void checklength(int length) 
	      {
	        //qInfo() << "length:" << RECORD_LENGTH;
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
	      T& get(T & var) 
	      {
	        checklength(sizeof(T));
                // Simple little-endian conversion assuming a fixed structure
                uint8_t *ptr = buffer + cursor;
                if constexpr (sizeof(T) == 1) 
                {
                  var = *reinterpret_cast<T*>(ptr);
                } else if constexpr (sizeof(T) == 2) 
                {
                  uint16_t val = ptr[0] | (ptr[1] << 8);
                  var = *reinterpret_cast<T*>(&val);
                } else if constexpr (sizeof(T) == 4) 
                {
                  uint32_t val = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
                  var = *reinterpret_cast<T*>(&val);
                } else if constexpr (sizeof(T) == 8) // <-- ADDED 8-BYTE HANDLING
                {
                  uint64_t val = (uint64_t)ptr[0] | 
                                 ((uint64_t)ptr[1] << 8) | 
                                 ((uint64_t)ptr[2] << 16) | 
                                 ((uint64_t)ptr[3] << 24) |
                                 ((uint64_t)ptr[4] << 32) | 
                                 ((uint64_t)ptr[5] << 40) | 
                                 ((uint64_t)ptr[6] << 48) | 
                                 ((uint64_t)ptr[7] << 56);
                  var = *reinterpret_cast<T*>(&val);
                } else 
                {
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
// <-- ADDED NAMED 8-BYTE FUNCTIONS
	int64_t getInt64() { int64_t var; return get(var); } 
	uint64_t getUint64() { uint64_t var; return get(var); }
	template<typename T>
	void getArray(T * var, size_t len) {
		for (size_t i = 0; i < len; i++) {
			get(var[i]);
		}
	}
};

//--------------------------------------------------------------------------
void writeCharacterToDumbo(const Character& m) {
    QFile file("dumbo.csv");
    // Open in WriteOnly mode (recreates the file) or Append if you prefer
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Error: Could not open dumbo.csv for writing.";
        return;
    }

    QTextStream out(&file);

    // 1. Write the Header Row
    // Note: This matches the fields in your Character struct definition
    out << "name,atk,def,id,hits,numGroups,picID,lockedChance,levelFound,";
    out << "ResFire,ResCold,ResElectric,ResMind,ResPoison,ResDisease,ResMagic,ResPhysical,ResWeapon,ResSpell,ResSpecial,ResUnused1,ResUnused2,";
    out << "specialPropertyFlags,specialAttackFlags,spellFlags,chance,";
    out << "boxChance0,boxChance1,boxChance2,boxChance3,";
    out << "alignment,ingroup,goldFactor,trapFlags,guildlevel,";
    out << "Stat0,Stat1,Stat2,Stat3,Stat4,Stat5,Stat6,";
    out << "type,damageMod,companionType,companionSpawnMode,companionID,";
    out << "Item0,Item1,Item2,Item3,Item4,Item5,Item6,Item7,Item8,Item9,Item10,";
    out << "subtype,companionSubtype,deleted\n";

    // 2. Write the Data Row
    out << QString::fromStdString(m.name) << ",";
    out << m.atk << "," << m.def << "," << m.id << "," << m.hits << ",";
    out << m.numGroups << "," << m.picID << "," << m.lockedChance << "," << m.levelFound << ",";

    // Resistances Array (12 elements)
    for (int i = 0; i < 12; ++i) {
        out << m.resistances[i] << ",";
    }

    out << m.specialPropertyFlags << "," << m.specialAttackFlags << "," << m.spellFlags << "," << m.chance << ",";

    // BoxChance Array (4 elements)
    for (int i = 0; i < 4; ++i) {
        out << m.boxChance[i] << ",";
    }

    out << m.alignment << "," << m.ingroup << "," << m.goldFactor << "," << m.trapFlags << "," << m.guildlevel << ",";

    // Stats Array (7 elements)
    //for (int i = 0; i < 7; ++i) {
    //    out << m.stats[i] << ",";
    //}

    out << m.type << "," << m.damageMod << "," << m.companionType << "," << m.companionSpawnMode << "," << m.companionID << ",";

    // Items Array (11 elements)
    for (int i = 0; i < 11; ++i) {
        out << m.items[i] << ",";
    }

    out << m.subtype << "," << m.companionSubtype << "," << m.deleted << "\n";

    file.close();
    qDebug() << "Successfully wrote character data to dumbo.csv";
}
//--------------------------------------------------------------------------

// --------------------------------------------------------------------------

// --- MLoader.cpp Integration (The loadMonsters function) ---
Characters loadCharacter(std::string filename) {
    Characters characters;
    // Monster record size is 160 bytes, inferred from MSaver.cpp
    RecordReader<2900> rr(filename);
    // Read Header Records
    rr.read();
    // 1. Create a new Character struct instance for this record
    Character person;
    // 2. Read the name (30 bytes) and assign it to the struct
    //    person.name = rr.getString(30);
    // 3. (Optional) Populate other fields to keep the struct complete
    //    person.id = static_cast<int16_t>(i); // Using the loop index as ID
    std::string version = rr.getString();
    assert(version == "1.1");
    qInfo() << "Version:" << version;
    rr.read();
    uint16_t nCharacters = rr.getWord();
    qInfo() << "Characters:" << nCharacters;
    for (size_t i = 0; i < nCharacters; i++)
    {
        rr.read();
        qInfo() << "------------CharacterID------------:" << i;
        person.id = i;
        //std::string characterName = rr.getString(30);
        person.name = rr.getString(30);
        qInfo() << "Name (30 bytes):" << QString::fromStdString(person.name);
        // 2. Race (Offsets 30-31: 2 bytes)
        int16_t race = rr.getWord();
        qInfo() << "Race (int16):" << race;
        // 3. Alignment (Offsets 32-33: 2 bytes)
        int16_t alignment = rr.getWord();
        qInfo() << "Alignment (int16):" << alignment;
        // 4. Sex (Offsets 34-35: int16)
        int16_t sex = rr.getWord();
        qInfo() << "Sex (int16):" << sex; // Cursor at 36
        // 5. Days Old (Offsets 36-39: 4 bytes) - 'single' = float
        float daysOld = rr.getFloat();
        qInfo() << "Days Old (float):" << daysOld;
        // 6. Current SP (Offsets 40-41: 2 bytes) - int16
        int16_t currentSP = rr.getWord(); 
        qInfo() << "Current SP (int16):" << currentSP;
        // 7. Current dungeon level (Offsets 42-43: 2 bytes) - int16
        int16_t currentDungeonLevel = rr.getWord(); 
        qInfo() << "Dungeon Level (int16):" << currentDungeonLevel;
        // 8. Current x coordinates (Offsets 44-45: 2 bytes) - int16
        int16_t currentX = rr.getWord(); 
        qInfo() << "Current X (int16):" << currentX;
        // 9. Current y coordinates (Offsets 46-47: 2 bytes) - int16
        int16_t currentY = rr.getWord(); 
        qInfo() << "Current Y (int16):" << currentY;
        // 10. Atk (Offsets 48-51: 4 bytes) - 'single' = float
        float atk = rr.getDword();
        person.atk = atk;
        qInfo() << "Atk (float):" << atk;
        // 11. Def (Offsets 52-55: 4 bytes) - 'single' = float
        float def = rr.getDword();
        person.def = def;
        qInfo() << "Def (float):" << def;
        // 12. Base stats (Offsets 56-69: 7 * 2 = 14 bytes) - int16 array
        // We can define a temporary array to hold these values.
        int16_t baseStats[7];
        rr.getArray(baseStats, 7);
        // Outputting the array elements
        QString baseStatsStr;
        for (int i = 0; i < 7; ++i) {
            baseStatsStr += QString::number(baseStats[i]) + (i < 6 ? ", " : "");
        }
        qInfo() << "Base Stats (7x int16):" << baseStatsStr;
        // 13. Modded stats (Offsets 70-83: 7 * 2 = 14 bytes) - int16 array
        int16_t moddedStats[7];
        rr.getArray(moddedStats, 7);
        // Outputting the array elements
        QString moddedStatsStr;
        for (int i = 0; i < 7; ++i) {
            moddedStatsStr += QString::number(moddedStats[i]) + (i < 6 ? ", " : "");
        }
        qInfo() << "Modded Stats (7x int16):" << moddedStatsStr;
    // Inventory Array (Offsets 84-821: 41 Items * 18 bytes)
    const int NUM_ITEMS = 41;
    qInfo() << "--- Starting Inventory Read (41 Items) ---";

    // Loop 41 times to read each item record
    for (int i = 0; i < NUM_ITEMS; ++i) {
        // 9 fields, each is an int16 (2 bytes) = 18 bytes total
        int16_t atk        = rr.getWord(); // +2 bytes
        int16_t def        = rr.getWord(); // +2 bytes
        int16_t itemIndex  = rr.getWord(); // +2 bytes
        int16_t itemID     = rr.getWord(); // +2 bytes
        int16_t alignment  = rr.getWord(); // +2 bytes
        int16_t charges    = rr.getWord(); // +2 bytes
        int16_t equipped   = rr.getWord(); // +2 bytes
        int16_t idLevel    = rr.getWord(); // +2 bytes
        int16_t cursed     = rr.getWord(); // +2 bytes (Total 18 bytes)

        // Log a summary for the first 3 items and the last item for debugging
        if (i < 3 || i == NUM_ITEMS - 1) { 
            qInfo() << QString("Item %1 | ID: %2, Index: %3, Eq: %4, Atk/Def: %5/%6")
                .arg(i)
                .arg(itemID)
                .arg(itemIndex)
                .arg(equipped)
                .arg(atk)
                .arg(def);
        } else if (i == 3) {
            qInfo() << "   ... (Skipping remaining items for brevity) ...";
        }
    }
    // The cursor is now at offset 84 + 738 = 822, ready for the next fi
    // Bank Array (Offsets 822-1559: 41 Items * 18 bytes)
    qInfo() << "--- Starting Bank Read (41 Items) ---";

    // Loop 41 times to read each item record
    for (int i = 0; i < NUM_ITEMS; ++i) {
        // Read 9 fields, each is an int16 (2 bytes) = 18 bytes total
        int16_t atk        = rr.getWord(); // +2 bytes
        int16_t def        = rr.getWord(); // +2 bytes
        int16_t itemIndex  = rr.getWord(); // +2 bytes
        int16_t itemID     = rr.getWord(); // +2 bytes
        int16_t alignment  = rr.getWord(); // +2 bytes
        int16_t charges    = rr.getWord(); // +2 bytes
        int16_t equipped   = rr.getWord(); // +2 bytes
        int16_t idLevel    = rr.getWord(); // +2 bytes
        int16_t cursed     = rr.getWord(); // +2 bytes (Total 18 bytes)

        // Log a summary for the first 3 items and the last item for debugging
        if (i < 3 || i == NUM_ITEMS - 1) { 
            qInfo() << QString("Bank %1 | ID: %2, Index: %3, Atk/Def: %4/%5")
                .arg(i)
                .arg(itemID)
                .arg(itemIndex)
                .arg(atk)
                .arg(def);
        } else if (i == 3) {
            qInfo() << "   ... (Skipping remaining bank items for brevity) ...";
        }
    }
    // The cursor is now at offset 822 + 738 = 1560, ready for the next field.
    qInfo() << "--- Bank Read Complete. Cursor at offset 1560. ---";
    // ------------------------------------------------------------
    // --- START OF NEW/COMPLETED FIELDS (Offsets 1560 onwards) ---
    // ------------------------------------------------------------
    // 1. Equipped Items Array (Offsets 1560-1631: int16(36))
      const int NUM_EQUIPPED_SLOTS = 36;
      int16_t equippedItems[NUM_EQUIPPED_SLOTS];
      rr.getArray(equippedItems, NUM_EQUIPPED_SLOTS); // Cursor at 1632

      QString equippedStr;
      for (int i = 0; i < NUM_EQUIPPED_SLOTS; ++i) {
          equippedStr += QString::number(equippedItems[i]) + (i < NUM_EQUIPPED_SLOTS - 1 ? ", " : "");
      }
      qInfo() << "Equipped Items (36x int16):" << equippedStr;
      // 2. Unused (Offsets 1632-1639: int64)
      int64_t unused64_1 = rr.getInt64(); 
      qInfo() << "Unused (int64):" << unused64_1; // Cursor at 1640
      // 3. Unused (Offsets 1640-1641: int16)
      int16_t unused16_1 = rr.getWord(); 
      qInfo() << "Unused (int16):" << unused16_1; // Cursor at 1642
      // 4. Direction Facing (Offsets 1642-1643: int16)
      int16_t directionFacing = rr.getWord(); 
      qInfo() << "Direction Facing (int16):" << directionFacing; // Cursor at 1644
      // 5. Unused (Offsets 1644-1645: int16)
      int16_t unused16_2 = rr.getWord(); 
      qInfo() << "Unused (int16):" << unused16_2; // Cursor at 1646
      // 6. Total experience (Offsets 1646-1653: currency = int64_t)
      int64_t totalExperience = rr.getInt64(); 
      qInfo() << "Total Experience (int64/currency):" << totalExperience; // Cursor at 1654
      // 7. Gold on hand (Offsets 1654-1661: currency = int64_t)
      int64_t goldOnHand = rr.getInt64(); 
      qInfo() << "Gold on Hand (int64/currency):" << goldOnHand; // Cursor at 1662
      // 8. Current HP (Offsets 1662-1663: int16)
      int16_t currentHP = rr.getWord(); 
      qInfo() << "Current HP (int16):" << currentHP; // Cursor at 1664
      // 9. Max HP (Offsets 1664-1665: int16)
      int16_t maxHP = rr.getWord(); 
      qInfo() << "Max HP (int16):" << maxHP; // Cursor at 1666
      // 10. Gold in bank (Offsets 1666-1673: currency = int64_t)
      int64_t goldInBank = rr.getInt64(); 
      qInfo() << "Gold in Bank (int64/currency):" << goldInBank; // Cursor at 1674
      // 11. Extra swings (Offsets 1674-1675: int16)
      int16_t extraSwings = rr.getWord(); 
      qInfo() << "Extra Swings (int16):" << extraSwings; // Cursor at 1676
      // 12. Active Guild (Offsets 1676-1677: int16)
      int16_t activeGuild = rr.getWord(); 
      qInfo() << "Active Guild (int16):" << activeGuild; // Cursor at 1678
      qInfo() << "--- Block 1 Read Complete. Cursor at offset 1678. ---";
      // --- Block 2: GuildStatus Array (Offsets 1678-2061: 16 Records * 24 bytes) ---
      const int NUM_GUILDS = 16;
      qInfo() << "--- Starting GuildStatus Read (16 Records) ---";

      for (int i = 0; i < NUM_GUILDS; ++i) {
          // Record size is 24 bytes: int16 (2) + currency(int64) (8) + 3x int16 (6) + 2x single(float) (8) = 24 bytes
          int16_t currentLevel     = rr.getWord(); 
          int64_t currentExperience = rr.getInt64(); 
          int16_t quest            = rr.getWord(); 
          int16_t questedID        = rr.getWord(); 
          int16_t isQuestCompleted = rr.getWord(); 
          float atk_g              = rr.getFloat(); 
          float def_g              = rr.getFloat(); 

          if (i < 2 || i == NUM_GUILDS - 1) { 
              qInfo() << QString("Guild %1 | Level: %2, XP: %3, QuestID: %4, Atk/Def: %5/%6")
                  .arg(i).arg(currentLevel).arg(currentExperience).arg(questedID).arg(atk_g).arg(def_g);
          } else if (i == 2) {
              qInfo() << "   ... (Skipping remaining guild records for brevity) ...";
          }
      }
      qInfo() << "--- GuildStatus Read Complete. Cursor at offset 2062. ---";
      // --- Block 3: Companion Array (Offsets 2062-2226: 5 Records * 33 bytes) ---
      const int NUM_COMPANIONS = 5;
      qInfo() << "--- Starting Companion Read (5 Records) ---";

      for (int i = 0; i < NUM_COMPANIONS; ++i) {
          // Record size is 33 bytes: string(15) (15) + 9x int16 (18) = 33 bytes
          std::string customName = rr.getString(15); 
          int16_t monsterID      = rr.getWord(); 
          int16_t companionSlot  = rr.getWord(); 
          int16_t currentHP_c    = rr.getWord(); 
          int16_t maxHP_c        = rr.getWord(); 
          int16_t alignment_c    = rr.getWord(); 
          int16_t atk_c          = rr.getWord(); 
          int16_t def_c          = rr.getWord(); 
          int16_t bindLevel      = rr.getWord(); 
          int16_t idLevel_c      = rr.getWord(); 
          
          qInfo() << "name:" << customName;
          qInfo() << "monsterid:" << monsterID;
          qInfo() << "currenthp:" << companionSlot;
          qInfo() << "maxhp:" << currentHP_c;
          qInfo() << "alignmnet:" << maxHP_c;
          qInfo() << "atk:" << atk_c;
          qInfo() << "def:" << def_c;
          qInfo() << "bindlvl:" << bindLevel;
          qInfo() << "idlevel:" << idLevel_c;
      }
      qInfo() << "--- Companion Read Complete. Cursor at offset 2227. ---";
      // --- Block 4: Rez/Resistances Block (Offsets 2227-2428) ---
      // 2227-2228: int16
      int16_t handsOccupied = rr.getWord(); 
      qInfo() << "Hands occupied (int16):" << handsOccupied; // Cursor at 2229
      // 2229-2230: int16
      int16_t rezSpellCast = rr.getWord(); 
      qInfo() << "Rez spell cast (int16):" << rezSpellCast; // Cursor at 2231
      // 2231-2232: int16
      int16_t raiseSuccessChance = rr.getWord(); 
      qInfo() << "Raise success chance (int16):" << raiseSuccessChance; // Cursor at 2233
      // 2233-2262: string(30)
      std::string rezCharName = rr.getString(30); 
      qInfo() << "Rez char name (30 bytes):" << QString::fromStdString(rezCharName); // Cursor at 2263
      // 2263-2298: single(9) (9 * 4 = 36 bytes)
      const int NUM_FLOAT_STATS = 9;
      float charStats[NUM_FLOAT_STATS];
      rr.getArray(charStats, NUM_FLOAT_STATS); 
      
      qInfo() << "Character Stats (9x float):";
      qInfo() << QString("  Kills: %1, Deaths: %2, Comp Kills: %3, Quests: %4/%5, Play Time: %6, Creation Date: %7")
          .arg(charStats[0]).arg(charStats[1]).arg(charStats[2]).arg(charStats[3]).arg(charStats[4]).arg(charStats[5]).arg(charStats[6]);
      
      // Read 36 bytes of padding/unused space to align the cursor with the next field at 2335
      // 2299-2334 is 36 bytes. Since float is 4 bytes, this is 9 float-equivalents.
      float paddingFloat[9]; 
      rr.getArray(paddingFloat, 9); // Cursor at 2335
      qInfo() << "Read 36 bytes of padding/unused space to align cursor to 2335.";

      // 2335-2346: int16(6)
      int16_t charOptions[6];
      rr.getArray(charOptions, 6); 
      qInfo() << "Character Options (6x int16) read.";

      // 2347-2362: int16(8)
      int16_t statusEffects[8];
      rr.getArray(statusEffects, 8); 
      qInfo() << "Status Effects (8x int16) read.";

      // 2363-2386: int16(12)
      int16_t resistances[12];
      rr.getArray(resistances, 12); 
      qInfo() << "Resistances (12x int16) read.";

      // 2387-2390: int32
      int32_t tempBuffs = rr.getDword(); 
      qInfo() << "Temp Buffs (int32):" << tempBuffs;

      // 2391-2414: int16(12)
      int16_t tempResistances[12];
      rr.getArray(tempResistances, 12); 
      qInfo() << "Temp Resistances (12x int16) read.";

      // 2415-2416: int16
      int16_t typeOfDeadness = rr.getWord(); 
      qInfo() << "Type of Deadness (int16):" << typeOfDeadness;

      // 2417-2418: int16
      int16_t carriedCorpseID = rr.getWord(); 
      qInfo() << "Carried Corpse ID (int16):" << carriedCorpseID;

      // 2419-2428: string(10)
      std::string password = rr.getString(10); 
      qInfo() << "Password (10 bytes):" << QString::fromStdString(password); 
      qInfo() << "--- Block 4 Read Complete. Cursor at offset 2429. ---";


      // --- Block 5: Saved window states Array (Offsets 2429-2806: 21 Records * 18 bytes) ---
      // Total size: 378 bytes. 378 / 18 bytes/record = 21 records.
      const int NUM_WINDOWS = 21;
      qInfo() << "--- Starting Window States Read (21 Records) ---";

      for (int i = 0; i < NUM_WINDOWS; ++i) {
          // Record size is 18 bytes: 4x int32 (16) + int16 (2) = 18 bytes
          int32_t left = rr.getDword(); 
          int32_t top = rr.getDword(); 
          int32_t height = rr.getDword(); 
          int32_t width = rr.getDword(); 
          int16_t windowID = rr.getWord(); 
          
          if (i < 2) {
               qInfo() << QString("Window %1 (ID: %2) | L/T/H/W: %3/%4/%5/%6").arg(i).arg(windowID).arg(left).arg(top).arg(height).arg(width);
          } else if (i == 2) {
              qInfo() << "   ... (Skipping remaining window records for brevity) ...";
          }
      }
      qInfo() << "--- Block 5 Read Complete. Cursor at offset 2807. ---";
      // --- Block 6: Final Misc Fields (Offsets 2807-2892) ---
      // 2807-2808: int16
      int16_t recordLineNumber = rr.getWord(); 
      qInfo() << "Record line number (int16):" << recordLineNumber; 
      // 2809-2812: int32
      int32_t xPNeededToPin = rr.getDword(); 
      qInfo() << "XP needed to pin (int32):" << xPNeededToPin; 
      // 2813-2816: int32
      int32_t abilitiesMask = rr.getDword(); 
      qInfo() << "Abilities from items (int32 mask):" << abilitiesMask; 
      // 2817-2840: int16(12)
      int16_t resistsFromItems[12];
      rr.getArray(resistsFromItems, 12); 
      qInfo() << "Resists from items (12x int16) read.";
      // 2841-2844: int16(2)
      int16_t itemsInEachHand[2];
      rr.getArray(itemsInEachHand, 2); 
      qInfo() << "Items equipped in each hand (2x int16) read.";
      // 2845-2846: int16
      int16_t atkDefPlaceholder = rr.getWord(); 
      qInfo() << "AtkDef placeholder (int16):" << atkDefPlaceholder; 
      // 2847-2868: int16(11)
      int16_t bufferSlots[11];
      rr.getArray(bufferSlots, 11); 
      qInfo() << "Buffer slots (11x int16) read.";
      // 2869-2870: int16
      int16_t currentMapAreaNumber = rr.getWord(); 
      qInfo() << "Current map area number (int16):" << currentMapAreaNumber; 
      // 2871-2872: int16 (Unused)
      rr.getWord(); 
      // 2873-2874: int16 (Unused)
      rr.getWord(); 
      // 2875-2876: int16
      int16_t sanctuaryX = rr.getWord(); 
      qInfo() << "Sanctuary X (int16):" << sanctuaryX; 
      // 2877-2878: int16
      int16_t sanctuaryY = rr.getWord(); 
      qInfo() << "Sanctuary Y (int16):" << sanctuaryY; 
      // 2879-2880: int16
      int16_t sanctuaryLevel = rr.getWord(); 
      qInfo() << "Sanctuary Level (int16):" << sanctuaryLevel; 
      // 2881-2892: int32(3)
      int32_t locationAwareness[3];
      rr.getArray(locationAwareness, 3); 
      qInfo() << QString("Location Awareness (3x int32): %1, %2, %3")
          .arg(locationAwareness[0]).arg(locationAwareness[1]).arg(locationAwareness[2]);
      // --- FINAL CHECK ---
      // The cursor is now at offset 2893. 7 unused bytes remain out of 2900.
      qInfo() << "--- All fields read up to offset 2892. Reading final 7 bytes of padding. ---";
      // Read remaining 7 bytes of padding (to reach 2900 bytes for the next record read)
      uint8_t finalPadding[7];
      for (int k = 0; k < 7; ++k) {
          uint8_t dummy; // Use a dummy variable for the read
          rr.get(dummy);
          finalPadding[k] = dummy;
      }
      qInfo() << "--- Block 6 Read Complete. Final Cursor at offset 2900 (end of record). ---";
    }

    // 4. Add the populated struct to our vector
    characters.push_back(person);

    return characters;
}
// --------------------------------------------------------------------------


// Header string derived directly from the field output order in monstersheet.cpp
const QString CHARACTER_HEADERS = "name, atk, def, id, hits, numGroups, picID, lockedChance, levelFound, ResFire, ResCold, ResElectric, ResMind, ResPoison, ResDisease, ResMagic, ResPhysical, ResWeapon, ResSpell, ResSpecial, specialPropertyFlags, specialAttackFlags, spellFlags, chance, boxChance0, boxChance1, boxChance2, alignment, ingroup, goldFactor, trapFlags, guildlevel, StatStr, StatInt, StatWis, StatCon, StatCha, StatDex, type, damageMod, companionType, companionSpawnMode, companionID, Item0, Item1, Item2, Item3, Item4, Item5, Item6, Item7, Item8, Item9, subtype, companionSubtype, deleted";

int main(int argc, char *argv[]) {

    qDebug() << "write dumbo now!!!!!!!!!!!!!!";

    // 1. Setup Qt Application and Command Line Parser
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("CharacterConverter");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Converts Dejenol monster data (.MDR) to various formats (.csv, .json, .js).");
    parser.addHelpOption();
    parser.addVersionOption();

    // The original program used positional arguments
    QCommandLineOption inFileOption("infile", "Input file to convert (e.g., MDATA4.MDR).", "file");
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
    
    // The original monstersheet.cpp used "MDATA4.MDR" as default
    if (inPath.isEmpty()) inPath = "MDATA4.MDR";
    
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
    Characters characters;
    try {
        characters = loadCharacter(inPath.toStdString()); // Call the C++ loader
    } catch (const char* msg) {
        qCritical() << "Error loading characters from file:" << msg;
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
        f << "characterVarNames = [\"" << CHARACTER_HEADERS.split(", ").join("\", \"") << "\"]\n";
        f << "characters = [" << "\n";
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
        f << CHARACTER_HEADERS << "\n";
        ender = "\n";
        ender2 = "\n";
    }

    // 5. Write Character Data
    bool first = true;
    for (const auto& m : characters) {
        if (!first) f << ender;
        first = false;
        
        f << starter;
        
        // Convert std::string name to QString for output
        QString characterName = QString::fromStdString(m.name);

        if (quotes) {
            f << "\"" << characterName << "\",";
        } else {
            f << characterName << ",";
        }
        
        // Write all standard fields
        f << QString::number(m.atk) << ",";
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
        //for (int i = 0; i < 6; i++) {
        //    f << QString::number(m.stats[i]) << ",";
        //}
        
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

    qDebug() << "write dumbo now!!!!!!!!!!!!!!";

    //if (!characters.empty()) {
        writeCharacterToDumbo(characters.at(0));
    //}

    qDebug() << "Successfully converted character from" << inPath << "to" << outPath;
    
    return 0;
}
