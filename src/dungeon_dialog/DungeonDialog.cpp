#include "DungeonDialog.h"
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

// Constants that depend on MAP_SIZE (which is now in the header)
const int TILE_SIZE = 10;
const int MAP_WIDTH_PIXELS = MAP_SIZE * TILE_SIZE; 
const int MAP_HEIGHT_PIXELS = MAP_SIZE * TILE_SIZE; 
const int MAP_MIN = 0;
const int MAP_MAX = MAP_SIZE - 1;

void DungeonDialog::handleWater(int x, int y)
{
    if (m_waterPositions.contains({x, y})) {
        logMessage("You get soaking wet!");
    }
}

void DungeonDialog::revealAroundPlayer(int x, int y)
{
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
void DungeonDialog::handleExtinguisher(int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_extinguisherPositions.contains(pos)) {
        GameStateManager* gsm = GameStateManager::instance();
        
        if (gsm->isCharacterOnFire()) {
            gsm->setCharacterOnFire(false);
            logMessage("Magic waters spray from the ceiling! The flames are extinguished.");
        } else {
            logMessage("A refreshing mist falls upon you.");
        }
    }
}
void DungeonDialog::handleAntimagic(int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_antimagicPositions.contains(pos)) {
        logMessage("You enter an Antimagic Field! Your spells feel suppressed.");
        // You can add logic here to disable the Spell button or set a GameState flag
    }
}

void DungeonDialog::refreshHealthUI()
{
    GameStateManager* gsm = GameStateManager::instance();
    int currentHp = gsm->getGameValue("CurrentCharacterHP").toInt();
    int maxHp = gsm->getGameValue("MaxCharacterHP").toInt();

    QTableWidgetItem *hpItem = m_partyTable->item(0, 2);
    if (hpItem) {
        hpItem->setText(QString("%1/%2").arg(currentHp).arg(maxHp));
        hpItem->setForeground(currentHp <= (maxHp * 0.2) ? Qt::red : Qt::black);
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
    
    // Only handling row 0 (Main Character) for this example
    if (row == 0) {
        int currentHp = gsm->getGameValue("CurrentCharacterHP").toInt();
        int newHp = qMax(0, currentHp - damage);
        
        gsm->setGameValue("CurrentCharacterHP", newHp);
        
        if (newHp <= 0) {
            logMessage("You have been defeated!");
        }
        
        refreshHealthUI();
    }
}

// --- Dungeon Management (Movement, Drawing, Encounters) ---

void DungeonDialog::movePlayer(int dx, int dy)
{
    GameStateManager* gsm = GameStateManager::instance();
    
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    int currentLevel = gsm->getGameValue("DungeonLevel").toInt();

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
    
    updateLocation(QString("Dungeon Level %1, (%2, %3)").arg(currentLevel).arg(newX).arg(newY));

    m_visitedTiles.insert({newX, newY});
    updateMinimap(newX, newY);
    handleEncounter(newX, newY);
    handleTreasure(newX, newY); // Now only logs chest presence
    handleTrap(newX, newY);     // Still automatic
    handleChute(newX, newY);    // NEW: Automatic hazard
    handleAntimagic(newX, newY);
    handleExtinguisher(newX, newY);
    handleWater(newX, newY);
    logMessage(QString("You move to (%1, %2).").arg(newX).arg(newY));
}

void DungeonDialog::updateCompass(const QString& direction)
{
    m_compassLabel->setText(QString("Facing %1").arg(direction));
}

void DungeonDialog::updateLocation(const QString& location)
{
    m_locationLabel->setText(location);
}


void DungeonDialog::handleEncounter(int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_monsterPositions.contains(pos)) {
        QString monster = m_monsterPositions.value(pos);
        QString attitude = m_MonsterAttitude.value(monster, "Hostile");
        logMessage(QString("You encounter a **%1**! It looks **%2**.").arg(monster).arg(attitude));
    }
}

void DungeonDialog::handleTreasure(int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_treasurePositions.contains(pos)) {
        logMessage("There is a treasure chest here! Use the Open button to see what's inside.");
    }
}

void DungeonDialog::handleTrap(int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_trapPositions.contains(pos)) {
        QString trapType = m_trapPositions.value(pos);
        
        int damage = QRandomGenerator::global()->bounded(1, 10);
        updatePartyMemberHealth(0, damage);
        
        logMessage(QString("You step on a **%1** trap and take %2 damage!").arg(trapType).arg(damage));

        m_trapPositions.remove(pos);
        drawMinimap();
    }
}

