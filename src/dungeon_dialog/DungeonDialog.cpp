
#include "src/character_dialog/CharacterDialog.h"
#include "DungeonDialog.h"
#include "DungeonHandlers.h"
#include "../../gameStateManager.h"
#include "src/items/ItemDatabase.h"
#include "src/core/DungeonThemes.h"
#include "src/core/DungeonLevelState.h"
#include "src/core/BossEncounter.h"
#include "src/core/DoorAndSearch.h"
#include "src/traps_calculations.h"
#include "src/core/SoundEffects.h"
#include "../event/EventManager.h"
#include "src/spell_casting/SpellCastingDialog.h"
#include "src/quest_board/QuestBoardDialog.h"
#include "src/automap/automap_dialog.h"
#include "src/core/Endgame.h"
#include "src/victory_dialog/VictoryDialog.h"
#include <cmath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QKeyEvent>
#include <QTimer>
#include <QtDebug>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QGraphicsPolygonItem>
#include <QScreen>
#include <QGuiApplication>
#include <QGraphicsScene>
#include <QGraphicsView>


// Constants that depend on MAP_SIZE (which is now in the header)
const int TILE_SIZE = 10;
const int MAP_WIDTH_PIXELS = MAP_SIZE * TILE_SIZE; 
const int MAP_HEIGHT_PIXELS = MAP_SIZE * TILE_SIZE; 
const int MAP_MIN = 0;
const int MAP_MAX = MAP_SIZE - 1;

QPushButton* DungeonDialog::createButton(const QString& text, const char* slot) {
    QPushButton* button = new QPushButton(text, this);
    button->setFocusPolicy(Qt::NoFocus); // Keeps focus on the main window for movement
    connect(button, SIGNAL(clicked()), this, slot);
    return button;
}

void DungeonDialog::setupControls() {
    // Define the buttons and their corresponding slots
    QMap<QString, const char*> buttonConfigs = {
        {"Fight",   SLOT(on_fightButton_clicked())},
        {"Spell",   SLOT(on_spellButton_clicked())},
        {"Rest",    SLOT(on_restButton_clicked())},
        {"Talk",    SLOT(on_talkButton_clicked())},
        {"Search",  SLOT(on_searchButton_clicked())},
        {"Disarm",  SLOT(on_disarmButton_clicked())},
        {"Pickup",  SLOT(on_pickupButton_clicked())},
        {"Drop",    SLOT(on_dropButton_clicked())},
        {"Open",    SLOT(on_openButton_clicked())},
        {"Map",     SLOT(on_mapButton_clicked())},
        {"Chest",   SLOT(on_chestButton_clicked())},
        {"Teleport",SLOT(on_teleportButton_clicked())},
        {"Exit",    SLOT(on_exitButton_clicked())},
        {"Stairs Up",    SLOT(on_stairsUpButton_clicked())},
        {"Stairs Down",    SLOT(on_stairsDownButton_clicked())},
    };


    // Loop through and create them automatically
    for (auto it = buttonConfigs.begin(); it != buttonConfigs.end(); ++it) {
        QPushButton* btn = new QPushButton(it.key(), this);
        connect(btn, SIGNAL(clicked()), this, it.value());
        m_controls.insert(it.key(), btn);
    }
}


void DungeonDialog::populateRandomTreasures(int level)
{
    gameStateManager* gsm = gameStateManager::instance();
    // MDATA3 is loaded into m_itemData in gameStateManager
    const QList<QVariantMap>& allItems = gsm->itemData(); 
    if (allItems.isEmpty()) {
        qDebug() << "MDATA3 not loaded or empty.";
        return;
    }
    QRandomGenerator* rng = QRandomGenerator::global();
    int itemsPlaced = 0;
    int chestCount = 0;
    while (itemsPlaced < 100) {
        // 1. Pick a random coordinate
        int x = rng->bounded(MAP_SIZE);
        int y = rng->bounded(MAP_SIZE);
        QPair<int, int> pos = {x, y};
        // 2. Ensure it is a floor tile (not a wall/obstacle) and not already a treasure
        if (!m_obstaclePositions.contains(pos) && !m_treasurePositions.contains(pos)) {
            // 3. Pick a random item from the MDATA3 list
            int itemIdx = rng->bounded(allItems.size());
            QString itemName = allItems.at(itemIdx).value("name").toString();
            // 4. Add to local level map (for rendering/interaction)
            m_treasurePositions.insert(pos, itemName);
            // 5. Add to the Global Array in gameStateManager
            gsm->addPlacedItem(level, x, y, itemName);
            // 6. 15% of chests are locked and need a specific key.
            if (itemName.contains("Chest") && chestCount < 15) {
                m_lockedChests.insert(pos);
                // Find a key item in the database (e.g. "Copper Key", "Iron Key").
                QString keyName;
                for (const QVariantMap& item : allItems) {
                    QString name = item.value("name").toString();
                    if (name.contains("Key")) {
                        keyName = name;
                        break;
                    }
                }
                if (!keyName.isEmpty()) {
                    m_chestKeys.insert(pos, keyName);
                    chestCount++;
                }
            }
            itemsPlaced++;
        }
    }
    qDebug() << "Successfully placed 10 random items from MDATA3 on level" << level;
}

void DungeonDialog::revealAroundPlayer(int x, int y, int z=0)
{
    Q_UNUSED(z);
    // Light radius depends on whether the torch is burning.
    int radius = torchLit() ? 2 : 1;
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dy = -radius; dy <= radius; ++dy) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < MAP_SIZE && ny >= 0 && ny < MAP_SIZE) {
                m_visitedTiles.insert({nx, ny});
            }
        }
    }
}

// --- Phase 4: Torch / Light ---

bool DungeonDialog::torchLit() const
{
    return m_torchFuel > 0;
}

int DungeonDialog::torchFuel() const
{
    return m_torchFuel;
}

void DungeonDialog::updateTorchState()
{
    if (m_torchFuel > 0) {
        m_torchFuel--;
        if (m_torchFuel == 0) {
            logMessage("<font color='orange'>Your torch sputters and goes out. You are in darkness.</font>");
        } else if (m_torchFuel == 5) {
            logMessage("<font color='orange'>Your torch is burning low.</font>");
        }
    }
    // Persist to the snapshot so fuel survives leaving and returning.
    DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();
    int level = gameStateManager::instance()->getGameValue("DungeonLevel").toInt();
    if (registry.hasLevel(level)) {
        registry.levelForEdit(level).torchTurnsRemaining = m_torchFuel;
    }
}

void DungeonDialog::restForTorch()
{
    m_torchFuel = DEFAULT_TORCH_FUEL;
    logMessage("You stop to rest and relight your torch. The flames steady.");
    DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();
    int level = gameStateManager::instance()->getGameValue("DungeonLevel").toInt();
    if (registry.hasLevel(level)) {
        registry.levelForEdit(level).torchTurnsRemaining = m_torchFuel;
    }
}

void DungeonDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    // Defer until the child views have been laid out at the new size, then
    // scale the first-person view to fill the viewport.
    QTimer::singleShot(0, this, [this]() { fitViewport(); });
}

