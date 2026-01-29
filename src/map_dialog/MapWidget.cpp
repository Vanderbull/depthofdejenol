#include "MapWidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>

MapWidget::MapWidget(Map *map, QWidget *parent)
    : QWidget(parent), m_map(map)
{
    // Set a fixed size based on the map dimensions and cell size
    setFixedSize(sizeHint());
}

QSize MapWidget::sizeHint() const
{
    return QSize(Map::WIDTH * CELL_SIZE + 1, Map::HEIGHT * CELL_SIZE + 1);
}

void MapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    // Define styles
    QPen borderPen(Qt::black, 1);
    painter.setPen(borderPen);
    // Loop through all cells and draw them
    for (int y = 0; y < Map::HEIGHT; ++y) {
        for (int x = 0; x < Map::WIDTH; ++x) {
            // Get the data for the current cell
            int cellValue = m_map->cellAt(x, y);
            // 1. Determine the color (Minesweeper style)
            QColor cellColor;
            if (cellValue == 0) {
                // Empty/unopened cell (light gray)
                cellColor = QColor("#C0C0C0");
            } else if (cellValue == 1) {
                // Example: Opened cell (a lighter shade)
                cellColor = QColor("#E0E0E0");
            } else {
                // Example: Bomb/special cell (red)
                cellColor = QColor("#FF0000");
            }
            // Set the brush for the cell fill
            painter.setBrush(QBrush(cellColor));
            // Calculate the cell rectangle coordinates
            int left = x * CELL_SIZE;
            int top = y * CELL_SIZE;
            // 2. Draw the cell rectangle
            painter.drawRect(left, top, CELL_SIZE, CELL_SIZE);
            // 3. Optional: Draw the value (like the number of adjacent bombs)
            if (cellValue > 0) {
                painter.drawText(left, top, CELL_SIZE, CELL_SIZE,
                                 Qt::AlignCenter, QString::number(cellValue));
            }
        }
    }
}
