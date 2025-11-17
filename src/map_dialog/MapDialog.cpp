#include "MapDialog.h"
#include <QVBoxLayout>

MapDialog::MapDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Map");

    // 1. Initialize the map data
    // m_gameMap is initialized in the member list
    // Optional: Set some test data to see different colors
    m_gameMap.setCellAt(5, 5, 1);
    m_gameMap.setCellAt(10, 10, 2);
    m_gameMap.setCellAt(15, 15, 99); // Use 99 for "bomb" example

    // 2. Create the custom map widget, passing in the map data
    m_mapWidget = new MapWidget(&m_gameMap, this);

    // 3. Set up the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_mapWidget, 0, Qt::AlignCenter);
    
    // Set a fixed size for the dialog based on the widget size
//    setFixedSize(m_mapWidget->sizeHint().expandedTo(QSize(300, 300))); 
adjustSize();
}

MapDialog::~MapDialog()
{
    // m_mapWidget is parented to the dialog, so it's automatically deleted.
}