void DungeonDialog::fitViewport()
{
    if (m_graphicsView && m_dungeonScene) {
        m_graphicsView->fitInView(m_dungeonScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void DungeonDialog::logMessage(const QString& message)
{
    if (m_messageLog) {
        // Create a standard list item
        QListWidgetItem* item = new QListWidgetItem(m_messageLog);
        
        // Create a QLabel to hold the HTML content
        QLabel* label = new QLabel(message);
        label->setStyleSheet("background: transparent;"); // Keep log background
        
        // Set the label as the widget for this item
        m_messageLog->addItem(item);
        m_messageLog->setItemWidget(item, label);
        
        // Auto-scroll to the bottom
        m_messageLog->scrollToBottom();
    }
}

void DungeonDialog::updateGoldLabel()
{
    // Party gold is the single source of truth for on-hand gold.
    int partyGold = gameStateManager::instance()->getPartyGold();
    if (m_goldLabel) {
        m_goldLabel->setText(QString("You have a total of **%1 Gold**.").arg(partyGold));
    }
}
// --- Party Management Helper Function ---
void DungeonDialog::updatePartyMemberHealth(int row, int damage)
{
    gameStateManager* gsm = gameStateManager::instance();   
    
    // Defense is handled in combat via CombatActions::defend().
    int finalDamage = damage;

    if (row == 0) {
        int currentHp = gsm->getGameValue("CurrentCharacterHP").toInt();
        int newHp = qMax(0, currentHp - finalDamage);
        gsm->setGameValue("CurrentCharacterHP", newHp);

        if (newHp <= 0) {
            logMessage("You have been defeated!");
            gsm->setGameValue("isAlive", 0);
            this->close(); 
            emit exitedDungeonToCity(); 
        }
    }
}
// --- Dungeon Management (Movement, Drawing, Encounters) ---
void DungeonDialog::movePlayer(int dx, int dy, int dz=0)
{
    Q_UNUSED(dz);
    gameStateManager* gsm = gameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    int currentZ = gsm->getGameValue("DungeonLevel").toInt();
    int newX = currentX + dx;
    int newY = currentY + dy;
    if (newX < MAP_MIN || newX > MAP_MAX || newY < MAP_MIN || newY > MAP_MAX) {
        logMessage("You hit the dungeon wall.");
        return;
    }
    QPair<int, int> newPos = {newX, newY};
    if (m_obstaclePositions.contains(newPos)) {
        logMessage("A solid rock wall blocks your path.");
        return;
    }
    // Door collision: a locked door blocks movement; an undiscovered secret
    // door is indistinguishable from a wall until found.
    auto doorIt = m_hiddenDoorPositions.find(newPos);
    if (doorIt != m_hiddenDoorPositions.end()) {
        const DoorState& door = doorIt.value();
        if (door.secret && !m_visitedTiles.contains(newPos)) {
            logMessage("A solid wall blocks your path.");
            return;
        }
        if (door.locked) {
            // Check if the party holds the key.
            bool hasKey = false;
            auto& members = gsm->getPartyMembers();
            for (const auto& member : members) {
                for (const auto& item : member.inventory) {
                    if (item.name == door.keyName) {
                        hasKey = true;
                        break;
                    }
                }
                if (hasKey) break;
            }
            if (!hasKey) {
                logMessage(QString("The door is locked. It needs the %1.").arg(door.keyName));
                return;
            }
            // Unlock and pass through.
            doorIt.value().locked = false;
            logMessage(QString("The %1 turns in the lock; the door opens.").arg(door.keyName));
        }
    }
    // Record current position as a breadcrumb before moving
    m_breadcrumbPath.append({currentX, currentY});
    if (m_breadcrumbPath.size() > MAX_BREADCRUMBS) {
        m_breadcrumbPath.removeFirst(); // Keep the trail from getting too long
    }
    gsm->setGameValue("DungeonX", newX);
    gsm->setGameValue("DungeonY", newY);

    // Hunger: each step costs 1 hunger. At 0, starvation deals 1 damage per step.
    auto& members = gsm->getPartyMembers();
    for (int i = 0; i < members.size(); ++i) {
        if (!members[i].isAlive) continue;
        members[i].consumeHunger(1);
        if (members[i].isStarving()) {
            members[i].hp = qMax(0, members[i].hp - 1);
            if (members[i].hp == 0) {
                members[i].setDead();
                logMessage(QString("<font color='red'>%1 has starved to death!</font>").arg(members[i].name));
            } else {
                logMessage(QString("<font color='orange'>%1 is starving!</font>").arg(members[i].name));
            }
        }
    }

    revealAroundPlayer(newX, newY);
    updateLocation(QString("Dungeon Level %1, (%2, %3)").arg(currentZ).arg(newX).arg(newY));
    m_visitedTiles.insert({newX, newY});
    updateMinimap(newX, newY, 0);
    DungeonHandlers::handleTreasure(this, newX, newY); // Now only logs chest presence
    DungeonHandlers::handleWater(this, newX, newY);
    DungeonHandlers::handleAntimagic(this, newX, newY);
    DungeonHandlers::handleTrap(this, newX, newY);
    DungeonHandlers::handleChute(this, newX, newY);
    DungeonHandlers::handleExtinguisher(this, newX, newY);
    DungeonHandlers::handleEncounters(this, newX, newY);
    DungeonHandlers::handlePit(this, newX, newY); // Added pit check
    updateTorchState();
    logMessage(QString("You move to (%1, %2).").arg(newX).arg(newY));

    // Describe the area when stepping into a new room or corridor.
    if (m_roomFloorTiles.contains(newPos) && !m_visitedTiles.contains(newPos)) {
        FloorTheme theme = DungeonThemes::forLevel(currentZ);
        logMessage(QString("<font color='gray'>%1</font>").arg(theme.description));
    }
    // Monsters wander after the player moves.
    moveMonsters();

    drawMinimap();
    renderWireframeView();
    updatePartyPanel();
}

int DungeonDialog::distanceBetween(QPair<int, int> a, QPair<int, int> b)
{
    int dx = a.first - b.first;
    int dy = a.second - b.second;
    return static_cast<int>(std::sqrt(dx * dx + dy * dy));
}

bool DungeonDialog::hasLineOfSight(QPair<int, int> from, QPair<int, int> to)
{
    // Bresenham line algorithm — check each tile along the line for walls.
    int x0 = from.first, y0 = from.second;
    int x1 = to.first, y1 = to.second;
    int dx = qAbs(x1 - x0), dy = qAbs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 == x1 && y0 == y1) return true;
        if (m_obstaclePositions.contains(qMakePair(x0, y0))) return false;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

QVector<QPair<int, int>> DungeonDialog::getWalkableNeighbors(QPair<int, int> pos)
{
    QVector<QPair<int, int>> neighbors;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            QPair<int, int> n = {pos.first + dx, pos.second + dy};
            if (n.first < MAP_MIN || n.first > MAP_MAX ||
                n.second < MAP_MIN || n.second > MAP_MAX) continue;
            if (m_obstaclePositions.contains(n)) continue;
            neighbors.append(n);
        }
    }
    return neighbors;
}

void DungeonDialog::moveMonsters()
{
    gameStateManager* gsm = gameStateManager::instance();
    int px = gsm->getGameValue("DungeonX").toInt();
    int py = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> playerPos = qMakePair(px, py);

    QMap<QPair<int, int>, QString> newPositions;
    for (auto it = m_monsterPositions.begin(); it != m_monsterPositions.end(); ++it) {
        QPair<int, int> oldPos = it.key();
        QString name = it.value();

        // Chase AI: if the player is within aggro range (6 tiles) and there
        // is line of sight, move greedily toward the player.
        int dist = distanceBetween(oldPos, playerPos);
        bool chasing = (dist <= 6 && hasLineOfSight(oldPos, playerPos));

        if (chasing) {
            // Greedy step: pick the walkable neighbor closest to the player.
            QVector<QPair<int, int>> neighbors = getWalkableNeighbors(oldPos);
            QPair<int, int> best = oldPos;
            int bestDist = dist;
            for (const QPair<int, int>& n : neighbors) {
                if (n == playerPos) continue;
                if (m_monsterPositions.contains(n)) continue;
                if (newPositions.contains(n)) continue;
                int d = distanceBetween(n, playerPos);
                if (d < bestDist) {
                    bestDist = d;
                    best = n;
                }
            }
            if (best != oldPos) {
                newPositions.insert(best, name);
            }
        } else {
            // Wander AI: 25% chance to shuffle one tile in a random direction.
            if (QRandomGenerator::global()->bounded(4) != 0) continue;

            int dx = QRandomGenerator::global()->bounded(-1, 2);
            int dy = QRandomGenerator::global()->bounded(-1, 2);
            if (dx == 0 && dy == 0) continue;

            QPair<int, int> newPos = {oldPos.first + dx, oldPos.second + dy};

            // Bounds + wall check.
            if (newPos.first < MAP_MIN || newPos.first > MAP_MAX ||
                newPos.second < MAP_MIN || newPos.second > MAP_MAX) continue;
            if (m_obstaclePositions.contains(newPos)) continue;

            // Don't step onto the player or another monster (including ones already moved this tick).
            if (newPos == playerPos) continue;
            if (m_monsterPositions.contains(newPos)) continue;
            if (newPositions.contains(newPos)) continue;

            // Move is valid.
            newPositions.insert(newPos, name);
        }
    }

    // Apply moves: remove old, insert new.
    for (auto it = newPositions.constBegin(); it != newPositions.constEnd(); ++it) {
        // Find the old position by name (names are unique per floor).
        for (auto old = m_monsterPositions.begin(); old != m_monsterPositions.end(); ++old) {
            if (old.value() == it.value()) {
                m_monsterPositions.erase(old);
                break;
            }
        }
        m_monsterPositions.insert(it.key(), it.value());
    }
}

void DungeonDialog::updateCompass(const QString& direction)
{
    m_compassLabel->setText(QString("Facing %1").arg(direction));
}

void DungeonDialog::updateLocation(const QString& location)
{
    m_locationLabel->setText(location);
}

void DungeonDialog::generateRandomObstacles(int roomCount, QRandomGenerator& rng)
{
    m_obstaclePositions.clear();
    // 1. Fill entire map with rock
    for (int x = 0; x < MAP_SIZE; ++x)
        for (int y = 0; y < MAP_SIZE; ++y)
            m_obstaclePositions.insert({x, y});
    struct Room { int x, y, w, h; };
    QList<Room> rooms;
    QList<Room> processingQueue;
    // 2. Start Room in a random corner
    int startX = rng.bounded(2) == 0 ? 1 : MAP_SIZE - 6;
    int startY = rng.bounded(2) == 0 ? 1 : MAP_SIZE - 6;
    Room seed = {startX, startY, rng.bounded(3, 5), rng.bounded(3, 5)};
    for (int rx = seed.x; rx < seed.x + seed.w; ++rx)
        for (int ry = seed.y; ry < seed.y + seed.h; ++ry)
            m_obstaclePositions.remove({rx, ry});
    rooms.append(seed);
    processingQueue.append(seed);
    int roomsCreated = 1;
    // 3. Sprout until we hit the target count or run out of space
    while (!processingQueue.isEmpty() && roomsCreated < roomCount) {
        // Pick a room from the queue (shuffling makes it more "web-like")
        int idx = rng.bounded(processingQueue.size());
        Room current = processingQueue.takeAt(idx);
        // Try to sprout in all 4 directions
        QList<int> directions = {0, 1, 2, 3};
        for(int i=0; i<4; ++i) { // Randomize direction order
            int swapIdx = rng.bounded(4);
            directions.swapItemsAt(i, swapIdx);
        }
        for (int dir : directions) {
            if (roomsCreated >= roomCount) break;
            int corridorLen = rng.bounded(2, 5); // Shorter corridors allow more rooms
            int newW = rng.bounded(3, 6);
            int newH = rng.bounded(3, 6);
            int cX = current.x + current.w / 2;
            int cY = current.y + current.h / 2;
            int endX = cX, endY = cY;
            int roomX = 0, roomY = 0;
            // Direction Logic
            if (dir == 0) { // North
                endY = current.y - corridorLen;
                roomX = endX - newW / 2; roomY = endY - newH;
            } else if (dir == 1) { // East
                endX = current.x + current.w + corridorLen;
                roomY = endY - newH / 2; roomX = endX;
            } else if (dir == 2) { // South
                endY = current.y + current.h + corridorLen;
                roomX = endX - newW / 2; roomY = endY;
            } else { // West
                endX = current.x - corridorLen;
                roomY = endY - newH / 2; roomX = endX - newW;
            }
            // Boundary and Overlap Check
            if (roomX > 0 && roomY > 0 && roomX + newW < MAP_MAX && roomY + newH < MAP_MAX) {
                bool overlaps = false;
                for (const auto& r : rooms) {
                    // Using 1-tile buffer to keep corridors distinct
                    if (roomX < r.x + r.w + 1 && roomX + newW > r.x - 1 && 
                        roomY < r.y + r.h + 1 && roomY + newH > r.y - 1) {
                        overlaps = true; break;
                    }
                }
                if (!overlaps) {
                    // Carve Corridor
                    int stepX = cX, stepY = cY;
                    while (stepX != endX || stepY != endY) {
                        if (stepX < endX) stepX++; else if (stepX > endX) stepX--;
                        if (stepY < endY) stepY++; else if (stepY > endY) stepY--;
                        m_obstaclePositions.remove({stepX, stepY});
                    }
                    // Carve Room
                    Room nextRoom = {roomX, roomY, newW, newH};
                    for (int rx = roomX; rx < roomX + newW; ++rx) {
                        for (int ry = roomY; ry < roomY + newH; ++ry) {
                            m_obstaclePositions.remove({rx, ry});
                            m_roomFloorTiles.insert(qMakePair(rx, ry));
                        }
                    }
                    rooms.append(nextRoom);
                    processingQueue.append(nextRoom);
                    roomsCreated++;
                }
            }
        }
    }
    // Start player in the first room created
    gameStateManager::instance()->setGameValue("DungeonX", rooms[0].x + 1);
    gameStateManager::instance()->setGameValue("DungeonY", rooms[0].y + 1);
}

void DungeonDialog::generateStairs(QRandomGenerator& rng)
{
    gameStateManager* gsm = gameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> currentPos = {currentX, currentY};
    do {
        m_stairsUpPosition = {rng.bounded(MAP_SIZE), rng.bounded(MAP_SIZE)};
        //m_stairsUpPosition = {QRandomGenerator::global()->bounded(MAP_SIZE), QRandomGenerator::global()->bounded(MAP_SIZE)};
    } while (m_stairsUpPosition == currentPos || m_obstaclePositions.contains(m_stairsUpPosition));
    do {
        m_stairsDownPosition = {rng.bounded(MAP_SIZE), rng.bounded(MAP_SIZE)};
        //m_stairsDownPosition = {QRandomGenerator::global()->bounded(MAP_SIZE), QRandomGenerator::global()->bounded(MAP_SIZE)};
    } while (m_stairsDownPosition == currentPos || m_stairsDownPosition == m_stairsUpPosition || m_obstaclePositions.contains(m_stairsDownPosition));
    m_obstaclePositions.remove(m_stairsUpPosition);
    m_obstaclePositions.remove(m_stairsDownPosition);
}

void DungeonDialog::generateSpecialTiles(int tileCount, QRandomGenerator& rng)
{
    m_bodyPositions.clear();
    // 1. Reset all containers
    m_antimagicPositions.clear();
    m_extinguisherPositions.clear();
    m_fogPositions.clear();
    m_pitPositions.clear();
    m_rotatorPositions.clear();
    m_studPositions.clear();
    m_chutePositions.clear();
    m_monsterPositions.clear();
    //m_treasurePositions.clear();
    m_trapPositions.clear();
    m_waterPositions.clear();
    m_teleporterPositions.clear();
    m_hiddenDoorPositions.clear();
    gameStateManager* gsm = gameStateManager::instance();
int currentLevel = gsm->getGameValue("DungeonLevel").toInt();
    QPair<int, int> playerPos = {gsm->getGameValue("DungeonX").toInt(), gsm->getGameValue("DungeonY").toInt()};
    // Helper A: Get a tile ONLY from a room (No corridors!)
    auto getValidRoomTile = [&]() -> QPair<int, int> {
        if (m_roomFloorTiles.isEmpty()) return {-1, -1};
        QList<QPair<int, int>> roomList = m_roomFloorTiles.values();
        for (int i = 0; i < 100; ++i) {
            QPair<int, int> p = roomList.at(rng.bounded(roomList.size()));
            if (p != m_stairsUpPosition && p != m_stairsDownPosition && p != playerPos) return p;
        }
        return {-1, -1};
    };
    // Helper: Get ANY floor tile (Rooms OR Corridors)
    auto getAnyFloorTile = [&]() -> QPair<int, int> {
        for (int i = 0; i < 500; ++i) {
            int x = rng.bounded(MAP_SIZE);
            int y = rng.bounded(MAP_SIZE);
            QPair<int, int> p = {x, y};
            if (!m_obstaclePositions.contains(p) && p != playerPos && 
                p != m_stairsUpPosition && p != m_stairsDownPosition &&
                !m_treasurePositions.contains(p)) { // Also check if treasure is already there
                return p;
            }
        }
        return {-1, -1};
    };
    // 2. Guaranteed Room-Only Spawns (Chutes & Teleporters)
    // We do these first so they get priority in the rooms
    for (int i = 0; i < 4; ++i) {
        QPair<int, int> cp = getValidRoomTile();
        if (cp.first != -1) m_chutePositions.insert(cp);
        
        QPair<int, int> tp = getValidRoomTile();
        if (tp.first != -1) m_teleporterPositions.insert(tp);
    }
    // 3. General Population Loop
    // Get thematic monsters for this floor's theme
    QStringList thematicMonsters = getThematicMonsters(currentLevel);
    for (int i = 0; i < tileCount; ++i) {
        int roll = rng.bounded(100); // Using 100 for better percentage control
        QPair<int, int> pos;
        if (roll < 15) { // 15% Monsters
            pos = getAnyFloorTile();
            if (pos.first != -1) {
                // Use thematic monster or fallback to "Orc"
                QString monsterName = thematicMonsters.isEmpty() ? "Orc" 
                    : thematicMonsters.at(rng.bounded(thematicMonsters.size()));
                m_monsterPositions.insert(pos, monsterName);
            }
        }
        else if (roll < 30) { // 15% Treasures
            pos = getAnyFloorTile();
            if (pos.first != -1) m_treasurePositions.insert(pos, "Gold Pouch");
        } 
        else if (roll < 55) { // 10% Water
            pos = getAnyFloorTile();
            if (pos.first != -1) m_waterPositions.insert(pos);
        }
        else if (roll < 65) { // 10% Anti-magic/Extinguisher
            pos = getAnyFloorTile();
            if (pos.first != -1) m_antimagicPositions.insert(pos);
        }
        // Extra Room-Only Chutes/Teleporters via random roll
        else if (roll < 70) { 
            pos = getValidRoomTile();
            if (pos.first != -1) m_chutePositions.insert(pos);
        }
        else if (roll < 75) { // 5% Pits
            pos = getAnyFloorTile();
            if (pos.first != -1) m_pitPositions.insert(pos);
        }
        else if (roll < 85) { // 10% Traps — placed with a type drawn from the floor's danger
            pos = getAnyFloorTile();
            if (pos.first != -1) {
                const char* trapTypes[] = {"Spike", "Poison Needler", "Dart", "Pit Cover", "Snare"};
                int idx = rng.bounded(5);
                m_trapPositions.insert(pos, trapTypes[idx]);
            }
        }
    }
    if (currentLevel == 1) {
        QList<QPair<int, int>> wallList = m_obstaclePositions.values();
        bool placed = false;
        
        // Shuffle wall list to get a random wall tile for the door
        std::shuffle(wallList.begin(), wallList.end(), std::default_random_engine(rng.generate()));

        for (const auto& wallPos : wallList) {
            // Check 4 cardinal directions for a floor tile
            QList<QPair<int, int>> neighbors = {
                {wallPos.first, wallPos.second - 1}, // North
                {wallPos.first, wallPos.second + 1}, // South
                {wallPos.first - 1, wallPos.second}, // West
                {wallPos.first + 1, wallPos.second}  // East
            };

            for (const auto& neighbor : neighbors) {
                // Check boundaries and ensure neighbor is NOT a wall
                if (neighbor.first >= 0 && neighbor.first < MAP_SIZE && 
                    neighbor.second >= 0 && neighbor.second < MAP_SIZE &&
                    !m_obstaclePositions.contains(neighbor)) {
                    
                    DoorState door;
                    door.position = wallPos;
                    door.secret = true;
                    door.locked = false;
                    door.difficulty = DoorAndSearch::secretDoorDifficulty(currentLevel);
                    m_hiddenDoorPositions.insert(wallPos, door);
                    qDebug() << "Accessible Hidden Door placed in wall at:" << wallPos << " next to floor at:" << neighbor;
                    placed = true;
                    break; 
                }
            }
            if (placed) break;
        }
    }
}
// --- Constructor Implementation ---
DungeonDialog::DungeonDialog(QWidget *parent)
    : QDialog(parent),
      m_dungeonScene(new QGraphicsScene(this))
{
    gameStateManager* gsm = gameStateManager::instance();
    if (!gameStateManager::instance()) {
        qCritical() << "CRITICAL: gameStateManager is NULL!";
        return; 
    }
    m_experienceLabel = new QLabel(this);
    m_standaloneMinimap = new MinimapDialog(this);
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = screenGeometry.width() - m_standaloneMinimap->width() - 20; // 20px padding
        int y = 50; // 50px padding from top
        m_standaloneMinimap->move(x, y);
    }
    connect(m_standaloneMinimap, &MinimapDialog::requestMapUpdate, this, &DungeonDialog::drawMinimap);
    setWindowTitle("Dungeon: Depth of Dejenol");
    setMinimumSize(900, 640);
    resize(1280, 800);
    // Load gameStateManager data

    // Retrieve Constitution to calculate a simple placeholder for maximum HP
    // int con = gsm->getGameValue("CurrentCharacterConstitution").toInt();
    //int maxHP = (con > 0) ? con * 5 : 50; 
    // --- Retrieve/Set persistent dungeon state (New GameState logic) ---
    int initialLevel = gsm->getGameValue("DungeonLevel").toInt();
    int initialX = gsm->getGameValue("DungeonX").toInt();
    int initialY = gsm->getGameValue("DungeonY").toInt();
    // Set defaults if state data is missing (e.g., first time entering)
    if (initialLevel == 0) {
        initialLevel = 1;
        initialX = MAP_SIZE / 2;
        initialY = MAP_SIZE / 2;
        // Initialize party gold to 1500 for a new game
        gameStateManager::instance()->addPartyGold(1500);
        // Save initial defaults to GameState
        gsm->setGameValue("DungeonLevel", initialLevel);
        gsm->setGameValue("DungeonX", initialX);
        gsm->setGameValue("DungeonY", initialY);
    }
    // -----------------------------------------------------------------
    // --- Main Layout Setup ---
    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    setupControls();

    QGridLayout *actionLayout = new QGridLayout();
        
    QStringList actions = {"Fight", "Spell", "Rest", "Talk", "Search", "Pickup", "Drop", "Open", "Map", "Chest", "Teleport", "Exit", "Stairs Up", "Stairs Down"};
    int row = 0, col = 0;
    for (const QString& name : actions) {
        if (m_controls.contains(name)) {
            actionLayout->addWidget(m_controls[name], row, col);
            if (++col > 2) { col = 0; row++; } // Grid layout: 3 buttons per row
        }
    }

    // --- Left Panel: main first-person view (grows) + adventure log ---
    QVBoxLayout *leftPanelLayout = new QVBoxLayout();
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);
    leftPanelLayout->setSpacing(6);

    // The main viewport: the first-person wireframe renderer fills this.
    m_graphicsView = new QGraphicsView(m_dungeonScene);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setFocusPolicy(Qt::NoFocus);
    m_graphicsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_graphicsView->setMinimumSize(320, 240);
    m_graphicsView->setObjectName("dungeonViewport");
    leftPanelLayout->addWidget(m_graphicsView, 4);

    // The adventure log sits under the viewport.
    QGroupBox *logBox = new QGroupBox("Adventure Log");
    QVBoxLayout *logLayout = new QVBoxLayout(logBox);
    m_messageLog = new QListWidget();
    m_messageLog->setFocusPolicy(Qt::NoFocus);
    logLayout->addWidget(m_messageLog);
    leftPanelLayout->addWidget(logBox, 1);

    rootLayout->addLayout(leftPanelLayout, 3);

    // --- Right Panel: fixed-width sidebar (info, party, minimap, controls) ---
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setLayout(rightPanelLayout);
    rightPanel->setFixedWidth(320);
    rightPanel->setObjectName("dungeonSidebar");
    rightPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    rootLayout->addWidget(rightPanel, 0);
    rightPanelLayout->setSpacing(15);
    // 1. Location and Compass
    QGroupBox *infoBox = new QGroupBox("Current Location");
    QGridLayout *infoLayout = new QGridLayout(infoBox);
    m_locationLabel = new QLabel("Dungeon Level 1, (15, 15)"); // Placeholder
    m_locationLabel->setStyleSheet("font-weight: bold;");
    infoLayout->addWidget(m_locationLabel, 0, 0, 1, 2);
    m_compassLabel = new QLabel("Facing North");
    infoLayout->addWidget(m_compassLabel, 1, 0);
    // Display Character Alignment as part of info
    QString alignment = gameStateManager::instance()->getGameValue("CurrentCharacterAlignment").toString();
    if (alignment.isEmpty()) alignment = "Neutral";
    QLabel *alignmentLabel = new QLabel(QString("Alignment: **%1**").arg(alignment));
    infoLayout->addWidget(alignmentLabel, 1, 1);
    rightPanelLayout->addWidget(infoBox);
    updateExperienceLabel();
    rightPanelLayout->addWidget(m_experienceLabel);
    // 2. Gold Display
    m_goldLabel = new QLabel();
    updateGoldLabel(); // Call to update label with GameState value
    rightPanelLayout->addWidget(m_goldLabel);

    // 3. Party Status Panel (real-time HP)
    QGroupBox* partyBox = new QGroupBox("Party Status");
    QVBoxLayout* partyBoxLayout = new QVBoxLayout(partyBox);
    m_partyStatusList = new QListWidget(partyBox);
    m_partyStatusList->setMaximumHeight(100);
    m_partyStatusList->setFocusPolicy(Qt::NoFocus);
    partyBoxLayout->addWidget(m_partyStatusList);
    rightPanelLayout->addWidget(partyBox);

    // 4. Integrated Minimap
    QGroupBox* miniMapBox = new QGroupBox("Automap");
    QVBoxLayout* miniMapBoxLayout = new QVBoxLayout(miniMapBox);
    m_miniMapViewIntegrated = new QGraphicsView(miniMapBox);
    m_miniMapViewIntegrated->setMinimumSize(150, 150);
    m_miniMapViewIntegrated->setFocusPolicy(Qt::NoFocus);
    m_miniMapViewIntegrated->setBackgroundRole(QPalette::Dark);
    miniMapBoxLayout->addWidget(m_miniMapViewIntegrated);
    rightPanelLayout->addWidget(miniMapBox);
    updatePartyPanel();

    // Initialize map state and draw the initial view
    QRandomGenerator initialRng(1 + 12345); 
    // 2. Pass 'initialRng' to the functions as the second argument
    generateRandomObstacles(40, initialRng);  
    generateStairs(initialRng);               
    generateSpecialTiles(20, initialRng);
    drawMinimap();

    // 4. Action Buttons
    //QGridLayout *actionLayout = new QGridLayout();
