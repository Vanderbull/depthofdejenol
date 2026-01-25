#include "src/character_dialog/CharacterDialog.h"
#include "DungeonDialog.h"
#include "DungeonHandlers.h"
#include "../../GameStateManager.h" // REQUIRED: Include for GameState access
#include "../event/EventManager.h"
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

// Constants that depend on MAP_SIZE (which is now in the header)
const int TILE_SIZE = 10;
const int MAP_WIDTH_PIXELS = MAP_SIZE * TILE_SIZE; 
const int MAP_HEIGHT_PIXELS = MAP_SIZE * TILE_SIZE; 
const int MAP_MIN = 0;
const int MAP_MAX = MAP_SIZE - 1;

void DungeonDialog::populateRandomTreasures(int level)
{
    GameStateManager* gsm = GameStateManager::instance();
    // MDATA3 is loaded into m_itemData in GameStateManager
    const QList<QVariantMap>& allItems = gsm->itemData(); 
    if (allItems.isEmpty()) {
        qDebug() << "MDATA3 not loaded or empty.";
        return;
    }
    QRandomGenerator* rng = QRandomGenerator::global();
    int itemsPlaced = 0;
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
            // 5. Add to the Global Array in GameStateManager
            gsm->addPlacedItem(level, x, y, itemName);
            qDebug() << itemName;
            itemsPlaced++;
        }
    }
    qDebug() << "Successfully placed 10 random items from MDATA3 on level" << level;
}

void DungeonDialog::revealAroundPlayer(int x, int y, int z=0)
{
    Q_UNUSED(z);
    // Loops through the 3x3 grid centered on the player
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            int nx = x + dx;
            int ny = y + dy;

            // Ensure we stay within map boundaries defined in the header
            if (nx >= 0 && nx < MAP_SIZE && ny >= 0 && ny < MAP_SIZE) {
                m_visitedTiles.insert({nx, ny});
            }
        }
    }
}

void DungeonDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    // This ensures the dungeon graphic scales to fit the new window size
    QGraphicsView* dungeonView = findChild<QGraphicsView*>();
    if (dungeonView && m_dungeonScene) {
        dungeonView->fitInView(m_dungeonScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

// --- Helper Function: logMessage (Must be defined as a member) ---
void DungeonDialog::logMessage(const QString& message)
{
    if (m_messageLog) {
        m_messageLog->addItem(message);
        m_messageLog->scrollToBottom();
    }
}
// --- Gold Management Helper Function ---
void DungeonDialog::updateGoldLabel()
{
    // UPDATED: Retrieve gold from GameStateManager
    quint64 currentGold = GameStateManager::instance()->getGameValue("PlayerGold").toULongLong();
    if (m_goldLabel) {
        QString goldString = QStringLiteral("%L1").arg(currentGold);
        m_goldLabel->setText(QString("You have a total of **%1 Gold**.").arg(goldString));
    }
}
// --- Party Management Helper Function ---
void DungeonDialog::updatePartyMemberHealth(int row, int damage)
{
    GameStateManager* gsm = GameStateManager::instance();   
    
    // Check if player is defending to halve damage
    int finalDamage = damage;
    if (m_isDefending && row == 0) {
        finalDamage = qMax(1, damage / 2); // Halve damage, minimum of 1
    }

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
    GameStateManager* gsm = GameStateManager::instance();
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
    // Record current position as a breadcrumb before moving
    m_breadcrumbPath.append({currentX, currentY});
    if (m_breadcrumbPath.size() > MAX_BREADCRUMBS) {
        m_breadcrumbPath.removeFirst(); // Keep the trail from getting too long
    }
    gsm->setGameValue("DungeonX", newX);
    gsm->setGameValue("DungeonY", newY);
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
    logMessage(QString("You move to (%1, %2).").arg(newX).arg(newY));
    drawMinimap();
    renderWireframeView();
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
    GameStateManager::instance()->setGameValue("DungeonX", rooms[0].x + 1);
    GameStateManager::instance()->setGameValue("DungeonY", rooms[0].y + 1);
}

void DungeonDialog::generateStairs(QRandomGenerator& rng)
{
    GameStateManager* gsm = GameStateManager::instance();
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
    GameStateManager* gsm = GameStateManager::instance();
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
    for (int i = 0; i < tileCount; ++i) {
        int roll = rng.bounded(100); // Using 100 for better percentage control
        QPair<int, int> pos;
        if (roll < 15) { // 15% Monsters
            pos = getAnyFloorTile();
            if (pos.first != -1) m_monsterPositions.insert(pos, "Orc");
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
    }
}
// --- Constructor Implementation ---
DungeonDialog::DungeonDialog(QWidget *parent)
    : QDialog(parent),
      m_dungeonScene(new QGraphicsScene(this))
{
    GameStateManager* gsm = GameStateManager::instance();
    if (!GameStateManager::instance()) {
        qCritical() << "CRITICAL: GameStateManager is NULL!";
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
    setMinimumSize(1000, 750);
    // Load GameStateManager data

    // Retrieve Constitution to calculate a simple placeholder for maximum HP
    // int con = gsm->getGameValue("CurrentCharacterConstitution").toInt();
    //int maxHP = (con > 0) ? con * 5 : 50; 
    // --- Retrieve/Set persistent dungeon state (New GameState logic) ---
    int initialLevel = gsm->getGameValue("DungeonLevel").toInt();
    int initialX = gsm->getGameValue("DungeonX").toInt();
    int initialY = gsm->getGameValue("DungeonY").toInt();
    //quint64 initialGold = gsm->getGameValue("PlayerGold").toULongLong();
    gsm->setGameValue("PlayerGold", gsm->getPC().at(0).Gold);
    //initialGold = gsm->getPC().at(0).gold;
    
    // Set defaults if state data is missing (e.g., first time entering)
    if (initialLevel == 0) {
        initialLevel = 1;
        initialX = MAP_SIZE / 2;
        initialY = MAP_SIZE / 2;
        quint64 initialGold = 1500;
        // Save initial defaults to GameState
        gsm->setGameValue("DungeonLevel", initialLevel);
        gsm->setGameValue("DungeonX", initialX);
        gsm->setGameValue("DungeonY", initialY);
        gsm->setGameValue("PlayerGold", initialGold);
    }
    // -----------------------------------------------------------------
    // --- Main Layout Setup ---
    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    // Ensure the left panel takes up the majority of the space (e.g., a ratio of 3:1)
    // --- Left Panel: Party, Map, Log ---
    QVBoxLayout *leftPanelLayout = new QVBoxLayout();
    // --- Right Panel: Info, Compass, Minimap, Controls ---
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();
    rootLayout->addLayout(leftPanelLayout, 3);
    rootLayout->addLayout(rightPanelLayout, 1);
    // 2. Dungeon View (Dungeon Scene/View will be here)
    QGraphicsView *dungeonView = new QGraphicsView(m_dungeonScene);
    dungeonView->setRenderHint(QPainter::Antialiasing);
    //dungeonView->setFixedSize(600, 350); 
    dungeonView->setMinimumSize(400, 300);
    leftPanelLayout->addWidget(dungeonView);
    // 3. Message Log
    QGroupBox *logBox = new QGroupBox("Adventure Log");
    QVBoxLayout *logLayout = new QVBoxLayout(logBox);
    m_messageLog = new QListWidget();
    logLayout->addWidget(m_messageLog);
    leftPanelLayout->addWidget(logBox, 1); 
    rootLayout->addLayout(leftPanelLayout, 1); 
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
    QLabel *alignmentLabel = new QLabel(QString("Alignment: **%1**").arg("TODO"));
    infoLayout->addWidget(alignmentLabel, 1, 1);
    rightPanelLayout->addWidget(infoBox);
    updateExperienceLabel();
    rightPanelLayout->addWidget(m_experienceLabel);
    // 2. Gold Display
    m_goldLabel = new QLabel();
    updateGoldLabel(); // Call to update label with GameState value
    rightPanelLayout->addWidget(m_goldLabel);
    // Initialize map state and draw the initial view
    QRandomGenerator initialRng(1 + 12345); 
    // 2. Pass 'initialRng' to the functions as the second argument
    generateRandomObstacles(40, initialRng);  
    generateStairs(initialRng);               
    generateSpecialTiles(20, initialRng);
    drawMinimap(); 
    // 4. Action Buttons
    QGridLayout *actionLayout = new QGridLayout();
    m_fightButton = new QPushButton("Fight (F)");
    m_spellButton = new QPushButton("Spell (S)");
    m_restButton = new QPushButton("Rest (R)");
    m_talkButton = new QPushButton("Talk (T)");
    m_searchButton = new QPushButton("Search (Z)");
    m_pickupButton = new QPushButton("Pickup (P)");
    m_dropButton = new QPushButton("Drop (D)");
    m_openButton = new QPushButton("Open (O)");
    m_mapButton = new QPushButton("Map (M)");
    m_chestButton = new QPushButton("Chest (C)"); 
    m_exitButton = new QPushButton("Exit");
    m_teleportButton = new QPushButton("Teleport (U)");
    actionLayout->addWidget(m_fightButton, 0, 0);
    actionLayout->addWidget(m_spellButton, 0, 1);
    actionLayout->addWidget(m_restButton, 0, 2);
    actionLayout->addWidget(m_talkButton, 1, 0);
    actionLayout->addWidget(m_searchButton, 1, 1);
    actionLayout->addWidget(m_pickupButton, 1, 2);
    actionLayout->addWidget(m_dropButton, 2, 0);
    actionLayout->addWidget(m_openButton, 2, 1);
    actionLayout->addWidget(m_mapButton, 2, 2);
    actionLayout->addWidget(m_chestButton, 3, 0);
    actionLayout->addWidget(m_teleportButton, 3, 1);
    actionLayout->addWidget(m_exitButton, 3, 2);
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
    moveLayout->addWidget(m_rotateLeftButton, 0, 0);
    moveLayout->addWidget(m_upButton, 0, 1);
    moveLayout->addWidget(m_rotateRightButton, 0, 2);
    moveLayout->addWidget(m_leftButton, 1, 0);
    moveLayout->addWidget(m_downButton, 1, 1);
    moveLayout->addWidget(m_rightButton, 1, 2);
    rightPanelLayout->addWidget(moveBox);
    // 6. Stairs Buttons
    QHBoxLayout *stairsLayout = new QHBoxLayout();
    QPushButton *stairsUpButton = new QPushButton("Stairs Up");
    QPushButton *stairsDownButton = new QPushButton("Stairs Down");
    stairsLayout->addWidget(stairsUpButton);
    stairsLayout->addWidget(stairsDownButton);
    rightPanelLayout->addLayout(stairsLayout);
    rootLayout->addLayout(rightPanelLayout); 
    // Initial log messages
    enterLevel(initialLevel); // Use initialLevel retrieved from GameState
    // Connections (Movements)
    connect(m_upButton, &QPushButton::clicked, this, &DungeonDialog::moveForward);
    connect(m_downButton, &QPushButton::clicked, this, &DungeonDialog::moveBackward);
    connect(m_leftButton, &QPushButton::clicked, this, &DungeonDialog::moveStepLeft);  // Updated
    connect(m_rightButton, &QPushButton::clicked, this, &DungeonDialog::moveStepRight); // Updated
    connect(m_rotateLeftButton, &QPushButton::clicked, this, &DungeonDialog::on_rotateLeftButton_clicked);
    connect(m_rotateRightButton, &QPushButton::clicked, this, &DungeonDialog::on_rotateRightButton_clicked);
    // Connections (Actions)
    connect(m_fightButton, &QPushButton::clicked, this, &DungeonDialog::on_fightButton_clicked);
    connect(m_spellButton, &QPushButton::clicked, this, &DungeonDialog::on_spellButton_clicked);
    connect(m_restButton, &QPushButton::clicked, this, &DungeonDialog::on_restButton_clicked);
    connect(m_talkButton, &QPushButton::clicked, this, &DungeonDialog::on_talkButton_clicked);
    connect(m_searchButton, &QPushButton::clicked, this, &DungeonDialog::on_searchButton_clicked);
    connect(m_pickupButton, &QPushButton::clicked, this, &DungeonDialog::on_pickupButton_clicked);
    connect(m_dropButton, &QPushButton::clicked, this, &DungeonDialog::on_dropButton_clicked);
    connect(m_openButton, &QPushButton::clicked, this, &DungeonDialog::on_openButton_clicked);
    connect(m_mapButton, &QPushButton::clicked, this, &DungeonDialog::on_mapButton_clicked);
    connect(m_chestButton, &QPushButton::clicked, this, &DungeonDialog::on_chestButton_clicked);
    connect(m_exitButton, &QPushButton::clicked, this, &DungeonDialog::on_exitButton_clicked);
    connect(m_teleportButton, &QPushButton::clicked, this, &DungeonDialog::on_teleportButton_clicked);
    // Connections (Stairs)
    connect(stairsDownButton, &QPushButton::clicked, this, &DungeonDialog::on_stairsDownButton_clicked);
    connect(stairsUpButton, &QPushButton::clicked, this, &DungeonDialog::on_stairsUpButton_clicked);
    m_graphicsView = new QGraphicsView(this);
    m_graphicsView->setFixedSize(300, 300); // Force the widget to be square
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_dungeonScene = new QGraphicsScene(0, 0, 300, 300, this);
    m_graphicsView->setScene(m_dungeonScene);

    // ADD THESE LINES AT THE END OF THE CONSTRUCTOR:
    drawMinimap();
    renderWireframeView();
    // ADD THIS LINE TO PREVENT THE CRASH:
    m_combatTimer = new QTimer(this);
    // Add this to your Constructor for every button and widget
    m_upButton->setFocusPolicy(Qt::NoFocus);
    m_downButton->setFocusPolicy(Qt::NoFocus);
    m_leftButton->setFocusPolicy(Qt::NoFocus);
    m_rightButton->setFocusPolicy(Qt::NoFocus);
    m_messageLog->setFocusPolicy(Qt::NoFocus);
    m_graphicsView->setFocusPolicy(Qt::NoFocus);
    // Do this for ALL buttons (Fight, Spell, Map, etc.)
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();
}

void DungeonDialog::updateExperienceLabel()
{
    // Retrieve Experience from GameStateManager
    // Assuming the key is "PlayerExperience"
    quint64 currentXP = GameStateManager::instance()->getGameValue("PlayerExperience").toULongLong();
    if (m_experienceLabel) {
        QString xpString = QStringLiteral("%L1").arg(currentXP);
        m_experienceLabel->setText(QString("Experience: **%1 XP**").arg(xpString));
    }
}

DungeonDialog::~DungeonDialog()
{
}

void DungeonDialog::enterLevel(int level, bool movingUp)
{
    m_breadcrumbPath.clear();
    m_visitedTiles.clear();
    // Clear treasures specifically at the start of level generation
    m_treasurePositions.clear();
    GameStateManager* gsm = GameStateManager::instance();
    // 1. Generate the map using Room-and-Corridor logic
    // Seed by level to ensure the layout is deterministic
    QRandomGenerator levelRng(level + 12345);
    populateRandomTreasures(level);
    // We pass a 'Room Count' instead of 'Obstacle Count'. 
    // 7 rooms at level 1, increasing slightly as you go deeper.
    generateRandomObstacles(40, levelRng); 
    // 2. Place Stairs and Special Tiles
    // These must be called AFTER generateRandomObstacles so they know where the floor is.
    generateStairs(levelRng);
    // Scale the number of special tiles (monsters/traps) with the level
    generateSpecialTiles(20, levelRng);
    // 3. Determine Landing Position
    // Arrive at the Down stairs if moving Up, or Up stairs if moving Down
    QPair<int, int> landingPos = movingUp ? m_stairsDownPosition : m_stairsUpPosition;
    // 4. Update Game State and UI
    gsm->setGameValue("DungeonLevel", level);
    gsm->setGameValue("DungeonX", landingPos.first);
    gsm->setGameValue("DungeonY", landingPos.second);
    revealAroundPlayer(landingPos.first, landingPos.second);
    m_visitedTiles.insert(landingPos);
    updateLocation(QString("Dungeon Level %1, (%2, %3)").arg(level).arg(landingPos.first).arg(landingPos.second));
    drawMinimap();
    logMessage(QString("You have entered **Dungeon Level %1**.").arg(level));
}

void DungeonDialog::on_attackCompanionButton_clicked() 
{
    logMessage("Attacking companions is bad.");
}

void DungeonDialog::on_carryCompanionButton_clicked() 
{
    logMessage("Carrying companions is tiring.");
}

void DungeonDialog::on_mapButton_clicked() 
{
    logMessage("You start looking at your map.");
    if (m_standaloneMinimap->isVisible()) {
        m_standaloneMinimap->hide();
    } else {
        m_standaloneMinimap->show();
        // Optional: Keep focus on the main window so the map is just an overlay
        this->activateWindow(); 
        drawMinimap(); 
    }
}

void DungeonDialog::on_pickupButton_clicked() 
{
    logMessage("You try to pickup something but is unable to.");
}

void DungeonDialog::on_dropButton_clicked() 
{
    GameStateManager* gsm = GameStateManager::instance();

    QPair<int, int> pos = {10, 10};
    m_bodyPositions.insert(pos);
    
    // Check if the player is actually carrying a body
    // Assuming "IsCarryingBody" is a flag in your GameStateManager
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
    GameStateManager* gsm = GameStateManager::instance();
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
    // If it exists, kill it so we start fresh for this test
    if (m_combatTimer) {
        m_combatTimer->stop();
        delete m_combatTimer;
    }

    m_combatTimer = new QTimer(this);
    
    // Use the Lambda to bypass all slot logic
    connect(m_combatTimer, &QTimer::timeout, [this]() {
        qDebug() << "--- ACTUAL HARDWARE TICK ---"; 
        this->processCombatTick();
    });

    m_isFighting = true;
    m_combatTimer->start(100); 
    logMessage("FORCING TIMER START...");
}
/*
void DungeonDialog::processCombatTick() 
{
    // Verification: Look at your console output!
    qDebug() << "TICK - isFighting:" << m_isFighting;

    if (!m_isFighting) return;

    // Mordor Logic: Build up energy for an attack
    // We increment by 100 because our timer runs every 100ms
    m_playerAttackCooldown += 100; 

    // Once we reach 1000ms (1 second), we swing
    if (m_playerAttackCooldown >= 1000) {
        performPlayerAttack();
        m_playerAttackCooldown = 0; // Reset for next swing
    }
}*/
void DungeonDialog::processCombatTick() 
{
    if (!m_isFighting) return;

    // --- PLAYER LOGIC ---
    m_playerAttackCooldown += 100; 
    if (m_playerAttackCooldown >= 1000) { // Player swings every 1.0s
        performPlayerAttack();
        m_playerAttackCooldown = 0;
    }

    // --- MONSTER LOGIC ---
    // Increment monster cooldown by 100ms per tick
    m_monsterAttackCooldown += 100;

    // Let's say the monster is slightly slower, attacking every 1.4 seconds
    if (m_monsterAttackCooldown >= 1400) {
        performMonsterAttack();
        m_monsterAttackCooldown = 0;
    }
}

void DungeonDialog::performPlayerAttack() {
    int damage = QRandomGenerator::global()->bounded(5, 15);
    m_activeMonsterHP -= damage;
    logMessage(QString("You hit the monster for %1 damage!").arg(damage));

    if (m_activeMonsterHP <= 0) {
        logMessage("The monster falls!");
        m_isFighting = false;
        m_isDefending = false; // Reset defense state
        m_combatTimer->stop();
        
        QPair<int, int> pos = getCurrentPosition();
        m_monsterPositions.remove(pos);
        renderWireframeView();
    }
}

void DungeonDialog::performMonsterAttack() {
    int damage = QRandomGenerator::global()->bounded(1, 10);
    updatePartyMemberHealth(0, damage);
    logMessage(QString("<font color='red'>The monster hits you for %1 damage!</font>").arg(damage));
}

// DungeonDialog.cpp
QPair<int, int> DungeonDialog::getCurrentPosition()
{
    GameStateManager* gsm = GameStateManager::instance();
    return qMakePair(gsm->getGameValue("DungeonX").toInt(), 
                     gsm->getGameValue("DungeonY").toInt());
}

void DungeonDialog::on_spellButton_clicked() 
{ 
    logMessage("You try to cast some sort of spell but fail."); 
}

void DungeonDialog::on_takeButton_clicked() 
{ 
    logMessage("You try to take something, but there is nothing there."); 
}

void DungeonDialog::on_searchButton_clicked() 
{ 
    logMessage("You carefully search the area. Found nothing."); 
}

void DungeonDialog::on_talkButton_clicked() 
{ 
    logMessage("You try talking, but the silence replies."); 
}

void DungeonDialog::on_chestButton_clicked() 
{
    processTreasureOpening();
}

void DungeonDialog::checkMonsterSpawn() {}

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
            GameStateManager* gsm = GameStateManager::instance();
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
            if (m_isFighting) {
                // Toggle defending state
                m_isDefending = !m_isDefending; 
                if (m_isDefending) {
                    logMessage("<font color='blue'>You raise your guard! (Half damage taken)</font>");
                } else {
                    logMessage("You lower your guard.");
                }
            } else {
                // Not fighting: Trigger the drop functionality
                on_dropButton_clicked();
            }
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
    GameStateManager* gsm = GameStateManager::instance();
    int constitution = gsm->getGameValue("CurrentCharacterConstitution").toInt();
    int healAmount = qMax(1, constitution / 2);    
    int currentHp = gsm->getGameValue("CurrentCharacterHP").toInt();
    int maxHp = gsm->getGameValue("MaxCharacterHP").toInt();
    int newHp = qMin(maxHp, currentHp + healAmount);
    gsm->setGameValue("CurrentCharacterHP", newHp);
    logMessage(QString("You rest and recover %1 HP.").arg(newHp - currentHp));
}

void DungeonDialog::on_stairsDownButton_clicked()
{
    transitionLevel(StairDirection::Down);
}

void DungeonDialog::on_stairsUpButton_clicked()
{
    transitionLevel(StairDirection::Up);
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
    QGraphicsView* dungeonView = findChild<QGraphicsView*>();
    if (dungeonView && dungeonView->scene() == m_dungeonScene) {
        dungeonView->fitInView(m_dungeonScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void DungeonDialog::on_exitButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Exit", "Exit to the City?", QMessageBox::Yes | QMessageBox::No
    );
    if (reply == QMessageBox::Yes) {
        handleSurfaceExit(); // Reuse the logic above
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
    GameStateManager* gsm = GameStateManager::instance();
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
        } else {
            handleSurfaceExit();
        }
    } else {
        int newLevel = currentZ + 1;
        logMessage(QString("You take the **stairs down** to Level %1.").arg(newLevel));
        enterLevel(newLevel, false);
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
    GameStateManager* gsm = GameStateManager::instance();
    QPair<int, int> pos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };
    if (m_treasurePositions.contains(pos)) {
        QString treasure = m_treasurePositions.value(pos);
        int activeIdx = gsm->getGameValue("ActiveCharacterIndex").toInt(); //

        if (treasure.contains("Gold")) {
            // Existing Gold logic...
            quint64 foundGold = QRandomGenerator::global()->bounded(500, 5000);
            quint64 currentGold = gsm->getGameValue("PlayerGold").toULongLong();
            gsm->setGameValue("PlayerGold", currentGold + foundGold);
            logMessage(QString("You gain %L1 Gold.").arg(foundGold));
        } else {
            // Add item to character inventory
            gsm->addItemToCharacter(activeIdx, treasure);
            logMessage(QString("You found a %1 and added it to your inventory!").arg(treasure));
        }
        m_treasurePositions.remove(pos);
        drawMinimap();
    }
}
void DungeonDialog::update3DView() {
    m_threeDScene->clear();
    m_threeDScene->setBackgroundBrush(Qt::black);
    QPen wirePen(Qt::green, 2); // Classic green phosphor look

    GameStateManager* gsm = GameStateManager::instance();
    int px = gsm->getGameValue("DungeonX").toInt();
    int py = gsm->getGameValue("DungeonY").toInt();
    QString facing = m_compassLabel->text(); // e.g., "Facing North"

    // Scan up to 3 tiles ahead
    for (int d = 3; d >= 0; --d) {
        int tx = px, ty = py;
        // Determine target coordinates based on facing
        if (facing.contains("North")) ty -= d;
        else if (facing.contains("South")) ty += d;
        else if (facing.contains("East")) tx += d;
        else if (facing.contains("West")) tx -= d;

        // Check for walls (Logic depends on your map data structure)
        bool hasFrontWall = isWallAt(tx, ty); 
        bool hasLeftWall = isWallAtSide(tx, ty, "left");
        bool hasRightWall = isWallAtSide(tx, ty, "right");

        drawWireframeWall(d, hasLeftWall, hasRightWall, hasFrontWall);
    }
}
void DungeonDialog::drawWireframeWall(int depth, bool left, bool right, bool front) {
    // Example coordinates for a 400x400 view
    int w = 400, h = 400;
    int inset = depth * 50; // Each step forward shrinks the box
    
    QRect currentRect(inset, inset, w - (inset * 2), h - (inset * 2));
    QRect nextRect(inset + 50, inset + 50, w - ((inset + 50) * 2), h - ((inset + 50) * 2));

    QPen pen(Qt::green, 2);

    if (front) {
        m_threeDScene->addRect(currentRect, pen);
    } else {
        // Draw floor and ceiling lines connecting current depth to next depth
        m_threeDScene->addLine(currentRect.left(), currentRect.top(), nextRect.left(), nextRect.top(), pen);
        m_threeDScene->addLine(currentRect.right(), currentRect.top(), nextRect.right(), nextRect.top(), pen);
        m_threeDScene->addLine(currentRect.left(), currentRect.bottom(), nextRect.left(), nextRect.bottom(), pen);
        m_threeDScene->addLine(currentRect.right(), currentRect.bottom(), nextRect.right(), nextRect.bottom(), pen);
    }
    if (left) {
        m_threeDScene->addLine(currentRect.left(), currentRect.top(), currentRect.left(), currentRect.bottom(), pen);
    }
    if (right) {
        m_threeDScene->addLine(currentRect.right(), currentRect.top(), currentRect.right(), currentRect.bottom(), pen);
    }
    // Repeat logic for right walls...
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

    GameStateManager *gsm = GameStateManager::instance();
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
/*
void DungeonDialog::togglePartyInfo() {
    // If it exists and is visible, close it
    if (m_charSheet) {
        m_charSheet->close();
        m_charSheet = nullptr; // Pointer is reset because WA_DeleteOnClose is set
        return;
    }

    logMessage("Opening Character Sheet...");
    m_charSheet = new PartyInfoDialog(this);

    // Set flags for floating, stay-on-top behavior
    m_charSheet->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    
    // Crucial: ensure pointer is cleared when user clicks the 'X' button
    m_charSheet->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_charSheet, &QObject::destroyed, this, [this]() { m_charSheet = nullptr; });

    m_charSheet->show();

    // Position in Bottom-Right Corner
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int dialogWidth = m_charSheet->frameGeometry().width();
        int dialogHeight = m_charSheet->frameGeometry().height();
        int padding = 10;

        m_charSheet->move(screenGeometry.width() - dialogWidth - padding,
                          screenGeometry.height() - dialogHeight - padding);
    }
    
    m_charSheet->raise();
    m_charSheet->activateWindow();
}
*/
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
