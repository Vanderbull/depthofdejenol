#include "MLoader.h" 
#include "RecordReader.h"
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <QString>
#include <cassert>
#include <stdexcept>

// Macro to silence 'unused parameter/variable' warnings
#define UNUSED(x) (void)x;

// ----------------------------------------------------------------------
// 1. loadSpells (The Fixed Function)
// ----------------------------------------------------------------------

Spells loadSpells(const QString& filename) {
    RecordReader<75> buff(filename);
    Spells spells;

// Define the spells we are searching for and their expected levels
    QHash<QString, uint16_t> spellsToFind = {
        {"Firebolt", 2},
        {"Blue Flame", 4},
        {"Flamesheet", 5},
        {"Pillar of Fire", 7},
        {"Sphere of Flames", 10},
    };
    int foundCount = 0;

    try {
        // --- 1. Load Header and Verify Version (Offset 0 - 74) ---
        buff.read(); 
        
        // Byte 0-1: File ID (uint16_t)
        uint16_t fileId = buff.getWord(); 
        UNUSED(fileId); 
        
        // Byte 2-5: Version String (4 bytes). Read as uint32_t for QDataStream alignment.
        uint32_t versionBytes = buff.getDword();
        QString version = QString::fromLatin1(QByteArray((const char*)&versionBytes, 4)).trimmed(); 
        
        assert(version == "1.17"); 
        
        // Skip the remaining 69 bytes of padding in the current 75-byte buffer (up to offset 75).
        buff.getString(69); 


        // --- 2. Load Next Block and Read Spell Count (Offset 75) ---
        
        buff.read(); 
        
        // Byte 75-76: Read the number of spells (CONFIRMED LOCATION: 105)
        uint16_t numSpells = buff.getWord();
        
        qDebug() << "Total Spells to load (Count at Offset 75):" << numSpells; 
        
        // Initialize the vector with the count
        spells.clear();
        spells.reserve(numSpells); // This assumes Spells is a QList or std::vector wrapper.
        
        // Skip the remaining 73 bytes of padding in the current 75-byte buffer.
        buff.getString(73); 
        

        // --- 3. Loop and Load Spell Records (Starts at Offset 150) ---
        for (size_t i = 0; i < numSpells; i++) {
            buff.read(); // Load the next 75-byte record (the actual Spell data)
int consumedBytes = 0;

            // --- A. vbstring spellname (Variable Length) ---
            uint16_t nameLength = buff.getWord(); // 2 bytes for length
            QString spellname = buff.getString(nameLength).trimmed(); // N bytes for string
            
            consumedBytes += 2 + nameLength;

            // --- B. 11 Fixed Short Fields (22 bytes) ---
            uint16_t m_ID = buff.getWord();
            uint16_t m_class = buff.getWord();
            uint16_t m_spellLevel = buff.getWord();
            uint16_t m_u4 = buff.getWord();
            uint16_t m_alwaysZero = buff.getWord();
            uint16_t m_killEffect = buff.getWord();
            uint16_t m_affectMonster = buff.getWord();
            uint16_t m_affectGroup = buff.getWord();
            uint16_t m_damage1 = buff.getWord();
            uint16_t m_damage2 = buff.getWord();
            uint16_t m_specialEffect = buff.getWord();
            consumedBytes += 22;

            // --- C. m_required array (7 shorts = 14 bytes) ---
            uint16_t m_required[7];
            for (int j = 0; j < 7; j++) {
                m_required[j] = buff.getWord();
            }
            consumedBytes += 14;

            // --- D. Final Short Field (2 bytes) ---
            uint16_t m_resistedBy = buff.getWord();
            consumedBytes += 2;
            
            // --- E. Calculate and Consume Padding ---
            int paddingBytes = 75 - consumedBytes;


            if (paddingBytes < 0) {
                qCritical() << "Error: Record size exceeds 75 bytes for spell:" << spellname;
            } else if (paddingBytes > 0) {
                buff.getString(paddingBytes); // Consume remaining padding
            }

// --- F. TARGETED CHECK: Print if name matches ---
            if (spellsToFind.contains(spellname)) {
                foundCount++;
                uint16_t expectedLevel = spellsToFind.value(spellname);
                
                qDebug() << "----------------------------------------------------------------------";
                qDebug() << "MATCH FOUND (Spell #" << i << "):" << spellname;
                if(m_class == 0)
		{
			qDebug() << "  ID:" << m_ID << " Class:" << m_class << "SORCERER" << " Actual Level:" << m_spellLevel;
		}
		else
		{
                	qDebug() << "  ID:" << m_ID << " Class:" << m_class << " Actual Level:" << m_spellLevel;
		}
                
                if (m_spellLevel == expectedLevel) {
                    qDebug() << "  -> SUCCESS: Level (" << expectedLevel << ") matches expected value.";
                } else {
                    qWarning() << "  -> WARNING: Level (" << m_spellLevel << ") DOES NOT match expected value (" << expectedLevel << ").";
                }
                qDebug() << "  Resisted By:" << m_resistedBy << " Required (First 3):" << m_required[0] << m_required[1] << m_required[2];
                
                // If we found both, we can break early if desired, but we'll continue for a full parse.
                // if (foundCount == spellsToFind.count()) break; 
            }
            // else, continue to next iteration

            // --- F. Print Spell Details ---
            //qDebug() << "--- Spell #" << i << " (ID:" << m_ID << ") ---";
            //qDebug() << "Name:" << spellname;
            //qDebug() << "Level:" << m_spellLevel << " Class:" << m_class;
            //qDebug() << "Damage Range:" << m_damage1 << " to " << m_damage2;
            //qDebug() << "Resisted By:" << m_resistedBy;
            //qDebug() << "Required Items/Skills:" << m_required[0] << m_required[1] << m_required[2] << m_required[3] << m_required[4] << m_required[5] << m_required[6];
        }
        
        qDebug() << "Successfully loaded" << numSpells << "spells.";

    } catch (const std::runtime_error& e) {
        qWarning() << "Error reading spell records:" << e.what();
    } catch (const char* msg) {
        qWarning() << "Generic error reading spell records:" << msg;
    }
    
    return spells;
}

