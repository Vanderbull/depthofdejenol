#include "mapwidget.h"
#include <QPainter>

MapWidget::MapWidget(MapEditor* editor, QWidget* parent) :
    QWidget(parent),
    m_mapEditor(editor) {
    setFixedSize(m_mapEditor->getWidth() * TILE_SIZE, m_mapEditor->getHeight() * TILE_SIZE);
}

void MapWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    for (int y = 0; y < m_mapEditor->getHeight(); ++y) {
        for (int x = 0; x < m_mapEditor->getWidth(); ++x) {
            // Get the tile type from the MapEditor data model.
            TileType type = m_mapEditor->getTile(x, y);

            // Set the color based on the tile type.
            QColor color;
            switch (type) {
                case EMPTY:
                    color = Qt::black;
                    break;
                case WALL:
                    color = Qt::gray;
                    break;
                case FLOOR:
                    color = Qt::white;
                    break;
                case WATER:
                    color = Qt::blue;
                    break;
                default:
                    color = Qt::magenta; // A color for unknown tile types.
            }
            painter.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
        }
    }
}
