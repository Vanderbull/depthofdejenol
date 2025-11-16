#include "dungeonmap.h"
#include <QPainter>
#include <QColor>
#include <QRandomGenerator>

DungeonMap::DungeonMap(QWidget *parent) : QWidget(parent) {
    generateMap();
}

void DungeonMap::generateMap() {
    // This is a simple placeholder to generate a random 20x20 grid.
    const int mapSize = 20;
    mapData.resize(mapSize, std::vector<int>(mapSize, 0));

    // Fill the map with some walls and floor tiles for demonstration.
    for (int i = 0; i < mapSize; ++i) {
        for (int j = 0; j < mapSize; ++j) {
            if (QRandomGenerator::global()->bounded(10) < 2) {
                mapData[i][j] = 1; // Wall
            } else {
                mapData[i][j] = 0; // Floor
            }
        }
    }
}

void DungeonMap::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (mapData.empty() || mapData[0].empty()) {
        return;
    }

    int cellWidth = width() / mapData[0].size();
    int cellHeight = height() / mapData.size();

    for (size_t i = 0; i < mapData.size(); ++i) {
        for (size_t j = 0; j < mapData[i].size(); ++j) {
            QColor color;
            if (mapData[i][j] == 1) {
                color = QColor("#646464"); // Wall color
            } else {
                color = QColor("#c1c1c1"); // Floor color
            }
            painter.fillRect(j * cellWidth, i * cellHeight, cellWidth, cellHeight, color);
        }
    }
}
