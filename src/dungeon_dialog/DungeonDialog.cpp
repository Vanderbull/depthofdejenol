#include "DungeonDialog.h"
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
#include <QJsonObject> // Needed for EventManager

const int MAP_SIZE = 30;
const int TILE_SIZE = 10;
const int MAP_WIDTH_PIXELS = MAP_SIZE * TILE_SIZE; // 300
const int MAP_HEIGHT_PIXELS = MAP_SIZE * TILE_SIZE; // 300
const int MAP_MIN = 0;
const int MAP_MAX = MAP_SIZE - 1;

// --- Helper Function ---
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
    if (m_goldLabel) {
        // Use QStringLiteral("%L1") for locale-aware number formatting (e.g., adding commas)
        QString goldString = QStringLiteral("%L1").arg(m_currentGold);
        m_goldLabel->setText(QString("You have a total of %1 Gold").arg(goldString));
    }
}

// --- Health Management Helper Function ---
void DungeonDialog::updatePartyMemberHealth(int row, int damage)
{
    if (!m_partyTable || row < 0 || row >= m_partyTable->rowCount()) {
        return;
    }

    QTableWidgetItem *healthItem = m_partyTable->item(row, 1); // Column 1 is "Hits"
    QTableWidgetItem *nameItem = m_partyTable->item(row, 0);   // Column 0 is "Name"
    if (!healthItem || !nameItem) {
        return;
    }

    QString currentHealthText = healthItem->text(); 
    QStringList parts = currentHealthText.split('/');
    
    if (parts.size() != 2) return; 

    int currentHP = parts[0].toInt();
    int maxHP = parts[1].toInt();
    QString memberName = nameItem->text();

    int newHP = qMax(0, currentHP - damage);
    
    // Update the table item with new health
    healthItem->setText(QString("%1/%2").arg(newHP).arg(maxHP));

    // Log the damage taken
    if (newHP > 0) {
        logMessage(QString("    - **%1** takes **%2 damage** (%3/%4 HP remaining).").arg(memberName).arg(damage).arg(newHP).arg(maxHP));
    } else {
        logMessage(QString("    - **%1** takes **%2 damage** and is **DEFEATED**!").arg(memberName).arg(damage));
    }
    
    // Update Status (column 3 is "Status")
    QTableWidgetItem *statusItem = m_partyTable->item(row, 3);
    if (!statusItem) {
        statusItem = new QTableWidgetItem();
        m_partyTable->setItem(row, 3, statusItem);
    }

    if (newHP <= 0) {
        statusItem->setText("Defeated");
    } else if (statusItem->text() == "Defeated" && newHP > 0) {
        statusItem->setText(""); 
    }
}

// --- Obstacle Generation ---
void DungeonDialog::generateRandomObstacles(int obstacleCount)
{
    m_obstaclePositions.clear();
    while (m_obstaclePositions.size() < obstacleCount) {
        int ox = QRandomGenerator::global()->bounded(MAP_SIZE);
        int oy = QRandomGenerator::global()->bounded(MAP_SIZE);
        // Ensure obstacles are not on the player's current (or intended starting) position
        if (ox != m_playerMapX || oy != m_playerMapY) {
            m_obstaclePositions.insert(qMakePair(ox, oy));
        }
    }
}

// --- Stairs Generation ---
void DungeonDialog::generateStairs()
{
    // Clear previous positions
    m_stairsUpPosition = qMakePair(-1, -1);
    m_stairsDownPosition = qMakePair(-1, -1);

    auto getRandomValidPos = [this]() {
        int x, y;
        QPair<int, int> pos;
        do {
            x = QRandomGenerator::global()->bounded(MAP_SIZE);
            y = QRandomGenerator::global()->bounded(MAP_SIZE);
            pos = qMakePair(x, y);
        // Ensure stairs are not on obstacles and not on each other
        } while (m_obstaclePositions.contains(pos) || pos == m_stairsUpPosition || pos == m_stairsDownPosition);
        return pos;
    };

    // Generate down stairs
    m_stairsDownPosition = getRandomValidPos();

    // Generate up stairs
    m_stairsUpPosition = getRandomValidPos();

    logMessage(QString("Level %1 map generated: Stairs Down at (%2, %3), Stairs Up at (%4, %5)").arg(m_currentLevel).arg(m_stairsDownPosition.first).arg(m_stairsDownPosition.second).arg(m_stairsUpPosition.first).arg(m_stairsUpPosition.second));
}

