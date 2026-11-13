#include "MapLoader.h"

//----------------------------------------------------------------------
// CONSTRUCTOR
//----------------------------------------------------------------------

MapLoader::MapLoader() {
    // Initialize the map structure to the correct dimensions (15 levels * 30 * 30 cells)
    m_mapData.resize(MAP_LEVELS);
    for (int i = 0; i < MAP_LEVELS; ++i) {
        m_mapData[i].resize(MAP_HEIGHT);
        for (int j = 0; j < MAP_HEIGHT; ++j) {
            m_mapData[i][j].resize(MAP_WIDTH);
        }
    }
}

//----------------------------------------------------------------------
// PRIVATE READ HELPERS
//----------------------------------------------------------------------

// Helper function to read a VBString (Length-prefixed string)
bool MapLoader::readVBString(QDataStream& stream, QString& outString) {
    quint8 length; // Assuming a 1-byte length prefix
    stream >> length;

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Error reading VBString length.";
        return false;
    }

    if (length == 0) {
        outString = QString();
        return true;
    }

    QByteArray data;
    data.resize(length);
    qint64 bytesRead = stream.readRawData(data.data(), length);

    if (bytesRead != length) {
        qWarning() << "Error reading VBString data. Expected" << length << "bytes, read" << bytesRead;
        return false;
    }

    // Assuming the string content is Latin-1 or ASCII, typical for older formats
    outString = QString::fromLatin1(data);
    return true;
}

// Helper function to read all the fixed-size Level Data
bool MapLoader::readLevelData(QDataStream& stream) {
    quint32 bitmask;
    int cellCount = 0;

    for (int l = 0; l < MAP_LEVELS; ++l) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                // Read the 4-byte FieldBitmask
                stream >> bitmask;

                if (stream.status() != QDataStream::Ok) {
                    qWarning() << "Error reading cell data at Level:" << l << "Y:" << y << "X:" << x;
                    return false;
                }

                m_mapData[l][y][x].fieldBitmask = bitmask;
                cellCount++;
            }
        }
    }
    qDebug() << "Successfully read" << cellCount << "cells of level data.";
    return true;
}

//----------------------------------------------------------------------
// PUBLIC LOAD FUNCTION
//----------------------------------------------------------------------

// Main map loading function
bool MapLoader::loadMap(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return false;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_15);
    // stream.setByteOrder(QDataStream::LittleEndian); // Uncomment if needed

    // 1. HEADER RECORD (Version)
    if (!readVBString(stream, m_version)) {
        qWarning() << "Failed to read Version string.";
        return false;
    }
    qDebug() << "Version:" << m_version;

    // 2. DEEPEST LEVEL (2 bytes)
    stream >> m_deepestLevelExplored;

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Failed to read DeepestLevelExplored (2 bytes).";
        return false;
    }
    qDebug() << "Deepest Level Explored:" << m_deepestLevelExplored;


    // 3. LEVEL DATA
    if (!readLevelData(stream)) {
        qWarning() << "Failed to read Level Data.";
        return false;
    }

    if (!file.atEnd()) {
        qWarning() << "Warning: Data remains in the file after reading all expected records.";
    }

    file.close();
    qDebug() << "Map file loaded successfully!";
    return true;
}

//----------------------------------------------------------------------
// PRIVATE WRITE HELPERS (FOR GENERATION)
//----------------------------------------------------------------------

// Helper function to write a VBString (Length-prefixed string)
bool MapLoader::writeVBString(QDataStream& stream, const QString& inString) {
    QByteArray data = inString.toLatin1(); // Convert to QByteArray (assuming Latin-1/ASCII)
    quint8 length = static_cast<quint8>(data.length()); // 1-byte length prefix

    stream << length;

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Error writing VBString length.";
        return false;
    }

    qint64 bytesWritten = stream.writeRawData(data.constData(), length);

    if (bytesWritten != length) {
        qWarning() << "Error writing VBString data. Expected" << length << "bytes, wrote" << bytesWritten;
        return false;
    }

    return true;
}

// Helper to randomly populate the map data
void MapLoader::populateRandomData() {
    QRandomGenerator *randGen = QRandomGenerator::global();
    
    // Max bitmask is 2^24 - 1 (0x00FFFFFF)
    quint32 maxBitmaskValue = 0x00FFFFFF; 

    for (int l = 0; l < MAP_LEVELS; ++l) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                // Generate a random 4-byte value, masked to the relevant bits.
                m_mapData[l][y][x].fieldBitmask = randGen->generate() & maxBitmaskValue;

                // Ensure a base level of "Explored" for most cells, typical for save games
                if (randGen->bounded(100) > 10) { // 90% chance of being explored
                    m_mapData[l][y][x].fieldBitmask |= static_cast<quint32>(MapFeature::EXPLORED);
                }
            }
        }
    }
    qDebug() << "Map data randomly populated.";
}

// Helper function to write all the Level Data
bool MapLoader::writeLevelData(QDataStream& stream) {
    int cellCount = 0;

    for (int l = 0; l < MAP_LEVELS; ++l) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAP_WIDTH; ++x) {
                // Write the 4-byte FieldBitmask
                stream << m_mapData[l][y][x].fieldBitmask;

                if (stream.status() != QDataStream::Ok) {
                    qWarning() << "Error writing cell data at Level:" << l << "Y:" << y << "X:" << x;
                    return false;
                }
                cellCount++;
            }
        }
    }
    qDebug() << "Successfully wrote" << cellCount << "cells of level data.";
    return true;
}

//----------------------------------------------------------------------
// PUBLIC GENERATE FUNCTION
//----------------------------------------------------------------------

bool MapLoader::generateRandomMap(const QString& filePath, const QString& version, quint16 deepestLevel) {
    
    // 1. Prepare data members
    m_version = version;
    m_deepestLevelExplored = deepestLevel;
    populateRandomData();

    // 2. Open file for writing
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Could not open file for writing:" << filePath;
        return false;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_15);
    // stream.setByteOrder(QDataStream::LittleEndian); // Uncomment if needed

    // 3. Write HEADER RECORD (Version)
    if (!writeVBString(stream, m_version)) {
        qWarning() << "Failed to write Version string.";
        return false;
    }

    // 4. Write DEEPEST LEVEL (2 bytes)
    stream << m_deepestLevelExplored;

    if (stream.status() != QDataStream::Ok) {
        qWarning() << "Failed to write DeepestLevelExplored (2 bytes).";
        return false;
    }

    // 5. Write LEVEL DATA
    if (!writeLevelData(stream)) {
        qWarning() << "Failed to write Level Data.";
        return false;
    }

    file.close();
    qDebug() << "Random map file generated successfully at:" << filePath;
    return true;
}
