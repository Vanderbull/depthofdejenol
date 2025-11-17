#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QVector>

class Map : public QObject
{
    Q_OBJECT
public:
    // Constants for the map dimensions
    static constexpr int WIDTH = 30;
    static constexpr int HEIGHT = 30;

    // Type definition for a single cell's data (e.g., a simple integer type)
    using CellData = int;

    // Type definition for the entire map grid
    using Grid = QVector<QVector<CellData>>;

    explicit Map(QObject *parent = nullptr);

    // Accessor for the cell data at (x, y)
    CellData cellAt(int x, int y) const;

    // Mutator to set the cell data at (x, y)
    void setCellAt(int x, int y, CellData data);

    // Method to initialize the map (e.g., fill with zeros or a default value)
    void initialize();

private:
    Grid m_grid;
};

#endif // MAP_H