// --- Special Tile Generation (Antimagic, Extinguisher, etc.) ---
void DungeonDialog::generateSpecialTiles(int tileCount) 
{
    // Clear all special tile sets
    m_antimagicPositions.clear();
    m_extinguisherPositions.clear();
    m_fogPositions.clear();
    m_pitPositions.clear();
    m_rotatorPositions.clear();
    m_studPositions.clear();
    m_chutePositions.clear();
    m_teleportPositions.clear();
    m_waterPositions.clear();

    QList<QSet<QPair<int, int>>*> tileSets;
    tileSets << &m_antimagicPositions 
             << &m_extinguisherPositions
             << &m_fogPositions
             << &m_pitPositions
             << &m_rotatorPositions
             << &m_studPositions
             << &m_chutePositions
             << &m_teleportPositions
             << &m_waterPositions;

    // Loop through each tile type and add 'tileCount' random, non-overlapping positions
    for (QSet<QPair<int, int>>* currentSet : tileSets) {
        while (currentSet->size() < tileCount) {
            int x = QRandomGenerator::global()->bounded(MAP_SIZE);
            int y = QRandomGenerator::global()->bounded(MAP_SIZE);
            QPair<int, int> pos = qMakePair(x, y);

            // Check for overlaps with player start, obstacles, stairs, and all other special tile sets
            bool isOverlap = (x == m_playerMapX && y == m_playerMapY) ||
                             m_obstaclePositions.contains(pos) ||
                             pos == m_stairsDownPosition ||
                             pos == m_stairsUpPosition;

            // Check against all *existing* tile positions across all sets
            for (QSet<QPair<int, int>>* existingSet : tileSets) {
                if (existingSet->contains(pos)) {
                    isOverlap = true;
                    break;
                }
            }

            if (!isOverlap) {
                currentSet->insert(pos);
            }
        }
    }
    logMessage(QString("Generated %1 different types of special tiles (%2 each).").arg(tileSets.size()).arg(tileCount));
}


// --- Level Change Logic ---
void DungeonDialog::enterLevel(int level) {
    m_currentLevel = level;

    // 1. Regenerate map/obstacles/stairs/special tiles
    generateRandomObstacles(30); 
    generateStairs(); 
    generateSpecialTiles(10); // Generates 10 of each special tile

    // 2. Set player position to the Stairs Up position (NEW STARTING POINT)
    m_playerMapX = m_stairsUpPosition.first;
    m_playerMapY = m_stairsUpPosition.second;

    logMessage(QString("You descend/ascend to **Level %1** and start at the Stairs Up tile.").arg(level));

    QString eventTriggerName = QString("ENTER_LEVEL_%1").arg(level);
    QJsonObject context;
    EventManager::instance()->update(eventTriggerName, context);
    
    // Ensure the minimap updates immediately
    updateMinimap(m_playerMapX, m_playerMapY);
}


// --- UI Update Functions ---
void DungeonDialog::updateCompass(const QString& direction)
{
    if (m_compassLabel) {
        m_compassLabel->setText(direction);
    }
}

void DungeonDialog::updateLocation(const QString& location)
{
    if (m_locationLabel) {
        // Show current level in location string for clarity
        m_locationLabel->setText(QString("L%1: %2,%3").arg(m_currentLevel).arg(m_playerMapX).arg(m_playerMapY));
    }
}

