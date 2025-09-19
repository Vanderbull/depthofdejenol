#include "gamemenu.h"
#include "hallofrecordsdialog.h"
#include "createcharacterdialog.h"
#include "dungeondialog.h"
#include "inventorydialog.h"
#include "marlith_dialog.h"
#include "optionsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>

GameMenu::GameMenu(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Mordor: The Depths of Dejenol");

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
    int windowWidth = screenWidth * 0.70;
    int windowHeight = screenHeight * 0.65;
    
    // Set a minimum size to prevent the window from becoming too small
    setMinimumSize(800, 600);
    resize(windowWidth, windowHeight);

    // Main layout using a grid to replicate the image layout
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(50, 50, 50, 50);
    gridLayout->setSpacing(20);

    // Placeholder for top-left image (Character List)
    QLabel *topLeftImage = new QLabel();
    gridLayout->addWidget(topLeftImage, 0, 0, 2, 1);

    // Main title
    QLabel *titleLabel = new QLabel("MORDOR");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 30px; font-weight: bold;");

    QLabel *subTitleLabel = new QLabel("The Depths of Dejenol");
    subTitleLabel->setAlignment(Qt::AlignCenter);
    subTitleLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subTitleLabel);

    QWidget *titleWidget = new QWidget();
    titleWidget->setLayout(titleLayout);
    titleWidget->setStyleSheet("background-color: #c1c1c1; color: #000000; border: 1px solid #ffffff; border-top-color: #e0e0e0; border-left-color: #e0e0e0; border-right-color: #646464; border-bottom-color: #646464; padding: 10px;");
    
    gridLayout->addWidget(titleWidget, 0, 1, 1, 2, Qt::AlignCenter);

    // Placeholder for top-right image
    QLabel *topRightImage = new QLabel();
    gridLayout->addWidget(topRightImage, 0, 3, 2, 1);

    // Main buttons
    QPushButton *newButton = new QPushButton("Create a Character");
    QPushButton *loadButton = new QPushButton("Load Character");
    QPushButton *recordsButton = new QPushButton("Hall of Records");
    QPushButton *characterListButton = new QPushButton("Character List");
    QPushButton *helpButton = new QPushButton("Help/Lesson");
    QPushButton *optionsButton = new QPushButton("Options...");
    QPushButton *exitButton = new QPushButton("Exit");
    QPushButton *orderingButton = new QPushButton("Ordering/About");
    QPushButton *inventoryButton = new QPushButton("invenotry");
    QPushButton *marlithButton = new QPushButton("marlith");

    gridLayout->addWidget(newButton, 1, 1, 1, 2, Qt::AlignBottom | Qt::AlignCenter);
    gridLayout->addWidget(loadButton, 2, 1, 1, 2, Qt::AlignTop | Qt::AlignCenter);
    gridLayout->addWidget(characterListButton, 2, 0);
    gridLayout->addWidget(recordsButton, 2, 3);
    gridLayout->addWidget(helpButton, 3, 1);
    gridLayout->addWidget(optionsButton, 3, 2);
    gridLayout->addWidget(exitButton, 4, 1);
    gridLayout->addWidget(orderingButton, 4, 2);
    gridLayout->addWidget(inventoryButton, 5, 2);
    gridLayout->addWidget(marlithButton, 5, 1);
    

    // Placeholder for bottom-left image
    QLabel *bottomLeftImage = new QLabel();
    gridLayout->addWidget(bottomLeftImage, 3, 0, 2, 1);

    // Placeholder for bottom-right image
    QLabel *bottomRightImage = new QLabel();
    gridLayout->addWidget(bottomRightImage, 3, 3, 2, 1);

    // Connections
    connect(newButton, &QPushButton::clicked, this, &GameMenu::startNewGame);
    connect(loadButton, &QPushButton::clicked, this, &GameMenu::loadGame);
    connect(recordsButton, &QPushButton::clicked, this, &GameMenu::showRecords);
    connect(exitButton, &QPushButton::clicked, this, &GameMenu::quitGame);
    connect(inventoryButton, &QPushButton::clicked, this, &GameMenu::onInventoryClicked);
    connect(marlithButton, &QPushButton::clicked, this, &GameMenu::onMarlithClicked);
    connect(optionsButton, &QPushButton::clicked, this, &GameMenu::onOptionsClicked);
}

// Function definitions
void GameMenu::startNewGame() {
    qDebug() << "Start New Game button clicked";
    CreateCharacterDialog *dialog = new CreateCharacterDialog(this);
    dialog->show();
}

void GameMenu::loadGame() {
    qDebug() << "Load Game button clicked";
    DungeonDialog *dialog = new DungeonDialog(this);
    dialog->show();
}

void GameMenu::showRecords() {
    qDebug() << "Hall of Records button clicked";
    HallOfRecordsDialog *recordsDialog = new HallOfRecordsDialog(this);
    recordsDialog->show();
}

void GameMenu::quitGame() {
    qDebug() << "Exit button clicked";
    QApplication::quit();
}

void GameMenu::showCredits() {
    qDebug() << "Show Credits placeholder";
    // TODO: Implement the Credits Dialog here
}

void GameMenu::onInventoryClicked() {
    InventoryDialog *inventoryDialog = new InventoryDialog(this);
    inventoryDialog->exec(); // Use exec() to make it a modal dialog
}

void GameMenu::onMarlithClicked() {
    MarlithDialog *dialog = new MarlithDialog(this);
    dialog->show(); // Use exec() to make it a modal dialog
}

void GameMenu::onOptionsClicked() {
    qDebug() << "Options button clicked";
    OptionsDialog *optionsDialog = new OptionsDialog(this);
    optionsDialog->exec(); // Use exec() to make it a modal dialog
}

GameMenu::~GameMenu() {}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    GameMenu w;
    w.show();
    return a.exec();
}
