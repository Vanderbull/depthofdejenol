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

DungeonDialog::DungeonDialog(QWidget *parent) :
    QDialog(parent),
    m_dungeonScene(new QGraphicsScene(this))
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

    // --- 2. Main Center Area (Dungeon View, Map, Action Buttons) ---
    QGraphicsView *dungeonView = new QGraphicsView(this);
    dungeonView->setScene(m_dungeonScene);
    dungeonView->setRenderHint(QPainter::Antialiasing);
    dungeonView->setFixedSize(250, 250); // Set fixed size for dungeon view
    // Placeholder image for the dungeon view (optional)
    QPixmap placeholder(250, 250);
    placeholder.fill(Qt::darkGray);
    m_dungeonScene->addPixmap(placeholder);

    // Placeholder for the Autoscroll Map
    QGraphicsView *miniMapView = new QGraphicsView;
    miniMapView->setFixedSize(200, 250);
    miniMapView->setSceneRect(0, 0, 200, 250);
    QGraphicsScene *miniMapScene = new QGraphicsScene(miniMapView);
    miniMapScene->addRect(0, 0, 200, 250, QPen(Qt::black), QBrush(Qt::lightGray));
    miniMapView->setScene(miniMapScene);

    QHBoxLayout *viewLayout = new QHBoxLayout;
    viewLayout->addWidget(dungeonView);
    viewLayout->addWidget(miniMapView);

    // Compass and Location
    QLabel *locationLabel = new QLabel("21,4,3");
    QLabel *compassLabel = new QLabel("West [1]");
    QHBoxLayout *compassLayout = new QHBoxLayout;
    compassLayout->addWidget(locationLabel);
    compassLayout->addWidget(compassLabel);
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
    actionButtonLayout->setColumnStretch(2, 1); // Stretch the last column

    // Combine views, compass, and actions
    QVBoxLayout *centerLeftLayout = new QVBoxLayout;
    centerLeftLayout->addLayout(viewLayout);
    centerLeftLayout->addLayout(compassLayout);
    centerLeftLayout->addLayout(actionButtonLayout);

    // --- 3. Message Log ---
    QListWidget *messageLog = new QListWidget;
    messageLog->addItem("The door is locked.");
    messageLog->addItem("You just hit a teleporter!");
    messageLog->addItem("You attacked Companion #11!");
    messageLog->setMinimumHeight(100);

    // --- 4. Party and Companion Action Area ---
    QTableWidget *partyTable = new QTableWidget(3, 5); // 3 rows for party members, 5 columns
    partyTable->setHorizontalHeaderLabels({"Name", "Hits", "Spells", "Status", "Option"});
    partyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    partyTable->verticalHeader()->setVisible(false);
    // Example data
    partyTable->setItem(0, 0, new QTableWidgetItem("Player"));
    partyTable->setItem(1, 0, new QTableWidgetItem("Goodie Gil'N'Rhaile"));
    partyTable->setItem(2, 0, new QTableWidgetItem("Companion #11"));
    partyTable->setMinimumHeight(partyTable->rowHeight(0) * partyTable->rowCount() + partyTable->horizontalHeader()->height());

    // Companion Actions (Teleport, Attack, Carry)
    QPushButton *teleportButton = new QPushButton("Teleport");
    QPushButton *attackCompanionButton = new QPushButton("Attack Companion");
    QPushButton *carryCompanionButton = new QPushButton("Carry Companion");

    QHBoxLayout *companionActionLayout = new QHBoxLayout;
    companionActionLayout->addWidget(teleportButton);
    companionActionLayout->addWidget(attackCompanionButton);
    companionActionLayout->addWidget(carryCompanionButton);

    // Party Action Buttons (Switch To, Options, Leave)
    QPushButton *switchToButton = new QPushButton("Switch To");
    QPushButton *optionsButton = new QPushButton("Options");
    QPushButton *leaveButton = new QPushButton("Leave");

    QHBoxLayout *partyActionLayout = new QHBoxLayout;
    partyActionLayout->addWidget(switchToButton);
    partyActionLayout->addWidget(optionsButton);
    partyActionLayout->addWidget(leaveButton);

    // Combine party and companion actions
    QVBoxLayout *partyAreaLayout = new QVBoxLayout;
    partyAreaLayout->addWidget(partyTable);
    partyAreaLayout->addLayout(companionActionLayout);
    partyAreaLayout->addLayout(partyActionLayout);

    // --- 5. Assemble Main Dialog Layout ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topBarLayout);
    mainLayout->addLayout(centerLeftLayout);
    mainLayout->addWidget(messageLog);
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

    // Store references to widgets we need to update externally
    // This is a necessary step when building without Designer, as we
    // need external access to update the view/labels later.
    // For this example, we'll use local variables and rely on the update functions.
    // In a production app, you would make these `QLabel`s members of the class.
    // Example: this->locationLabel = locationLabel;
}

