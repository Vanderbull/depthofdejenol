#include <QApplication>
#include <QMainWindow>
#include "mapeditor.h"
#include "mapwidget.h"

int main(int argc, char *argv[]) {
    // Create the Qt application instance.
    QApplication a(argc, argv);

    // Define the dimensions of the map.
    const int MAP_WIDTH = 20;
    const int MAP_HEIGHT = 15;

    // Create a MapEditor instance to hold the map data.
    MapEditor mapEditor(MAP_WIDTH, MAP_HEIGHT);

    // Populate the map with some example tiles.
    // Set a few walls.
    mapEditor.setTile(5, 5, WALL);
    mapEditor.setTile(5, 6, WALL);
    mapEditor.setTile(6, 5, WALL);
    mapEditor.setTile(6, 6, WALL);

    // Create a small pool of water.
    mapEditor.setTile(10, 8, WATER);
    mapEditor.setTile(11, 8, WATER);
    mapEditor.setTile(10, 9, WATER);
    mapEditor.setTile(11, 9, WATER);

    // Create the MapWidget, passing the MapEditor instance to it.
    MapWidget mapWidget(&mapEditor);

    // Create the main window.
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Map Editor");

    // Set the MapWidget as the central widget of the main window.
    mainWindow.setCentralWidget(&mapWidget);

    // Display the main window.
    mainWindow.show();

    // Start the application's event loop.
    return a.exec();
}
