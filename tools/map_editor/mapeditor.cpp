#include "mapeditor.h"

MapEditor::MapEditor(int width, int height) : m_width(width), m_height(height) {
    m_baseMapData.resize(height, std::vector<TileType>(width, EMPTY));
    m_wallMapData.resize(height, std::vector<bool>(width, false));
}

int MapEditor::getWidth() const {
    return m_width;
}

int MapEditor::getHeight() const {
    return m_height;
}

void MapEditor::setBaseTile(int x, int y, TileType type) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_baseMapData[y][x] = type;
    }
}

void MapEditor::setWallTile(int x, int y, bool isWall) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_wallMapData[y][x] = isWall;
    }
}

MapEditor::TileType MapEditor::getBaseTile(int x, int y) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        return m_baseMapData[y][x];
    }
    return EMPTY;
}

bool MapEditor::getWallTile(int x, int y) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        return m_wallMapData[y][x];
    }
    return false;
}

const std::vector<std::vector<MapEditor::TileType>>& MapEditor::getBaseMap() const {
    return m_baseMapData;
}

const std::vector<std::vector<bool>>& MapEditor::getWallMap() const {
    return m_wallMapData;
}

bool MapEditor::saveMap(const std::string& filename) const {
    QFile saveFile(QString::fromStdString(filename));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonObject mapObject;
    mapObject["width"] = m_width;
    mapObject["height"] = m_height;

    QJsonArray baseArray;
    for (const auto& row : m_baseMapData) {
        QJsonArray rowArray;
        for (const auto& tile : row) {
            rowArray.append(tile);
        }
        baseArray.append(rowArray);
    }
    mapObject["base"] = baseArray;

    QJsonArray wallArray;
    for (const auto& row : m_wallMapData) {
        QJsonArray rowArray;
        for (const auto& isWall : row) {
            rowArray.append(isWall);
        }
        wallArray.append(rowArray);
    }
    mapObject["walls"] = wallArray;

    QJsonDocument saveDoc(mapObject);
    saveFile.write(saveDoc.toJson());
    return true;
}

bool MapEditor::loadMap(const std::string& filename) {
    QFile loadFile(QString::fromStdString(filename));

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open load file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject mapObject = loadDoc.object();

    if (mapObject.contains("width") && mapObject.contains("height")) {
        m_width = mapObject["width"].toInt();
        m_height = mapObject["height"].toInt();
        m_baseMapData.resize(m_height, std::vector<TileType>(m_width));
        m_wallMapData.resize(m_height, std::vector<bool>(m_width));

        if (mapObject.contains("base") && mapObject["base"].isArray() && mapObject.contains("walls") && mapObject["walls"].isArray()) {
            QJsonArray baseArray = mapObject["base"].toArray();
            for (int y = 0; y < baseArray.size(); ++y) {
                QJsonArray rowArray = baseArray[y].toArray();
                for (int x = 0; x < rowArray.size(); ++x) {
                    m_baseMapData[y][x] = static_cast<TileType>(rowArray[x].toInt());
                }
            }
            QJsonArray wallArray = mapObject["walls"].toArray();
            for (int y = 0; y < wallArray.size(); ++y) {
                QJsonArray rowArray = wallArray[y].toArray();
                for (int x = 0; x < rowArray.size(); ++x) {
                    m_wallMapData[y][x] = rowArray[x].toBool();
                }
            }
            return true;
        }
    }
    return false;
}

void MapEditor::loadExampleMap() {
    m_baseMapData.resize(m_height, std::vector<TileType>(m_width, FLOOR));
    m_wallMapData.resize(m_height, std::vector<bool>(m_width, false));
    
    // Walls
    m_wallMapData[5][5] = true;
    m_wallMapData[5][6] = true;
    m_wallMapData[6][5] = true;
    m_wallMapData[6][6] = true;
    
    // Water
    m_baseMapData[8][10] = WATER;
    m_baseMapData[8][11] = WATER;
    m_baseMapData[9][10] = WATER;
    m_baseMapData[9][11] = WATER;
}
