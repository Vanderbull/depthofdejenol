#include "mapwidget.h"

MapWidget::MapWidget(MapEditor* mapEditor, QWidget* parent)
    : QWidget(parent), m_mapEditor(mapEditor) {
    setFixedSize(m_mapEditor->getWidth() * 20, m_mapEditor->getHeight() * 20);
    setMouseTracking(true);
}

MapEditor::TileType MapWidget::getCurrentTileType() const {
    return m_currentTileType;
}

void MapWidget::setCurrentTileType(MapEditor::TileType type) {
    m_currentTileType = type;
}

void MapWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    const int tileSize = 20;

    // First, draw the base map layer
    const auto& baseMap = m_mapEditor->getBaseMap();
    for (size_t y = 0; y < baseMap.size(); ++y) {
        for (size_t x = 0; x < baseMap[y].size(); ++x) {
            MapEditor::TileType tileType = baseMap[y][x];
            switch (tileType) {
                case MapEditor::EMPTY:
                    painter.fillRect(x * tileSize, y * tileSize, tileSize, tileSize, Qt::black);
                    break;
                case MapEditor::FLOOR:
                    painter.fillRect(x * tileSize, y * tileSize, tileSize, tileSize, Qt::lightGray);
                    break;
                case MapEditor::WATER:
                    painter.fillRect(x * tileSize, y * tileSize, tileSize, tileSize, Qt::blue);
                    break;
                default:
                    break;
            }
        }
    }

    // Second, draw the wall overlay layer
    const auto& wallMap = m_mapEditor->getWallMap();
    painter.setPen(QPen(Qt::darkGray, 2));
    for (size_t y = 0; y < wallMap.size(); ++y) {
        for (size_t x = 0; x < wallMap[y].size(); ++x) {
            if (wallMap[y][x]) {
                painter.drawRect(x * tileSize, y * tileSize, tileSize, tileSize);
            }
        }
    }
}

void MapWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isMousePressed = true;
        handleMouseInput(event->pos());
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_isMousePressed) {
        handleMouseInput(event->pos());
    }
}

void MapWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        setCurrentTileType(MapEditor::WALL);
    } else if (event->key() == Qt::Key_F) {
        setCurrentTileType(MapEditor::FLOOR);
    } else if (event->key() == Qt::Key_E) {
        setCurrentTileType(MapEditor::EMPTY);
    } else if (event->key() == Qt::Key_A) {
        setCurrentTileType(MapEditor::WATER);
    }
}

void MapWidget::handleMouseInput(const QPoint& pos) {
    int gridX = pos.x() / 20;
    int gridY = pos.y() / 20;

    if (gridX >= 0 && gridX < m_mapEditor->getWidth() && gridY >= 0 && gridY < m_mapEditor->getHeight()) {
        if (m_currentTileType == MapEditor::WALL) {
            m_mapEditor->setWallTile(gridX, gridY, true);
        } else {
            m_mapEditor->setBaseTile(gridX, gridY, m_currentTileType);
            m_mapEditor->setWallTile(gridX, gridY, false);
        }
        update(); // Force a repaint
    }
}
