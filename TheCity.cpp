#include "TheCity.h"
#include "include/game_resources.h"
#include "src/network_manager/NetworkManager.h"
#include "src/partyinfo_dialog/partyinfodialog.h"
#include "src/general_store/GeneralStore.h"
#include "src/guilds_dialog/GuildsDialog.h"
#include "src/morgue_dialog/MorgueDialog.h"
#include "src/seer_dialog/SeerDialog.h"
#include "src/confinement_dialog/ConfinementDialog.h"
#include "src/bank_dialog/BankDialog.h"
#include "src/dungeon_dialog/DungeonDialog.h"
#include <QDebug>
#include <QDateTime>
#include <QScrollBar>

TheCity::TheCity(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("The City - Online");
    setFixedSize(800, 600); // Expanded for multiplayer UI
    
    setupUi();
    loadButtonIcons();
    setupStyling();
    setupMultiplayerConnections();

    // 1. Set up a timeout (e.g., 3 seconds)
    QTimer *connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);

    connect(connectionTimer, &QTimer::timeout, this, [this]() {
        if (NetworkManager::instance()->state() != QAbstractSocket::ConnectedState) {
            qDebug() << "Server not found. Switching to Offline Mode.";
            startOfflineMode();
        }
    });

    // 2. Handle successful connection
    connect(NetworkManager::instance(), &NetworkManager::connected, this, [this, connectionTimer]() {
        connectionTimer->stop(); // Stop the "Offline" timer
        isOfflineMode = false;
        setWindowTitle("The City - Online");
        
        // Register with server
        quint32 randomId = QRandomGenerator::global()->bounded(1000);
        NetworkManager::instance()->sendAction("enter_zone", {
            {"zone", "TheCity"}, 
            {"username", "Hero_" + QString::number(randomId)}
        });
    });

    // 3. Try to connect
    NetworkManager::instance()->connectToServer("127.0.0.1", 12345);
    connectionTimer->start(3000); // Wait 3 seconds for server
}

void TheCity::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    titleLabel = new QLabel("The City", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // --- Middle Section: Sidebar + Grid ---
    QHBoxLayout *midLayout = new QHBoxLayout();
    
    playerList = new QListWidget(this);
    playerList->setFixedWidth(150);
    playerList->setObjectName("playerList");
    midLayout->addWidget(playerList);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(10);

    generalStoreButton = new QToolButton(this);
    morgueButton = new QToolButton(this);
    guildsButton = new QToolButton(this);
    dungeonButton = new QToolButton(this);
    confinementButton = new QToolButton(this);
    seerButton = new QToolButton(this);
    bankButton = new QToolButton(this);
    exitButton = new QToolButton(this);

    gridLayout->addWidget(generalStoreButton, 0, 0);
    gridLayout->addWidget(morgueButton,      0, 1);
    gridLayout->addWidget(guildsButton,      0, 2);
    gridLayout->addWidget(dungeonButton,     0, 3);
    gridLayout->addWidget(confinementButton, 1, 0);
    gridLayout->addWidget(seerButton,        1, 1);
    gridLayout->addWidget(bankButton,        1, 2);
    gridLayout->addWidget(exitButton,        1, 3);
    
    midLayout->addLayout(gridLayout);
    mainLayout->addLayout(midLayout);

    // --- Bottom Section: Chat ---
    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);
    chatDisplay->setObjectName("chatDisplay");
    mainLayout->addWidget(chatDisplay);

    QHBoxLayout *chatInputLayout = new QHBoxLayout();
    chatInput = new QLineEdit(this);
    sendButton = new QPushButton("Send", this);
    chatInputLayout->addWidget(chatInput);
    chatInputLayout->addWidget(sendButton);
    mainLayout->addLayout(chatInputLayout);

    // Basic Signal Connections
    connect(generalStoreButton, &QToolButton::clicked, this, &TheCity::on_generalStoreButton_clicked);
    connect(morgueButton,      &QToolButton::clicked, this, &TheCity::on_morgueButton_clicked);
    connect(guildsButton,      &QToolButton::clicked, this, &TheCity::on_guildsButton_clicked);
    connect(dungeonButton,     &QToolButton::clicked, this, &TheCity::on_dungeonButton_clicked);
    connect(confinementButton, &QToolButton::clicked, this, &TheCity::on_confinementButton_clicked);
    connect(seerButton,        &QToolButton::clicked, this, &TheCity::on_seerButton_clicked);
    connect(bankButton,        &QToolButton::clicked, this, &TheCity::on_bankButton_clicked);
    connect(exitButton,        &QToolButton::clicked, this, &TheCity::on_exitButton_clicked);
    connect(sendButton,        &QPushButton::clicked, this, &TheCity::sendChatMessage);
    connect(chatInput,         &QLineEdit::returnPressed, this, &TheCity::sendChatMessage);
}