void DungeonDialog::handleChute(int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_chutePositions.contains(pos)) {
        logMessage("AAAHHH! You fall through a hidden chute!");

        // 1. Deal Fall Damage
        int fallDamage = QRandomGenerator::global()->bounded(5, 15);
        updatePartyMemberHealth(0, fallDamage); // Damage main character

        // 2. Determine New Level
        GameStateManager* gsm = GameStateManager::instance();
        int nextLevel = gsm->getGameValue("DungeonLevel").toInt() + 1;

        // 3. Trigger Level Transition
        enterLevel(nextLevel);
    }
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
    // 1. Reset all containers
    m_antimagicPositions.clear();
    m_extinguisherPositions.clear();
    m_fogPositions.clear();
    m_pitPositions.clear();
    m_rotatorPositions.clear();
    m_studPositions.clear();
    m_chutePositions.clear();
    m_monsterPositions.clear();
    m_treasurePositions.clear();
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

    // Helper B: Get ANY floor tile (Rooms OR Corridors)
    auto getAnyFloorTile = [&]() -> QPair<int, int> {
        for (int i = 0; i < 500; ++i) {
            int x = rng.bounded(MAP_SIZE);
            int y = rng.bounded(MAP_SIZE);
            QPair<int, int> p = {x, y};
            // Ensure it's not a wall, not a player, and not a staircase
            if (!m_obstaclePositions.contains(p) && p != playerPos && 
                p != m_stairsUpPosition && p != m_stairsDownPosition) {
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
        else if (roll < 45) { // 15% Traps (Can be in corridors!)
            pos = getAnyFloorTile();
            if (pos.first != -1) m_trapPositions.insert(pos, "Spike Trap");
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
    m_experienceLabel = new QLabel(this);

    m_standaloneMinimap = new MinimapDialog(this);
    connect(m_standaloneMinimap, &MinimapDialog::requestMapUpdate, this, &DungeonDialog::drawMinimap);

    setWindowTitle("Dungeon: Depth of Dejenol");
    setMinimumSize(1000, 750);
    // Load GameStateManager data
    GameStateManager* gsm = GameStateManager::instance();
    
    // RETRIEVE CHARACTER DATA FROM GAME STATE MANAGER
    m_partyLeaderName = gsm->getGameValue("CurrentCharacterName").toString();
    m_partyLeaderRace = gsm->getGameValue("CurrentCharacterRace").toString();
    m_partyLeaderAlignment = gsm->getGameValue("CurrentCharacterAlignment").toString();
    
    // Retrieve Constitution to calculate a simple placeholder for maximum HP
    int con = gsm->getGameValue("CurrentCharacterConstitution").toInt();
    int maxHP = (con > 0) ? con * 5 : 50; 

    if (m_partyLeaderName.isEmpty()) {
        m_partyLeaderName = "Unnamed Hero";
        m_partyLeaderRace = "Human";
        m_partyLeaderAlignment = "Neutral";
        maxHP = 50;
        logMessage("WARNING: No character data found in GameState. Using defaults.");
    }
    
    // --- Retrieve/Set persistent dungeon state (New GameState logic) ---
    int initialLevel = gsm->getGameValue("DungeonLevel").toInt();
    int initialX = gsm->getGameValue("DungeonX").toInt();
    int initialY = gsm->getGameValue("DungeonY").toInt();
    quint64 initialGold = gsm->getGameValue("PlayerGold").toULongLong();

    // Set defaults if state data is missing (e.g., first time entering)
    if (initialLevel == 0) {
        initialLevel = 1;
        initialX = MAP_SIZE / 2;
        initialY = MAP_SIZE / 2;
        initialGold = 123456789; // Using the original default value
        
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
    // 1. Party Status Table
    QGroupBox *partyBox = new QGroupBox("Party Status");
    QVBoxLayout *partyLayout = new QVBoxLayout(partyBox);
    
    m_partyTable = new QTableWidget(1, 4); // This member is now confirmed in the header
    // 1. Party Status Table - Change Height to Minimum/Preferred
    m_partyTable->setMinimumHeight(70); 
    m_partyTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_partyTable->setHorizontalHeaderLabels({"Name", "Race", "HP", "Status"});
    m_partyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_partyTable->verticalHeader()->setVisible(false);
    m_partyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //m_partyTable->setFixedHeight(70); 

    // Insert the character data retrieved from Game State
    m_partyTable->setItem(0, 0, new QTableWidgetItem(m_partyLeaderName));
    m_partyTable->setItem(0, 1, new QTableWidgetItem(m_partyLeaderRace));
    m_partyTable->setItem(0, 2, new QTableWidgetItem(QString("%1/%1").arg(maxHP)));
    m_partyTable->setItem(0, 3, new QTableWidgetItem("Healthy"));

    partyLayout->addWidget(m_partyTable);
    leftPanelLayout->addWidget(partyBox);

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
    QLabel *alignmentLabel = new QLabel(QString("Alignment: **%1**").arg(m_partyLeaderAlignment));
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
    logMessage(QString("Welcome, **%1** (the %2 %3), to the Dungeon!").arg(m_partyLeaderName).arg(m_partyLeaderAlignment).arg(m_partyLeaderRace));
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
    GameStateManager* gsm = GameStateManager::instance();

    // 1. Generate the map using Room-and-Corridor logic
    // Seed by level to ensure the layout is deterministic
    QRandomGenerator levelRng(level + 12345);

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

void DungeonDialog::on_attackCompanionButton_clicked() { logMessage("Attacking companions is bad."); }
void DungeonDialog::on_carryCompanionButton_clicked() { logMessage("Carrying companions is tiring."); }

void DungeonDialog::on_mapButton_clicked() {
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
void DungeonDialog::on_pickupButton_clicked() { logMessage("You try to pickup something but is unable to."); }
void DungeonDialog::on_dropButton_clicked() { logMessage("You try to drop something but is unable to."); }

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
    handleEncounter(newX, newY);
    handleTreasure(newX, newY);
    handleTrap(newX, newY);

    // Emit the signal defined in the header
    emit teleporterUsed();
}

void DungeonDialog::on_fightButton_clicked() 
{ 
    // UPDATED: Get position from GameState
    GameStateManager* gsm = GameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> currentPos = {currentX, currentY};

    if (m_monsterPositions.contains(currentPos)) {
        logMessage(QString("You engage the %1 in combat!").arg(m_monsterPositions.value(currentPos)));
    } else {
        logMessage("You swing your weapon at empty air.");
    }
}
void DungeonDialog::on_spellButton_clicked() { logMessage("You try to cast some sort of spell but fail."); }
void DungeonDialog::on_takeButton_clicked() { logMessage("You try to take something, but there is nothing there."); }

void DungeonDialog::on_searchButton_clicked() { logMessage("You carefully search the area. Found nothing."); }
void DungeonDialog::on_talkButton_clicked() { logMessage("You try talking, but the silence replies."); }
void DungeonDialog::on_chestButton_clicked() {
    GameStateManager* gsm = GameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> pos = {currentX, currentY};

    if (m_treasurePositions.contains(pos)) {
        QString treasure = m_treasurePositions.value(pos);
        logMessage(QString("You open the chest and find: %1!").arg(treasure));

        if (treasure.contains("Gold")) {
            quint64 foundGold = QRandomGenerator::global()->bounded(500, 5000);
            quint64 currentGold = gsm->getGameValue("PlayerGold").toULongLong();
            gsm->setGameValue("PlayerGold", currentGold + foundGold);

            updateGoldLabel();
            logMessage(QString("You gain %L1 Gold.").arg(foundGold));
        }

        // Remove from map after opening
        m_treasurePositions.remove(pos);
        drawMinimap();
    } else {
        logMessage("There is nothing here to open.");
    }
}

void DungeonDialog::checkMonsterSpawn() {}
void DungeonDialog::initiateFight() {}
void DungeonDialog::on_winBattle_trigger() {}

void DungeonDialog::onEventTriggered(const GameEvent& ) { /* handle event */ }

void DungeonDialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        // --- Movement (WASD) ---
        case Qt::Key_W:
            moveForward();
            break;
        case Qt::Key_S:
            moveBackward();
            break;
        case Qt::Key_A:
            moveStepLeft();
            break;
        case Qt::Key_D:
            moveStepRight();
            break;

        // --- Rotation (QE) ---
        case Qt::Key_Q:
            on_rotateLeftButton_clicked();
            break;
        case Qt::Key_E:
            on_rotateRightButton_clicked();
            break;

        // --- Other Action Shortcuts ---
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

        // Fall back to base class for default dialog behavior (like Esc to close)
        default:
            QDialog::keyPressEvent(event);
            break;
    }
}

// Suppress unused parameter warnings
void DungeonDialog::spawnMonsters(const QString& , int ) { /* spawn logic */ }

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
    refreshHealthUI();
}

void DungeonDialog::on_stairsDownButton_clicked()
{
    GameStateManager* gsm = GameStateManager::instance();
    // UPDATED: Get position and level from GameState
    int currentLevel = gsm->getGameValue("DungeonLevel").toInt();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> currentPos = {currentX, currentY};

    if (currentPos == m_stairsDownPosition) {
        int newLevel = currentLevel + 1;
        logMessage(QString("You take the **stairs down** to Level %1.").arg(newLevel));
        enterLevel(newLevel);
    } else {
        logMessage("There are no stairs here to take.");
    }
}

void DungeonDialog::on_stairsUpButton_clicked()
{
    GameStateManager* gsm = GameStateManager::instance();
    // UPDATED: Get position and level from GameState
    int currentLevel = gsm->getGameValue("DungeonLevel").toInt();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> currentPos = {currentX, currentY};
    
    if (currentPos == m_stairsUpPosition) {
        int newLevel = currentLevel - 1;
        if (newLevel >= 1) {
            logMessage(QString("You take the **stairs up** to Level %1.").arg(newLevel));
            enterLevel(newLevel, true);
        } else {
            // Current level is 1: ascend to surface/city
            logMessage("You ascend the **stairs up** and step onto the surface!");
            GameStateManager::instance()->setGameValue("GhostHoundPending", true);
            // --- RANDOM MONSTER CAPTURE LOGIC ---
            // Check if we have monster data loaded in the manager
            if (gsm->monsterCount() > 0) {
                // Pick a random index based on the size of the monster list
                int randomIndex = QRandomGenerator::global()->bounded(gsm->monsterCount());
                
                // Get the monster name from the random data entry
                QVariantMap monster = gsm->getMonster(randomIndex);
                QString monsterName = monster["name"].toString();

                if (!monsterName.isEmpty()) {
                    // Add the monster to the permanent Confinement stock
                    gsm->incrementStock(monsterName);
                    
                    // Optional: Provide feedback to the player
                    //logMessage("You captured a " + monsterName + " while escaping!");
                    qDebug() << "You captured a " + monsterName + " while escaping! ";
                }
            }
            emit exitedDungeonToCity();
            this->close(); 
        }
    } else {
        logMessage("There are no stairs here to take.");
    }
}

void DungeonDialog::on_openButton_clicked()
{
    GameStateManager* gsm = GameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> pos = {currentX, currentY};

    if (m_treasurePositions.contains(pos)) {
        QString treasure = m_treasurePositions.value(pos);
        logMessage(QString("You open the chest and find: %1!").arg(treasure));

        if (treasure.contains("Gold")) {
            quint64 foundGold = QRandomGenerator::global()->bounded(500, 5000);
            quint64 currentGold = gsm->getGameValue("PlayerGold").toULongLong();
            gsm->setGameValue("PlayerGold", currentGold + foundGold);
            
            updateGoldLabel();
            logMessage(QString("You gain %L1 Gold.").arg(foundGold));
        }

        // Remove from map after opening
        m_treasurePositions.remove(pos);
        drawMinimap();
    } else {
        logMessage("There is nothing here to open.");
    }
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
        this,
        "Exit",
        "Exit to main menu?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        qDebug() << "User clicked Yes. Application should exit or return to main menu.";
        this->close();
    }
}
void DungeonDialog::on_rotateLeftButton_clicked()
{
    QStringList directions = {"Facing North", "Facing West", "Facing South", "Facing East"};
    int currentIndex = directions.indexOf(m_compassLabel->text());
    int nextIndex = (currentIndex + 1) % 4;
    
    QString newDir = directions[nextIndex].mid(7); // Extract "North", "West", etc.
    updateCompass(newDir);
    logMessage(QString("You turn to the left."));
    drawMinimap();
}

void DungeonDialog::on_rotateRightButton_clicked()
{
    QStringList directions = {"Facing North", "Facing East", "Facing South", "Facing West"};
    int currentIndex = directions.indexOf(m_compassLabel->text());
    int nextIndex = (currentIndex + 1) % 4;
    
    QString newDir = directions[nextIndex].mid(7);
    updateCompass(newDir);
    logMessage(QString("You turn to the right."));
    drawMinimap();
}
void DungeonDialog::moveForward()
{
    QString currentFacing = m_compassLabel->text();

    if (currentFacing == "Facing North") movePlayer(0, -1);
    else if (currentFacing == "Facing South") movePlayer(0, 1);
    else if (currentFacing == "Facing East") movePlayer(1, 0);
    else if (currentFacing == "Facing West") movePlayer(-1, 0);
}

void DungeonDialog::moveBackward()
{
    QString currentFacing = m_compassLabel->text();

    // Backward is the inverse of the forward vector
    if (currentFacing == "Facing North") movePlayer(0, 1);
    else if (currentFacing == "Facing South") movePlayer(0, -1);
    else if (currentFacing == "Facing East") movePlayer(-1, 0);
    else if (currentFacing == "Facing West") movePlayer(1, 0);
}
void DungeonDialog::moveStepLeft()
{
    QString currentFacing = m_compassLabel->text();

    if (currentFacing == "Facing North")      movePlayer(-1, 0); // West
    else if (currentFacing == "Facing South") movePlayer(1, 0);  // East
    else if (currentFacing == "Facing East")  movePlayer(0, -1); // North
    else if (currentFacing == "Facing West")  movePlayer(0, 1);  // South
}

void DungeonDialog::moveStepRight()
{
    QString currentFacing = m_compassLabel->text();

    if (currentFacing == "Facing North")      movePlayer(1, 0);  // East
    else if (currentFacing == "Facing South") movePlayer(-1, 0); // West
    else if (currentFacing == "Facing East")  movePlayer(0, 1);  // South
    else if (currentFacing == "Facing West")  movePlayer(0, -1); // North
}