void DungeonDialog::updateMinimap(int x, int y)
{
    if (!m_fullMapScene) return;

    m_fullMapScene->clear();

    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            QRectF rect(i * TILE_SIZE, j * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            QBrush brush;
            QPair<int, int> currentTile = qMakePair(i, j); 

            if (i == x && j == y) {
                brush = QBrush(Qt::red); // Player (Highest priority)
            } else if (m_antimagicPositions.contains(currentTile)) {
                brush = QBrush(Qt::darkMagenta); // Antimagic
            } else if (m_extinguisherPositions.contains(currentTile)) {
                brush = QBrush(Qt::yellow); // Extinguisher
            } else if (m_fogPositions.contains(currentTile)) {
                brush = QBrush(Qt::lightGray); // Fog
            } else if (m_pitPositions.contains(currentTile)) {
                brush = QBrush(Qt::darkRed); // Pit
            } else if (m_rotatorPositions.contains(currentTile)) {
                brush = QBrush(Qt::white); // Rotator
            } else if (m_studPositions.contains(currentTile)) {
                brush = QBrush(Qt::gray); // Stud
            } else if (m_chutePositions.contains(currentTile)) {
                brush = QBrush(Qt::darkYellow); // Chute
            } else if (m_teleportPositions.contains(currentTile)) {
                brush = QBrush(Qt::magenta); // Teleport
            } else if (m_waterPositions.contains(currentTile)) {
                brush = QBrush(Qt::darkCyan); // Water
            } else if (m_obstaclePositions.contains(currentTile)) {
                brush = QBrush(Qt::darkGreen); // Obstacle
            } else if (currentTile == m_stairsDownPosition) { 
                brush = QBrush(Qt::blue); // Stairs Down
            } else if (currentTile == m_stairsUpPosition) { 
                brush = QBrush(Qt::cyan); // Stairs Up
            } else if ((i + j) % 2 == 0) {
                brush = QBrush(Qt::darkGray);
            } else {
                brush = QBrush(Qt::black);
            }
            m_fullMapScene->addRect(rect, QPen(Qt::darkGray), brush);
        }
    }

    if (m_miniMapView) {
        m_miniMapView->fitInView(m_fullMapScene->sceneRect(), Qt::KeepAspectRatio);
    }

    logMessage(QString("Map updated. Current position: (%1, %2)").arg(x).arg(y));
    updateLocation(QString("%1,%2").arg(x).arg(y));
}