//    m_fightButton = new QPushButton("Fight (F)");
//    m_spellButton = new QPushButton("Spell (S)");
//    m_restButton = new QPushButton("Rest (R)");
//    m_talkButton = new QPushButton("Talk (T)");
//    m_searchButton = new QPushButton("Search (Z)");
//    m_pickupButton = new QPushButton("Pickup (P)");
//    m_dropButton = new QPushButton("Drop (D)");
//    m_openButton = new QPushButton("Open (O)");
//    m_mapButton = new QPushButton("Map (M)");
//    m_chestButton = new QPushButton("Chest (C)"); 
//    m_exitButton = new QPushButton("Exit");
//    m_teleportButton = new QPushButton("Teleport (U)");
//    actionLayout->addWidget(m_fightButton, 0, 0);
//    actionLayout->addWidget(m_spellButton, 0, 1);
//    actionLayout->addWidget(m_restButton, 0, 2);
//    actionLayout->addWidget(m_talkButton, 1, 0);
//    actionLayout->addWidget(m_searchButton, 1, 1);
//    actionLayout->addWidget(m_pickupButton, 1, 2);
//    actionLayout->addWidget(m_dropButton, 2, 0);
//    actionLayout->addWidget(m_openButton, 2, 1);
//    actionLayout->addWidget(m_mapButton, 2, 2);
//    actionLayout->addWidget(m_chestButton, 3, 0);
//    actionLayout->addWidget(m_teleportButton, 3, 1);
//    actionLayout->addWidget(m_exitButton, 3, 2);

    rightPanelLayout->addLayout(actionLayout);
    // 5. Directional Buttons (Movement)
    QGroupBox *moveBox = new QGroupBox("Movement");
    QGridLayout *moveLayout = new QGridLayout(moveBox);
    m_upButton = new QPushButton("^");
    m_downButton = new QPushButton("v");
    m_leftButton = new QPushButton("<");
    m_rightButton = new QPushButton(">");
    // Rotate
    m_rotateLeftButton = new QPushButton("Rotate L");
    m_rotateRightButton = new QPushButton("Rotate R");
    // Auto-backtrack
    m_backtrackButton = new QPushButton("Back");
    m_backtrackButton->setToolTip("Retrace your steps");
    moveLayout->addWidget(m_rotateLeftButton, 0, 0);
    moveLayout->addWidget(m_upButton, 0, 1);
    moveLayout->addWidget(m_rotateRightButton, 0, 2);
    moveLayout->addWidget(m_leftButton, 1, 0);
    moveLayout->addWidget(m_downButton, 1, 1);
    moveLayout->addWidget(m_rightButton, 1, 2);
    moveLayout->addWidget(m_backtrackButton, 2, 0, 1, 3);
    rightPanelLayout->addWidget(moveBox);
    // 6. Stairs Buttons
    //QHBoxLayout *stairsLayout = new QHBoxLayout();
    //QPushButton *stairsUpButton = new QPushButton("Stairs Up");
    //QPushButton *stairsDownButton = new QPushButton("Stairs Down");
    //stairsLayout->addWidget(stairsUpButton);
    //stairsLayout->addWidget(stairsDownButton);
    //rightPanelLayout->addLayout(stairsLayout);
    // 6. Stairs Buttons
    //QHBoxLayout *stairsLayout = new QHBoxLayout();
    // Connections (Movements)
    connect(m_upButton, &QPushButton::clicked, this, &DungeonDialog::moveForward);
    connect(m_downButton, &QPushButton::clicked, this, &DungeonDialog::moveBackward);
    connect(m_leftButton, &QPushButton::clicked, this, &DungeonDialog::moveStepLeft);
    connect(m_rightButton, &QPushButton::clicked, this, &DungeonDialog::moveStepRight);
    connect(m_rotateLeftButton, &QPushButton::clicked, this, &DungeonDialog::on_rotateLeftButton_clicked);
    connect(m_rotateRightButton, &QPushButton::clicked, this, &DungeonDialog::on_rotateRightButton_clicked);
    connect(m_backtrackButton, &QPushButton::clicked, this, &DungeonDialog::on_backtrackButton_clicked);
    // Connections (Actions)
//    connect(m_fightButton, &QPushButton::clicked, this, &DungeonDialog::on_fightButton_clicked);
//    connect(m_spellButton, &QPushButton::clicked, this, &DungeonDialog::on_spellButton_clicked);
//    connect(m_restButton, &QPushButton::clicked, this, &DungeonDialog::on_restButton_clicked);
//    connect(m_talkButton, &QPushButton::clicked, this, &DungeonDialog::on_talkButton_clicked);
//    connect(m_searchButton, &QPushButton::clicked, this, &DungeonDialog::on_searchButton_clicked);
//    connect(m_pickupButton, &QPushButton::clicked, this, &DungeonDialog::on_pickupButton_clicked);
//    connect(m_dropButton, &QPushButton::clicked, this, &DungeonDialog::on_dropButton_clicked);
//    connect(m_openButton, &QPushButton::clicked, this, &DungeonDialog::on_openButton_clicked);
//    connect(m_mapButton, &QPushButton::clicked, this, &DungeonDialog::on_mapButton_clicked);
//    connect(m_chestButton, &QPushButton::clicked, this, &DungeonDialog::on_chestButton_clicked);
//    connect(m_exitButton, &QPushButton::clicked, this, &DungeonDialog::on_exitButton_clicked);
//    connect(m_teleportButton, &QPushButton::clicked, this, &DungeonDialog::on_teleportButton_clicked);
    // Connections (Stairs)
