#include "include/game_resources.h"
#include "TheCity.h"
#include "src/partyinfo_dialog/partyinfodialog.h"
#include "src/general_store/GeneralStore.h"
#include "src/guilds_dialog/GuildsDialog.h"
#include "src/morgue_dialog/MorgueDialog.h"
#include "src/seer_dialog/SeerDialog.h"
#include "src/confinement_dialog/ConfinementDialog.h"
#include "src/bank_dialog/BankDialog.h"
#include "src/dungeon_dialog/DungeonDialog.h"
#include <QDebug>
#include <QPixmap>
#include <QIcon>
#include <QFile>
#include <QSize>
#include <Qt>
#include <QToolButton>

TheCity::TheCity(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("The City");
    setFixedSize(640, 480); 
    setupUi();
    loadButtonIcons();
    setupStyling();
    QPixmap testPixmap = GameResources::getPixmap("general_store");
    if (!testPixmap.isNull()) 
    {
        qDebug() << "SUCCESS: general_store.png loaded correctly! Size:" << testPixmap.size();
        this->setWindowIcon(QIcon(testPixmap)); 
    } else
    {
        qDebug() << "ERROR: general_store.png failed to load. Check resource paths and file names.";
    }
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
    // --- Grid Layout for Main Buttons ---
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(10);
    // Initialize QToolButtons
    generalStoreButton = new QToolButton(this);
    morgueButton = new QToolButton(this);
    guildsButton = new QToolButton(this);
    dungeonButton = new QToolButton(this);
    confinementButton = new QToolButton(this);
    seerButton = new QToolButton(this);
    bankButton = new QToolButton(this);
    exitButton = new QToolButton(this);
    // Add buttons to the 2x4 grid layout
    gridLayout->addWidget(generalStoreButton, 0, 0);
    gridLayout->addWidget(morgueButton,      0, 1);
    gridLayout->addWidget(guildsButton,      0, 2);
    gridLayout->addWidget(dungeonButton,     0, 3);
    gridLayout->addWidget(confinementButton, 1, 0);
    gridLayout->addWidget(seerButton,        1, 1);
    gridLayout->addWidget(bankButton,        1, 2);
    gridLayout->addWidget(exitButton,        1, 3);
    mainLayout->addLayout(gridLayout);
    // --- Connect Signals (using QToolButton::clicked) ---
    connect(generalStoreButton, &QToolButton::clicked, this, &TheCity::on_generalStoreButton_clicked);
    connect(morgueButton,      &QToolButton::clicked, this, &TheCity::on_morgueButton_clicked);
    connect(guildsButton,      &QToolButton::clicked, this, &TheCity::on_guildsButton_clicked);
    connect(dungeonButton,     &QToolButton::clicked, this, &TheCity::on_dungeonButton_clicked);
    connect(confinementButton, &QToolButton::clicked, this, &TheCity::on_confinementButton_clicked);
    connect(seerButton,        &QToolButton::clicked, this, &TheCity::on_seerButton_clicked);
    connect(bankButton,        &QToolButton::clicked, this, &TheCity::on_bankButton_clicked);
    connect(exitButton,        &QToolButton::clicked, this, &TheCity::on_exitButton_clicked);
}

void TheCity::loadButtonIcons()
{
    const QSize iconSize(120, 90); 
    QPixmap generalStorePixmap = GameResources::getPixmap("general_store");
    QPixmap morguePixmap = GameResources::getPixmap("morgue");
    QPixmap guildsPixmap = GameResources::getPixmap("guilds");
    QPixmap dungeonPixmap = GameResources::getPixmap("dungeon");
    QPixmap confinementPixmap = GameResources::getPixmap("confinement");
    QPixmap seerPixmap = GameResources::getPixmap("seer");
    QPixmap bankPixmap = GameResources::getPixmap("bank");
    QPixmap exitIconPixmap = GameResources::getPixmap("exit_icon");
    generalStoreButton->setIcon(QIcon(generalStorePixmap));
    generalStoreButton->setIconSize(iconSize);
    generalStoreButton->setToolButtonStyle(Qt::ToolButtonIconOnly); 
    generalStoreButton->setToolTip("General Store");
    morgueButton->setIcon(QIcon(morguePixmap));
    morgueButton->setIconSize(iconSize);
    morgueButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    morgueButton->setToolTip("morgue");
    guildsButton->setIcon(QIcon(guildsPixmap));
    guildsButton->setIconSize(iconSize);
    guildsButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    guildsButton->setToolTip("guilds");
    dungeonButton->setIcon(QIcon(dungeonPixmap));
    dungeonButton->setIconSize(iconSize);
    dungeonButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    dungeonButton->setToolTip("dungeon");
    confinementButton->setIcon(QIcon(confinementPixmap));
    confinementButton->setIconSize(iconSize);
    confinementButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    confinementButton->setToolTip("confinement");
    seerButton->setIcon(QIcon(seerPixmap));
    seerButton->setIconSize(iconSize);
    seerButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    seerButton->setToolTip("seer");
    bankButton->setIcon(QIcon(bankPixmap));
    bankButton->setIconSize(iconSize);
    bankButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    bankButton->setToolTip("bank");
    exitButton->setIcon(QIcon(exitIconPixmap));
    exitButton->setIconSize(iconSize);
    exitButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    exitButton->setToolTip("EXIT");
}

