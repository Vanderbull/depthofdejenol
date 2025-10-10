#include "gamemenu.h"
#include "hallofrecordsdialog.h"
#include "createcharacterdialog.h"
#include "dungeondialog.h"
#include "inventorydialog.h"
#include "marlith_dialog.h"
#include "optionsdialog.h"
#include "AboutDialog.h"
#include "MonsterEditorDialog.h"
#include "SpellbookEditorDialog.h"
#include "CharacterDialog.h"
#include "MessageWindow.h"
#include "SenderWindow.h"
#include "library_dialog.h"
#include "automap_dialog.h"

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
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QDateTime>

// If this code is running in a context where 'this' is not available:
void launchAutomapDialog() {
    // Using nullptr as the parent widget, as 'this' is not available here.
    // If you are inside a class like MainGameWindow, use AutomapDialog mapDialog(this);
    AutomapDialog mapDialog(nullptr); 

    // Execute the dialog modally
    mapDialog.exec(); 

    qDebug() << "Automap dialog closed.";
}



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
    QPushButton *aboutButton = new QPushButton("About");
    QPushButton *inventoryButton = new QPushButton("invenotry");
    QPushButton *marlithButton = new QPushButton("marlith");
    QPushButton *creditsButton = new QPushButton("Credits");

    gridLayout->addWidget(newButton, 1, 1, 1, 2, Qt::AlignBottom | Qt::AlignCenter);
    gridLayout->addWidget(loadButton, 2, 1, 1, 2, Qt::AlignTop | Qt::AlignCenter);
    gridLayout->addWidget(characterListButton, 2, 0);
    gridLayout->addWidget(recordsButton, 2, 3);
    gridLayout->addWidget(helpButton, 3, 1);
    gridLayout->addWidget(optionsButton, 3, 2);
    gridLayout->addWidget(exitButton, 4, 1);
    gridLayout->addWidget(aboutButton, 4, 2);
    gridLayout->addWidget(inventoryButton, 5, 2);
    gridLayout->addWidget(marlithButton, 5, 1);
    gridLayout->addWidget(creditsButton, 6, 0);
    

    // Placeholder for bottom-left image
    QLabel *bottomLeftImage = new QLabel();
    gridLayout->addWidget(bottomLeftImage, 3, 0, 2, 1);

    // Placeholder for bottom-right image
    QLabel *bottomRightImage = new QLabel();
    gridLayout->addWidget(bottomRightImage, 3, 3, 2, 1);


    // 1. CREATE THE LOGGER WINDOW
    // Create it on the heap. We'll make it a top-level window (no parent).
    MessagesWindow *loggerWindow = new MessagesWindow();

    // If you made it a private member (m_loggerWindow) you would use:
    // m_loggerWindow = new MessagesWindow();

    // 2. ESTABLISH THE CONNECTION
    // Connect THIS object's signal (GameMenu::logMessageTriggered)
    // to the loggerWindow's slot (MessagesWindow::logMessage).
    QObject::connect(this, &GameMenu::logMessageTriggered,
                     loggerWindow, &MessagesWindow::logMessage);

    // 3. SHOW THE LOGGER WINDOW
    loggerWindow->show();

    // 4. (DEMO) EMIT A TEST MESSAGE
    // Use the signal you just connected to immediately log a message.
    emit logMessageTriggered("GameMenu has successfully initialized the Messages Log.");

    // 5. ... rest of GameMenu setup (buttons, layouts, etc.)
    // Connections
    connect(newButton, &QPushButton::clicked, this, &GameMenu::startNewGame);
    connect(loadButton, &QPushButton::clicked, this, &GameMenu::loadGame);
    connect(recordsButton, &QPushButton::clicked, this, &GameMenu::showRecords);
    connect(exitButton, &QPushButton::clicked, this, &GameMenu::quitGame);
    connect(inventoryButton, &QPushButton::clicked, this, &GameMenu::onInventoryClicked);
    connect(marlithButton, &QPushButton::clicked, this, &GameMenu::onMarlithClicked);
    connect(optionsButton, &QPushButton::clicked, this, &GameMenu::onOptionsClicked);
    connect(aboutButton, &QPushButton::clicked, this, &GameMenu::onAboutClicked);
    connect(creditsButton, &QPushButton::clicked, this, &GameMenu::showCredits);
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

