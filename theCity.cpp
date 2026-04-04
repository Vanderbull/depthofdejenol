
#include "theCity.h"
#include "src/core/game_resources.h"
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


void theCity::processLocation(GameConstants::CityLocation location) {
    switch (location) {
        case GameConstants::CityLocation::GeneralStore:
            titleLabel->setText("General Store - 'Supplies for the Brave'");
            break;
        case GameConstants::CityLocation::Bank:
            titleLabel->setText("The Royal Bank - 'The gold is safe'");
            break;
        case GameConstants::CityLocation::Tavern:
            titleLabel->setText("Tavern - 'Have a beer'");
            // Handle Tavern logic: recruit or rest
            break;
        case GameConstants::CityLocation::Temple:
            titleLabel->setText("Temple - 'Pray you will win'");
            // Handle healing and resurrection
            break;
        case GameConstants::CityLocation::Guild:
            titleLabel->setText("Guild - 'Where heros go'");
            // Access character bank and party management
            break;
        case GameConstants::CityLocation::Street:
        default:
            titleLabel->setText("The City Square");
            // Return to the main city hub
            break;
    }
}

theCity::theCity(QWidget *parent) :
    QDialog(parent)
{
    //gameStateManager::instance()->playMusic("resources/waves/city.wav");

    setWindowTitle("The City - Online");
    setMinimumSize(1000, 800); // Larger default and minimum size
    resize(1000, 800);
    
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
            {"zone", "theCity"}, 
            {"username", "Hero_" + QString::number(randomId)}
        });
    });

    // 3. Try to connect
    NetworkManager::instance()->connectToServer("127.0.0.1", 12345);
    connectionTimer->start(3000); // Wait 3 seconds for server
}

void theCity::setupUi()
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
    connect(generalStoreButton, &QToolButton::clicked, this, &theCity::handleLocationClick);
    connect(morgueButton,      &QToolButton::clicked, this, &theCity::handleLocationClick);
    connect(guildsButton,      &QToolButton::clicked, this, &theCity::handleLocationClick);
    connect(dungeonButton,     &QToolButton::clicked, this, &theCity::handleLocationClick);
    connect(confinementButton, &QToolButton::clicked, this, &theCity::handleLocationClick);
    connect(seerButton,        &QToolButton::clicked, this, &theCity::handleLocationClick);
    connect(bankButton,        &QToolButton::clicked, this, &theCity::handleLocationClick);
    connect(exitButton,        &QToolButton::clicked, this, &theCity::handleLocationClick);

    connect(sendButton,        &QPushButton::clicked, this, &theCity::sendChatMessage);
    connect(chatInput,         &QLineEdit::returnPressed, this, &theCity::sendChatMessage);
/*
    connect(generalStoreButton, &QToolButton::clicked, this, &theCity::on_generalStoreButton_clicked);
    connect(morgueButton,      &QToolButton::clicked, this, &theCity::on_morgueButton_clicked);
    connect(guildsButton,      &QToolButton::clicked, this, &theCity::on_guildsButton_clicked);
    connect(dungeonButton,     &QToolButton::clicked, this, &theCity::on_dungeonButton_clicked);
    connect(confinementButton, &QToolButton::clicked, this, &theCity::on_confinementButton_clicked);
    connect(seerButton,        &QToolButton::clicked, this, &theCity::on_seerButton_clicked);
    connect(bankButton,        &QToolButton::clicked, this, &theCity::on_bankButton_clicked);
    connect(exitButton,        &QToolButton::clicked, this, &theCity::on_exitButton_clicked);
    connect(sendButton,        &QPushButton::clicked, this, &theCity::sendChatMessage);
    connect(chatInput,         &QLineEdit::returnPressed, this, &theCity::sendChatMessage);
*/
}

void theCity::setupMultiplayerConnections()
{
    connect(NetworkManager::instance(), &NetworkManager::playerJoined, this, &theCity::handlePlayerJoined);
    connect(NetworkManager::instance(), &NetworkManager::playerLeft, this, &theCity::handlePlayerLeft);
    connect(NetworkManager::instance(), &NetworkManager::chatReceived, this, &theCity::handleChatReceived);
}

void theCity::handlePlayerJoined(QString name) {
    playerList->addItem(name);
    chatDisplay->append(QString("<i style='color:gray;'>%1 has arrived.</i>").arg(name));
}

void theCity::handlePlayerLeft(QString name) {
    QList<QListWidgetItem*> items = playerList->findItems(name, Qt::MatchExactly);
    for(auto item : items) delete playerList->takeItem(playerList->row(item));
    chatDisplay->append(QString("<i style='color:gray;'>%1 has departed.</i>").arg(name));
}

void theCity::handleChatReceived(QString from, QString message) {
    QString time = QDateTime::currentDateTime().toString("hh:mm");
    chatDisplay->append(QString("[%1] <b>%2:</b> %3").arg(time, from, message));
    chatDisplay->verticalScrollBar()->setValue(chatDisplay->verticalScrollBar()->maximum());
}