void TheCity::setupStyling()
{
    QFile styleFile("TheCity.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        this->setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qDebug() << "Warning: Failed to load stylesheet from resource file! Using default or inline styles.";
    }
    titleLabel->setStyleSheet("font-size: 20px; color: #CCCCCC; background-color: #444444; padding: 5px; border: 1px solid #777777;");
}

void TheCity::on_generalStoreButton_clicked() 
{
    qDebug() << "General Store opened (Modeless)!";
    GeneralStore *store = new GeneralStore(this);
    store->setAttribute(Qt::WA_DeleteOnClose);
    store->show();
}

void TheCity::on_morgueButton_clicked()      
{
    qDebug() << "Morgue opened (Modeless)!";
    MorgueDialog *m = new MorgueDialog(this);
    m->setAttribute(Qt::WA_DeleteOnClose);
    m->show();
}

void TheCity::on_guildsButton_clicked()      
{
    qDebug() << "Guilds opened (Modeless)!";
    GuildsDialog *g = new GuildsDialog(this);
    g->setAttribute(Qt::WA_DeleteOnClose);
    g->show();
}

void TheCity::on_dungeonButton_clicked() 
{
    DungeonDialog *d = new DungeonDialog(nullptr); 
    d->setAttribute(Qt::WA_DeleteOnClose);    
    // Ensure that when the dungeon emits the exit signal, the city shows back up
    connect(d, &DungeonDialog::exitedDungeonToCity, [this]() {
        this->show(); 
    });
    d->show();
    this->hide(); // Hide the city while in the dungeon
}

void TheCity::on_confinementButton_clicked() 
{
    qDebug() << "Confinement opened (Modeless)!";
    ConfinementAndHoldingDialog *c = new ConfinementAndHoldingDialog(this);
    c->setAttribute(Qt::WA_DeleteOnClose);
    c->show();
}

void TheCity::on_seerButton_clicked() 
{
    qDebug() << "Seer opened (Modeless)!";
    SeerDialog *s = new SeerDialog(this);
    s->setAttribute(Qt::WA_DeleteOnClose);
    s->show();
}

void TheCity::on_bankButton_clicked() 
{
    qDebug() << "Bank opened (Modeless)!";
    BankDialog *b = new BankDialog(this);
    b->setAttribute(Qt::WA_DeleteOnClose);
    b->show();
}

void TheCity::on_exitButton_clicked() 
{
    qDebug() << "Exit clicked! Resetting character state and returning to menu.";    
    // 1. Reset the character loaded flag so the Main Menu updates its buttons
    // Based on game_menu.cpp logic, toggleMenuState(false) triggers when the flag is reset
    GameStateManager::instance()->setGameValue("ResourcesLoaded", false);
    // 2. Optional: Reset other character-specific session data
    GameStateManager::instance()->setGameValue("IsCharacterActive", false);
    // 3. Close the City dialog, which will trigger the 'show()' call in GameMenu
    accept(); 
}

void TheCity::keyPressEvent(QKeyEvent *event)
{
    // Check if the "I" key was pressed
    if (event->key() == Qt::Key_I) {
        qDebug() << "Inventory shortcut (I) pressed.";
        InventoryDialog *inv = new InventoryDialog(this);
        // Set to delete on close so we don't leak memory
        inv->setAttribute(Qt::WA_DeleteOnClose);
        // Show as a Modal dialog so user finishes inventory before returning to city
        inv->exec(); 
    } else {
        // Pass other key events (like Escape) to the base class
        QDialog::keyPressEvent(event);
    }
}

TheCity::~TheCity()
{
}

