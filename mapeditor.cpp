#include "mapeditor.h"

// MapEditor constructor implementation.
MapEditor::MapEditor(int width, int height) :
    m_width(width),
    m_height(height) {
    // Resize the 2D vector to match the specified width and height.
    m_mapData.resize(m_height, std::vector<TileType>(m_width, EMPTY));
}

// Set a tile's type at a given coordinate.
void MapEditor::setTile(int x, int y, TileType type) {
    // Check if the coordinates are within the map boundaries before setting the tile.
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_mapData[y][x] = type;
    }
}

// Get a tile's type at a given coordinate.
TileType MapEditor::getTile(int x, int y) const {
    // Check if the coordinates are within the map boundaries.
    // Return EMPTY or an error type if they are not.
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        return m_mapData[y][x];
    }
    return EMPTY; // Return default value for out-of-bounds access.
}

// Getter for the map width.
int MapEditor::getWidth() const {
    return m_width;
}

// Getter for the map height.
int MapEditor::getHeight() const {
    return m_height;
}

// Save the current map to a JSON file.
bool MapEditor::saveMap(const QString& filePath) {
    QFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonObject mapObject;
    mapObject["width"] = m_width;
    mapObject["height"] = m_height;

    QJsonArray tilesArray;
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            tilesArray.append(m_mapData[y][x]);
        }
    }
    mapObject["tiles"] = tilesArray;

    QJsonDocument saveDoc(mapObject);
    saveFile.write(saveDoc.toJson());

    return true;
}

// Load a map from a JSON file.
bool MapEditor::loadMap(const QString& filePath) {
    QFile loadFile(filePath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open load file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject mapObject = loadDoc.object();

    if (mapObject.contains("width") && mapObject["width"].isDouble() &&
        mapObject.contains("height") && mapObject["height"].isDouble() &&
        mapObject.contains("tiles") && mapObject["tiles"].isArray()) {

        m_width = mapObject["width"].toInt();
        m_height = mapObject["height"].toInt();

        QJsonArray tilesArray = mapObject["tiles"].toArray();
        if (tilesArray.size() != m_width * m_height) {
            qWarning("Loaded map dimensions do not match tile count.");
            return false;
        }

        m_mapData.resize(m_height, std::vector<TileType>(m_width));
        int index = 0;
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                m_mapData[y][x] = static_cast<TileType>(tilesArray[index].toInt());
                index++;
            }
        }
        return true;
    }

    qWarning("Invalid map data in JSON file.");
    return false;
}