//    connect(stairsDownButton, &QPushButton::clicked, this, &DungeonDialog::on_stairsDownButton_clicked);
//    connect(stairsUpButton, &QPushButton::clicked, this, &DungeonDialog::on_stairsUpButton_clicked);
    // --- Finalize the viewport ---
    // The wireframe renderer draws into a logical 300x300 coordinate space.
    // Pin the scene rect so the view can scale it up to fill the window.
    m_dungeonScene->setSceneRect(0, 0, 300, 300);
    enterLevel(initialLevel); // Use initialLevel retrieved from GameState
    drawMinimap();
    renderWireframeView();
    QTimer::singleShot(0, this, [this]() { fitViewport(); });
    // Keep focus on the dialog so movement keys work.
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();

    // --- Combat System Setup ---
    m_combatState = new CombatState();
    m_turnEngine = new TurnEngine();
    m_combatActions = new CombatActions(m_combatState, m_turnEngine);
    m_monsterAI = new MonsterAI(m_combatState, m_turnEngine, m_combatActions);
    m_deathHandler = new CombatDeathHandler(m_combatState, m_turnEngine, m_combatActions);
    m_turnEngine->setCombatState(m_combatState);

    // --- Combat UI ---
    m_combatGroup = new QGroupBox("Combat", this);
    QVBoxLayout* combatLayout = new QVBoxLayout(m_combatGroup);

    m_combatMonsterHpLabel = new QLabel("Monster: -", m_combatGroup);
    m_combatMonsterHpLabel->setStyleSheet("font-weight: bold; color: red;");
    combatLayout->addWidget(m_combatMonsterHpLabel);

    m_combatPartyHpLabel = new QLabel("Party: -", m_combatGroup);
    m_combatPartyHpLabel->setStyleSheet("font-weight: bold; color: green;");
    combatLayout->addWidget(m_combatPartyHpLabel);

    QHBoxLayout* combatBtnLayout = new QHBoxLayout();
    m_combatAttackBtn = new QPushButton("Attack", m_combatGroup);
    m_combatDefendBtn = new QPushButton("Defend", m_combatGroup);
    m_combatFleeBtn = new QPushButton("Flee", m_combatGroup);
    m_combatUseItemBtn = new QPushButton("Item", m_combatGroup);
    m_combatSpellBtn = new QPushButton("Spell", m_combatGroup);

    m_combatAttackBtn->setFocusPolicy(Qt::NoFocus);
    m_combatDefendBtn->setFocusPolicy(Qt::NoFocus);
    m_combatFleeBtn->setFocusPolicy(Qt::NoFocus);
    m_combatUseItemBtn->setFocusPolicy(Qt::NoFocus);
    m_combatSpellBtn->setFocusPolicy(Qt::NoFocus);

    combatBtnLayout->addWidget(m_combatAttackBtn);
    combatBtnLayout->addWidget(m_combatDefendBtn);
    combatBtnLayout->addWidget(m_combatFleeBtn);
    combatBtnLayout->addWidget(m_combatUseItemBtn);
    combatBtnLayout->addWidget(m_combatSpellBtn);
    combatLayout->addLayout(combatBtnLayout);

    m_combatGroup->setVisible(false);
    rightPanelLayout->addWidget(m_combatGroup);

    // Combat button connections
    connect(m_combatAttackBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatAttackButton_clicked);
    connect(m_combatDefendBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatDefendButton_clicked);
    connect(m_combatFleeBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatFleeButton_clicked);
    connect(m_combatUseItemBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatUseItemButton_clicked);
    connect(m_combatSpellBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatSpellButton_clicked);
}

void DungeonDialog::updateExperienceLabel()
{
    // Retrieve Experience from gameStateManager
    // Assuming the key is "PlayerExperience"
    quint64 currentXP = gameStateManager::instance()->getGameValue("PlayerExperience").toULongLong();
    if (m_experienceLabel) {
        QString xpString = QStringLiteral("%L1").arg(currentXP);
        m_experienceLabel->setText(QString("Experience: **%1 XP**").arg(xpString));
    }
}

void DungeonDialog::enterLevel(int level, bool movingUp)
{
    m_breadcrumbPath.clear();
    m_visitedTiles.clear();
    // Clear treasures specifically at the start of level generation
    m_treasurePositions.clear();
    m_lockedChests.clear();
    m_chestKeys.clear();
    gameStateManager* gsm = gameStateManager::instance();

    // Theme drives the floor's population and difficulty.
    FloorTheme theme = DungeonThemes::forLevel(level);
    DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();

    // Restoring a floor the party has already visited: keep it as they left it.
    if (registry.hasLevel(level)) {
        const LevelSnapshot* snap = registry.level(level);
        m_monsterPositions = snap->monsterPositions;
        m_treasurePositions = snap->treasurePositions;
        m_visitedTiles = snap->visitedTiles;
        m_stairsUpPosition = snap->stairsUp;
        m_stairsDownPosition = snap->stairsDown;
        m_trapPositions = snap->trapPositions;
        m_triggeredTraps = snap->triggeredTraps;
        m_openedChests = snap->openedChests;
        m_roomFloorTiles.clear();

        // Some of the cleared monsters have moved back in.
        QRandomGenerator respawnRng(static_cast<quint32>(level * 7919 + 13));
        registry.respawnMonsters(level, 0.3, theme.monsterCount, respawnRng,
                                 getThematicMonsters(level));
        m_monsterPositions = registry.level(level)->monsterPositions;

        QPair<int, int> landing = movingUp ? m_stairsDownPosition : m_stairsUpPosition;
        gsm->setGameValue("DungeonLevel", level);
        gsm->setGameValue("DungeonX", landing.first);
        gsm->setGameValue("DungeonY", landing.second);
        revealAroundPlayer(landing.first, landing.second);
        m_visitedTiles.insert(landing);
        updateLocation(QString("Dungeon Level %1 — %2, (%3, %4)")
                       .arg(level).arg(theme.name).arg(landing.first).arg(landing.second));
        drawMinimap();
        logMessage(QString("You return to **%1** (Level %2).").arg(theme.name).arg(level));
        // Restore torch fuel from the snapshot.
        m_torchFuel = snap->torchTurnsRemaining;
        return;
    }

    // 1. Generate the map using Room-and-Corridor logic
    // Seed by level to ensure the layout is deterministic
    QRandomGenerator levelRng(level + 12345);
    populateRandomTreasures(theme.treasureCount);
    // We pass a 'Room Count' instead of 'Obstacle Count'.
    generateRandomObstacles(40, levelRng);
    // 2. Place Stairs and Special Tiles
    // These must be called AFTER generateRandomObstacles so they know where the floor is.
    generateStairs(levelRng);
    // Scale the number of special tiles with the theme.
    generateSpecialTiles(theme.monsterCount + theme.trapCount, levelRng);

    // 3. Boss floors get their guardian placed on a room tile.
    if (theme.isBossFloor && !theme.bossName.isEmpty()) {
        QList<QPair<int, int>> roomTiles = m_roomFloorTiles.values();
        if (!roomTiles.isEmpty()) {
            QPair<int, int> bossPos = roomTiles.at(levelRng.bounded(roomTiles.size()));
            if (bossPos != m_stairsUpPosition && bossPos != m_stairsDownPosition) {
                m_monsterPositions.insert(bossPos, theme.bossName);
                logMessage(QString("<font color='red'>A dreadful presence stirs: %1!</font>")
                           .arg(theme.bossName));
            }
        }
    }

    // 4. Determine Landing Position
    // Arrive at the Down stairs if moving Up, or Up stairs if moving Down
    QPair<int, int> landingPos = movingUp ? m_stairsDownPosition : m_stairsUpPosition;

    // 5. Persist this floor so it survives leaving and returning.
    LevelSnapshot snap;
    snap.level = level;
    snap.generated = true;
    snap.monsterPositions = m_monsterPositions;
    snap.treasurePositions = m_treasurePositions;
    snap.visitedTiles = m_visitedTiles;
    snap.stairsUp = m_stairsUpPosition;
    snap.stairsDown = m_stairsDownPosition;
    snap.trapPositions = m_trapPositions;
    snap.triggeredTraps = m_triggeredTraps;
    snap.openedChests = m_openedChests;
    registry.store(snap);

    // 6. Update Game State and UI
    gsm->setGameValue("DungeonLevel", level);
    gsm->setGameValue("DungeonX", landingPos.first);
    gsm->setGameValue("DungeonY", landingPos.second);
    revealAroundPlayer(landingPos.first, landingPos.second);
    m_visitedTiles.insert(landingPos);
    updateLocation(QString("Dungeon Level %1 — %2, (%3, %4)")
                   .arg(level).arg(theme.name).arg(landingPos.first).arg(landingPos.second));
    drawMinimap();
    logMessage(QString("You have entered **%1** (Dungeon Level %2).").arg(theme.name).arg(level));
    if (!theme.description.isEmpty()) {
        logMessage(QString("<font color='gray'>%1</font>").arg(theme.description));
    }
    // Light a fresh torch on a new floor.
    m_torchFuel = DEFAULT_TORCH_FUEL;
    DungeonLevelRegistry::instance().levelForEdit(level).torchTurnsRemaining = m_torchFuel;
}



void DungeonDialog::on_mapButton_clicked()
{
    logMessage("You start looking at your map.");
    if (m_standaloneMinimap->isVisible()) {
        m_standaloneMinimap->hide();
    } else {
        m_standaloneMinimap->show();
        this->activateWindow();
        drawMinimap();
    }
    openAutomap();
}

void DungeonDialog::openAutomap()
{
    gameStateManager* gsm = gameStateManager::instance();
    int x = gsm->getGameValue("DungeonX").toInt();
    int y = gsm->getGameValue("DungeonY").toInt();
    int z = gsm->getGameValue("DungeonLevel").toInt();
    QString facing = m_compassLabel->text().replace("Facing ", "").toUpper();

    AutomapDialog* dlg = new AutomapDialog(this);
    dlg->updatePlayerPosition(x, y, facing);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void DungeonDialog::on_pickupButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> pos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };
    if (m_treasurePositions.contains(pos)) {
        processTreasureOpening();
    } else {
        logMessage("There is nothing here to pick up.");
    }
}

void DungeonDialog::on_dropButton_clicked() 
{
    gameStateManager* gsm = gameStateManager::instance();

    int x = gsm->getGameValue("DungeonX").toInt();
    int y = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> pos = {x, y};
    m_bodyPositions.insert(pos);
    
    // Check if the player is actually carrying a body
    // Assuming "IsCarryingBody" is a flag in your gameStateManager
    if (gsm->getGameValue("IsCarryingBody").toBool()) {
        
        // Get current position
        int curX = gsm->getGameValue("DungeonX").toInt();
        int curY = gsm->getGameValue("DungeonY").toInt();
        QPair<int, int> pos = {curX, curY};

        // Logic to check if the player is actually carrying a body would go here
        // For now, we add the tile at the current location
        m_bodyPositions.insert(pos);
        // Update GameState: No longer carrying, and place body on map
        gsm->setGameValue("IsCarryingBody", false);
        
        // You might want to add this to a specific set like m_monsterPositions 
        // or a new m_deadBodyPositions if you want it to persist/be seen.
        // For now, we log the action:
        logMessage("<font color='gray'>You carefully lay the carried body onto the cold stone floor.</font>");
        
        // Refresh view to show the dropped object if applicable
        renderWireframeView();
    } else {
        logMessage("You aren't carrying anything to drop.");
    }
    drawMinimap();
}

void DungeonDialog::on_teleportButton_clicked() 
{ 
    gameStateManager* gsm = gameStateManager::instance();
    int newX, newY;
    QPair<int, int> newPos;
    // 1. Find a random valid location (not a wall)
    do {
        newX = QRandomGenerator::global()->bounded(MAP_SIZE);
        newY = QRandomGenerator::global()->bounded(MAP_SIZE);
        newPos = {newX, newY};
    } while (m_obstaclePositions.contains(newPos));
    // 2. Update the Game State
    gsm->setGameValue("DungeonX", newX);
    gsm->setGameValue("DungeonY", newY);
    // 3. Log the event to the user
    logMessage(QString("A mystical force teleports you to (%1, %2)!")
               .arg(newX).arg(newY));
    // 4. Update the UI
    updateLocation(QString("Dungeon Level %1, (%2, %3)")
                   .arg(gsm->getGameValue("DungeonLevel").toInt()).arg(newX).arg(newY));
    // Refresh the map and trigger encounter checks at the new location
    drawMinimap();
    DungeonHandlers::handleTreasure(this, newX, newY);
    DungeonHandlers::handleTrap(this, newX, newY);
    DungeonHandlers::handleEncounters(this, newX, newY);
    // Emit the signal defined in the header
    emit teleporterUsed();
}

