#include "DungeonDialog.h"
#include "src/core/gameStateManager.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QPen>
#include <QBrush>
#include <QDebug>
// These constants match the definitions in DungeonDialog.cpp
static const int TILE_SIZE = 10;   // pixels per tile in the integrated minimap

// Renders the dungeon into `scene` at `tileSize` pixels per tile. Shared by the
// integrated minimap and the AutoMap dialog so the two views cannot drift apart.
// `revealAll` lifts fog of war for this render only; it does not persist.
void DungeonDialog::drawMapTo(QGraphicsScene *scene, int tileSize, bool revealAll)
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
    QPixmap scaledAntimagic = antimagicPixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    QPixmap scaledChute = chutePixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    QPixmap scaledExtinguisher = extinguisherPixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    QPixmap scaledRotator = rotatorPixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    QPixmap scaledStud = studPixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    QPixmap scaledTeleporter = teleporterPixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    QPixmap scaledStairsDown = stairsdownPixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    QPixmap scaledWater = waterPixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    QPixmap scaledStairsUp = stairsupPixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    QPixmap fogPixmap(fogPath);
    // Check if the image loaded correctly for debugging
    if (fogPixmap.isNull()) {
        qDebug() << "Error: Fog image not found at" << fogPath;
    }
    // Scale the image once to 10x10 pixels (tileSize)
    QPixmap scaledFog = fogPixmap.scaled(tileSize, tileSize,
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);
    // Load and scale the rock pixmap
    QPixmap rockPixmap(rockPath);
     // Check if the image loaded correctly for debugging
    if (rockPixmap.isNull()) {
         qDebug() << "Error: rock image not found at" << rockPath;
    }
    QPixmap scaledRock = rockPixmap.scaled(tileSize, tileSize,
                                           Qt::IgnoreAspectRatio,
                                           Qt::SmoothTransformation);
    gameStateManager* gsm = gameStateManager::instance();
    // Retrieve player position from GameState
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> currentPos = {currentX, currentY};
    // Mark the current position as visited for the Fog of War
    m_visitedTiles.insert(currentPos);
    scene->setSceneRect(0, 0, MAP_SIZE * tileSize, MAP_SIZE * tileSize);
    // 2. Draw Obstacles (Walls)
    for (const auto& pos : m_obstaclePositions) {
        // Walls are visible only if the tile has been visited
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!rockPixmap.isNull()) {
                // Use the rock image tile
                QGraphicsPixmapItem* rockTile = scene->addPixmap(scaledRock);
                rockTile->setPos(pos.first * tileSize, pos.second * tileSize);
            } else {
                // Fallback to dark gray rectangle if image is missing
                scene->addRect(pos.first * tileSize, pos.second * tileSize,
                               tileSize, tileSize, QPen(Qt::black), QBrush(Qt::darkGray));
            }
        }
    }
    // 3. Draw Stairs (Cyan)
    if (revealAll || m_visitedTiles.contains(m_stairsUpPosition)) {
        if (!stairsupPixmap.isNull()) {
            QGraphicsPixmapItem* upTile = scene->addPixmap(scaledStairsUp);
            upTile->setPos(m_stairsUpPosition.first * tileSize,
                           m_stairsUpPosition.second * tileSize);
            upTile->setZValue(1); // Ensure it stays above the floor/grid
        } else {
            // Fallback to Cyan if image fails to load
            QGraphicsRectItem* stairsRect = scene->addRect(m_stairsUpPosition.first * tileSize,
                           m_stairsUpPosition.second * tileSize,
                           tileSize, tileSize, QPen(Qt::black), QBrush(Qt::cyan));
            stairsRect->setZValue(1);
        }
    }
    if (revealAll || m_visitedTiles.contains(m_stairsDownPosition)) {
        if (!stairsdownPixmap.isNull()) {
            QGraphicsPixmapItem* downTile = scene->addPixmap(scaledStairsDown);
            downTile->setPos(m_stairsDownPosition.first * tileSize,
                           m_stairsDownPosition.second * tileSize);
            downTile->setZValue(1); // Ensure it stays above the floor/grid
        } else {
            // Fallback to Cyan if image fails to load
            QGraphicsRectItem* stairsRect = scene->addRect(m_stairsDownPosition.first * tileSize,
                           m_stairsDownPosition.second * tileSize,
                           tileSize, tileSize, QPen(Qt::black), QBrush(Qt::cyan));
            stairsRect->setZValue(1);
        }
    }
    // 4. Draw Chutes (Only if visited)
    for (const auto& pos : m_chutePositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledChute.isNull()) {
                // Use the chute image tile
                QGraphicsPixmapItem* chuteTile = scene->addPixmap(scaledChute);
                chuteTile->setPos(pos.first * tileSize, pos.second * tileSize);
            } else {
                // Fallback to the ellipse if image fails to load
                scene->addEllipse(pos.first * tileSize + 1, pos.second * tileSize + 1,
                                  tileSize - 2, tileSize - 2, QPen(Qt::gray), QBrush(Qt::black));
            }
        }
    }
    // 5. Draw Monsters (Red Ellipses)
    for (auto it = m_monsterPositions.begin(); it != m_monsterPositions.end(); ++it) {
        QPair<int, int> pos = it.key();
        // Only show if the tile is visited or "Reveal All" is on
        if (revealAll || m_visitedTiles.contains(pos)) {
            scene->addEllipse(pos.first * tileSize + tileSize/4,
                              pos.second * tileSize + tileSize/4,
                              tileSize/2, tileSize/2,
                              QPen(Qt::black), QBrush(Qt::red));
        }
    }
    // 6. Draw Treasure/Chests (Yellow Rectangles)
    for (auto it = m_treasurePositions.begin(); it != m_treasurePositions.end(); ++it) {
        QPair<int, int> pos = it.key();
        if (revealAll || m_visitedTiles.contains(pos)) {
            scene->addRect(pos.first * tileSize + tileSize/4,
                           pos.second * tileSize + tileSize/4,
                           tileSize/2, tileSize/2,
                           QPen(Qt::black), QBrush(Qt::yellow));
        }
    }
    // 7. Draw Traps (Dark Green Rectangles)
    for (auto it = m_trapPositions.begin(); it != m_trapPositions.end(); ++it) {
        QPair<int, int> pos = it.key();
        if (revealAll || m_visitedTiles.contains(pos)) {
            scene->addRect(pos.first * tileSize + tileSize/4,
                           pos.second * tileSize + tileSize/4,
                           tileSize/2, tileSize/2,
                           QPen(Qt::black), QBrush(Qt::darkGreen));
        }
    }
    // 7.5. Draw Antimagic Fields (Only if visited)
    for (const auto& pos : m_antimagicPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledAntimagic.isNull()) {
                QGraphicsPixmapItem* antiTile = scene->addPixmap(scaledAntimagic);
                antiTile->setPos(pos.first * tileSize, pos.second * tileSize);
            } else {
                // Fallback to Purple if image fails to load
                scene->addRect(pos.first * tileSize + 1, pos.second * tileSize + 1,
                               tileSize - 2, tileSize - 2, QPen(Qt::magenta), QBrush(Qt::NoBrush));
            }
        }
    }
    // 7.6. Draw Rotator Tiles (Only if visited)
    for (const auto& pos : m_rotatorPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledRotator.isNull()) {
                QGraphicsPixmapItem* rotTile = scene->addPixmap(scaledRotator);
                rotTile->setPos(pos.first * tileSize, pos.second * tileSize);
            } else {
                // Fallback: A dark green/yellow circle if image fails to load
                scene->addEllipse(pos.first * tileSize + 1, pos.second * tileSize + 1,
                                 tileSize - 2, tileSize - 2, QPen(Qt::darkYellow), QBrush(Qt::NoBrush));
            }
        }
    }
    // 7.7. Draw Water Tiles (Only if visited)
    for (const auto& pos : m_waterPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledWater.isNull()) {
                QGraphicsPixmapItem* waterTile = scene->addPixmap(scaledWater);
                waterTile->setPos(pos.first * tileSize, pos.second * tileSize);
            } else {
                // Fallback: Blue square if image fails to load
                scene->addRect(pos.first * tileSize, pos.second * tileSize,
                               tileSize, tileSize, QPen(Qt::blue), QBrush(Qt::blue));
            }
        }
    }
    // 7.8. Draw Teleporter Tiles (Only if visited)
    for (const auto& pos : m_teleporterPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledTeleporter.isNull()) {
                // Use the teleporter image tile
                QGraphicsPixmapItem* teleTile = scene->addPixmap(scaledTeleporter);
                teleTile->setPos(pos.first * tileSize, pos.second * tileSize);
            } else {
                // Fallback: Magenta circle if image fails to load
                scene->addEllipse(pos.first * tileSize + 1, pos.second * tileSize + 1,
                                  tileSize - 2, tileSize - 2, QPen(Qt::magenta), QBrush(Qt::NoBrush));
            }
        }
    }
    // Draw Stud Tiles (Only if visited)
    for (const auto& pos : m_studPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledStud.isNull()) {
                // Use the stud image tile
                QGraphicsPixmapItem* studTile = scene->addPixmap(scaledStud);
                studTile->setPos(pos.first * tileSize, pos.second * tileSize);
            } else {
                // Fallback: A simple gray rectangle if image fails to load
                scene->addRect(pos.first * tileSize + 2, pos.second * tileSize + 2,
                               tileSize - 4, tileSize - 4, QPen(Qt::gray), QBrush(Qt::lightGray));
            }
        }
    }
    //Draw extinguisher tiles
    for (const auto& pos : m_extinguisherPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!scaledExtinguisher.isNull()) {
                QGraphicsPixmapItem* extTile = scene->addPixmap(scaledExtinguisher);
                extTile->setPos(pos.first * tileSize, pos.second * tileSize);
            } else {
                // Fallback: Dark Blue/Gray square to represent "extinguishing" light
                scene->addRect(pos.first * tileSize + 1, pos.second * tileSize + 1,
                               tileSize - 2, tileSize - 2, QPen(Qt::blue), QBrush(Qt::darkBlue));
            }
        }
    }
    // Draw pits
    for (const auto& pos : m_pitPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            scene->addRect(pos.first * tileSize + 1, pos.second * tileSize + 1,
                           tileSize - 2, tileSize - 2,
                           QPen(Qt::darkRed), QBrush(Qt::black));
        }
    }

    // Define a path and pixmap for the door icon
    QString doorPath = "resources/images/minimap/hiddendoor.png";
    QPixmap doorPixmap(doorPath);
    QPixmap scaledDoor = doorPixmap.scaled(tileSize, tileSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // Draw Hidden Doors
    bool partySeeInvisible = false;
    auto& members = gameStateManager::instance()->getPartyMembers();
    for (int i = 0; i < members.size(); ++i) {
        if (members[i].seeInvisibleTurns > 0) {
            partySeeInvisible = true;
            break;
        }
    }
    for (auto it = m_hiddenDoorPositions.constBegin(); it != m_hiddenDoorPositions.constEnd(); ++it) {
        const auto& pos = it.key();
        // Logic: Show if debug 'revealAll' is on, OR if the tile is visited,
        // OR if a party member has true sight active
        if (revealAll || m_visitedTiles.contains(pos) || partySeeInvisible) {
            if (!doorPixmap.isNull()) {
                QGraphicsPixmapItem* doorTile = scene->addPixmap(scaledDoor);
                doorTile->setPos(pos.first * tileSize, pos.second * tileSize);
                doorTile->setZValue(1);
            } else {
                // Fallback: Draw a yellow rectangle for testing if image is missing
                scene->addRect(pos.first * tileSize + 2, pos.second * tileSize + 2,
                               tileSize - 4, tileSize - 4,
                               QPen(Qt::yellow), QBrush(Qt::yellow));
            }
        }
    }

    QString bodyPath = "resources/images/minimap/body.png";
    QPixmap bodyPixmap(bodyPath);
    QPixmap scaledBody = bodyPixmap.scaled(tileSize, tileSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    // ... (Inside the drawing loop after other special tiles) ...
    for (const auto& pos : m_bodyPositions) {
        if (revealAll || m_visitedTiles.contains(pos)) {
            if (!bodyPixmap.isNull()) {
                QGraphicsPixmapItem* bodyTile = scene->addPixmap(scaledBody);
                bodyTile->setPos(pos.first * tileSize, pos.second * tileSize);
                bodyTile->setZValue(1);
            } else {
                // Fallback: Draw a red cross if the image is missing
                scene->addRect(pos.first * tileSize + 2, pos.second * tileSize + 2,
                               tileSize - 4, tileSize - 4,
                               QPen(Qt::red), QBrush(Qt::red));
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
                        fogTile->setPos(x * tileSize, y * tileSize);
                    } else {
                        // Fallback to semi-transparent black if image is missing
                        scene->addRect(x * tileSize, y * tileSize, tileSize, tileSize,
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
        scene->addEllipse(pos.first * tileSize + (tileSize / 3),
                            pos.second * tileSize + (tileSize / 3),
                            tileSize / 4, tileSize / 4,
                            Qt::NoPen,
                            
                            QBrush(QColor(255, 255, 0, opacity + 100))); // Brighter yellow trail
    }
    // 9. Draw Player Arrow (Blue)
    QGraphicsPolygonItem* playerArrow = new QGraphicsPolygonItem();
    QPolygonF arrowHead;
    // Defining an arrow shape relative to its origin
    arrowHead << QPointF(tileSize / 2, 0)               // Tip
              << QPointF(0, tileSize)                   // Bottom Left
              << QPointF(tileSize / 2, tileSize * 0.7) // Inner Notch
              << QPointF(tileSize, tileSize);          // Bottom Right
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
    playerArrow->setTransformOriginPoint(tileSize / 2, tileSize / 2);
    playerArrow->setRotation(rotation);
    playerArrow->setPos(currentPos.first * tileSize, currentPos.second * tileSize);
    scene->addItem(playerArrow);

}

// Integrated minimap: fixed 10px tiles, fog of war always on.
void DungeonDialog::drawMinimap()
{
    auto *scene = new QGraphicsScene(this);
    drawMapTo(scene, TILE_SIZE, false);
    if (m_miniMapViewIntegrated) {
        // setScene() does not delete the scene it replaces, and the old ones are
        // parented to this dialog, so drop them or every redraw leaks one.
        delete m_miniMapViewIntegrated->scene();
        m_miniMapViewIntegrated->setScene(scene);
        m_miniMapViewIntegrated->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    } else {
        delete scene;
    }
    // Keep an open AutoMap dialog in step with the same data.
    refreshAutoMap();
}

// Opens the full-size AutoMap (Ctrl+Shift+M). Created on first use and owned by
// this dialog, so it dies with it.
void DungeonDialog::showAutoMap()
{
    if (!m_autoMapDialog) {
        m_autoMapDialog = new AutoMapDialog(this);
        connect(m_autoMapDialog, &AutoMapDialog::revealAllChanged,
                this, [this]() { refreshAutoMap(); });
    }
    m_autoMapDialog->show();
    m_autoMapDialog->raise();
    refreshAutoMap();
}

// Re-renders the AutoMap if it is open. Called from drawMinimap() so the big map
// tracks the same tile data as the integrated minimap. No-op before first open.
void DungeonDialog::refreshAutoMap()
{
    if (m_autoMapDialog && m_autoMapDialog->isVisible()) {
        m_autoMapDialog->refreshFrom(this);
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
