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
            logMessage("<font color='red'><b>You have been defeated!</b></font>");
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

    gsm->setGameValue("DungeonX", newX);
    gsm->setGameValue("DungeonY", newY);
    
    updateLocation(QString("Dungeon Level %1, (%2, %3)").arg(currentLevel).arg(newX).arg(newY));

//    if (dy < 0) updateCompass("North");
//    else if (dy > 0) updateCompass("South");
//    else if (dx < 0) updateCompass("West");
//    else if (dx > 0) updateCompass("East");

    m_visitedTiles.insert({newX, newY});
    updateMinimap(newX, newY);
    handleEncounter(newX, newY);
    handleTreasure(newX, newY); // Now only logs chest presence
    handleTrap(newX, newY);     // Still automatic
    handleChute(newX, newY);    // NEW: Automatic hazard
    
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
        logMessage("There is a <b>treasure chest</b> here! Use the <b>Open</b> button to see what's inside.");
    }
}

void DungeonDialog::handleTrap(int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_trapPositions.contains(pos)) {
        QString trapType = m_trapPositions.value(pos);
        
        int damage = QRandomGenerator::global()->bounded(1, 10);
        updatePartyMemberHealth(0, damage); // This is now declared/implemented correctly.
        
        logMessage(QString("You step on a **%1** trap and take %2 damage!").arg(trapType).arg(damage));

        m_trapPositions.remove(pos);
        drawMinimap();
    }
}