void DungeonDialog::on_fightButton_clicked()
{
    // Start combat with the monster at the current position
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> pos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };

    if (m_monsterPositions.contains(pos)) {
        QString monsterName = m_monsterPositions.value(pos);
        int level = gsm->getGameValue("DungeonLevel").toInt();
        bool isBoss = BossEncounter::hasBoss(level) && monsterName == BossEncounter::bossName(level);

        m_combatMonsterName = monsterName;
        m_combatMonsterLevel = level;
        m_combatIsBoss = isBoss;
        m_inCombat = true;

        // Build combat encounter
        m_combatState->clear();

        // Add party members
        auto& members = gsm->getPartyMembers();
        for (int i = 0; i < members.size(); ++i) {
            const Character& c = members[i];
            if (!c.isAlive || c.hp <= 0) continue;

            CombatParticipant p;
            p.name = c.name;
            p.isPlayer = true;
            p.isAlive = true;
            p.hp = c.hp;
            p.maxHp = c.maxHp;
            p.level = c.level;
            p.att = 5 + c.level + (c.effectiveStrength() - 10) / 2;
            p.def = 10 + (c.effectiveDexterity() - 10) / 2;
            p.speed = c.effectiveDexterity();
            p.dex = c.effectiveDexterity();
            p.mana = c.mana;
            p.maxMana = c.maxMana;

            // Add weapon bonus
            for (const HeldItem& item : c.equipped) {
                if (const ItemDef* def = ItemDatabase::instance().byName(item.name)) {
                    if (def->slot() == ItemSlot::MainHand || def->slot() == ItemSlot::OffHand) {
                        p.att += def->att;
                    }
                    if (def->slot() == ItemSlot::Body || def->slot() == ItemSlot::Head ||
                        def->slot() == ItemSlot::OffHand) {
                        p.def += def->def;
                    }
                }
            }

            m_combatState->addParticipant(p);
        }

        // Add monsters
        if (isBoss) {
            QVariantMap bossData = BossEncounter::buildBoss(level);
            CombatParticipant boss;
            boss.name = monsterName;
            boss.isPlayer = false;
            boss.isAlive = true;
            boss.hp = bossData["hp"].toInt();
            boss.maxHp = boss.hp;
            boss.att = bossData["att"].toInt();
            boss.def = bossData["def"].toInt();
            boss.speed = bossData["speed"].toInt();
            boss.dex = bossData["dex"].toInt();
            boss.level = bossData["level"].toInt();
            boss.damageMod = bossData["damageMod"].toInt();
            boss.swings = bossData["swings"].toInt();
            m_combatState->addParticipant(boss);
        } else {
            // Monster stats come from the game's own MDATA5 table (loaded by
            // gameStateManager): hits = HP, att/def, numGroups x ingroup = the
            // encounter size.
            QList<CombatParticipant> monsters = EncounterBuilder::buildEncounter(
                monsterName, gsm->getMonsterData(),
                gsm->getNgPlusLevel(),
                gsm->getGameValue("DungeonLevel").toInt());
            for (const auto& m : monsters) {
                m_combatState->addParticipant(m);
            }
        }

        // Start combat
        m_turnEngine->startRound();
        m_combatGroup->setVisible(true);
        updateCombatUI();

        logMessage(QString("<font color='red'>⚔️ Combat begins with %1!</font>").arg(monsterName));

        // If monster goes first, take its turn
        if (m_turnEngine->hasCurrentParticipant() && !m_combatActions->isPlayerTurn()) {
            QString aiResult = m_monsterAI->takeTurn();
            logMessage(aiResult);
            advanceCombat();
        }
    } else {
        logMessage("There is nothing to fight here.");
    }
}

// --- Combat Action Slots ---
void DungeonDialog::on_combatAttackButton_clicked()
{
    if (!m_inCombat || !m_combatActions || !m_turnEngine->hasCurrentParticipant()) return;
    if (!m_combatActions->isPlayerTurn()) return;

    // Find first living monster
    int targetIdx = -1;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        if (!m_combatState->participant(i).isPlayer && m_combatState->participant(i).isAlive) {
            targetIdx = i;
            break;
        }
    }
    if (targetIdx < 0) return;

    QString result;
    m_combatActions->attack(targetIdx, result);
    logMessage(result);

    // Process deaths
    QStringList deaths = m_deathHandler->processDeaths();
    for (const QString& d : deaths) logMessage(d);

    // Update game state HP
    syncCombatToGameState();

    m_turnEngine->markCurrentActed();
    updateCombatUI();
    advanceCombat();
}

void DungeonDialog::on_combatDefendButton_clicked()
{
    if (!m_inCombat || !m_combatActions || !m_turnEngine->hasCurrentParticipant()) return;
    if (!m_combatActions->isPlayerTurn()) return;

    QString result;
    m_combatActions->defend(result);
    logMessage(result);

    m_turnEngine->markCurrentActed();
    updateCombatUI();
    advanceCombat();
}

void DungeonDialog::on_combatFleeButton_clicked()
{
    if (!m_inCombat || !m_combatActions) return;

    QString result;
    bool fled = m_combatActions->flee(result);
    logMessage(result);

    if (fled) {
        m_inCombat = false;
        m_combatGroup->setVisible(false);
        logMessage("You flee from combat!");
    } else {
        // Failed to flee — monster gets a free attack
        m_turnEngine->markCurrentActed();
        advanceCombat();
    }
    updateCombatUI();
}

void DungeonDialog::on_combatUseItemButton_clicked()
{
    if (!m_inCombat || !m_combatActions || !m_turnEngine->hasCurrentParticipant()) return;
    if (!m_combatActions->isPlayerTurn()) return;

    // Use a healing potion if available
    gameStateManager* gsm = gameStateManager::instance();
    Character& c = gsm->getPartyMember(gsm->getCurrentCharacterIndex());
    for (int i = 0; i < c.inventory.size(); ++i) {
        const HeldItem& item = c.inventory[i];
        if (item.name.toLower().contains("healing") || item.name.toLower().contains("health")) {
            QString result;
            m_combatActions->useItem(i, c, result);
            logMessage(result);
            m_turnEngine->markCurrentActed();
            updateCombatUI();
            advanceCombat();
            return;
        }
    }
    logMessage("No usable items in inventory.");
}

void DungeonDialog::on_combatSpellButton_clicked()
{
    if (!m_inCombat || !m_combatActions || !m_turnEngine->hasCurrentParticipant()) return;
    if (!m_combatActions->isPlayerTurn()) return;

    // Open spell casting dialog
    on_spellButton_clicked();
}

void DungeonDialog::updateCombatUI()
{
    if (!m_combatState) return;

    // Monster HP
    QStringList monsterHp;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        const auto& p = m_combatState->participant(i);
        if (!p.isPlayer && p.isAlive) {
            monsterHp << QString("%1: %2/%3").arg(p.name).arg(p.hp).arg(p.maxHp);
        }
    }
    m_combatMonsterHpLabel->setText("Monster: " + (monsterHp.isEmpty() ? "None" : monsterHp.join(", ")));

    // Party HP
    QStringList hpList;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        const auto& p = m_combatState->participant(i);
        if (p.isPlayer) {
            hpList << QString("%1: %2/%3").arg(p.name).arg(p.hp).arg(p.maxHp);
        }
    }
    m_combatPartyHpLabel->setText("Party: " + hpList.join(" | "));
}

void DungeonDialog::advanceCombat()
{
    if (!m_inCombat || !m_turnEngine || !m_combatState) return;

    // Check combat end
    if (m_combatState->isCombatOver()) {
        if (m_deathHandler->isVictory()) {
            handleVictory();
        } else if (m_deathHandler->isPartyWipe()) {
            handlePartyWipe();
        }
        return;
    }

    // Advance to next turn
    if (!m_turnEngine->nextTurn()) {
        // Round over — start new round
        m_turnEngine->startRound();
        // Tick status effects
        QStringList statusMsgs = m_combatActions->tickStatusEffects();
        for (const QString& msg : statusMsgs) logMessage(msg);
    }

    // If monster's turn, take AI action
    if (m_turnEngine->hasCurrentParticipant() && !m_combatActions->isPlayerTurn()) {
        QString aiResult = m_monsterAI->takeTurn();
        logMessage(aiResult);
        QStringList deaths = m_deathHandler->processDeaths();
        for (const QString& d : deaths) logMessage(d);
        syncCombatToGameState();
        updateCombatUI();
        advanceCombat();  // Recurse until player turn or combat ends
    }
}

void DungeonDialog::handleVictory()
{
    gameStateManager* gsm = gameStateManager::instance();
    int level = gsm->getGameValue("DungeonLevel").toInt();

    // Calculate rewards from the same monster table the encounter came from.
    const QList<QVariantMap>& monsters = gsm->getMonsterData();
    int xp = VictoryReward::calculateXp(m_combatMonsterName, monsters);
    int gold = VictoryReward::calculateGold(m_combatMonsterName, monsters);
    QStringList loot = VictoryReward::calculateLoot(m_combatMonsterName, monsters, level);

    // Boss bonus
    if (m_combatIsBoss) {
        xp = BossEncounter::bossXp(level);
    }

    // Award rewards
    gsm->addExperienceToParty(xp);
    gsm->addPartyGold(gold);

    // Report kill to the quest board so kill-quests progress.
    QuestBoardDialog::reportKill(m_combatMonsterName, level);

    logMessage(QString("<font color='gold'>🏆 Victory! Gained %1 XP and %2 gold.</font>").arg(xp).arg(gold));
    for (const QString& item : loot) {
        logMessage(QString("<font color='gold'>💰 Loot: %1</font>").arg(item));
    }

    // Remove monster from map
    QPair<int, int> pos = { gsm->getGameValue("DungeonX").toInt(),
                            gsm->getGameValue("DungeonY").toInt() };
    m_monsterPositions.remove(pos);

    // Boss defeated?
    if (m_combatIsBoss) {
        DungeonLevelRegistry::instance().levelForEdit(level).bossDefeated = true;
    }

    m_inCombat = false;
    m_combatGroup->setVisible(false);

    // Check for final victory: the Prince of Devils is the floor-15 boss.
    // When he falls, the game is won.
    if (m_combatIsBoss && level == 15) {
        bool won = true;  // isVictory() checks for floor-15 boss defeat
        if (won) {
            QString title = Endgame::victoryTitle();
            QStringList paragraphs = Endgame::victoryParagraphs();
            VictoryDialog* dlg = new VictoryDialog(title, paragraphs, true, this);
            dlg->setAttribute(Qt::WA_DeleteOnClose);
            connect(dlg, &VictoryDialog::startNewGamePlus, gsm, [gsm]() {
                gsm->setNgPlusLevel(gsm->getNgPlusLevel() + 1);
            });
            // Record the run for the Hall of Records.
            GameRecord rec;
            rec.heroName = gsm->getPartyMembers().isEmpty() ? "Unknown" : gsm->getPartyMembers()[0].name;
            rec.highestLevel = gsm->getPartyMembers().isEmpty() ? 1 : gsm->getPartyMembers()[0].level;
            rec.mostGold = gsm->getPartyGold();
            rec.deepestFloor = level;
            rec.completionTimeSeconds = 0;  // TODO: track actual play time
            rec.won = true;
            gsm->addGameRecord(rec);
            dlg->exec();
        }
    }

    drawMinimap();
    renderWireframeView();
}

void DungeonDialog::handlePartyWipe()
{
    gameStateManager* gsm = gameStateManager::instance();
    logMessage("<font color='red'>💀 Your party has been defeated!</font>");
    gsm->setGameValue("isAlive", 0);
    m_inCombat = false;
    m_combatGroup->setVisible(false);
    this->close();
    emit exitedDungeonToCity();
}

void DungeonDialog::syncCombatToGameState()
{
    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();
    int memberIdx = 0;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        const auto& p = m_combatState->participant(i);
        if (p.isPlayer && memberIdx < members.size()) {
            members[memberIdx].hp = p.hp;
            members[memberIdx].isAlive = p.isAlive;
            memberIdx++;
        }
    }
    updatePartyPanel();
}

void DungeonDialog::updatePartyPanel()
{
    if (!m_partyStatusList) return;
    m_partyStatusList->clear();

    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();
    for (const Character& c : members) {
        QString status;
        if (!c.isAlive || c.hp <= 0) {
            status = "DEAD";
        } else {
            int pct = (c.maxHp > 0) ? (c.hp * 100 / c.maxHp) : 0;
            status = QString("%1/%2 HP (%3%)").arg(c.hp).arg(c.maxHp).arg(pct);
        }
        QListWidgetItem* item = new QListWidgetItem(
            QString("%1 (Lv %2) — %3").arg(c.name).arg(c.level).arg(status));
        if (!c.isAlive || c.hp <= 0) {
            item->setForeground(Qt::red);
        } else if (c.hp * 100 / qMax(1, c.maxHp) < 30) {
            item->setForeground(Qt::yellow);
        } else {
            item->setForeground(Qt::green);
        }
        m_partyStatusList->addItem(item);
    }
}

void DungeonDialog::on_backtrackButton_clicked()
{
    if (m_breadcrumbPath.isEmpty()) {
        logMessage("You have no trail to follow back.");
        return;
    }

    // Pop the last breadcrumb and move there
    QPair<int, int> prev = m_breadcrumbPath.takeLast();
    gameStateManager* gsm = gameStateManager::instance();

    // Move directly to the previous position (bypassing direction logic)
    gsm->setGameValue("DungeonX", prev.first);
    gsm->setGameValue("DungeonY", prev.second);
    m_visitedTiles.insert(prev);
    revealAroundPlayer(prev.first, prev.second);
    updateLocation(QString("Dungeon Level %1, (%2, %3)")
                   .arg(gsm->getGameValue("DungeonLevel").toInt())
                   .arg(prev.first).arg(prev.second));
    logMessage(QString("You retrace your steps to (%1, %2).").arg(prev.first).arg(prev.second));

    // Trigger handlers at the new position
    DungeonHandlers::handleTreasure(this, prev.first, prev.second);
    DungeonHandlers::handleTrap(this, prev.first, prev.second);
    DungeonHandlers::handleEncounters(this, prev.first, prev.second);

    drawMinimap();
    renderWireframeView();
    updatePartyPanel();
}

void DungeonDialog::moveDiagonalForwardLeft()
{
    // Move forward then sidestep left
    QString facing = m_compassLabel->text();
    int dx = 0, dy = 0;
    if (facing == "Facing North") { dx = -1; dy = -1; }
    else if (facing == "Facing South") { dx = 1; dy = 1; }
    else if (facing == "Facing East") { dx = 1; dy = -1; }
    else if (facing == "Facing West") { dx = -1; dy = 1; }
    movePlayer(dx, dy);
}

void DungeonDialog::moveDiagonalForwardRight()
{
    QString facing = m_compassLabel->text();
    int dx = 0, dy = 0;
    if (facing == "Facing North") { dx = 1; dy = -1; }
    else if (facing == "Facing South") { dx = -1; dy = 1; }
    else if (facing == "Facing East") { dx = 1; dy = 1; }
    else if (facing == "Facing West") { dx = -1; dy = -1; }
    movePlayer(dx, dy);
}