void TheCity::setupMultiplayerConnections()
{
    connect(NetworkManager::instance(), &NetworkManager::playerJoined, this, &TheCity::handlePlayerJoined);
    connect(NetworkManager::instance(), &NetworkManager::playerLeft, this, &TheCity::handlePlayerLeft);
    connect(NetworkManager::instance(), &NetworkManager::chatReceived, this, &TheCity::handleChatReceived);
}

void TheCity::handlePlayerJoined(QString name) {
    playerList->addItem(name);
    chatDisplay->append(QString("<i style='color:gray;'>%1 has arrived.</i>").arg(name));
}

void TheCity::handlePlayerLeft(QString name) {
    QList<QListWidgetItem*> items = playerList->findItems(name, Qt::MatchExactly);
    for(auto item : items) delete playerList->takeItem(playerList->row(item));
    chatDisplay->append(QString("<i style='color:gray;'>%1 has departed.</i>").arg(name));
}

void TheCity::handleChatReceived(QString from, QString message) {
    QString time = QDateTime::currentDateTime().toString("hh:mm");
    chatDisplay->append(QString("[%1] <b>%2:</b> %3").arg(time, from, message));
    chatDisplay->verticalScrollBar()->setValue(chatDisplay->verticalScrollBar()->maximum());
}

void TheCity::sendChatMessage() {
    if (isOfflineMode) {
        chatDisplay->append("<b>You (Offline):</b> " + chatInput->text());
        chatInput->clear();
        return;
    }

    QString msg = chatInput->text().trimmed();
    if (!msg.isEmpty()) {
        NetworkManager::instance()->sendAction("chat", {{"message", msg}});
        chatInput->clear();
    }
}

void TheCity::loadButtonIcons()
{
    const QSize iconSize(120, 90); 
    generalStoreButton->setIcon(QIcon(GameResources::getPixmap("general_store")));
    generalStoreButton->setIconSize(iconSize);
    generalStoreButton->setToolTip("General Store");

    morgueButton->setIcon(QIcon(GameResources::getPixmap("morgue")));
    morgueButton->setIconSize(iconSize);
    morgueButton->setToolTip("Morgue");

    guildsButton->setIcon(QIcon(GameResources::getPixmap("guilds")));
    guildsButton->setIconSize(iconSize);
    guildsButton->setToolTip("Guilds");

    dungeonButton->setIcon(QIcon(GameResources::getPixmap("dungeon")));
    dungeonButton->setIconSize(iconSize);
    dungeonButton->setToolTip("Dungeon");

    confinementButton->setIcon(QIcon(GameResources::getPixmap("confinement")));
    confinementButton->setIconSize(iconSize);
    confinementButton->setToolTip("Confinement");

    seerButton->setIcon(QIcon(GameResources::getPixmap("seer")));
    seerButton->setIconSize(iconSize);
    seerButton->setToolTip("Seer");

    bankButton->setIcon(QIcon(GameResources::getPixmap("bank")));
    bankButton->setIconSize(iconSize);
    bankButton->setToolTip("Bank");

    exitButton->setIcon(QIcon(GameResources::getPixmap("exit_icon")));
    exitButton->setIconSize(iconSize);
    exitButton->setToolTip("Exit to main menu");
}