void GameMenu::showRecords() {    // Create a dynamic message
    //QString message = QString("Button clicked at %1.").arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"));
    // ... game logic
    emit logMessageTriggered("User entered hall of records");
    qDebug() << "User entered hall of records";
    HallOfRecordsDialog *recordsDialog = new HallOfRecordsDialog(this);
    recordsDialog->show();
}

void GameMenu::quitGame() {
    qDebug() << "Exit button clicked";
    QApplication::quit();
}

void GameMenu::showCredits() {

    qDebug() << "Showing Character Stats (Non-Modal)";

    // 1. Create the dialog on the heap.
    CharacterDialog *charDialog = new CharacterDialog("Goodie Gil'thrialle");

    // 2. IMPORTANT: Tell Qt to delete the object when the user closes the window.
    charDialog->setAttribute(Qt::WA_DeleteOnClose);

    // 3. Call show() to display the dialog non-modally.
    // Execution continues immediately, and the main Qt event loop handles the dialog.
    charDialog->show();
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
void GameMenu::onAboutClicked() {
    qDebug() << "About button clicked";
    AboutDialog *aboutDialog = new AboutDialog(this);
    aboutDialog->exec(); // Use exec() to make it a modal dialog
}

void GameMenu::onEditMonsterClicked() {
    MonsterEditorDialog editor(this);

    // Show the dialog modally and check the result
    if (editor.exec() == QDialog::Accepted) {
        // If the user clicked "Save Monster"
        QString monsterData = editor.getMonsterData();
        qDebug() << "New Monster Data Saved:\n" << monsterData;
        // Logic to process/save the data goes here
    } else {
        // If the user clicked "Cancel"
        qDebug() << "Monster editing cancelled.";
    }
}


void GameMenu::onEditSpellbookClicked() {
    SpellbookEditorDialog editor(this);

    // Run the dialog modally (blocks until closed)
    if (editor.exec() == QDialog::Accepted) {
        // Retrieve and use the saved spell data
        QString spellData = editor.getSpellData();
        qDebug() << "New Spell Data Saved:\n" << spellData;
    } else {
        qDebug() << "Spell editing cancelled.";
    }
}

GameMenu::~GameMenu() {}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    GameMenu w;
    w.show();

    // Create a media player object
    QMediaPlayer *player = new QMediaPlayer;
    
    // Create an audio output object
    QAudioOutput *audioOutput = new QAudioOutput;
    player->setAudioOutput(audioOutput);
    
    // Set the audio source. Replace "path/to/your/audio.mp3" with the actual path.
    // Use QUrl::fromLocalFile for a local file path.
    player->setSource(QUrl::fromLocalFile("mordor.mp3"));

    // Connect signals to handle different states (optional but recommended for robustness)
    QObject::connect(player, &QMediaPlayer::mediaStatusChanged, [](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia) {
            qDebug() << "Media loaded successfully.";
        } else if (status == QMediaPlayer::EndOfMedia) {
            qDebug() << "Playback finished.";
        }
    });

    QObject::connect(player, &QMediaPlayer::errorOccurred, [](QMediaPlayer::Error error, const QString &errorString) {
        qDebug() << "Error:" << errorString;
    });

    // Start playing the audio
    audioOutput->setVolume(50); // Set volume (0.0 to 100.0)
    player->play();

    qDebug() << "Playing audio...";

    LibraryDialog library;
    library.exec();

    launchAutomapDialog();

    return a.exec();
}