// DungeonDialog.cpp
QPair<int, int> DungeonDialog::getCurrentPosition()
{
    gameStateManager* gsm = gameStateManager::instance();
    return qMakePair(gsm->getGameValue("DungeonX").toInt(), 
                     gsm->getGameValue("DungeonY").toInt());
}

void DungeonDialog::on_takeButton_clicked() 
{ 
    logMessage("You try to take something, but there is nothing there."); 
}

void DungeonDialog::on_searchButton_clicked() 
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> currentPos = getCurrentPosition();
    int floorLevel = gsm->getGameValue("DungeonLevel").toInt();
    if (floorLevel < 1) floorLevel = 1;

    int wisdom = gsm->getGameValue("CurrentCharacterWisdom").toInt();
    int intelligence = gsm->getGameValue("CurrentCharacterIntelligence").toInt();

    // Hidden doors on this floor, as searchable DoorStates.
    QMap<QPair<int, int>, DoorState> doors;
    for (auto it = m_hiddenDoorPositions.constBegin(); it != m_hiddenDoorPositions.constEnd(); ++it) {
        DoorState d = it.value();
        d.position = it.key();
        d.secret = true;
        d.difficulty = DoorAndSearch::secretDoorDifficulty(floorLevel);
        doors.insert(it.key(), d);
    }

    QList<QPair<int, int>> found;
    QRandomGenerator rng(QRandomGenerator::global()->generate());
    int count = DoorAndSearch::searchForSecretDoors(doors, currentPos.first, currentPos.second,
                                                    wisdom, intelligence, found, rng);

    if (count > 0) {
        for (const QPair<int, int>& pos : found) {
            logMessage(QString("Your search reveals a hidden door at %1, %2!")
                       .arg(pos.first).arg(pos.second));
            SoundEffects::instance()->play(SoundEffects::Type::SecretDoor);
            // Add to visited tiles so it stays on the map.
            m_visitedTiles.insert(pos);
        }
        drawMinimap(); // Redraw map to show the discovered door
    }

    // Also search for traps in the same 3x3 area.
    QList<QPair<int, int>> foundTraps;
    int trapCount = DoorAndSearch::searchForTraps(m_trapPositions, currentPos.first, currentPos.second,
                                                   wisdom, intelligence, floorLevel, foundTraps, rng);
    if (trapCount > 0) {
        for (const QPair<int, int>& pos : foundTraps) {
            logMessage(QString("You notice a trap at %1, %2!")
                       .arg(pos.first).arg(pos.second));
            SoundEffects::instance()->play(SoundEffects::Type::SecretDoor);
            m_visitedTiles.insert(pos);
        }
        drawMinimap();
    } else if (count == 0) {
        logMessage("You search the area but find nothing hidden.");
    }
}
void DungeonDialog::on_talkButton_clicked()
{
    logMessage("You try talking, but the silence replies.");
}

void DungeonDialog::on_disarmButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> pos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };

    // Check if there is a trap at the player's position.
    if (!m_trapPositions.contains(pos)) {
        logMessage("There is no trap here to disarm.");
        return;
    }

    // Already triggered traps cannot be disarmed.
    if (m_triggeredTraps.contains(pos)) {
        logMessage("The trap has already been triggered.");
        return;
    }

    QString trapType = m_trapPositions.value(pos);
    int floorLevel = gsm->getGameValue("DungeonLevel").toInt();

    // Get character stats for disarm chance.
    auto& members = gsm->getPartyMembers();
    if (members.isEmpty()) {
        logMessage("No party member to disarm the trap.");
        return;
    }

    int dex = members[0].dexterity;
    int wis = members[0].wisdom;
    int gLvl = members[0].level;
    int mLvl = floorLevel;  // monster level = floor level
    int dLvl = floorLevel;  // dungeon level

    // Thief modifier: 1.0 for non-thieves, higher for thief/scavenger guilds.
    double thiefMod = 1.0;
    if (members[0].guildLevel("Thief") > 0 || members[0].guildLevel("Scavenger") > 0) {
        thiefMod = 1.5;
    }

    double chance = disarmTrapChance(dex, wis, gLvl, thiefMod, mLvl, dLvl);
    int roll = QRandomGenerator::global()->bounded(1, 101);

    if (roll <= static_cast<int>(chance)) {
        // Success! Remove the trap.
        m_triggeredTraps.insert(pos);
        logMessage(QString("<font color='green'>You successfully disarm the %1 trap!</font>").arg(trapType));
        SoundEffects::instance()->play(SoundEffects::Type::SecretDoor);
    } else {
        // Failure! The trap triggers.
        logMessage(QString("<font color='red'>You fail to disarm the %1 trap!</font>").arg(trapType));
        DungeonHandlers::handleTrap(this, pos.first, pos.second);
    }

    drawMinimap();
}

void DungeonDialog::on_chestButton_clicked() 
{
    processTreasureOpening();
}

void DungeonDialog::initiateFight() {}

void DungeonDialog::on_winBattle_trigger() {}

void DungeonDialog::onEventTriggered(const GameEvent& ) { }

void DungeonDialog::keyPressEvent(QKeyEvent *event)
{
    // Add this to see if the event is even reaching the function
    qDebug() << "Key Pressed:" << event->key();
    switch (event->key()) {
        // --- Movement (WASD) ---
        case Qt::Key_Up:
            qDebug() << "up";
            moveForward();
            event->accept();
            break;
        case Qt::Key_Down:
            qDebug() << "down";
            moveBackward();
            event->accept();
            break;
        case Qt::Key_Left:
            qDebug() << "left";
            moveStepLeft();
            event->accept();
            break;
        case Qt::Key_Right:
            qDebug() << "right";
            moveStepRight();
            event->accept();
            break;
        // --- Rotation (QE) ---
        case Qt::Key_Q:
            on_rotateLeftButton_clicked();
            break;
        case Qt::Key_E:
            on_rotateRightButton_clicked();
            break;
        // --- Other Action Shortcuts ---
        case Qt::Key_T: {
            gameStateManager* gsm = gameStateManager::instance();
            QPair<int, int> currentPos = { 
                gsm->getGameValue("DungeonX").toInt(), 
                gsm->getGameValue("DungeonY").toInt() 
            };

            if (currentPos == m_stairsUpPosition) {
                logMessage("Taking shortcut: Climbing up...");
                transitionLevel(StairDirection::Up);
            } 
            else if (currentPos == m_stairsDownPosition) {
                logMessage("Taking shortcut: Descending down...");
                transitionLevel(StairDirection::Down);
            } 
            else {
                logMessage("There are no stairs here to use.");
            }
            break;
        }
        case Qt::Key_F:
            on_fightButton_clicked();
            break;
        case Qt::Key_O:
            on_openButton_clicked();
            break;
        case Qt::Key_R:
            on_restButton_clicked();
            break;
        case Qt::Key_S:
            on_spellButton_clicked();
            break;
        case Qt::Key_U:
            on_teleportButton_clicked();
            break;
        case Qt::Key_I: 
        { // The opening brace starts a new local scope
            logMessage("Inventory Key Pressed"); 
            // Using 'new' ensures the dialog stays open after this function ends
            InventoryDialog *inventory = new InventoryDialog(this); 
            inventory->setAttribute(Qt::WA_DeleteOnClose); 
            inventory->show();
            inventory->raise();
            inventory->activateWindow();
            break; // Break MUST be inside or immediately after the case
        }
        // NEW: Character Sheet Hotkey
        case Qt::Key_C:
        {
            togglePartyInfo();
            break;
        }
        case Qt::Key_1: {
            logMessage("Opening Character Dialog...");
            CharacterDialog *charDialog = new CharacterDialog(this);
            charDialog->setAttribute(Qt::WA_DeleteOnClose);
            charDialog->show();
            charDialog->raise();
            charDialog->activateWindow();
            break;
        }
        case Qt::Key_D: {
            // Drop functionality (defend is now a combat button)
            on_dropButton_clicked();
            event->accept();
            break;
        }
        default:
            QDialog::keyPressEvent(event);
            break;
    }
}
// Suppress unused parameter warnings
void DungeonDialog::spawnMonsters(const QString& , int ) 
{
}

void DungeonDialog::on_restButton_clicked() 
{
    gameStateManager* gsm = gameStateManager::instance();
    int constitution = gsm->getGameValue("CurrentCharacterConstitution").toInt();
    int healAmount = qMax(1, constitution / 2);    
    int currentHp = gsm->getGameValue("CurrentCharacterHP").toInt();
    int maxHp = gsm->getGameValue("MaxCharacterHP").toInt();
    int newHp = qMin(maxHp, currentHp + healAmount);
    gsm->setGameValue("CurrentCharacterHP", newHp);
    logMessage(QString("You rest and recover %1 HP.").arg(newHp - currentHp));
    // Resting also relights the torch.
    restForTorch();
    SoundEffects::instance()->play(SoundEffects::Type::Rest);
}

void DungeonDialog::on_stairsDownButton_clicked()
{
    transitionLevel(StairDirection::Down);
}

void DungeonDialog::on_stairsUpButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> currentPos = { 
        gsm->getGameValue("DungeonX").toInt(), 
        gsm->getGameValue("DungeonY").toInt() 
    };

    if (currentPos == m_stairsUpPosition) {
        logMessage("Taking shortcut: Climbing up...");
        transitionLevel(StairDirection::Up);
    }

    //transitionLevel(StairDirection::Up);
}

void DungeonDialog::on_openButton_clicked()
{
    processTreasureOpening();
}

void DungeonDialog::updateDungeonView(const QImage& dungeonImage)
{
    m_dungeonScene->clear();
    QPixmap pixmap = QPixmap::fromImage(dungeonImage);
    m_dungeonScene->addPixmap(pixmap);
    m_dungeonScene->setSceneRect(pixmap.rect());
    if (m_graphicsView && m_graphicsView->scene() == m_dungeonScene) {
        m_graphicsView->fitInView(m_dungeonScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void DungeonDialog::on_exitButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Exit", "Exit to the City?", QMessageBox::Yes | QMessageBox::No
    );
    if (reply == QMessageBox::Yes) {
        handleSurfaceExit();
    }
}

void DungeonDialog::on_rotateLeftButton_clicked()
{
    rotate(-1);
}

void DungeonDialog::on_rotateRightButton_clicked()
{
    rotate(1);
}

void DungeonDialog::moveForward()
{
    // Index 0: Forward, 1: Backward, 2: Left, 3: Right
    handleMovement(0); 
}

void DungeonDialog::moveBackward()
{
    // Index 0: Forward, 1: Backward, 2: Left, 3: Right
    handleMovement(1); 
}
void DungeonDialog::moveStepLeft()
{
    // Index 0: Forward, 1: Backward, 2: Left, 3: Right
    handleMovement(2); 
}

void DungeonDialog::moveStepRight() {
    // Index 0: Forward, 1: Backward, 2: Left, 3: Right
    handleMovement(3); 
}

void DungeonDialog::handleMovement(int actionIndex) 
{
    // Map current facing to a list of vectors
    static const QMap<QString, QVector<QPoint>> moveMap = {
        {"Facing North", {QPoint(0,-1), QPoint(0,1),  QPoint(-1,0), QPoint(1,0)}}, // Right is (+1, 0)
        {"Facing South", {QPoint(0,1),  QPoint(0,-1), QPoint(1,0),  QPoint(-1,0)}}, // Right is (-1, 0)
        {"Facing East",  {QPoint(1,0),  QPoint(-1,0), QPoint(0,-1), QPoint(0,1)}},  // Right is (0, +1)
        {"Facing West",  {QPoint(-1,0), QPoint(1,0),  QPoint(0,1),  QPoint(0,-1)}}  // Right is (0, -1)
    };
    QString facing = m_compassLabel->text(); //
    if (moveMap.contains(facing)) {
        QPoint delta = moveMap[facing][actionIndex];
        movePlayer(delta.x(), delta.y()); //
    }
}

void DungeonDialog::rotate(int step) 
{
    // List must match the layout of your compass logic
    static const QStringList dirs = {"North", "East", "South", "West"};    
    // Find current index
    int currentIdx = dirs.indexOf(m_compassLabel->text().mid(7));
    // Calculate new index using modulo to wrap around
    // Adding 4 ensures the result is positive when turning left from North (index 0)
    int nextIdx = (currentIdx + step + 4) % 4;
    updateCompass(dirs[nextIdx]);
    logMessage(QString("You turn to the %1.").arg(step > 0 ? "right" : "left"));
    drawMinimap();
    renderWireframeView();
}

void DungeonDialog::transitionLevel(StairDirection direction)
{
    gameStateManager* gsm = gameStateManager::instance();
    // Retrieve current position once
    int currentZ = gsm->getGameValue("DungeonLevel").toInt();
    QPair<int, int> currentPos = { 
        gsm->getGameValue("DungeonX").toInt(), 
        gsm->getGameValue("DungeonY").toInt() 
    };
    // Use the Enum to pick the correct target
    bool isGoingUp = (direction == StairDirection::Up);
    QPair<int, int> targetStair = isGoingUp ? m_stairsUpPosition : m_stairsDownPosition;
    if (currentPos != targetStair) {
        logMessage("There are no stairs here to take.");
        return;
    }
    if (isGoingUp) {
        int newLevel = currentZ - 1;
        if (newLevel >= 1) {
            logMessage(QString("You take the **stairs up** to Level %1.").arg(newLevel));
            enterLevel(newLevel, true);
            SoundEffects::instance()->play(SoundEffects::Type::Stairs);
        } else {
            handleSurfaceExit();
        }
    } else {
        int newLevel = currentZ + 1;

        // Boss floors: the guardian must fall before the stairs will work.
        if (BossEncounter::hasBoss(currentZ)) {
            const LevelSnapshot* snap = DungeonLevelRegistry::instance().level(currentZ);
            bool bossDefeated = snap ? snap->bossDefeated : false;
            if (!BossEncounter::canDescend(currentZ, bossDefeated)) {
                logMessage(QString("<font color='red'>%1</font>")
                           .arg(BossEncounter::blockedMessage(currentZ)));
                return;
            }
        }

        logMessage(QString("You take the **stairs down** to Level %1.").arg(newLevel));
        enterLevel(newLevel, false);
        SoundEffects::instance()->play(SoundEffects::Type::Stairs);
    }
}

void DungeonDialog::handleSurfaceExit()
{
    // Log the exit for the user
    logMessage("You climb the stairs and emerge into the bright sunlight of The City.");
    // 1. Emit the signal so the parent/manager knows we are leaving
    emit exitedDungeonToCity();
    // 2. Close the dungeon dialog
    this->close();
}

void DungeonDialog::processTreasureOpening()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> pos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };
    if (m_treasurePositions.contains(pos)) {
        // Already opened — nothing left to take.
        if (m_openedChests.contains(pos)) {
            logMessage("<font color='gray'>The chest is already empty.</font>");
            return;
        }

        QString treasure = m_treasurePositions.value(pos);
        int activeIdx = gsm->getGameValue("ActiveCharacterIndex").toInt(); //

        // Locked chest: requires a specific key item in the party inventory.
        if (m_lockedChests.contains(pos)) {
            QString requiredKey = m_chestKeys.value(pos);
            bool hasKey = false;
            auto& members = gsm->getPartyMembers();
            for (const auto& member : members) {
                for (const auto& item : member.inventory) {
                    if (item.name == requiredKey) {
                        hasKey = true;
                        break;
                    }
                }
                if (hasKey) break;
            }
            if (!hasKey) {
                logMessage(QString("<font color='orange'>The chest is locked. It needs the %1.</font>").arg(requiredKey));
                return;
            }
            logMessage(QString("<font color='gold'>You unlock the chest with the %1!</font>").arg(requiredKey));
            m_lockedChests.remove(pos);
            m_chestKeys.remove(pos);
        }

        if (treasure.contains("Gold")) {
            // Existing Gold logic...
            quint64 foundGold = QRandomGenerator::global()->bounded(500, 5000);
            gsm->addPartyGold(static_cast<int>(foundGold));
            logMessage(QString("You gain %L1 Gold.").arg(foundGold));
        } else {
            // Add item to character inventory
            HeldItem lootItem;
            lootItem.name = treasure;
            lootItem.identified = false;  // dungeon loot starts unidentified
            if (const ItemDef* def = ItemDatabase::instance().byName(treasure)) {
                lootItem.M4E97 = static_cast<int16_t>(def->id);
            }
            gsm->addItemToCharacter(activeIdx, lootItem);
            logMessage(QString("You found a %1 and added it to your inventory!").arg(treasure));
        }
        m_treasurePositions.remove(pos);
        m_openedChests.insert(pos);
        DungeonLevelRegistry::instance().levelForEdit(gsm->getGameValue("DungeonLevel").toInt()).openedChests = m_openedChests;
        drawMinimap();
    }
}
bool DungeonDialog::isWallAt(int x, int y) {
    // Check if the coordinates are outside the map boundaries
    if (x < 0 || x >= MAP_SIZE || y < 0 || y >= MAP_SIZE) {
        return true; 
    }
    // Check if the position is in our obstacle set
    return m_obstaclePositions.contains({x, y});
}

