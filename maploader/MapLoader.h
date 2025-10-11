#ifndef MAPLOADER_H
#define MAPLOADER_H

// 1. INCLUDE ALL NECESSARY QT HEADERS
#include <QString>
#include <QVector>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QRandomGenerator> // For map generation
#include <QtGlobal>        // For quint types

// --- Data Structures ---

// Define dimensions of the map grid
constexpr int MAP_LEVELS = 15;
constexpr int MAP_WIDTH = 30;
constexpr int MAP_HEIGHT = 30;

// The structure for a single cell's data
struct CellData {
    quint32 fieldBitmask = 0; // 4 bytes

    // Utility to check for a specific feature
    bool hasFeature(quint32 featureMask) const {
        return (fieldBitmask & featureMask) != 0;
    }
};

// Enum class for the FieldBitmask values (using standardized SCREAMING_SNAKE_CASE)
enum class MapFeature : quint32 {
    WALL_EAST        = 1,        
    WALL_NORTH       = 2,        
    DOOR_EAST        = 4,        
    DOOR_NORTH       = 8,        
    SECRET_DOOR_EAST = 16,       
    SECRET_DOOR_NORTH= 32,       
    FACE_NORTH       = 64,       
    FACE_EAST        = 128,      
    FACE_SOUTH       = 256,      
    FACE_WEST        = 512,      
    EXTINGUISHER     = 1024,     
    PIT              = 2048,     
    STAIRS_UP        = 4096,     
    STAIRS_DOWN      = 8192,     
    TELEPORTER       = 16384,    
    WATER            = 32768,    
    QUICKSAND        = 65536,    
    ROTATOR          = 131072,   
    ANTIMAGIC        = 262144,   
    ROCK             = 524288,   
    FOG              = 1048576,  
    CHUTE            = 2097152,  
    STUD             = 4194304,  
    EXPLORED         = 8388608   
};

// Typedef for the main map structure: Level, Y (Height), X (Width)
using MapData = QVector<QVector<QVector<CellData>>>;


// --- Class Definition ---

class MapLoader {
public:
    MapLoader();

    // Public I/O Functions
    bool loadMap(const QString& filePath);
    bool generateRandomMap(const QString& filePath, const QString& version, quint16 deepestLevel);

    // Accessors
    const QString& getVersion() const { return m_version; }
    quint16 getDeepestLevelExplored() const { return m_deepestLevelExplored; }
    const MapData& getMapData() const { return m_mapData; }

private:
    QString m_version;
    quint16 m_deepestLevelExplored = 0; // 2 bytes
    MapData m_mapData;

    // --- Private Read Helpers ---
    bool readVBString(QDataStream& stream, QString& outString);
    bool readLevelData(QDataStream& stream);
    
    // --- Private Write/Generate Helpers (The ones that caused the previous errors) ---
    bool writeVBString(QDataStream& stream, const QString& inString);
    bool writeLevelData(QDataStream& stream);
    void populateRandomData();
};

#endif // MAPLOADER_H
