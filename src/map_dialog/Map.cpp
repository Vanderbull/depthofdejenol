#include "Map.h"
#include <QDebug>

Map::Map(QObject *parent)
    : QObject(parent)
{
    // Initialize the grid in the constructor
    initialize();
}
/**
 * @brief Initializes the 30x30 grid.
 */
void Map::initialize()
{
    // Resize the grid to the defined dimensions
    m_grid.resize(HEIGHT);
    for (int y = 0; y < HEIGHT; ++y) {
        // Resize each inner vector (the row) to the width
        m_grid[y].resize(WIDTH);
        // Optional: Initialize all cells to a default value (e.g., 0)
        m_grid[y].fill(0);
    }
    qDebug() << "Map initialized with dimensions:" << WIDTH << "x" << HEIGHT;
}
/**
 * @brief Returns the data of the cell at coordinates (x, y).
 * @param x The column index (0 to WIDTH - 1).
 * @param y The row index (0 to HEIGHT - 1).
 * @return The data stored in the cell.
 */
Map::CellData Map::cellAt(int x, int y) const
{
    // Basic bounds checking
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        return m_grid[y][x];
    }
    qWarning() << "cellAt: Coordinates out of bounds:" << x << "," << y;
    // Return a safe/default value on error
    return 0;
}
/**
 * @brief Sets the data of the cell at coordinates (x, y).
 * @param x The column index (0 to WIDTH - 1).
 * @param y The row index (0 to HEIGHT - 1).
 * @param data The new data to store in the cell.
 */
void Map::setCellAt(int x, int y, CellData data)
{
    // Basic bounds checking
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        m_grid[y][x] = data;
    } else {
        qWarning() << "setCellAt: Coordinates out of bounds:" << x << "," << y;
    }
}
