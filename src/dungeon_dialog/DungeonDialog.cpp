#include "DungeonDialog.h"
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

// Define map constants
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
        m_locationLabel->setText(location);
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

            if (i == x && j == y) {
                // Player is red
                brush = QBrush(Qt::red);
            } else if ((i + j) % 2 == 0) {
                // Background pattern
                brush = QBrush(Qt::darkGray);
            } else {
                // Background pattern
                brush = QBrush(Qt::black);
            }
            m_fullMapScene->addRect(rect, QPen(Qt::darkGray), brush);
        }
    }
    
    // Ensure the view is fitted to the scene, as it's the size we want
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
    m_playerMapX(15),                          
    m_playerMapY(15),                          
    m_chestFound(false),
    m_locationLabel(nullptr), 
    m_compassLabel(nullptr),
    m_miniMapView(nullptr),
    m_messageLog(nullptr),
    m_chestButton(nullptr),
    m_currentMonsterAttitude(Hostile), // Initialize to Hostile (acts as None/No Encounter)
    m_partyInfoDialog(nullptr) // Initialize the new member pointer
{
    // --- 1. Top Bar Information Layout ---
    QLabel *firLabel = new QLabel("Fir: 1475");
    QLabel *colLabel = new QLabel("Col: 1300");
    QLabel *eleLabel = new QLabel("Ele: 1100");
    QLabel *minLabel = new QLabel("Min: 1200");
    QLabel *goldLabel = new QLabel("You have a total of 1,775,531,378 Gold");

    QHBoxLayout *topBarLayout = new QHBoxLayout;
    topBarLayout->addWidget(firLabel);
    topBarLayout->addWidget(colLabel);
    topBarLayout->addWidget(eleLabel);
    topBarLayout->addWidget(minLabel);
    topBarLayout->addStretch(1);
    topBarLayout->addWidget(goldLabel);

    // --- 2. Initialize Member UI Widgets ---
    m_locationLabel = new QLabel("21,4,3");
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

    // Map View Setup (Fixed Size and No Scrollbars)
    // Ensures the full map is visible without ugly scrollbars
    m_miniMapView->setFixedSize(MAP_WIDTH_PIXELS, MAP_HEIGHT_PIXELS); 
    m_miniMapView->setScene(m_fullMapScene);

    m_miniMapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_miniMapView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Set the scene bounds to match the map size
    m_fullMapScene->setSceneRect(0, 0, MAP_WIDTH_PIXELS, MAP_HEIGHT_PIXELS);

    // Initial map drawing and player position update
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

    // Combine views, compass, and actions
    QVBoxLayout *centerLeftLayout = new QVBoxLayout;
    centerLeftLayout->addLayout(viewLayout);
    centerLeftLayout->addLayout(compassLayout);
    centerLeftLayout->addLayout(actionButtonLayout);

    // Message Log Setup
    m_messageLog->addItem("Welcome to the Dungeon!");
    m_messageLog->addItem("Press arrow keys to move.");
    m_messageLog->setMinimumHeight(100);
    // FIX: Prevent the log window from capturing focus
    m_messageLog->setFocusPolicy(Qt::NoFocus); 

    // --- 4. Party and Companion Action Area ---
    QTableWidget *partyTable = new QTableWidget(3, 5);
    partyTable->setHorizontalHeaderLabels({"Name", "Hits", "Spells", "Status", "Option"});
    partyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    partyTable->verticalHeader()->setVisible(false);
    partyTable->setItem(0, 0, new QTableWidgetItem("Player"));
    partyTable->setItem(1, 0, new QTableWidgetItem("Goodie Gil'N'Rhaile"));
    partyTable->setItem(2, 0, new QTableWidgetItem("Companion #11"));
    partyTable->setMinimumHeight(partyTable->rowHeight(0) * partyTable->rowCount() + partyTable->horizontalHeader()->height());

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

    // TODO: Connect these party buttons to m_partyInfoDialog->show() or a similar slot
    // to open the dialog from here.

    QVBoxLayout *partyAreaLayout = new QVBoxLayout;
    partyAreaLayout->addWidget(partyTable);
    partyAreaLayout->addLayout(companionActionLayout);
    partyAreaLayout->addLayout(partyActionLayout);

    // --- 5. Assemble Main Dialog Layout ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topBarLayout);
    mainLayout->addLayout(centerLeftLayout);
    mainLayout->addWidget(m_messageLog);
    mainLayout->addLayout(partyAreaLayout);

    setWindowTitle("Dungeon Dialog");

    // --- 6. Connect Slots ---
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

    // --- 7. Initialize and Connect Monster Spawn Timer ---
    m_spawnTimer->setInterval(5000); 
    connect(m_spawnTimer, &QTimer::timeout, this, &DungeonDialog::checkMonsterSpawn);
    m_spawnTimer->start();

    // --- 8. Set Focus for Key Events ---
    // This ensures the QDialog, and thus keyPressEvent, receives the input
    setFocusPolicy(Qt::StrongFocus);
    
    // 9. Initialize and Show Party Info Dialog on Initiation
    m_partyInfoDialog = new PartyInfoDialog(this);
    m_partyInfoDialog->setPartyMembers({"Player", "Goodie Gil'N'Rhaile", "Companion #11"});
    
    // Note: exec() blocks the execution flow until the dialog is closed.
    // If you want a non-modal dialog, use show() instead.
    m_partyInfoDialog->show(); 
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

    // Only update the map if the position actually changed (respects boundaries)
    if (newX != m_playerMapX || newY != m_playerMapY) {
        m_playerMapX = newX;
        m_playerMapY = newY;
        
        updateMinimap(m_playerMapX, m_playerMapY); 
    }
    
    event->accept();
}