void TheCity::setupStyling()
{
    QFile styleFile("TheCity.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        this->setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }
    titleLabel->setStyleSheet("font-size: 20px; color: #CCCCCC; background-color: #444444; padding: 5px; border: 1px solid #777777;");
}

void TheCity::on_generalStoreButton_clicked() {
    NetworkManager::instance()->sendAction("enter_location", {{"location", "GeneralStore"}});
    GeneralStore *store = new GeneralStore(this);
    store->setAttribute(Qt::WA_DeleteOnClose);
    store->show();
}

void TheCity::on_morgueButton_clicked() {
    MorgueDialog *m = new MorgueDialog(this);
    m->setAttribute(Qt::WA_DeleteOnClose);
    m->show();
}

void TheCity::on_guildsButton_clicked() {
    GuildsDialog *g = new GuildsDialog(this);
    g->setAttribute(Qt::WA_DeleteOnClose);
    g->show();
}

void TheCity::on_dungeonButton_clicked() {
    DungeonDialog *d = new DungeonDialog(nullptr); 
    d->setAttribute(Qt::WA_DeleteOnClose);    
    connect(d, &DungeonDialog::exitedDungeonToCity, this, &TheCity::show);
    d->show();
    this->hide();
}

void TheCity::on_confinementButton_clicked() {
    ConfinementAndHoldingDialog *c = new ConfinementAndHoldingDialog(this);
    c->setAttribute(Qt::WA_DeleteOnClose);
    c->show();
}

void TheCity::on_seerButton_clicked() {
    SeerDialog *s = new SeerDialog(this);
    s->setAttribute(Qt::WA_DeleteOnClose);
    s->show();
}

void TheCity::on_bankButton_clicked() {
    BankDialog *b = new BankDialog(this);
    b->setAttribute(Qt::WA_DeleteOnClose);
    b->show();
}

void TheCity::on_exitButton_clicked() {
    NetworkManager::instance()->sendAction("leave_zone", {{"zone", "TheCity"}});
    accept(); 
}

void TheCity::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_I) {
        InventoryDialog *inv = new InventoryDialog(this);
        inv->setAttribute(Qt::WA_DeleteOnClose);
        inv->exec(); 
    }
    else if (event->key() == Qt::Key_1) {
        PartyInfoDialog *partyInfo = new PartyInfoDialog(this);
        partyInfo->setAttribute(Qt::WA_DeleteOnClose);
        partyInfo->show(); // Use show() for non-modal or exec() for modal
    }
    else {
        QDialog::keyPressEvent(event);
    }
}

void TheCity::startOfflineMode() {
    isOfflineMode = true;
    setWindowTitle("The City - Offline Mode");
    
    playerList->clear();

    // 1. ADD THE CURRENT ACTIVE PLAYER (IF IN CITY)
    int curX = GameStateManager::instance()->getGameValue("DungeonX").toInt();
    int curY = GameStateManager::instance()->getGameValue("DungeonY").toInt();
    int curLvl = GameStateManager::instance()->getGameValue("DungeonLevel").toInt();

    // Check if current player is at entrance (1, 17, 12)
    if (curLvl == 1 && curX == 17 && curY == 12) {
        QListWidgetItem* selfItem = new QListWidgetItem(
            GameStateManager::instance()->getGameValue("CurrentCharacterName").toString() + " (You)"
        );
        selfItem->setForeground(Qt::blue);
        playerList->addItem(selfItem);
    }

    // 2. SCAN FOR OTHER SAVED CHARACTERS
    QDir charDir("data/characters/");
    if (charDir.exists()) {
        QStringList saveFiles = charDir.entryList({"*.txt"}, QDir::Files);
        QString currentHero = GameStateManager::instance()->getGameValue("CurrentCharacterName").toString();

        for (const QString& fileName : saveFiles) {
            QString charName = fileName.left(fileName.lastIndexOf('.'));

            // Skip the character we are currently playing
            if (charName == currentHero) continue;
            
        // If verification fails, try to repair it
            if (!GameStateManager::instance()->verifySaveGame(charName)) {
                qDebug() << "Attempting to repair broken savegame:" << charName;
                if (!GameStateManager::instance()->repairSaveGame(charName)) {
                    continue; // Skip if repair also fails
                }
            }
            // Peek into the file to check position
            QFile file(charDir.absoluteFilePath(fileName));
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                int x = -1, y = -1, lvl = -1;

                while (!in.atEnd()) {
                    QString line = in.readLine();
                    if (line.startsWith("DungeonX:")) x = line.split(": ")[1].toInt();
                    else if (line.startsWith("DungeonY:")) y = line.split(": ")[1].toInt();
                    else if (line.startsWith("DungeonLevel:")) lvl = line.split(": ")[1].toInt();
                }

                // Only add if they are at the City Entrance coordinates
                if (lvl == 1 && x == 17 && y == 12) {
                    playerList->addItem(charName);
                }
                file.close();
            }
        }
    }
    
    chatDisplay->append("<i style='color:gray;'>Offline mode: Local heroes at the entrance are visible.</i>");
}

TheCity::~TheCity() {}
