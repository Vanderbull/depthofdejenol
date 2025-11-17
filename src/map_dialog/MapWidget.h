#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include "Map.h" // Include the Map class definition

class MapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapWidget(Map *map, QWidget *parent = nullptr);

    // Define the size for each cell
    static constexpr int CELL_SIZE = 20;
    // Overridden method to tell Qt how big the widget should be

    QSize sizeHint() const override;

protected:
    // Overridden method to handle painting/drawing
    void paintEvent(QPaintEvent *event) override;


private:
    Map *m_map;
};

#endif // MAPWIDGET_H