void DungeonDialog::handleChute(int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_chutePositions.contains(pos)) {
        logMessage("<font color='orange'><b>AAAHHH! You fall through a hidden chute!</b></font>");

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
    QString waterPath = "resources/images/minimap/water.png";

    QPixmap fogPixmap(fogPath);
    // Check if the image loaded correctly for debugging
    if (fogPixmap.isNull()) {
        qDebug() << "Error: Fog image not found at" << fogPath;
    }

    // Scale the image once to 10x10 pixels (TILE_SIZE)
    QPixmap scaledFog = fogPixmap.scaled(TILE_SIZE, TILE_SIZE, 
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

    // 1. Draw grid lines
    for (int i = 0; i <= MAP_SIZE; ++i) {
        scene->addLine(i * TILE_SIZE, 0, i * TILE_SIZE, MAP_HEIGHT_PIXELS, QPen(Qt::lightGray));
        scene->addLine(0, i * TILE_SIZE, MAP_WIDTH_PIXELS, i * TILE_SIZE, QPen(Qt::lightGray));
    }

    // 2. Draw Obstacles (Walls)
    for (const auto& pos : m_obstaclePositions) {
        // Walls are visible only if the tile has been visited
        if (m_visitedTiles.contains(pos)) {
            scene->addRect(pos.first * TILE_SIZE, pos.second * TILE_SIZE, 
                           TILE_SIZE, TILE_SIZE, QPen(Qt::black), QBrush(Qt::darkGray));
        }
    }

    // 3. Draw Stairs (Cyan)
    if (m_visitedTiles.contains(m_stairsUpPosition)) {
        scene->addRect(m_stairsUpPosition.first * TILE_SIZE, m_stairsUpPosition.second * TILE_SIZE, 
                       TILE_SIZE, TILE_SIZE, QPen(Qt::black), QBrush(Qt::cyan));
    }
    if (m_visitedTiles.contains(m_stairsDownPosition)) {
        scene->addRect(m_stairsDownPosition.first * TILE_SIZE, m_stairsDownPosition.second * TILE_SIZE, 
                       TILE_SIZE, TILE_SIZE, QPen(Qt::black), QBrush(Qt::cyan));
    }

    // 4. Draw Chutes (Only if visited)
    for (const auto& pos : m_chutePositions) {
        if (m_visitedTiles.contains(pos)) {
            scene->addEllipse(pos.first * TILE_SIZE + 1, pos.second * TILE_SIZE + 1, 
                              TILE_SIZE - 2, TILE_SIZE - 2, QPen(Qt::gray), QBrush(Qt::black));
        }
    }

    // 5. Draw Monsters (Only if visited)
    for (const auto& pos : m_monsterPositions.keys()) {
        if (m_visitedTiles.contains(pos)) {
            scene->addEllipse(pos.first * TILE_SIZE + TILE_SIZE/4, pos.second * TILE_SIZE + TILE_SIZE/4, 
                              TILE_SIZE/2, TILE_SIZE/2, QPen(Qt::red), QBrush(Qt::red));
        }
    }

    // 6. Draw Treasure/Chests (Only if visited)
    for (const auto& pos : m_treasurePositions.keys()) {
        if (m_visitedTiles.contains(pos)) {
            scene->addRect(pos.first * TILE_SIZE + TILE_SIZE/4, pos.second * TILE_SIZE + TILE_SIZE/4, 
                           TILE_SIZE/2, TILE_SIZE/2, QPen(Qt::black), QBrush(Qt::yellow));
        }
    }
    
    // 7. Draw Traps (Only if visited)
    for (const auto& pos : m_trapPositions.keys()) {
        if (m_visitedTiles.contains(pos)) {
            scene->addRect(pos.first * TILE_SIZE + TILE_SIZE/4, pos.second * TILE_SIZE + TILE_SIZE/4, 
                           TILE_SIZE/2, TILE_SIZE/2, QPen(Qt::black), QBrush(Qt::darkGreen));
        }
    }

    // 8. Draw Fog of War Overlay
    for (int x = 0; x < MAP_SIZE; ++x) { //
        for (int y = 0; y < MAP_SIZE; ++y) { //
            if (!m_visitedTiles.contains({x, y})) { //
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
 /*
    for (int x = 0; x < MAP_SIZE; ++x) {
        for (int y = 0; y < MAP_SIZE; ++y) {
            if (!m_visitedTiles.contains({x, y})) {
                // Draw a semi-transparent black square over unvisited tiles
                scene->addRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, 
                               Qt::NoPen, QBrush(QColor(0, 0, 0, 220))); 
            }
        }
    }
*/
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

    // Finalize the view
    m_miniMapView->setScene(scene);
    m_miniMapView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void DungeonDialog::updateMinimap(int x, int y)
{
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

void DungeonDialog::generateRandomObstacles(int obstacleCount)
{
    m_obstaclePositions.clear(); // This member is now confirmed in the header
    for (int i = 0; i < obstacleCount; ++i) {
        int x = QRandomGenerator::global()->bounded(MAP_SIZE);
        int y = QRandomGenerator::global()->bounded(MAP_SIZE);
        m_obstaclePositions.insert({x, y});
    }
    
    // UPDATED: Get player position from GameState
    GameStateManager* gsm = GameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    m_obstaclePositions.remove({currentX, currentY});
}

void DungeonDialog::generateStairs()
{
    GameStateManager* gsm = GameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> currentPos = {currentX, currentY};
    
    do {
        m_stairsUpPosition = {QRandomGenerator::global()->bounded(MAP_SIZE), QRandomGenerator::global()->bounded(MAP_SIZE)};
    } while (m_stairsUpPosition == currentPos || m_obstaclePositions.contains(m_stairsUpPosition));

    do {
        m_stairsDownPosition = {QRandomGenerator::global()->bounded(MAP_SIZE), QRandomGenerator::global()->bounded(MAP_SIZE)};
    } while (m_stairsDownPosition == currentPos || m_stairsDownPosition == m_stairsUpPosition || m_obstaclePositions.contains(m_stairsDownPosition));
    
    m_obstaclePositions.remove(m_stairsUpPosition);
    m_obstaclePositions.remove(m_stairsDownPosition);
}

void DungeonDialog::generateSpecialTiles(int tileCount)
{
    m_antimagicPositions.clear();
    m_extinguisherPositions.clear();
    m_fogPositions.clear();
    m_pitPositions.clear();
    m_rotatorPositions.clear();
    m_studPositions.clear();
    m_chutePositions.clear(); // Ensure this is cleared for the new level
    m_monsterPositions.clear();
    m_treasurePositions.clear();
    m_trapPositions.clear();
    m_MonsterAttitude.clear(); 

    m_MonsterAttitude["Kobold"] = "Wary";
    m_MonsterAttitude["Orc"] = "Hostile";
    m_MonsterAttitude["Giant Spider"] = "Aggressive";

    GameStateManager* gsm = GameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> currentPos = {currentX, currentY};

    for (int i = 0; i < tileCount; ++i) {
        int x, y;
        QPair<int, int> pos;
        do {
            x = QRandomGenerator::global()->bounded(MAP_SIZE);
            y = QRandomGenerator::global()->bounded(MAP_SIZE);
            pos = {x, y};
            // Ensure we don't spawn things on walls, the player, or stairs
        } while (m_obstaclePositions.contains(pos) || pos == currentPos || 
                 m_stairsUpPosition == pos || m_stairsDownPosition == pos);

        // Increased bounded range to 4 to include Chutes
        int type = QRandomGenerator::global()->bounded(4); 

        if (type == 0) {
            QStringList monsters = {"Kobold", "Orc", "Giant Spider"};
            m_monsterPositions.insert(pos, monsters.at(QRandomGenerator::global()->bounded(monsters.size())));
        } else if (type == 1) {
            QStringList treasures = {"Small Pouch of Gold", "Rusty Key", "Flickering Torch"};
            m_treasurePositions.insert(pos, treasures.at(QRandomGenerator::global()->bounded(treasures.size())));
        } else if (type == 2) {
            QStringList traps = {"Spike Trap", "Poison Gas Trap", "Net Trap"};
            m_trapPositions.insert(pos, traps.at(QRandomGenerator::global()->bounded(traps.size())));
        } else {
            // NEW: Add to chute positions
            m_chutePositions.insert(pos);
        }
    }
}

// --- Constructor Implementation ---
DungeonDialog::DungeonDialog(QWidget *parent)
    : QDialog(parent),
      m_dungeonScene(new QGraphicsScene(this))
{
    setWindowTitle("Dungeon: Depth of Dejenol");
    setFixedSize(1000, 750); 
    
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
    
    // --- Left Panel: Party, Map, Log ---
    QVBoxLayout *leftPanelLayout = new QVBoxLayout();
    
    // 1. Party Status Table
    QGroupBox *partyBox = new QGroupBox("Party Status");
    QVBoxLayout *partyLayout = new QVBoxLayout(partyBox);
    
    m_partyTable = new QTableWidget(1, 4); // This member is now confirmed in the header
    m_partyTable->setHorizontalHeaderLabels({"Name", "Race", "HP", "Status"});
    m_partyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_partyTable->verticalHeader()->setVisible(false);
    m_partyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_partyTable->setFixedHeight(70); 

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
    dungeonView->setFixedSize(600, 350); 
    leftPanelLayout->addWidget(dungeonView);
    
    // 3. Message Log
    QGroupBox *logBox = new QGroupBox("Adventure Log");
    QVBoxLayout *logLayout = new QVBoxLayout(logBox);
    m_messageLog = new QListWidget();
    logLayout->addWidget(m_messageLog);
    leftPanelLayout->addWidget(logBox, 1); 
    
    rootLayout->addLayout(leftPanelLayout, 1); 
    
    // --- Right Panel: Info, Compass, Minimap, Controls ---
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();
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

    // 2. Gold Display
    m_goldLabel = new QLabel();
    updateGoldLabel(); // Call to update label with GameState value
    rightPanelLayout->addWidget(m_goldLabel);

    // 3. Mini Map View
    QGroupBox *minimapBox = new QGroupBox("Mini Map");
    QVBoxLayout *minimapLayout = new QVBoxLayout(minimapBox);
    m_miniMapView = new QGraphicsView();
    m_miniMapView->setFixedSize(MAP_WIDTH_PIXELS + 2, MAP_HEIGHT_PIXELS + 2); 
    minimapLayout->addWidget(m_miniMapView);
    rightPanelLayout->addWidget(minimapBox);
    
    // Initialize map state and draw the initial view
    generateRandomObstacles(50); 
    generateStairs();
    generateSpecialTiles(10);
    drawMinimap(); 
    
    // 4. Action Buttons
    QGridLayout *actionLayout = new QGridLayout();
    
    QPushButton *fightButton = new QPushButton("Fight (F)");
    QPushButton *spellButton = new QPushButton("Spell (S)");
    QPushButton *restButton = new QPushButton("Rest (R)");
    QPushButton *talkButton = new QPushButton("Talk (T)");
    QPushButton *searchButton = new QPushButton("Search (Z)");
    QPushButton *pickupButton = new QPushButton("Pickup (P)");
    QPushButton *dropButton = new QPushButton("Drop (D)");
    QPushButton *openButton = new QPushButton("Open (O)");
    QPushButton *mapButton = new QPushButton("Map (M)");
    m_chestButton = new QPushButton("Chest (C)"); 
    QPushButton *exitButton = new QPushButton("Exit");
    QPushButton *teleportButton = new QPushButton("Teleport (U)"); // Add this line
    
    actionLayout->addWidget(fightButton, 0, 0);
    actionLayout->addWidget(spellButton, 0, 1);
    actionLayout->addWidget(restButton, 0, 2);
    actionLayout->addWidget(talkButton, 1, 0);
    actionLayout->addWidget(searchButton, 1, 1);
    actionLayout->addWidget(pickupButton, 1, 2);
    actionLayout->addWidget(dropButton, 2, 0);
    actionLayout->addWidget(openButton, 2, 1);
    actionLayout->addWidget(mapButton, 2, 2);
    actionLayout->addWidget(m_chestButton, 3, 0);
    actionLayout->addWidget(teleportButton, 3, 1); // Add to the grid at row 3, column 1
    actionLayout->addWidget(exitButton, 3, 2);

    rightPanelLayout->addLayout(actionLayout);

    // 5. Directional Buttons (Movement)
    QGroupBox *moveBox = new QGroupBox("Movement");
    QGridLayout *moveLayout = new QGridLayout(moveBox);
    
    QPushButton *upButton = new QPushButton("^");
    QPushButton *downButton = new QPushButton("v");
    QPushButton *leftButton = new QPushButton("<");
    QPushButton *rightButton = new QPushButton(">");
    // Rotate
    QPushButton *rotateLeftButton = new QPushButton("Rotate L");
    QPushButton *rotateRightButton = new QPushButton("Rotate R");

    moveLayout->addWidget(rotateLeftButton, 0, 0);
    moveLayout->addWidget(upButton, 0, 1);
    moveLayout->addWidget(rotateRightButton, 0, 2);
    moveLayout->addWidget(leftButton, 1, 0);
    moveLayout->addWidget(downButton, 1, 1);
    moveLayout->addWidget(rightButton, 1, 2);
    
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
    connect(upButton, &QPushButton::clicked, this, &DungeonDialog::moveForward);
    connect(downButton, &QPushButton::clicked, this, &DungeonDialog::moveBackward);
    //connect(upButton, &QPushButton::clicked, [this](){ movePlayer(0, -1); });
    //connect(downButton, &QPushButton::clicked, [this](){ movePlayer(0, 1); });
    connect(leftButton, &QPushButton::clicked, this, &DungeonDialog::moveStepLeft);  // Updated
    connect(rightButton, &QPushButton::clicked, this, &DungeonDialog::moveStepRight); // Updated
    //connect(leftButton, &QPushButton::clicked, [this](){ movePlayer(-1, 0); });
    //connect(rightButton, &QPushButton::clicked, [this](){ movePlayer(1, 0); });
    connect(rotateLeftButton, &QPushButton::clicked, this, &DungeonDialog::on_rotateLeftButton_clicked);
    connect(rotateRightButton, &QPushButton::clicked, this, &DungeonDialog::on_rotateRightButton_clicked);
    
    // Connections (Actions)
    connect(fightButton, &QPushButton::clicked, this, &DungeonDialog::on_fightButton_clicked);
    connect(spellButton, &QPushButton::clicked, this, &DungeonDialog::on_spellButton_clicked);
    connect(restButton, &QPushButton::clicked, this, &DungeonDialog::on_restButton_clicked);
    connect(talkButton, &QPushButton::clicked, this, &DungeonDialog::on_talkButton_clicked);
    connect(searchButton, &QPushButton::clicked, this, &DungeonDialog::on_searchButton_clicked);
    connect(pickupButton, &QPushButton::clicked, this, &DungeonDialog::on_pickupButton_clicked);
    connect(dropButton, &QPushButton::clicked, this, &DungeonDialog::on_dropButton_clicked);
    connect(openButton, &QPushButton::clicked, this, &DungeonDialog::on_openButton_clicked);
    connect(mapButton, &QPushButton::clicked, this, &DungeonDialog::on_mapButton_clicked);
    connect(m_chestButton, &QPushButton::clicked, this, &DungeonDialog::on_chestButton_clicked);
    connect(exitButton, &QPushButton::clicked, this, &DungeonDialog::on_exitButton_clicked);
    connect(teleportButton, &QPushButton::clicked, this, &DungeonDialog::on_teleportButton_clicked);
    // Connections (Stairs)
    connect(stairsDownButton, &QPushButton::clicked, this, &DungeonDialog::on_stairsDownButton_clicked);
    connect(stairsUpButton, &QPushButton::clicked, this, &DungeonDialog::on_stairsUpButton_clicked);
}

DungeonDialog::~DungeonDialog()
{
}

void DungeonDialog::enterLevel(int level)
{
    m_visitedTiles.clear(); // Wipe map memory for the new floor
    GameStateManager* gsm = GameStateManager::instance();
    int startX = MAP_SIZE / 2; //
    int startY = MAP_SIZE / 2; //
    
    m_visitedTiles.insert({startX, startY}); // Mark starting point as seen

    // UPDATED: SAVE new level and reset position to GameState
    gsm->setGameValue("DungeonLevel", level);
    gsm->setGameValue("DungeonX", MAP_SIZE / 2);
    gsm->setGameValue("DungeonY", MAP_SIZE / 2);
    
    int newX = MAP_SIZE / 2;
    int newY = MAP_SIZE / 2;

    // Regenerate the dungeon map for the new level
    generateRandomObstacles(50 + level * 5); 
    generateStairs();
    generateSpecialTiles(10 + level * 2);

    updateLocation(QString("Dungeon Level %1, (%2, %3)").arg(level).arg(newX).arg(newY));
    updateCompass("North");
    drawMinimap();

    logMessage(QString("You have entered **Dungeon Level %1**.").arg(level));
}

void DungeonDialog::on_attackCompanionButton_clicked() { logMessage("Attacking companions is bad."); }
void DungeonDialog::on_carryCompanionButton_clicked() { logMessage("Carrying companions is tiring."); }

void DungeonDialog::on_mapButton_clicked() { logMessage("Map button clicked!"); }
void DungeonDialog::on_pickupButton_clicked() { logMessage("Pickup button clicked!"); }
void DungeonDialog::on_dropButton_clicked() { logMessage("Drop button clicked!"); }

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
    logMessage(QString("<font color='purple'><b>A mystical force teleports you to (%1, %2)!</b></font>")
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
void DungeonDialog::on_spellButton_clicked() { logMessage("Spell button clicked!"); }
void DungeonDialog::on_takeButton_clicked() { logMessage("Take button clicked!"); }

void DungeonDialog::on_searchButton_clicked() { logMessage("You carefully search the area. Found nothing."); }
void DungeonDialog::on_talkButton_clicked() { logMessage("You try talking, but the silence replies."); }
void DungeonDialog::on_chestButton_clicked() { logMessage("Chest opened!"); } 

void DungeonDialog::checkMonsterSpawn() {}
void DungeonDialog::initiateFight() {}
void DungeonDialog::on_winBattle_trigger() {}

// Suppress unused parameter warning
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
            enterLevel(newLevel);
        } else {
            // Current level is 1: ascend to surface/city
            logMessage("You ascend the **stairs up** and step onto the surface!");
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
        logMessage(QString("<font color='gold'>You open the chest and find: <b>%1</b>!</font>").arg(treasure));

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