void theCity::sendChatMessage() {
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

void theCity::loadButtonIcons()
{
    const QSize iconSize(200, 150); 
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

void theCity::setupStyling()
{
    QFile styleFile("theCity.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        this->setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }
    titleLabel->setStyleSheet("font-size: 20px; color: #CCCCCC; background-color: #444444; padding: 5px; border: 1px solid #777777;");
}

void theCity::on_generalStoreButton_clicked() {
    NetworkManager::instance()->sendAction("enter_location", {{"location", "GeneralStore"}});
    // 1. Update the Global State
    gameStateManager::instance()->enterLocation(GameConstants::CityLocation::GeneralStore);
    // 2. Trigger local processing (music, text updates, etc.)
    processLocation(GameConstants::CityLocation::GeneralStore);
    // 3. Open the Dialog
    GeneralStore *store = new GeneralStore(nullptr);
    store->setAttribute(Qt::WA_DeleteOnClose);

    store->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);

    // When the store closes, reset the title to "The City"
    connect(store, &QDialog::finished, this, [this](){
        this->processLocation(GameConstants::CityLocation::Street);
    });

    store->exec();
}

void theCity::on_morgueButton_clicked() {
    MorgueDialog *m = new MorgueDialog(nullptr);
    m->setAttribute(Qt::WA_DeleteOnClose);
    m->show();
}

void theCity::on_guildsButton_clicked() {
    GuildsDialog *g = new GuildsDialog(nullptr);
    g->setAttribute(Qt::WA_DeleteOnClose);
    g->show();
}

void theCity::on_dungeonButton_clicked() {
    DungeonDialog *d = new DungeonDialog(this); 
    d->setAttribute(Qt::WA_DeleteOnClose);    
    connect(d, &DungeonDialog::exitedDungeonToCity, this, &theCity::show);
    d->show();
    d->raise();
    d->activateWindow();
    d->setFocus();
    this->hide();
}

void theCity::on_confinementButton_clicked() {
    ConfinementAndHoldingDialog *c = new ConfinementAndHoldingDialog(nullptr);
    c->setAttribute(Qt::WA_DeleteOnClose);
    c->show();
}

void theCity::on_seerButton_clicked() {
    SeerDialog *s = new SeerDialog(nullptr);
    s->setAttribute(Qt::WA_DeleteOnClose);
    s->show();
}

void theCity::on_bankButton_clicked() {
    gameStateManager::instance()->enterLocation(GameConstants::CityLocation::Bank);
    processLocation(GameConstants::CityLocation::Bank);

    BankDialog *bank = new BankDialog(this);
    bank->setAttribute(Qt::WA_DeleteOnClose);

    // When the store closes, reset the title to "The City"
    connect(bank, &QDialog::finished, this, [this](){
        this->processLocation(GameConstants::CityLocation::Street);
    });

    bank->exec();

    //BankDialog *b = new BankDialog(this);
    //b->setAttribute(Qt::WA_DeleteOnClose);
    //b->show();
}

void theCity::on_exitButton_clicked() {
    NetworkManager::instance()->sendAction("leave_zone", {{"zone", "theCity"}});
    accept(); 
}

void theCity::keyPressEvent(QKeyEvent *event) {
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

void theCity::startOfflineMode() {
    isOfflineMode = true;
    setWindowTitle("The City - Offline Mode");
    
    playerList->clear();

    // 1. ADD THE CURRENT ACTIVE PLAYER (IF IN CITY)
    int curX = gameStateManager::instance()->getGameValue("DungeonX").toInt();
    int curY = gameStateManager::instance()->getGameValue("DungeonY").toInt();
    int curLvl = gameStateManager::instance()->getGameValue("DungeonLevel").toInt();

    // Check if current player is at entrance (1, 17, 12)
    if (curLvl == 1 && curX == 17 && curY == 12) {
        QListWidgetItem* selfItem = new QListWidgetItem(
            gameStateManager::instance()->getGameValue("CurrentCharacterName").toString() + " (You)"
        );
        selfItem->setForeground(Qt::blue);
        playerList->addItem(selfItem);
    }

    // 2. SCAN FOR OTHER SAVED CHARACTERS
    QDir charDir("data/characters/");
    if (charDir.exists()) {
        QStringList saveFiles = charDir.entryList({"*.txt"}, QDir::Files);
        QString currentHero = gameStateManager::instance()->getGameValue("CurrentCharacterName").toString();

        for (const QString& fileName : saveFiles) {
            QString charName = fileName.left(fileName.lastIndexOf('.'));

            // Skip the character we are currently playing
            if (charName == currentHero) continue;
            
        // If verification fails, try to repair it
            if (!gameStateManager::instance()->verifySaveGame(charName)) {
                qDebug() << "Attempting to repair broken savegame:" << charName;
                if (!gameStateManager::instance()->repairSaveGame(charName)) {
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

void theCity::handleLocationClick() {
    QObject* binder = sender(); // Identify which button was pressed
    QDialog* subDialog = nullptr;

    if (binder == generalStoreButton) {
        processLocation(GameConstants::CityLocation::GeneralStore);
        subDialog = new GeneralStore(this);
    } 
    else if (binder == bankButton) {
        processLocation(GameConstants::CityLocation::Bank);
        subDialog = new BankDialog(this);
    }
    else if (binder == seerButton) {
        processLocation(GameConstants::CityLocation::Temple);
        subDialog = new SeerDialog(this);
    }
    else if (binder == morgueButton) {
        processLocation(GameConstants::CityLocation::None);
        subDialog = new MorgueDialog(this);
    }
    else if (binder == guildsButton) {
        processLocation(GameConstants::CityLocation::Guild);
        subDialog = new GuildsDialog(this);
    }
    else if (binder == confinementButton) {
        processLocation(GameConstants::CityLocation::None);
        subDialog = new ConfinementAndHoldingDialog(this);
    }
    else if (binder == dungeonButton) {
        on_dungeonButton_clicked();
        return;
    }
    else if (binder == exitButton) {
        on_exitButton_clicked();
        return;
    }
    // Add Tavern, Temple, or Morgue here easily...

    if (subDialog) {
        subDialog->setAttribute(Qt::WA_DeleteOnClose);
        
        // Reset title when closing
        connect(subDialog, &QDialog::finished, this, [this]() {
            processLocation(GameConstants::CityLocation::Street);
        });

        subDialog->exec(); 
    }
}

theCity::~theCity() {}

