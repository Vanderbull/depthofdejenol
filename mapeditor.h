#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <vector>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

// Define a simple enum for tile types to represent different map elements.
enum TileType {
    EMPTY,
    WALL,
    FLOOR,
    WATER
    // You can add more tile types as needed, like `DOOR`, `ENEMY_SPAWN`, etc.
};

class MapEditor {
public:
    // Constructor to initialize the map with a given width and height.
    MapEditor(int width, int height);

    // Set the tile type at a specific coordinate.
    void setTile(int x, int y, TileType type);

    // Get the tile type at a specific coordinate.
    TileType getTile(int x, int y) const;

    // Get the width of the map.
    int getWidth() const;

    // Get the height of the map.
    int getHeight() const;

    // Save the current map to a JSON file.
    bool saveMap(const QString& filePath);

    // Load a map from a JSON file.
    bool loadMap(const QString& filePath);

private:
    int m_width;
    int m_height;
    // A 2D vector to store the map data.
    std::vector<std::vector<TileType>> m_mapData;
};

#endif // MAPEDITOR_H
