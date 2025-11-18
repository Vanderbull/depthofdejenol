#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <vector>
#include <string>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QJsonDocument>
#include <QDebug>

class MapEditor {
public:
    enum TileType {
        EMPTY = 0,
        WALL = 1,
        FLOOR = 2,
        WATER = 3
    };

    MapEditor(int width, int height);

    int getWidth() const;
    int getHeight() const;
    
    // Setters for map data
    void setBaseTile(int x, int y, TileType type);
    void setWallTile(int x, int y, bool isWall);

    // Getters for map data
    TileType getBaseTile(int x, int y) const;
    bool getWallTile(int x, int y) const;
    const std::vector<std::vector<TileType>>& getBaseMap() const;
    const std::vector<std::vector<bool>>& getWallMap() const;

    bool saveMap(const std::string& filename) const;
    bool loadMap(const std::string& filename);

    void loadExampleMap();

private:
    int m_width;
    int m_height;
    std::vector<std::vector<TileType>> m_baseMapData;
    std::vector<std::vector<bool>> m_wallMapData;
};

#endif // MAPEDITOR_H
