#include "dungeondialog.h"
#include "dungeonmap.h"
#include "partyinfodialog.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QGuiApplication>
#include <QFile>
#include <QDebug>

DungeonDialog::DungeonDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Mordor 1.1");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi();
}

DungeonDialog::~DungeonDialog() {}

void DungeonDialog::setupUi() {
    // Load external style sheet
    QFile styleSheetFile("style.qss");
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }

    // Get screen geometry to make the window scale with screen resolution
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    
    // Calculate window size as a percentage of screen size
    int windowWidth = screenWidth * 0.90;
    int windowHeight = screenHeight * 0.90;
    
    setMinimumSize(1024, 768);
    resize(windowWidth, windowHeight);

    // Main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // Left side layout (Dungeon and Messages)
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    // Dungeon View
    QVBoxLayout *dungeonLayout = new QVBoxLayout();
    QLabel *dungeonLabel = new QLabel("Dungeon View");
    dungeonLabel->setStyleSheet("font-size: 16px; font-weight: bold; background-color: #2b2b2b; color: #878787; border: 2px solid #5a5a5a; padding: 5px;");
    dungeonLayout->addWidget(dungeonLabel);
    dungeonLayout->addWidget(new QLabel("Dungeon View Placeholder"));
    leftLayout->addLayout(dungeonLayout);

    // Messages
    QVBoxLayout *messagesLayout = new QVBoxLayout();
    QLabel *messagesLabel = new QLabel("Messages");
    messagesLabel->setStyleSheet("font-size: 16px; font-weight: bold; background-color: #2b2b2b; color: #878787; border: 2px solid #5a5a5a; padding: 5px;");
    messagesLayout->addWidget(messagesLabel);
    messagesLayout->addWidget(new QLabel("Messages Placeholder"));
    leftLayout->addLayout(messagesLayout);

    mainLayout->addLayout(leftLayout);
    // Middle section layout (Character Info and Controls)
    QVBoxLayout *middleLayout = new QVBoxLayout();
    // Party Information section
    QLabel *partyInfoLabel = new QLabel("Party Information");
    partyInfoLabel->setStyleSheet("font-size: 16px; font-weight: bold; background-color: #2b2b2b; color: #878787; border: 2px solid #5a5a5a; padding: 5px;");
    middleLayout->addWidget(partyInfoLabel);

    QPushButton *showPartyInfoButton = new QPushButton("Show Party Info");
    connect(showPartyInfoButton, &QPushButton::clicked, this, &DungeonDialog::showPartyInfoDialog);
    middleLayout->addWidget(showPartyInfoButton);

    // Placeholder for other information
    QLabel *infoLabel = new QLabel("Character Info Placeholder");
    infoLabel->setStyleSheet("background-color: #2b2b2b; color: #878787; border: 2px solid #5a5a5a; padding: 10px;");
    middleLayout->addWidget(infoLabel);

    // Controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    QPushButton *switchButton = new QPushButton("Switch To");
    QPushButton *optionsButton = new QPushButton("Options");
    QPushButton *leaveButton = new QPushButton("Leave");
    controlsLayout->addWidget(switchButton);
    controlsLayout->addWidget(optionsButton);
    controlsLayout->addWidget(leaveButton);
    middleLayout->addLayout(controlsLayout);


    mainLayout->addLayout(middleLayout);
    // Right section layout (Auto Map)
    QVBoxLayout *rightLayout = new QVBoxLayout();
    QLabel *mapLabel = new QLabel("Auto Map");
    mapLabel->setStyleSheet("font-size: 16px; font-weight: bold; background-color: #2b2b2b; color: #878787; border: 2px solid #5a5a5a; padding: 5px;");
    rightLayout->addWidget(mapLabel);
    DungeonMap *mapWidget = new DungeonMap(this);
    rightLayout->addWidget(mapWidget, 1);
    mainLayout->addLayout(rightLayout);
}

void DungeonDialog::showPartyInfoDialog() {
    qDebug() << "Show Party Info button clicked";
    PartyInfoDialog *dialog = new PartyInfoDialog(this);
    dialog->show();
}