// --- Constructor ---
DungeonDialog::DungeonDialog(QWidget *parent) :
    QDialog(parent),
    m_dungeonScene(new QGraphicsScene(this)),
    m_fullMapScene(new QGraphicsScene(this)),
    m_spawnTimer(new QTimer(this)),
    m_playerMapX(MAP_SIZE / 2), 
    m_playerMapY(MAP_SIZE / 2), 
    m_currentLevel(1), 
    m_chestFound(false),
    m_locationLabel(nullptr),
    m_compassLabel(nullptr),
    m_miniMapView(nullptr),
    m_messageLog(nullptr),
    m_chestButton(nullptr),
    m_currentMonsterAttitude(Hostile),
    m_partyInfoDialog(nullptr),
    m_goldLabel(nullptr), 
    m_currentGold(1775531),
    m_partyTable(nullptr),
    m_stairsUpPosition(qMakePair(-1, -1)), 
    m_stairsDownPosition(qMakePair(-1, -1)) 
{
    // --- 1. Top Bar Information Layout ---
    QLabel *firLabel = new QLabel("Fir: 1475");
    QLabel *colLabel = new QLabel("Col: 1300");
    QLabel *eleLabel = new QLabel("Ele: 1100");
    QLabel *minLabel = new QLabel("Min: 1200");
    
    // Initialize m_goldLabel and set its initial text
    m_goldLabel = new QLabel;
    updateGoldLabel();

    QHBoxLayout *topBarLayout = new QHBoxLayout;
    topBarLayout->addWidget(firLabel);
    topBarLayout->addWidget(colLabel);
    topBarLayout->addWidget(eleLabel);
    topBarLayout->addWidget(minLabel);
    topBarLayout->addStretch(1);
    topBarLayout->addWidget(m_goldLabel); 

    // --- 2. Initialize Member UI Widgets ---
    m_locationLabel = new QLabel(QString("L%1: 21,4,3").arg(m_currentLevel)); 
    m_compassLabel = new QLabel("West [1]");
    m_miniMapView = new QGraphicsView(this);
    m_messageLog = new QListWidget;
    m_chestButton = new QPushButton("Chest");
    m_chestButton->setEnabled(false);

    // --- 3. Main Center Area (Dungeon View, Map, Action Buttons) ---
    QGraphicsView *dungeonView = new QGraphicsView(this);
    dungeonView->setScene(m_dungeonScene);
    dungeonView->setRenderHint(QPainter::Antialiasing);

    QPixmap placeholder(250, 250);
    placeholder.fill(Qt::darkGray);
    m_dungeonScene->addPixmap(placeholder);

    m_miniMapView->setFixedSize(MAP_WIDTH_PIXELS, MAP_HEIGHT_PIXELS);
    m_miniMapView->setScene(m_fullMapScene);

    m_miniMapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_miniMapView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_fullMapScene->setSceneRect(0, 0, MAP_WIDTH_PIXELS, MAP_HEIGHT_PIXELS);

    // --- Map Feature Setup ---
    generateRandomObstacles(30); 
    generateStairs(); 
    generateSpecialTiles(10); // Generates all special tiles

    // Set player position to the Stairs Up position for the start of Level 1 
    m_playerMapX = m_stairsUpPosition.first;
    m_playerMapY = m_stairsUpPosition.second;

    updateMinimap(m_playerMapX, m_playerMapY);

    QHBoxLayout *viewLayout = new QHBoxLayout;
    viewLayout->addWidget(dungeonView);
    viewLayout->addWidget(m_miniMapView);

    // Compass and Location Layout
    QHBoxLayout *compassLayout = new QHBoxLayout;
    compassLayout->addWidget(m_locationLabel);
    compassLayout->addWidget(m_compassLabel);
    compassLayout->addStretch(1);

    // Action Buttons
    QPushButton *mapButton = new QPushButton("Map");
    QPushButton *pickupButton = new QPushButton("Pickup");
    QPushButton *dropButton = new QPushButton("Drop");
    QPushButton *fightButton = new QPushButton("Fight");
    QPushButton *spellButton = new QPushButton("Spell");
    QPushButton *takeButton = new QPushButton("Take"); 
    QPushButton *openButton = new QPushButton("Open");
    QPushButton *exitButton = new QPushButton("Exit");

    QGridLayout *actionButtonLayout = new QGridLayout;
    actionButtonLayout->addWidget(mapButton, 0, 0);
    actionButtonLayout->addWidget(pickupButton, 0, 1);
    actionButtonLayout->addWidget(dropButton, 0, 2);
    actionButtonLayout->addWidget(fightButton, 1, 0);
    actionButtonLayout->addWidget(spellButton, 1, 1);
    actionButtonLayout->addWidget(takeButton, 1, 2);
    actionButtonLayout->addWidget(openButton, 2, 0);
    actionButtonLayout->addWidget(exitButton, 2, 1);
    actionButtonLayout->addWidget(m_chestButton, 2, 2);
    actionButtonLayout->setColumnStretch(2, 1);

    QVBoxLayout *centerLeftLayout = new QVBoxLayout;
    centerLeftLayout->addLayout(viewLayout);
    centerLeftLayout->addLayout(compassLayout);
    centerLeftLayout->addLayout(actionButtonLayout);

    m_messageLog->addItem(QString("Welcome to Level %1!").arg(m_currentLevel));
    m_messageLog->addItem("Press arrow keys to move.");
    m_messageLog->setMinimumHeight(100);
    m_messageLog->setFocusPolicy(Qt::NoFocus);

    // --- 4. Party and Companion Action Area ---
    m_partyTable = new QTableWidget(3, 5); 
    m_partyTable->setHorizontalHeaderLabels({"Name", "Hits", "Spells", "Status", "Option"});
    m_partyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_partyTable->verticalHeader()->setVisible(false);
    
    // Set initial party stats
    // Row 0: Player
    m_partyTable->setItem(0, 0, new QTableWidgetItem("Player"));
    m_partyTable->setItem(0, 1, new QTableWidgetItem("100/100")); // Hits
    m_partyTable->setItem(0, 2, new QTableWidgetItem("5/5"));     // Spells (Current/Max)
    
    // Row 1: Goodie Gil'N'Rhaile
    m_partyTable->setItem(1, 0, new QTableWidgetItem("Goodie Gil'N'Rhaile"));
    m_partyTable->setItem(1, 1, new QTableWidgetItem("85/90"));   // Hits
    m_partyTable->setItem(1, 2, new QTableWidgetItem("1/3"));     // Spells (Current/Max)
    
    // Row 2: Companion #11
    m_partyTable->setItem(2, 0, new QTableWidgetItem("Companion #11"));
    m_partyTable->setItem(2, 1, new QTableWidgetItem("45/75"));   // Hits
    m_partyTable->setItem(2, 2, new QTableWidgetItem("0/0"));     // Spells (Current/Max)
    
    m_partyTable->setMinimumHeight(m_partyTable->rowHeight(0) * m_partyTable->rowCount() + m_partyTable->horizontalHeader()->height());

    QPushButton *teleportButton = new QPushButton("Teleport");
    QPushButton *attackCompanionButton = new QPushButton("Attack Companion");
    QPushButton *carryCompanionButton = new QPushButton("Carry Companion");

    QHBoxLayout *companionActionLayout = new QHBoxLayout;
    companionActionLayout->addWidget(teleportButton);
    companionActionLayout->addWidget(attackCompanionButton);
    companionActionLayout->addWidget(carryCompanionButton);

    QPushButton *switchToButton = new QPushButton("Switch To");
    QPushButton *optionsButton = new QPushButton("Options");
    QPushButton *leaveButton = new QPushButton("Leave");

    QHBoxLayout *partyActionLayout = new QHBoxLayout;
    partyActionLayout->addWidget(switchToButton);
    partyActionLayout->addWidget(optionsButton);
    partyActionLayout->addWidget(leaveButton);

    QVBoxLayout *partyAreaLayout = new QVBoxLayout;
    partyAreaLayout->addWidget(m_partyTable); 
    partyAreaLayout->addLayout(companionActionLayout);
    partyAreaLayout->addLayout(partyActionLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topBarLayout);
    mainLayout->addLayout(centerLeftLayout);
    mainLayout->addWidget(m_messageLog);
    mainLayout->addLayout(partyAreaLayout);

    setWindowTitle("Dungeon Dialog");
    setMinimumWidth(800); 

    connect(teleportButton, &QPushButton::clicked, this, &DungeonDialog::on_teleportButton_clicked);
    connect(attackCompanionButton, &QPushButton::clicked, this, &DungeonDialog::on_attackCompanionButton_clicked);
    connect(carryCompanionButton, &QPushButton::clicked, this, &DungeonDialog::on_carryCompanionButton_clicked);

    connect(mapButton, &QPushButton::clicked, this, &DungeonDialog::on_mapButton_clicked);
    connect(pickupButton, &QPushButton::clicked, this, &DungeonDialog::on_pickupButton_clicked);
    connect(dropButton, &QPushButton::clicked, this, &DungeonDialog::on_dropButton_clicked);
    connect(fightButton, &QPushButton::clicked, this, &DungeonDialog::on_fightButton_clicked);
    connect(spellButton, &QPushButton::clicked, this, &DungeonDialog::on_spellButton_clicked);
    connect(takeButton, &QPushButton::clicked, this, &DungeonDialog::on_takeButton_clicked); 
    connect(openButton, &QPushButton::clicked, this, &DungeonDialog::on_openButton_clicked);
    connect(exitButton, &QPushButton::clicked, this, &DungeonDialog::on_exitButton_clicked);

    connect(m_chestButton, &QPushButton::clicked, this, &DungeonDialog::on_chestButton_clicked);

    m_spawnTimer->setInterval(5000);
    connect(m_spawnTimer, &QTimer::timeout, this, &DungeonDialog::checkMonsterSpawn);
    m_spawnTimer->start();

    setFocusPolicy(Qt::StrongFocus);

    m_partyInfoDialog = new PartyInfoDialog(this);
    m_partyInfoDialog->setPartyMembers({"Player", "Goodie Gil'N'Rhaile", "Companion #11"});
    m_partyInfoDialog->show();
    connect(EventManager::instance(), &EventManager::eventTriggered,
            this, &DungeonDialog::onEventTriggered); 
}

