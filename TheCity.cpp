#include "game_resources.h"
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
#include <QToolButton> // Included for QToolButton usage


TheCity::TheCity(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("The City");
    setFixedSize(640, 480); 

    setupUi();
    loadButtonIcons();
    setupStyling();
    //QPixmap testPixmap("general_store.png");
    QPixmap testPixmap = GameResources::getPixmap("general_store");
    if (!testPixmap.isNull()) 
    {
        qDebug() << "SUCCESS: general_store.png loaded correctly! Size:" << testPixmap.size();
        this->setWindowIcon(QIcon(testPixmap)); 
    } else
    {
        qDebug() << "ERROR: general_store.png failed to load. Check resource paths and file names.";
    }
    // ADD THIS to show the Party Info dialog when entering TheCity
PartyInfoDialog *partyInfo = new PartyInfoDialog(this);
QStringList memberNames;
memberNames << "Alice" << "Bob" << "Charlie" << "Dana";
qDebug() << "Party size:" << memberNames.size() << memberNames;
partyInfo->setPartyMembers(memberNames);
partyInfo->setAttribute(Qt::WA_DeleteOnClose);
partyInfo->show();
}

TheCity::~TheCity()
{
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

    //QPixmap generalStorePixmap("general_store.png"); 
    //QPixmap morguePixmap("morgue.png");
    //QPixmap guildsPixmap("guilds.png");
    //QPixmap dungeonPixmap("dungeon.png");
    //QPixmap confinementPixmap("confinement.png");
    //QPixmap seerPixmap("seer.png");
    //QPixmap bankPixmap("bank.png");
    //QPixmap exitIconPixmap("exit_icon.png"); 

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

void TheCity::on_generalStoreButton_clicked() {
    qDebug() << "General Store clicked!";
    GeneralStore store;
    store.exec();
}
void TheCity::on_morgueButton_clicked()      {
    qDebug() << "Morgue clicked!";
    MorgueDialog m;
    m.exec();
}
void TheCity::on_guildsButton_clicked()      {
    qDebug() << "Guilds clicked!";
    GuildsDialog g;
    g.exec();
}
void TheCity::on_dungeonButton_clicked()     {
    qDebug() << "Dungeon clicked!";
    DungeonDialog d;
    d.exec();
}
void TheCity::on_confinementButton_clicked() {
    qDebug() << "Confinement clicked!";
    ConfinementAndHoldingDialog c;
    c.exec();
}
void TheCity::on_seerButton_clicked()        {
    qDebug() << "Seer clicked!";
    SeerDialog s;
    s.exec();
}
void TheCity::on_bankButton_clicked()        {
    qDebug() << "Bank clicked!";
    BankDialog b;
    b.exec();
}
void TheCity::on_exitButton_clicked()        {
    qDebug() << "Exit clicked! Closing dialog.";
    accept(); 
}