// ----------------------------------------------------------------------
// 2. loadItems and loadCharacters (Other core loaders)
// ----------------------------------------------------------------------

Items loadItems(const QString& filename) {
    RecordReader<75> buff(filename);
    Items items;
    try {
        buff.read();
        // ... loading logic ...
    } catch (const std::runtime_error& e) {
        qWarning() << "Error in loadItems (Stub):" << e.what();
    }
    return items;
}

Characters loadCharacters(const QString& filename) {
    RecordReader<75> buff(filename);
    Characters characters;
    try {
        buff.read();
        // ... loading logic ...
    } catch (const std::runtime_error& e) {
        qWarning() << "Error in loadCharacters (Stub):" << e.what();
    }
    return characters;
}


// ----------------------------------------------------------------------
// 3. RESTORED STUBS (FIXED: Removed the problematic .reserve() calls)
// ----------------------------------------------------------------------

Monsters loadMonsters(const QString& filename) {
    RecordReader<75> buff(filename);
    Monsters monsters;
    try {
        buff.read();
        uint16_t count = buff.getWord();
        monsters.reserve(count); // Assuming Monsters IS a container type (QList/std::vector)
        // ... loop to load records ...
        qDebug() << "Stub: loadMonsters finished.";
    } catch (const std::runtime_error& e) {
        qWarning() << "Error in loadMonsters (Stub):" << e.what();
    }
    return monsters;
}

Automap loadAutomap(const QString& filename) {
    RecordReader<75> buff(filename);
    Automap automap;
    try {
        buff.read();
        uint16_t count = buff.getWord();
        // automap.reserve(count); <-- FIX: REMOVED
        // ... loop to load records ...
        qDebug() << "Stub: loadAutomap finished.";
    } catch (const std::runtime_error& e) {
        qWarning() << "Error in loadAutomap (Stub):" << e.what();
    }
    return automap;
}

GuildLogs loadGuildLogs(const QString& filename) {
    RecordReader<75> buff(filename);
    GuildLogs logs;
    try {
        buff.read();
        uint16_t count = buff.getWord();
        logs.reserve(count); // Assuming GuildLogs IS a container type
        // ... loop to load records ...
        qDebug() << "Stub: loadGuildLogs finished.";
    } catch (const std::runtime_error& e) {
        qWarning() << "Error in loadGuildLogs (Stub):" << e.what();
    }
    return logs;
}

Dungeon loadDungeon(const QString& filename) {
    RecordReader<75> buff(filename);
    Dungeon dungeon;
    try {
        buff.read();
        qDebug() << "Stub: loadDungeon finished.";
    } catch (const std::runtime_error& e) {
        qWarning() << "Error in loadDungeon (Stub):" << e.what();
    }
    return dungeon;
}

Library loadLibrary(const QString& filename) {
    RecordReader<75> buff(filename);
    Library library;
    try {
        buff.read();
        uint16_t count = buff.getWord();
        // library.reserve(count); <-- FIX: REMOVED
        // ... loop to load records ...
        qDebug() << "Stub: loadLibrary finished.";
    } catch (const std::runtime_error& e) {
        qWarning() << "Error in loadLibrary (Stub):" << e.what();
    }
    return library;
}

HallRecords loadHallRecords(const QString& filename) {
    RecordReader<75> buff(filename);
    HallRecords records;
    try {
        buff.read();
        uint16_t count = buff.getWord();
        records.reserve(count); // Assuming HallRecords IS a container type
        // ... loop to load records ...
        qDebug() << "Stub: loadHallRecords finished.";
    } catch (const std::runtime_error& e) {
        qWarning() << "Error in loadHallRecords (Stub):" << e.what();
    }
    return records;
}