// Slot: Respond to triggered events
void DungeonDialog::onEventTriggered(const GameEvent& event) {
    QMessageBox::information(this, tr("Event!"), event.description);

    if (event.effect == "SPAWN_MONSTER") {
        QString monsterType = event.extraData["monsterType"].toString();
        int count = event.extraData["count"].toInt(1);
        spawnMonsters(monsterType, count);
        qDebug() << "Spawned" << count << monsterType << "(s) due to event:" << event.id;
    }
}

void DungeonDialog::spawnMonsters(const QString& monsterType, int count) {
    // Placeholder function definition
}

DungeonDialog::~DungeonDialog()
{
    delete m_partyInfoDialog;
}

// --- Event Handling ---

void DungeonDialog::keyPressEvent(QKeyEvent *event)
{
    int newX = m_playerMapX;
    int newY = m_playerMapY;

    if (event->key() == Qt::Key_Up) {
        newY = qMax(MAP_MIN, m_playerMapY - 1);
        updateCompass("North [1]");
    } else if (event->key() == Qt::Key_Down) {
        newY = qMin(MAP_MAX, m_playerMapY + 1);
        updateCompass("South [1]");
    } else if (event->key() == Qt::Key_Left) {
        newX = qMax(MAP_MIN, m_playerMapX - 1);
        updateCompass("West [1]");
    } else if (event->key() == Qt::Key_Right) {
        newX = qMin(MAP_MAX, m_playerMapX + 1);
        updateCompass("East [1]");
    } else {
        QDialog::keyPressEvent(event);
        return;
    }

    // Prevent moving into obstacles
    if (m_obstaclePositions.contains(qMakePair(newX, newY))) {
        logMessage("That way is blocked by an obstacle!");
        return;
    }

    if (newX != m_playerMapX || newY != m_playerMapY) {
        m_playerMapX = newX;
        m_playerMapY = newY;
        updateMinimap(m_playerMapX, m_playerMapY);
        
        // CHECK FOR SPECIAL TILE EFFECTS
        QPair<int, int> currentPos = qMakePair(m_playerMapX, m_playerMapY);

        if (m_antimagicPositions.contains(currentPos)) {
            logMessage("You are standing on **antimagic!**");
        } else if (m_extinguisherPositions.contains(currentPos)) {
            logMessage("You are standing on an **extinguisher tile**, spells fizzle out.");
        } else if (m_fogPositions.contains(currentPos)) {
            logMessage("You are shrouded in a **thick fog**.");
        } else if (m_pitPositions.contains(currentPos)) {
            logMessage("You are standing at the edge of a **deep pit**.");
        } else if (m_rotatorPositions.contains(currentPos)) {
            logMessage("You step onto a **rotator**. Your direction of view is randomized.");
        } else if (m_studPositions.contains(currentPos)) {
            logMessage("You are standing on a **stud tile**. It offers excellent grip.");
        } else if (m_chutePositions.contains(currentPos)) {
            logMessage("You are standing over a **chute**, ready to fall to the level below.");
        } else if (m_teleportPositions.contains(currentPos)) {
            logMessage("You are standing on a dormant **teleport pad**.");
        } else if (m_waterPositions.contains(currentPos)) {
            logMessage("You are standing in **shallow water**.");
        }
    }

    event->accept();
}