DungeonDialog::~DungeonDialog()
{
    // If we had member pointers for all created widgets, they would be deleted here.
    // Since they are parented to 'this', QDialog handles their destruction.
}

void DungeonDialog::updateDungeonView(const QImage& dungeonImage)
{
    m_dungeonScene->clear();
    m_dungeonScene->addPixmap(QPixmap::fromImage(dungeonImage));
    // The QGraphicsView widget is local to the constructor; a full implementation
    // would require making it a member or finding it via QObject::findChild.
    // For now, assume a QGraphicsView member named 'dungeonView' exists.
    // If not a member, this function is slightly complex:
    QGraphicsView* dungeonView = findChild<QGraphicsView*>();
    if (dungeonView) {
        dungeonView->fitInView(m_dungeonScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void DungeonDialog::updateCompass(const QString& direction)
{
    // You would need to store a pointer to the QLabel in the class members.
    // For a quick fix, assume the label is named 'compassLabel' in the class.
    // QLabel* compassLabel = findChild<QLabel*>("compassLabel"); // Requires setting object names
    // if (compassLabel) compassLabel->setText(direction);
    Q_UNUSED(direction);
}

void DungeonDialog::updateLocation(const QString& location)
{
    // You would need to store a pointer to the QLabel in the class members.
    Q_UNUSED(location);
}

void DungeonDialog::on_teleportButton_clicked()
{
    emit teleporterUsed();
    QMessageBox::information(this, "Action", "Teleporter used!");
}

void DungeonDialog::on_attackCompanionButton_clicked()
{
    int companionId = 1; // Placeholder
    emit companionAttacked(companionId);
    QMessageBox::information(this, "Action", QString("Attacked Companion #%1!").arg(companionId));
}

void DungeonDialog::on_carryCompanionButton_clicked()
{
    int companionId = 1; // Placeholder
    emit companionCarried(companionId);
    QMessageBox::information(this, "Action", QString("Carrying Companion #%1!").arg(companionId));
}

void DungeonDialog::on_mapButton_clicked() { QMessageBox::information(this, "Action", "Map button clicked!"); }
void DungeonDialog::on_pickupButton_clicked() { QMessageBox::information(this, "Action", "Pickup button clicked!"); }
void DungeonDialog::on_dropButton_clicked() { QMessageBox::information(this, "Action", "Drop button clicked!"); }
void DungeonDialog::on_fightButton_clicked() { QMessageBox::information(this, "Action", "Fight button clicked!"); }
void DungeonDialog::on_spellButton_clicked() { QMessageBox::information(this, "Action", "Spell button clicked!"); }
void DungeonDialog::on_takeButton_clicked() { QMessageBox::information(this, "Action", "Take button clicked!"); }
void DungeonDialog::on_openButton_clicked() { QMessageBox::information(this, "Action", "Open button clicked!"); }

void DungeonDialog::on_exitButton_clicked()
{
    QMessageBox::information(this, "Action", "Exit button clicked!");
    this->close();
}
