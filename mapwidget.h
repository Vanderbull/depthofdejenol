#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include "mapeditor.h"

class MapWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapWidget(MapEditor* editor, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    MapEditor* m_mapEditor;
    const int TILE_SIZE = 20; // Size in pixels for each tile.
};

#endif // MAPWIDGET_H