// --- Game Logic Slots ---

void DungeonDialog::checkMonsterSpawn()
{
    // If a monster is already present (not Hostile, which is used as the 'no encounter' state)
    if (m_currentMonsterAttitude != Hostile) {
        return; 
    }
    
    // Simple 20% chance to spawn an encounter every 5 seconds
    if ((QRandomGenerator::global()->generate() % 100) < 20) {
        
        // Randomly determine attitude: 50% Hostile, 30% Neutral, 20% Friendly
        int roll = QRandomGenerator::global()->generate() % 100;
        
        if (roll < 50) { // 0-49: Hostile (50%)
            m_currentMonsterAttitude = Hostile;
            logMessage("🚨 **A hostile monster** has appeared! Combat begins immediately!");
            initiateFight();
        } else if (roll < 80) { // 50-79: Neutral (30%)
            m_currentMonsterAttitude = Neutral;
            logMessage("🚶 A **neutral creature** is blocking the path. You can attack it or ignore it.");
        } else { // 80-99: Friendly (20%)
            m_currentMonsterAttitude = Friendly;
            logMessage("🤝 A **friendly creature** offers you guidance. You can attack it, but why would you?");
        }
    } 
}

void DungeonDialog::initiateFight()
{
    // If the fight is initiated, we assume there is a monster and it is now hostile.
    m_spawnTimer->stop();
    logMessage("**FIGHT INITIATED!** Placeholder fight logic runs now.");

    QTimer::singleShot(2000, this, [this]() {
        logMessage("You defeated the creature! (Placeholder)");
        
        // Reset the monster state to allow a new encounter/spawn
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
        logMessage(QString("💰 You open the chest and find **%1 Gold**!").arg(goldReward));
        
    } else {
        int damage = 5 + (QRandomGenerator::global()->generate() % 15);
        logMessage(QString("💥 **TRAP!** The chest explodes, dealing **%1 damage**.").arg(damage));
    }
    
    logMessage("The treasure chest is now gone.");
}

// --- Button Click Handlers ---

void DungeonDialog::on_teleportButton_clicked()
{
    emit teleporterUsed();
    logMessage("Teleporter used! You've been moved.");
    
    m_playerMapX = QRandomGenerator::global()->generate() % MAP_SIZE;
    m_playerMapY = QRandomGenerator::global()->generate() % MAP_SIZE;
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
        m_currentMonsterAttitude = Hostile; // Update state
        initiateFight();
    } else if (m_currentMonsterAttitude == Friendly) {
        logMessage("😡 You **betray** the friendly creature! It is now hostile.");
        m_currentMonsterAttitude = Hostile; // Update state
        initiateFight();
    } else {
        logMessage("There is nothing to fight here."); 
    }
}

void DungeonDialog::on_spellButton_clicked() { logMessage("Spell button clicked!"); }
void DungeonDialog::on_takeButton_clicked() { logMessage("Take button clicked!"); }
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
