#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include "mapeditor.h"

class MapWidget : public QWidget {
    Q_OBJECT

public:
    MapWidget(MapEditor* mapEditor, QWidget *parent = nullptr);
    MapEditor::TileType getCurrentTileType() const;
    void setCurrentTileType(MapEditor::TileType type);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void drawTile(int x, int y);
    void handleMouseInput(const QPoint& pos);
    
    MapEditor* m_mapEditor;
    MapEditor::TileType m_currentTileType = MapEditor::EMPTY;
    bool m_isMousePressed = false;
};

#endif // MAPWIDGET_H
