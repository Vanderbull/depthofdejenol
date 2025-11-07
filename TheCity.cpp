#include "TheCity.h"
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
// TEMPORARY TEST CODE in TheCity::TheCity(QWidget *parent) constructor
QPixmap testPixmap(":/general_store.png");
if (!testPixmap.isNull()) {
    qDebug() << "SUCCESS: general_store.png loaded correctly! Size:" << testPixmap.size();
    // Set a window icon using the loaded pixmap
    //this->setWindowIcon(QIcon(testPixmap)); 
} else {
    qDebug() << "ERROR: general_store.png failed to load. Check resource paths and file names.";
}

}

TheCity::~TheCity()
{
}

void TheCity::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // --- Title Label ---
    titleLabel = new QLabel("The City", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // --- Grid Layout for Main Buttons ---
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(10);

    //QSize buttonMinSize(160, 140);
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
//    connect(smallExitButton,   &QToolButton::clicked, this, &TheCity::on_exitButton_clicked);
}

void TheCity::loadButtonIcons()
{
    // IMPORTANT: Replace these with your actual QPixmap/QIcon loading logic 

    const QSize iconSize(120, 90); 

    // --- Load Pixmaps (Placeholders) ---
    QPixmap generalStorePixmap(":/general_store.png"); 
    QPixmap morguePixmap(":/morgue.png");
    QPixmap guildsPixmap(":/guilds.png");
    QPixmap dungeonPixmap(":/dungeon.png");
    QPixmap confinementPixmap(":/confinement.png");
    QPixmap seerPixmap(":/seer.png");
    QPixmap bankPixmap(":/bank.png");
    QPixmap exitIconPixmap(":/exit_icon.png"); 

    // --- Apply Icons and Styles (setToolButtonStyle now works!) ---
    // Standard buttons with text under icon
    generalStoreButton->setIcon(QIcon(generalStorePixmap));
    generalStoreButton->setIconSize(iconSize);
    generalStoreButton->setToolButtonStyle(Qt::ToolButtonIconOnly); 

    morgueButton->setIcon(QIcon(morguePixmap));
    morgueButton->setIconSize(iconSize);
    morgueButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    guildsButton->setIcon(QIcon(guildsPixmap));
    guildsButton->setIconSize(iconSize);
    guildsButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    dungeonButton->setIcon(QIcon(dungeonPixmap));
    dungeonButton->setIconSize(iconSize);
    dungeonButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    confinementButton->setIcon(QIcon(confinementPixmap));
    confinementButton->setIconSize(iconSize);
    confinementButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    seerButton->setIcon(QIcon(seerPixmap));
    seerButton->setIconSize(iconSize);
    seerButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    bankButton->setIcon(QIcon(bankPixmap));
    bankButton->setIconSize(iconSize);
    bankButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    exitButton->setIcon(QIcon(exitIconPixmap));
    exitButton->setIconSize(iconSize);
    exitButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

void TheCity::setupStyling()
{
    QFile styleFile(":/TheCity.qss"); 
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        this->setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qDebug() << "Warning: Failed to load stylesheet from resource file! Using default or inline styles.";
    }

    titleLabel->setStyleSheet("font-size: 20px; color: #CCCCCC; background-color: #444444; padding: 5px; border: 1px solid #777777;");
}

// --- Slot Implementations ---

void TheCity::on_generalStoreButton_clicked() { qDebug() << "General Store clicked!"; }
void TheCity::on_morgueButton_clicked()      { qDebug() << "Morgue clicked!"; }
void TheCity::on_guildsButton_clicked()      { qDebug() << "Guilds clicked!"; }
void TheCity::on_dungeonButton_clicked()     { qDebug() << "Dungeon clicked!"; }
void TheCity::on_confinementButton_clicked() { qDebug() << "Confinement clicked!"; }
void TheCity::on_seerButton_clicked()        { qDebug() << "Seer clicked!"; }
void TheCity::on_bankButton_clicked()        { qDebug() << "Bank clicked!"; }
void TheCity::on_exitButton_clicked()        {
    qDebug() << "Exit clicked! Closing dialog.";
    accept(); 
}