// --- Game Logic Slots ---

void DungeonDialog::checkMonsterSpawn()
{
    if (m_currentMonsterAttitude != Hostile) {
        return;
    }

    if ((QRandomGenerator::global()->generate() % 100) < 20) {
        int roll = QRandomGenerator::global()->generate() % 100;
        if (roll < 50) {
            m_currentMonsterAttitude = Hostile;
            logMessage("🚨 **A hostile monster** has appeared! Combat begins immediately!");
            initiateFight();
        } else if (roll < 80) {
            m_currentMonsterAttitude = Neutral;
            logMessage("🚶 A **neutral creature** is blocking the path. You can attack it or ignore it.");
        } else {
            m_currentMonsterAttitude = Friendly;
            logMessage("🤝 A **friendly creature** offers you guidance. You can attack it, but why would you?");
        }
    }
}

void DungeonDialog::initiateFight()
{
    m_spawnTimer->stop();
    logMessage("**FIGHT INITIATED!** Placeholder fight logic runs now.");

    QTimer::singleShot(2000, this, [this]() {
        logMessage("You defeated the creature! (Placeholder)");
        
        if (m_partyTable) {
            // Restore Player to full health (for demonstration)
            m_partyTable->setItem(0, 1, new QTableWidgetItem("100/100")); 
            QTableWidgetItem *statusItem = m_partyTable->item(0, 3);
            if (statusItem) statusItem->setText(""); // Clear status
        }
        
        m_currentMonsterAttitude = Hostile;
        on_winBattle_trigger();
        m_spawnTimer->start();
    });
}

void DungeonDialog::on_winBattle_trigger()
{
    if ((QRandomGenerator::global()->generate() % 100) < 50) {
        m_chestFound = true;
        m_chestButton->setEnabled(true);
        logMessage("Congratulations! A **Treasure Chest** has appeared!");
    } else {
        logMessage("You scour the area, but no treasure chest is found.");
    }
}

