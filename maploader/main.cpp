#include "MapLoader.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QString mapFilePath = QDir::currentPath() + "/random_map.dat";
    
    // 1. GENERATE THE RANDOM MAP FILE
    MapLoader generator;
    qDebug() << "--- GENERATING MAP ---";
    bool success = generator.generateRandomMap(
        mapFilePath, 
        "MAP_V1.3",     
        12              
    );

    if (success) {
        // 2. LOAD THE GENERATED MAP FILE
        MapLoader loader;
        qDebug() << "\n--- LOADING MAP ---";
        if (loader.loadMap(mapFilePath)) {
            qDebug() << "Verification successful:";
            
            // 3. SAMPLE DATA CHECK
            const CellData& cell = loader.getMapData()[5][15][15];
            
            qDebug() << "Sample Cell (5, 15, 15) Bitmask:" 
                     << QString::number(cell.fieldBitmask, 16).toUpper().rightJustified(8, '0');

            // ⚠️ CORRECTED USAGE: MapFeature::WALL_NORTH
            if (cell.hasFeature(static_cast<quint32>(MapFeature::WALL_NORTH))) {
                qDebug() << "-> Cell has WALL NORTH feature!";
            } else {
                qDebug() << "-> Cell does NOT have Wall North feature.";
            }
            
            // ⚠️ CORRECTED USAGE: MapFeature::EXPLORED
            if (cell.hasFeature(static_cast<quint32>(MapFeature::EXPLORED))) {
                qDebug() << "-> Cell is marked as EXPLORED.";
            }
            
        } else {
            qWarning() << "Verification failed: Could not load the generated map.";
        }
    } else {
        qWarning() << "Map generation failed.";
    }

    return 0;
}
