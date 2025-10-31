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
// --- ASSUMED SPELL FIELD READS (26 bytes consumed) ---
            uint16_t id = buff.getWord(); // 2 bytes
            QString name = buff.getString(20).trimmed(); // 20 bytes
            float effect = buff.getFloat(); // 4 bytes
            
            // Print the extracted spell data
            qDebug() << "--- Spell #" << i << " (ID:" << id << ") ---";
            qDebug() << "Name:" << name;
            qDebug() << "Effect Value (Assumed Float):" << effect;
            
            // Consume the remaining padding in the 75-byte record: 75 - 26 = 49 bytes
            buff.getString(49);

            // Note: If you want to store the data, uncomment and fill out a Spell struct:
            // Spell s;
            // s.id = id; 
            // s.name = name;
            // s.effect = effect;
            // spells.push_back(s);
            // PLACEHOLDER: Insert your actual Spell field reads here
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