bool DungeonDialog::isWallAtSide(int x, int y, const QString& side) {
    // Retrieve the current facing direction from the UI label
    QString facing = m_compassLabel->text(); 
    int targetX = x;
    int targetY = y;

    // Relative logic: If facing North, "left" is West (-1, 0).
    // If facing South, "left" is East (+1, 0), etc.
    if (facing == "Facing North") {
        targetX = (side == "left") ? x - 1 : x + 1;
    } else if (facing == "Facing South") {
        targetX = (side == "left") ? x + 1 : x - 1;
    } else if (facing == "Facing East") {
        targetY = (side == "left") ? y - 1 : y + 1;
    } else if (facing == "Facing West") {
        targetY = (side == "left") ? y + 1 : y - 1;
    }

    return isWallAt(targetX, targetY);
}

void DungeonDialog::renderWireframeView() {
    m_dungeonScene->clear();
    m_dungeonScene->setBackgroundBrush(Qt::black);

    gameStateManager *gsm = gameStateManager::instance();
    int px = gsm->getGameValue("DungeonX").toInt();
    int py = gsm->getGameValue("DungeonY").toInt();
    QString facing = m_compassLabel->text();

    const int w = 300; 
    const int h = 300;
    
    // Perspective points
    int xs[] = {0, 75, 120, 150}; 
    int ys[] = {0, 75, 120, 150};

    for (int d = 2; d >= 0; --d) {
        // 1. Calculate tile coordinates for the CENTER path
        int tx = px, ty = py;
        int dx = 0, dy = 0; 
        if (facing.contains("North")) dy = -1;
        else if (facing.contains("South")) dy = 1;
        else if (facing.contains("East"))  dx = 1;
        else if (facing.contains("West"))  dx = -1;

        tx += (dx * d);
        ty += (dy * d);

        // 2. Calculate coordinates for adjacent tiles (Left/Right) at this depth
        // This is necessary to see if the side-corridor is blocked by a wall
        int lx = tx + dy, ly = ty - dx; // Left tile relative to facing
        int rx = tx - dy, ry = ty + dx; // Right tile relative to facing

        // Wall Checks
        bool wallFront      = isWallAt(tx, ty);
        bool wallLeftSide   = isWallAtSide(tx, ty, "left");  // The plane parallel to you
        bool wallRightSide  = isWallAtSide(tx, ty, "right"); // The plane parallel to you
        bool wallInLeftTile = isWallAt(lx, ly);              // The wall inside the side corridor
        bool wallInRightTile = isWallAt(rx, ry);             // The wall inside the side corridor

        // Screen coordinates for this depth
        int xL = xs[d];     int xR = w - xs[d];
        int yT = ys[d];     int yB = h - ys[d];
        int nxL = xs[d+1];  int nxR = w - xs[d+1];
        int nyT = ys[d+1];  int nyB = h - ys[d+1];

        // --- 1. FLOOR & CEILING (Full Span) ---
        QPolygon floor, ceil;
        floor << QPoint(0, yB) << QPoint(w, yB) << QPoint(w, nyB) << QPoint(0, nyB);
        ceil << QPoint(0, yT) << QPoint(w, yT) << QPoint(w, nyT) << QPoint(0, nyT);
        
        int floorCol = qMax(0, 40 - (d * 10));
        int ceilR = qMax(0, 80 - (d * 20)); // Brown base
        m_dungeonScene->addPolygon(floor, QPen(Qt::NoPen), QBrush(QColor(floorCol, floorCol, floorCol)));
        m_dungeonScene->addPolygon(ceil, QPen(Qt::NoPen), QBrush(QColor(ceilR, qMax(0, 50-(d*15)), qMax(0, 30-(d*10)))));

        // --- 2. SIDE-FACING WALLS (Corridor Walls) ---
        int sideWallCol = qMax(0, 80 - (d * 15));
        if (wallLeftSide) {
            QPolygon p; p << QPoint(xL, yT) << QPoint(nxL, nyT) << QPoint(nxL, nyB) << QPoint(xL, yB);
            m_dungeonScene->addPolygon(p, QPen(Qt::black), QBrush(QColor(sideWallCol, sideWallCol, sideWallCol)));
            drawBrickPattern(p, d);
        }
        if (wallRightSide) {
            QPolygon p; p << QPoint(xR, yT) << QPoint(nxR, nyT) << QPoint(nxR, nyB) << QPoint(xR, yB);
            m_dungeonScene->addPolygon(p, QPen(Qt::black), QBrush(QColor(sideWallCol, sideWallCol, sideWallCol)));
            drawBrickPattern(p, d);
        }

        // --- 3. FRONT-FACING WALLS IN SIDE CORRIDORS ---
        // If there is NO side wall blocking the view, check if the adjacent tile has a wall
        int sideRoomWallCol = qMax(0, 65 - (d * 15));
        if (!wallLeftSide && wallInLeftTile) {
            // Draw a wall on the left wing of the screen
            m_dungeonScene->addRect(0, yT, xL, yB - yT, QPen(Qt::black), QBrush(QColor(sideRoomWallCol, sideRoomWallCol, sideRoomWallCol)));
        }
        if (!wallRightSide && wallInRightTile) {
            // Draw a wall on the right wing of the screen
            m_dungeonScene->addRect(xR, yT, w - xR, yB - yT, QPen(Qt::black), QBrush(QColor(sideRoomWallCol, sideRoomWallCol, sideRoomWallCol)));
        }
        
        bool hasAntimagic = m_antimagicPositions.contains({tx, ty});

        // 1. Draw floor/ceiling
        // ... 

        // 2. Draw Antimagic effect on the floor
        if (hasAntimagic) {
            drawAntimagic(d, xL, xR, yB, nxL, nxR, nyB);
        }
        // Identify if tile is water (Adjust based on your tile type logic)
        // Assuming you have a set or map for water tiles
        bool hasWater = m_waterPositions.contains({tx, ty});

        // 1. Draw floor and ceiling first
        // ... (existing floor drawing) ...

        // 2. Draw Water on top of the floor
        if (hasWater) {
            drawWater(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // 1. Check if this tile is a Spinner (Rotator)
        bool hasSpinner = m_rotatorPositions.contains({tx, ty});

        // ... (Existing Floor, Ceiling, and Side Wall rendering)

        // 2. Draw the Spinner effect
        if (hasSpinner) {
            drawSpinner(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // 1. Check for teleporter at this map coordinate
        bool hasTeleporter = m_teleporterPositions.contains({tx, ty});

        // ... existing Floor/Ceiling/Side Wall rendering ...

        // 2. Draw the Teleporter effect on the floor
        if (hasTeleporter) {
            drawTeleporter(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check if there is a monster at this coordinate
        bool hasMonster = m_monsterPositions.contains({tx, ty});

        // ... (draw floor and side walls)

        // Draw the monster if present
        if (hasMonster) {
            drawMonster(d, xL, xR, yB);
        }   

        // --- 4. FRONT WALL (Main Path) ---
        if (wallFront) {
            int frontCol = qMax(0, 110 - (d * 20));
            m_dungeonScene->addRect(xL, yT, xR - xL, yB - yT, QPen(Qt::black), QBrush(QColor(frontCol, frontCol, frontCol)));
        }
        
        // 1. Check if there is a chute at this specific map tile
        bool hasChute = m_chutePositions.contains({tx, ty});
        // 2. Draw the chute on top of the floor
        if (hasChute) {
            drawChute(d, xL, xR, yB, nxL, nxR, nyB);
        }
    }
}

void DungeonDialog::drawBrickPattern(const QPolygon& wallPoly, int depth) 
{
    Q_UNUSED(depth);
    QRect bounds = wallPoly.boundingRect();
    QPen mortarPen(QColor(40, 40, 40, 150)); // Semi-transparent dark gray
    mortarPen.setWidth(1);

    int rows = 6;    // Number of brick layers
    int columns = 4; // Number of bricks per row
    
    // Calculate row height
    double rowHeight = static_cast<double>(bounds.height()) / rows;

    for (int i = 1; i < rows; ++i) {
        int y = bounds.top() + (i * rowHeight);
        
        // Find left and right edges of the polygon at this specific Y height
        // to ensure bricks don't float outside the perspective wall
        int xMin = bounds.right();
        int xMax = bounds.left();
        
        // Basic scan-line logic to keep lines inside the trapezoid
        for (int x = bounds.left(); x <= bounds.right(); ++x) {
            if (wallPoly.containsPoint(QPoint(x, y), Qt::OddEvenFill)) {
                xMin = qMin(xMin, x);
                xMax = qMax(xMax, x);
            }
        }

        // Draw horizontal mortar line
        if (xMin < xMax) {
            m_dungeonScene->addLine(xMin, y, xMax, y, mortarPen);
            
            // Draw vertical "staggered" mortar lines
            double colWidth = static_cast<double>(xMax - xMin) / columns;
            double offset = (i % 2 == 0) ? 0 : colWidth / 2; // Stagger bricks
            
            for (int j = 0; j <= columns; ++j) {
                int vx = xMin + (j * colWidth) + offset;
                if (vx > xMin && vx < xMax) {
                    m_dungeonScene->addLine(vx, y, vx, y - rowHeight, mortarPen);
                }
            }
        }
    }
}

void DungeonDialog::drawChute(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    // We want the chute to be in the center of the tile floor
    // Calculate a 40% width/depth hole
    double margin = 0.3;     
    // Interpolate points for the "hole" on the floor
    int cxL = xL + (xR - xL) * margin;
    int cxR = xR - (xR - xL) * margin;
    int cnxL = nxL + (nxR - nxL) * margin;
    int cnxR = nxR - (nxR - nxL) * margin;
    // Perspective depth for the floor (yB to nyB)
    int cyNear = yB;
    int cyFar = yB + (nyB - yB) * 0.6; // The hole doesn't take the whole tile

    QPolygon chuteHole;
    chuteHole << QPoint(cxL, cyNear) << QPoint(cxR, cyNear) 
              << QPoint(cnxR, cyFar) << QPoint(cnxL, cyFar);
    // Draw the "void" (black hole)
    m_dungeonScene->addPolygon(chuteHole, QPen(Qt::black), QBrush(Qt::black));
    // Draw inner "walls" of the chute for a 3D effect
    QPolygon leftInner;
    leftInner << QPoint(cxL, cyNear) << QPoint(cnxL, cyFar) 
              << QPoint(cnxL, cyFar + 10) << QPoint(cxL, cyNear + 10);
    
    int depthShade = qMax(0, 30 - (d * 10));
    m_dungeonScene->addPolygon(leftInner, QPen(Qt::NoPen), QBrush(QColor(depthShade, depthShade, depthShade)));
}

void DungeonDialog::drawMonster(int d, int xL, int xR, int yB) {
    // Calculate size based on depth
    // d=0 (Near): Large, d=2 (Far): Small
    int monsterWidth = (xR - xL) * 0.6; 
    int monsterHeight = monsterWidth * 1.2;
    
    // Center horizontally, sit on the floor (yB)
    int centerX = xL + (xR - xL) / 2;
    int xPos = centerX - (monsterWidth / 2);
    int yPos = yB - monsterHeight;

    // Depth shading: make monsters darker in the distance
    int shade = qMax(0, 200 - (d * 60));
    QColor monsterColor(shade, 0, 0); // Dark red silhouette

    // Draw a simple head and body (billboard style)
    QRect body(xPos, yPos + (monsterHeight * 0.3), monsterWidth, monsterHeight * 0.7);
    QRect head(centerX - (monsterWidth / 4), yPos, monsterWidth / 2, monsterHeight * 0.4);

    m_dungeonScene->addEllipse(head, QPen(Qt::black), QBrush(monsterColor));
    m_dungeonScene->addRect(body, QPen(Qt::black), QBrush(monsterColor));
}

void DungeonDialog::drawTeleporter(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) 
{
    Q_UNUSED(nxR);
    Q_UNUSED(nxL);
    // Calculate the center and size of the tile floor
    int centerX = xL + (xR - xL) / 2;
    int centerY = yB + (nyB - yB) / 2;    
    // Scale radius based on depth
    int baseRadius = (xR - xL) / 3;
    // Create a shimmering effect with multiple ellipses
    for (int i = 0; i < 3; ++i) {
        int r = baseRadius - (i * (baseRadius / 4));
        QRectF glowRect(centerX - r, centerY - (r / 2), r * 2, r); // Flattened for perspective
        // Alternating cyan/blue glow
        QColor glowColor = (i % 2 == 0) ? QColor(0, 255, 255, 150) : QColor(0, 100, 255, 100);
        // Depth shading: make it dimmer in the distance
        int alpha = glowColor.alpha() - (d * 30);
        glowColor.setAlpha(qMax(0, alpha));
        m_dungeonScene->addEllipse(glowRect, QPen(Qt::white, 1), QBrush(glowColor));
    }
    // Optional: Add some "sparkles" (small white dots)
    for (int j = 0; j < 5; ++j) {
        int sx = centerX + (QRandomGenerator::global()->bounded(baseRadius * 2) - baseRadius);
        int sy = centerY + (QRandomGenerator::global()->bounded(baseRadius) - (baseRadius / 2));
        m_dungeonScene->addRect(sx, sy, 1, 1, QPen(Qt::white), QBrush(Qt::white));
    }
}

void DungeonDialog::drawSpinner(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    // Calculate the center of the tile floor using both near and far bounds
    int centerX = (xL + xR + nxL + nxR) / 4;
    int centerY = (yB + nyB) / 2;    
    // Scale the size of the spinner based on the available floor width at this depth
    int radius = (xR - xL) / 3;
    QPen spinnerPen(QColor(200, 200, 0), 2); 
    // Depth shading to make it darker in the distance
    int alpha = qMax(0, 200 - (d * 50));
    spinnerPen.setColor(QColor(200, 200, 0, alpha));
    // Define the bounding rect for the ellipse/arcs, flattened for perspective
    QRectF arcRect(centerX - radius, centerY - (radius / 2), radius * 2, radius);
    // QGraphicsScene doesn't have addArc; we use QPainterPath instead
    for (int i = 0; i < 4; ++i) {
        QPainterPath path;
        int startAngle = i * 90; // Angles in QPainterPath are in degrees
        int spanAngle = 60;
        
        path.arcMoveTo(arcRect, startAngle);
        path.arcTo(arcRect, startAngle, spanAngle);
        
        m_dungeonScene->addPath(path, spinnerPen);
    }
    // Add a center point (uses near/far center)
    m_dungeonScene->addEllipse(centerX - 2, centerY - 1, 4, 2, spinnerPen, QBrush(spinnerPen.color()));
}

void DungeonDialog::drawWater(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    // 1. Create the water surface polygon (the floor area)
    QPolygon waterPoly;
    waterPoly << QPoint(xL, yB) << QPoint(xR, yB) << QPoint(nxR, nyB) << QPoint(nxL, nyB);
    // Deep blue color, semi-transparent so you can still see the floor/chutes
    int alpha = qMax(0, 180 - (d * 40));
    QColor waterColor(0, 105, 148, alpha); // Sea Blue
    m_dungeonScene->addPolygon(waterPoly, QPen(Qt::NoPen), QBrush(waterColor));
    // 2. Add "Ripples" (shimmering lines)
    QPen ripplePen(QColor(255, 255, 255, qMax(0, 100 - (d * 30))));
    ripplePen.setWidth(1);
    // Draw 3 horizontal lines at different depths within the tile
    for (int i = 1; i <= 3; ++i) {
        double ratio = i / 4.0;
        // Interpolate Y position between near (yB) and far (nyB)
        int ry = yB + (nyB - yB) * ratio;
        // Interpolate X width based on perspective
        int rxL = xL + (nxL - xL) * ratio;
        int rxR = xR + (nxR - xR) * ratio;
        // Draw a partial shimmering line (not the whole width for a natural look)
        int rippleWidth = (rxR - rxL) * 0.6;
        int rippleStart = rxL + (rxR - rxL) * 0.2;
        
        m_dungeonScene->addLine(rippleStart, ry, rippleStart + rippleWidth, ry, ripplePen);
    }
}
void DungeonDialog::drawAntimagic(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    // Define the floor polygon for clipping or reference
    QPolygon floorPoly;
    floorPoly << QPoint(xL, yB) << QPoint(xR, yB) << QPoint(nxR, nyB) << QPoint(nxL, nyB);
    // Color: A muted, "dulling" purple or gray
    int alpha = qMax(0, 120 - (d * 30));
    QColor fieldColor(100, 100, 150, alpha); 
    // Draw a subtle tint first
    m_dungeonScene->addPolygon(floorPoly, QPen(Qt::NoPen), QBrush(fieldColor));
    // Draw "Static" lines (cross-hatch pattern)
    QPen staticPen(QColor(200, 200, 255, qMax(0, 150 - (d * 40))));
    staticPen.setWidth(1);

    int lines = 5;
    for (int i = 0; i <= lines; ++i) {
        double ratio = (double)i / lines;

        // Vertical-ish lines (converging towards vanishing point)
        int xNear = xL + (xR - xL) * ratio;
        int xFar = nxL + (nxR - nxL) * ratio;
        m_dungeonScene->addLine(xNear, yB, xFar, nyB, staticPen);

        // Horizontal lines (interpolated by depth)
        int yHoriz = yB + (nyB - yB) * ratio;
        int xHLeft = xL + (nxL - xL) * ratio;
        int xHRight = xR + (nxR - xR) * ratio;
        m_dungeonScene->addLine(xHLeft, yHoriz, xHRight, yHoriz, staticPen);
    }
}

void DungeonDialog::togglePartyInfo() {
    if (m_charSheet) {
        m_charSheet->close();
        m_charSheet = nullptr; 
        return;
    }

    m_charSheet = new PartyInfoDialog(this);
    // 1. ADD Qt::WindowDoesNotAcceptFocus
    // This allows the main window to keep keyboard focus for movement
    m_charSheet->setWindowFlags(Qt::Tool | 
                                Qt::WindowStaysOnTopHint | 
                                Qt::WindowDoesNotAcceptFocus |
                                Qt::CustomizeWindowHint | 
                                Qt::WindowTitleHint | 
                                Qt::WindowCloseButtonHint);

    m_charSheet->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_charSheet, &QObject::destroyed, this, [this]() { m_charSheet = nullptr; });
    // 2. Use show() but DO NOT call activateWindow() or raise()
    // activateWindow() is what forces the focus change; we want to avoid that.
    m_charSheet->show();
    // 3. Position in Bottom-Right
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int dialogWidth = m_charSheet->frameGeometry().width();
        int dialogHeight = m_charSheet->frameGeometry().height();
        int padding = 10;

        m_charSheet->move(screenGeometry.width() - dialogWidth - padding,
                          screenGeometry.height() - dialogHeight - padding);
    }
}

void DungeonDialog::on_spellButton_clicked() 
{ 
    logMessage("You try to cast some sort of spell but fail."); 
    // Check if in antimagic zone
    gameStateManager* gsm = gameStateManager::instance();
    int x = gsm->getGameValue("DungeonX").toInt();
    int y = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> pos = {x, y};
    
    if (m_antimagicPositions.contains(pos)) {
        logMessage("<font color='purple'>An antimagic field prevents you from casting spells here!</font>");
        return;
    }
    
    // Open the spell casting dialog
    SpellCastingDialog* spellDialog = new SpellCastingDialog(this, m_inCombat);
    spellDialog->setAttribute(Qt::WA_DeleteOnClose);

    // Connect spell effects to dungeon actions
    connect(spellDialog, &SpellCastingDialog::spellCast, this,
            [this](const QString& spellName, const SpellResult& result) {

    logMessage(QString("<font color='cyan'>%1</font>").arg(result.message));

        // Handle damage to current monster if in combat (turn-based).
        // Route through CombatActions so death bookkeeping and the defensive
        // stance are respected, instead of poking participant HP directly.
        if (result.damageDealt > 0 && m_inCombat && m_combatState && m_combatActions) {
            // Find first living monster
            int targetIdx = -1;
            for (int i = 0; i < m_combatState->participantCount(); ++i) {
                if (!m_combatState->participant(i).isPlayer && m_combatState->participant(i).isAlive) {
                    targetIdx = i;
                    break;
                }
            }
            if (targetIdx >= 0) {
                QString dmgResult;
                // Route through the school-aware path so fire splashes, cold
                // slows, lightning chains and mind stuns actually happen.
                m_combatActions->applySpellDamageBySchool(targetIdx, spellName,
                                                          result.damageDealt, dmgResult);
                logMessage(QString("<font color='yellow'>%1</font>").arg(dmgResult));

                if (!m_combatState->participant(targetIdx).isAlive) {
                    // Remove from map
                    QPair<int, int> pos = getCurrentPosition();
                    m_monsterPositions.remove(pos);
                    renderWireframeView();
                    drawMinimap();
                }

                // Check combat end
                QStringList deaths = m_deathHandler->processDeaths();
                for (const QString& d : deaths) logMessage(d);
                syncCombatToGameState();
                updateCombatUI();
                advanceCombat();
            }
        }

        // Handle teleport spell
        if (result.effectApplied == "Teleport") {
            on_teleportButton_clicked();
        }
    });

    spellDialog->exec();
}

void DungeonDialog::awardBattleLoot() {
    gameStateManager* gsm = gameStateManager::instance();
    // Retrieve the full item list loaded into gameStateManager
    const QList<QVariantMap>& allItems = gsm->itemData();
    if (allItems.isEmpty()) return;

    // Select a random item from the database
    int itemIdx = QRandomGenerator::global()->bounded(allItems.size());
    QString itemName = allItems.at(itemIdx).value("name").toString();

    // Store the item in the character's inventory
    HeldItem lootItem;
    lootItem.name = itemName;
    lootItem.identified = false;  // dungeon loot starts unidentified
    if (const ItemDef* def = ItemDatabase::instance().byName(itemName)) {
        lootItem.M4E97 = static_cast<int16_t>(def->id);
    }
    gsm->addItemToInventory(lootItem);

    // Show the item to the player in the message log
    logMessage(QString("<font color='gold'>The monster dropped a %1!</font>").arg(itemName));
}

QStringList DungeonDialog::getThematicMonsters(int level) const
{
    // Monsters come from the game's own monster table (MDATA5, loaded by
    // gameStateManager). Each row carries `levelFound`, the dungeon floor it
    // belongs on, so a floor draws from monsters of a matching level rather
    // than a hardcoded name list. Fall back to nearby levels if a floor is thin.
    const QList<QVariantMap>& table = gameStateManager::instance()->getMonsterData();
    QStringList monsters;

    auto collect = [&](int wantLevel) {
        for (const QVariantMap& m : table) {
            if (m.value("levelFound").toInt() == wantLevel) {
                const QString name = m.value("name").toString();
                if (!name.isEmpty() && !monsters.contains(name)) {
                    monsters << name;
                }
            }
        }
    };

    collect(level);
    // Widen the net if this floor has too few distinct monsters.
    for (int delta = 1; delta <= 3 && monsters.size() < 4; ++delta) {
        if (level - delta >= 1) collect(level - delta);
        collect(level + delta);
    }

    // Last resort: any monster at all, so the floor is never empty.
    if (monsters.isEmpty()) {
        for (const QVariantMap& m : table) {
            const QString name = m.value("name").toString();
            if (!name.isEmpty()) monsters << name;
        }
    }

    return monsters;
}

DungeonDialog::~DungeonDialog(){}
