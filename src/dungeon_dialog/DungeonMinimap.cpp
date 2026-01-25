#include "DungeonDialog.h"
#include "../../GameStateManager.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QPen>
#include <QBrush>
#include <QDebug>
// These constants match the definitions in DungeonDialog.cpp
static const int TILE_SIZE = 10;
static const int MAP_WIDTH_PIXELS = MAP_SIZE * TILE_SIZE; 
static const int MAP_HEIGHT_PIXELS = MAP_SIZE * TILE_SIZE; 

void DungeonDialog::drawMinimap()
{
    QString fogPath = "resources/images/minimap/fog.png";
    QString antimagicPath = "resources/images/minimap/antimagic.png";
    QString chutePath = "resources/images/minimap/chute.png";
    QString extinguisherPath = "resources/images/minimap/extinguisher.png";
    QString rotatorPath = "resources/images/minimap/rotator.png";
    QString rockPath = "resources/images/minimap/rock.png";
    QString studPath = "resources/images/minimap/stud.png";
    QString teleporterPath = "resources/images/minimap/teleporter.png";
    QString stairsdownPath = "resources/images/minimap/stairsdown.png";
    QString stairsupPath = "resources/images/minimap/stairsup.png";
    QString waterPath = "resources/images/minimap/water.png";
    QPixmap antimagicPixmap(antimagicPath);
    QPixmap chutePixmap(chutePath);
    QPixmap extinguisherPixmap(extinguisherPath);
    QPixmap rotatorPixmap(rotatorPath);
    QPixmap studPixmap(studPath);
    QPixmap teleporterPixmap(teleporterPath);
    QPixmap stairsdownPixmap(stairsdownPath);
    QPixmap waterPixmap(waterPath);
    QPixmap stairsupPixmap(stairsupPath);
    QPixmap scaledAntimagic = antimagicPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    QPixmap scaledChute = chutePixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    QPixmap scaledExtinguisher = extinguisherPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    QPixmap scaledRotator = rotatorPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    QPixmap scaledStud = studPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    QPixmap scaledTeleporter = teleporterPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    QPixmap scaledStairsDown = stairsdownPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    QPixmap scaledWater = waterPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    QPixmap scaledStairsUp = stairsupPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    QPixmap fogPixmap(fogPath);
    // Check if the image loaded correctly for debugging
    if (fogPixmap.isNull()) {
        qDebug() << "Error: Fog image not found at" << fogPath;
    }
    // Scale the image once to 10x10 pixels (TILE_SIZE)
    QPixmap scaledFog = fogPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                         Qt::IgnoreAspectRatio, 
                                         Qt::SmoothTransformation);
    // Load and scale the rock pixmap
    QPixmap rockPixmap(rockPath);
     // Check if the image loaded correctly for debugging
    if (rockPixmap.isNull()) {
         qDebug() << "Error: rock image not found at" << rockPath;
    }
    QPixmap scaledRock = rockPixmap.scaled(TILE_SIZE, TILE_SIZE, 
                                           Qt::IgnoreAspectRatio, 
                                           Qt::SmoothTransformation);
    GameStateManager* gsm = GameStateManager::instance();
    // Retrieve player position from GameState
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> currentPos = {currentX, currentY};
    // Mark the current position as visited for the Fog of War
    m_visitedTiles.insert(currentPos);
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, MAP_WIDTH_PIXELS, MAP_HEIGHT_PIXELS);
    // Check the toggle state from our new dialog
    bool revealAll = m_standaloneMinimap && m_standaloneMinimap->isRevealAllEnabled();
    // 2. Draw Obstacles (Walls)
    for (const auto& pos : m_obstaclePositions) {
        // Walls are visible only if the tile has been visited
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!rockPixmap.isNull()) {
                // Use the rock image tile
                QGraphicsPixmapItem* rockTile = scene->addPixmap(scaledRock);
                rockTile->setPos(pos.first * TILE_SIZE, pos.second * TILE_SIZE);
            } else {
                // Fallback to dark gray rectangle if image is missing
                scene->addRect(pos.first * TILE_SIZE, pos.second * TILE_SIZE, 
                               TILE_SIZE, TILE_SIZE, QPen(Qt::black), QBrush(Qt::darkGray));
            }
        }
    }
    // 3. Draw Stairs (Cyan)
    if (revealAll || m_visitedTiles.contains(m_stairsUpPosition)) {
        if (!stairsupPixmap.isNull()) {
            QGraphicsPixmapItem* upTile = scene->addPixmap(scaledStairsUp);
            upTile->setPos(m_stairsUpPosition.first * TILE_SIZE, 
                           m_stairsUpPosition.second * TILE_SIZE);
            upTile->setZValue(1); // Ensure it stays above the floor/grid
        } else {
            // Fallback to Cyan if image fails to load
            QGraphicsRectItem* stairsRect = scene->addRect(m_stairsUpPosition.first * TILE_SIZE, 
                           m_stairsUpPosition.second * TILE_SIZE, 
                           TILE_SIZE, TILE_SIZE, QPen(Qt::black), QBrush(Qt::cyan));
            stairsRect->setZValue(1);
        }
    }
    if (revealAll || m_visitedTiles.contains(m_stairsDownPosition)) {
        if (!stairsdownPixmap.isNull()) {
            QGraphicsPixmapItem* downTile = scene->addPixmap(scaledStairsDown);
            downTile->setPos(m_stairsDownPosition.first * TILE_SIZE, 
                           m_stairsDownPosition.second * TILE_SIZE);
            downTile->setZValue(1); // Ensure it stays above the floor/grid
        } else {
            // Fallback to Cyan if image fails to load
            QGraphicsRectItem* stairsRect = scene->addRect(m_stairsDownPosition.first * TILE_SIZE, 
                           m_stairsDownPosition.second * TILE_SIZE, 
                           TILE_SIZE, TILE_SIZE, QPen(Qt::black), QBrush(Qt::cyan));
            stairsRect->setZValue(1);
        }
    }
    // 4. Draw Chutes (Only if visited)
    for (const auto& pos : m_chutePositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledChute.isNull()) {
                // Use the chute image tile
                QGraphicsPixmapItem* chuteTile = scene->addPixmap(scaledChute);
                chuteTile->setPos(pos.first * TILE_SIZE, pos.second * TILE_SIZE);
            } else {
                // Fallback to the ellipse if image fails to load
                scene->addEllipse(pos.first * TILE_SIZE + 1, pos.second * TILE_SIZE + 1, 
                                  TILE_SIZE - 2, TILE_SIZE - 2, QPen(Qt::gray), QBrush(Qt::black));
            }
        }
    }
    // 5. Draw Monsters (Red Ellipses)
    for (auto it = m_monsterPositions.begin(); it != m_monsterPositions.end(); ++it) {
        QPair<int, int> pos = it.key();
        // Only show if the tile is visited or "Reveal All" is on
        if (revealAll || m_visitedTiles.contains(pos)) {
            scene->addEllipse(pos.first * TILE_SIZE + TILE_SIZE/4, 
                              pos.second * TILE_SIZE + TILE_SIZE/4, 
                              TILE_SIZE/2, TILE_SIZE/2, 
                              QPen(Qt::black), QBrush(Qt::red));
        }
    }
    // 6. Draw Treasure/Chests (Yellow Rectangles)
    for (auto it = m_treasurePositions.begin(); it != m_treasurePositions.end(); ++it) {
        QPair<int, int> pos = it.key();
        if (revealAll || m_visitedTiles.contains(pos)) {
            scene->addRect(pos.first * TILE_SIZE + TILE_SIZE/4, 
                           pos.second * TILE_SIZE + TILE_SIZE/4, 
                           TILE_SIZE/2, TILE_SIZE/2, 
                           QPen(Qt::black), QBrush(Qt::yellow));
        }
    }
    // 7. Draw Traps (Dark Green Rectangles)
    for (auto it = m_trapPositions.begin(); it != m_trapPositions.end(); ++it) {
        QPair<int, int> pos = it.key();
        if (revealAll || m_visitedTiles.contains(pos)) {
            scene->addRect(pos.first * TILE_SIZE + TILE_SIZE/4, 
                           pos.second * TILE_SIZE + TILE_SIZE/4, 
                           TILE_SIZE/2, TILE_SIZE/2, 
                           QPen(Qt::black), QBrush(Qt::darkGreen));
        }
    }
    // 7.5. Draw Antimagic Fields (Only if visited)
    for (const auto& pos : m_antimagicPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledAntimagic.isNull()) {
                QGraphicsPixmapItem* antiTile = scene->addPixmap(scaledAntimagic);
                antiTile->setPos(pos.first * TILE_SIZE, pos.second * TILE_SIZE);
            } else {
                // Fallback to Purple if image fails to load
                scene->addRect(pos.first * TILE_SIZE + 1, pos.second * TILE_SIZE + 1, 
                               TILE_SIZE - 2, TILE_SIZE - 2, QPen(Qt::magenta), QBrush(Qt::NoBrush));
            }
        }
    }
    // 7.6. Draw Rotator Tiles (Only if visited)
    for (const auto& pos : m_rotatorPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledRotator.isNull()) {
                QGraphicsPixmapItem* rotTile = scene->addPixmap(scaledRotator);
                rotTile->setPos(pos.first * TILE_SIZE, pos.second * TILE_SIZE);
            } else {
                // Fallback: A dark green/yellow circle if image fails to load
                scene->addEllipse(pos.first * TILE_SIZE + 1, pos.second * TILE_SIZE + 1, 
                                 TILE_SIZE - 2, TILE_SIZE - 2, QPen(Qt::darkYellow), QBrush(Qt::NoBrush));
            }
        }
    }
    // 7.7. Draw Water Tiles (Only if visited)
    for (const auto& pos : m_waterPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledWater.isNull()) {
                QGraphicsPixmapItem* waterTile = scene->addPixmap(scaledWater);
                waterTile->setPos(pos.first * TILE_SIZE, pos.second * TILE_SIZE);
            } else {
                // Fallback: Blue square if image fails to load
                scene->addRect(pos.first * TILE_SIZE, pos.second * TILE_SIZE, 
                               TILE_SIZE, TILE_SIZE, QPen(Qt::blue), QBrush(Qt::blue));
            }
        }
    }
    // 7.8. Draw Teleporter Tiles (Only if visited)
    for (const auto& pos : m_teleportPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledTeleporter.isNull()) {
                // Use the teleporter image tile
                QGraphicsPixmapItem* teleTile = scene->addPixmap(scaledTeleporter);
                teleTile->setPos(pos.first * TILE_SIZE, pos.second * TILE_SIZE);
            } else {
                // Fallback: Magenta circle if image fails to load
                scene->addEllipse(pos.first * TILE_SIZE + 1, pos.second * TILE_SIZE + 1, 
                                  TILE_SIZE - 2, TILE_SIZE - 2, QPen(Qt::magenta), QBrush(Qt::NoBrush));
            }
        }
    }
    // Draw Stud Tiles (Only if visited)
    for (const auto& pos : m_studPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledStud.isNull()) {
                // Use the stud image tile
                QGraphicsPixmapItem* studTile = scene->addPixmap(scaledStud);
                studTile->setPos(pos.first * TILE_SIZE, pos.second * TILE_SIZE);
            } else {
                // Fallback: A simple gray rectangle if image fails to load
                scene->addRect(pos.first * TILE_SIZE + 2, pos.second * TILE_SIZE + 2, 
                               TILE_SIZE - 4, TILE_SIZE - 4, QPen(Qt::gray), QBrush(Qt::lightGray));
            }
        }
    }
    //Draw extinguisher tiles
    for (const auto& pos : m_extinguisherPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledExtinguisher.isNull()) {
                QGraphicsPixmapItem* extTile = scene->addPixmap(scaledExtinguisher);
                extTile->setPos(pos.first * TILE_SIZE, pos.second * TILE_SIZE);
            } else {
                // Fallback: Dark Blue/Gray square to represent "extinguishing" light
                scene->addRect(pos.first * TILE_SIZE + 1, pos.second * TILE_SIZE + 1, 
                               TILE_SIZE - 2, TILE_SIZE - 2, QPen(Qt::blue), QBrush(Qt::darkBlue));
            }
        }
    }
    // 8. Draw Fog of War Overlay
    if (!revealAll) {
        for (int x = 0; x < MAP_SIZE; ++x) { 
            for (int y = 0; y < MAP_SIZE; ++y) {
                if (!m_visitedTiles.contains({x, y})) {
                    if (!fogPixmap.isNull()) {
                        // Use the image tile
                        QGraphicsPixmapItem* fogTile = scene->addPixmap(scaledFog);
                        fogTile->setPos(x * TILE_SIZE, y * TILE_SIZE);
                    } else {
                        // Fallback to semi-transparent black if image is missing
                        scene->addRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, 
                                       Qt::NoPen, QBrush(QColor(0, 0, 0, 220))); 
                    }
                }
            }
        }
    }
    // Draw Breadcrumb Trail
    for (int i = 0; i < m_breadcrumbPath.size(); ++i) {
        QPair<int, int> pos = m_breadcrumbPath.at(i);   
        // Calculate opacity so older dots fade out
        int opacity = static_cast<int>((static_cast<float>(i) / m_breadcrumbPath.size()) * 150);
        // Draw a small subtle dot in the center of the tile
        scene->addEllipse(pos.first * TILE_SIZE + (TILE_SIZE / 3), 
                          pos.second * TILE_SIZE + (TILE_SIZE / 3), 
                          TILE_SIZE / 4, TILE_SIZE / 4, 
                          Qt::NoPen, 
                          QBrush(QColor(0, 200, 255, opacity))); // Fading cyan trail
    }
    // 9. Draw Player Arrow (Blue)
    QGraphicsPolygonItem* playerArrow = new QGraphicsPolygonItem();
    QPolygonF arrowHead;
    // Defining an arrow shape relative to its origin
    arrowHead << QPointF(TILE_SIZE / 2, 0)               // Tip
              << QPointF(0, TILE_SIZE)                   // Bottom Left
              << QPointF(TILE_SIZE / 2, TILE_SIZE * 0.7) // Inner Notch
              << QPointF(TILE_SIZE, TILE_SIZE);          // Bottom Right
    playerArrow->setPolygon(arrowHead);
    playerArrow->setBrush(QBrush(Qt::blue));
    playerArrow->setPen(QPen(Qt::blue));
    // Calculate rotation based on facing direction string
    int rotation = 0;
    QString currentFacing = m_compassLabel->text();
    if (currentFacing == "Facing North") rotation = 0;
    else if (currentFacing == "Facing East") rotation = 90;
    else if (currentFacing == "Facing South") rotation = 180;
    else if (currentFacing == "Facing West") rotation = 270;
    // Center the arrow in the tile and rotate
    playerArrow->setTransformOriginPoint(TILE_SIZE / 2, TILE_SIZE / 2);
    playerArrow->setRotation(rotation);
    playerArrow->setPos(currentPos.first * TILE_SIZE, currentPos.second * TILE_SIZE);
    scene->addItem(playerArrow);
    if (m_standaloneMinimap) {
        m_standaloneMinimap->updateScene(scene);
    } else {
        delete scene; // Cleanup if window doesn't exist
    }
}

void DungeonDialog::updateMinimap(int x, int y, int z=0)
{
    Q_UNUSED(z);
    // Define the visibility radius (e.g., 2 tiles in every direction)
    const int VIEW_DISTANCE = 2;
    for (int dx = -VIEW_DISTANCE; dx <= VIEW_DISTANCE; ++dx) {
        for (int dy = -VIEW_DISTANCE; dy <= VIEW_DISTANCE; ++dy) {
            int targetX = x + dx;
            int targetY = y + dy;
            // Ensure we stay within the map boundaries
            if (targetX >= 0 && targetX < MAP_SIZE && targetY >= 0 && targetY < MAP_SIZE) {
                m_visitedTiles.insert({targetX, targetY});
            }
        }
    }
    drawMinimap(); 
}