void DungeonDialog::on_chestButton_clicked()
{
    if (!m_chestFound) {
        logMessage("There is no chest here to open.");
        return;
    }
    m_chestButton->setEnabled(false);
    m_chestFound = false;

    if ((QRandomGenerator::global()->generate() % 100) < 70) {
        int goldReward = 100 + (QRandomGenerator::global()->generate() % 900);
        
        // Update gold and refresh label
        m_currentGold += goldReward; 
        updateGoldLabel();
        
        logMessage(QString("💰 You open the chest and find **%1 Gold**!").arg(goldReward));
    } else {
        int damage = 5 + (QRandomGenerator::global()->generate() % 15);
        logMessage(QString("💥 **TRAP!** The chest explodes, dealing **%1 damage** to the party!").arg(damage));
        
        // Apply damage to all 3 party members (rows 0, 1, 2)
        for (int i = 0; i < 3; ++i) {
            updatePartyMemberHealth(i, damage);
        }
    }
    logMessage("The treasure chest is now gone.");
}

// --- Button Click Handlers ---

void DungeonDialog::on_teleportButton_clicked() 
{
    emit teleporterUsed();
    logMessage("Teleporter used! You've been moved.");

    // Stay off obstacles AND stairs when teleporting!
    do {
        m_playerMapX = QRandomGenerator::global()->generate() % MAP_SIZE;
        m_playerMapY = QRandomGenerator::global()->generate() % MAP_SIZE;
    } while (m_obstaclePositions.contains(qMakePair(m_playerMapX, m_playerMapY)) ||
             qMakePair(m_playerMapX, m_playerMapY) == m_stairsUpPosition ||
             qMakePair(m_playerMapX, m_playerMapY) == m_stairsDownPosition);

    updateMinimap(m_playerMapX, m_playerMapY);
}

void DungeonDialog::on_attackCompanionButton_clicked() 
{
    int companionId = 1;
    emit companionAttacked(companionId);
    logMessage(QString("Attacked Companion #%1!").arg(companionId));
}

void DungeonDialog::on_carryCompanionButton_clicked() 
{
    int companionId = 1;
    emit companionCarried(companionId);
    logMessage(QString("Carrying Companion #%1!").arg(companionId));
}

void DungeonDialog::on_mapButton_clicked() { logMessage("Map button clicked!"); }
void DungeonDialog::on_pickupButton_clicked() { logMessage("Pickup button clicked!"); }
void DungeonDialog::on_dropButton_clicked() { logMessage("Drop button clicked!"); }

void DungeonDialog::on_fightButton_clicked()
{
    if (m_currentMonsterAttitude == Hostile) {
        logMessage("Engaging the hostile creature!");
        initiateFight();
    } else if (m_currentMonsterAttitude == Neutral) {
        logMessage("You attack the neutral creature! It is now hostile.");
        m_currentMonsterAttitude = Hostile;
        initiateFight();
    } else if (m_currentMonsterAttitude == Friendly) {
        logMessage("😡 You **betray** the friendly creature! It is now hostile.");
        m_currentMonsterAttitude = Hostile;
        initiateFight();
    } else {
        logMessage("There is nothing to fight here.");
    }
}

void DungeonDialog::on_spellButton_clicked() { logMessage("Spell button clicked!"); }

// --- Level Change Logic (on_takeButton_clicked) ---
void DungeonDialog::on_takeButton_clicked() 
{ 
    QPair<int, int> playerPos = qMakePair(m_playerMapX, m_playerMapY);
    int newLevel = m_currentLevel;

    if (playerPos == m_stairsDownPosition) {
        newLevel = m_currentLevel + 1;
        logMessage(QString("You take the **stairs down** to Level %1.").arg(newLevel));
        enterLevel(newLevel);
    } else if (playerPos == m_stairsUpPosition) {
        if (m_currentLevel > 1) {
            // Standard ascent to a deeper level
            newLevel = m_currentLevel - 1;
            logMessage(QString("You take the **stairs up** to Level %1.").arg(newLevel));
            enterLevel(newLevel);
        } else {
            // Current level is 1: ascend to surface/city
            logMessage("You ascend the **stairs up** and step onto the surface!");
            emit exitedDungeonToCity(); // Emit signal to switch window
            this->close();              // Close the Dungeon dialog
        }
    } else {
        logMessage("There are no stairs here to take.");
    }
}

void DungeonDialog::on_openButton_clicked() { logMessage("Open button clicked!"); }

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
    } else {
        qDebug() << "User clicked No. Dialog closed.";
    }
}
