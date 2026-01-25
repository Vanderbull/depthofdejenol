
#include "GameStateManager.h"
#include "game_menu.h"
#include "TheCity.h"
#include "StoryDialog.h"

#include "tools/spellbook_editor/SpellbookEditorDialog.h"
#include "tools/monster_editor/MonsterEditorDialog.h"

#include "src/characterlist_dialog/characterlistdialog.h"
#include "src/hall_of_records/hallofrecordsdialog.h"
#include "src/create_character/createcharacterdialog.h"
#include "src/inventory_dialog/inventorydialog.h"
#include "src/options_dialog/optionsdialog.h"
#include "src/about_dialog/AboutDialog.h"
#include "src/character_dialog/CharacterDialog.h"
#include "src/message_window/MessageWindow.h"
#include "src/sender_window/SenderWindow.h"
#include "src/library_dialog/library_dialog.h"
#include "src/automap/automap_dialog.h"
#include "src/game_controller/game_controller.h"
#include "src/helplesson/helplesson.h"
#include "src/mordorstatistics/mordorstatistics.h"
#include "src/loadingscreen/LoadingScreen.h"
#include "src/race_data/RaceData.h"
#include "src/event/EventManager.h"
// Qt component includes
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QDateTime>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QComboBox>
#include <QFileInfoList>
#include <QResizeEvent>
// Global function to launch Automap
void launchAutomapDialog() 
{
    AutomapDialog mapDialog(nullptr); 
    mapDialog.show(); 
    qDebug() << "Automap dialog triggered";
}

GameMenu::GameMenu(QWidget *parent)
    : QWidget(parent)
    , m_settings("MyCompany", "MyApp")
    , m_subfolderName(m_settings.value("Paths/SubfolderName", "data/characters").toString()) 
{

    // --- Setup City Audio ---
    m_Player = new QMediaPlayer(this);
    m_AudioOutput = new QAudioOutput(this);
    m_Player->setAudioOutput(m_AudioOutput);
    
    // Replace "city.wav" with your actual file path
    m_Player->setSource(QUrl::fromLocalFile("resources/waves/main.wav")); 
    m_AudioOutput->setVolume(0.5); // Adjust volume as needed
    
    m_Player->play();

    // 1. DONT USE GLOBAL STYLESHEETS (prevents black buttons)
    // 2. USE PALETTE FOR BACKGROUND (Wayland friendly)
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, Qt::black);
    this->setPalette(pal);
    this->setAutoFillBackground(true);
    // This hint tells Wayland: "I want a title bar and an X button"
    this->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | 
                         Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    qDebug() << "Current Game Version:" << GameStateManager::instance()->getGameValue("GameVersion").toString();

    EventManager::instance()->loadEvents("./data/events-json");
    connect(EventManager::instance(), &EventManager::eventTriggered,
            this, &GameMenu::onEventTriggered);
    EventManager::instance()->update("GAME_START");
    qDebug() << "Configured Subfolder Name:" << m_subfolderName;

    setWindowTitle("The Depths of Dejenol: Black Lands");
    // 1. Get the directory of THIS .cpp file
    // __FILE__ is a built-in macro that gives the path to SeerDialog.cpp
    QFileInfo fileInfo(__FILE__);
    QString dir = fileInfo.absolutePath();

    // 2. Build the path to the .qss file in the same folder
    QString qssPath = dir + "/MainMenu.qss";
    // 3. Load and apply the style
    QFile styleFile(qssPath);
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&styleFile);
        this->setStyleSheet(ts.readAll());
        styleFile.close();
        qDebug() << "Style file found at:" << qssPath;
    } else {
        qDebug() << "Style file not found at:" << qssPath;
    }
    // --- Background Image Loading and Scaling Setup ---
    // 1. Get the directory of the running executable
    QString appDirPath = QCoreApplication::applicationDirPath();
    // 2. Construct the full, absolute path to the background image
    QString imagePath = QDir::cleanPath(appDirPath + QDir::separator() + "introtitle.png");
    qDebug() << "Attempting to load background image from:" << imagePath;
    // Load the original image into the member variable
    m_backgroundPixmap.load(imagePath);
    
    if (!m_backgroundPixmap.isNull()) {
        // Set auto-fill so the palette brush is used
        setAutoFillBackground(true);
        // Trigger a resize event immediately to apply the initial scaling
        resizeEvent(nullptr); 
        qDebug() << "Successfully loaded background image.";
        GameStateManager::instance()->setGameValue("ResourcesLoaded", true); 
    } else {
        qDebug() << "FATAL: Could not load background image from:" << imagePath;
        GameStateManager::instance()->setGameValue("ResourcesLoaded", false);
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
    
    // --- Button Widgets (using member variables) ---
    // SECOND MENU BUTTON: Run Character
    m_runButton = new QPushButton("Run Character");
    gridLayout->addWidget(m_runButton, 1, 1, 1, 2, Qt::AlignBottom | Qt::AlignCenter);
    connect(m_runButton, &QPushButton::clicked, this, &GameMenu::onRunClicked);
    // FIRST MENU BUTTONS
    m_newButton = new QPushButton("Create a Character");
    gridLayout->addWidget(m_newButton, 1, 1, 1, 2, Qt::AlignBottom | Qt::AlignCenter);
    connect(m_newButton, &QPushButton::clicked, this, &GameMenu::startNewGame);

    m_loadButton = new QPushButton("Load Character");
    gridLayout->addWidget(m_loadButton, 2, 1, 1, 2, Qt::AlignTop | Qt::AlignCenter);
    connect(m_loadButton, &QPushButton::clicked, this, &GameMenu::loadGame);
    
    m_helpButton = new QPushButton("Help/Lesson");
    gridLayout->addWidget(m_helpButton, 3, 1);
    connect(m_helpButton, &QPushButton::clicked, this, &GameMenu::onHelpClicked);

    m_recordsButton = new QPushButton("Hall of Records");
    gridLayout->addWidget(m_recordsButton, 2, 3);
    connect(m_recordsButton, &QPushButton::clicked, this, &GameMenu::showRecords);

    m_characterListButton = new QPushButton("Character List");
    gridLayout->addWidget(m_characterListButton, 2, 0);
    connect(m_characterListButton, &QPushButton::clicked, this, &GameMenu::onCharacterListClicked);

    m_optionsButton = new QPushButton("Options...");
    gridLayout->addWidget(m_optionsButton, 3, 2);
    connect(m_optionsButton, &QPushButton::clicked, this, &GameMenu::onOptionsClicked);

    m_exitButton = new QPushButton("Exit");
    gridLayout->addWidget(m_exitButton, 4, 1);
    connect(m_exitButton, &QPushButton::clicked, this, &GameMenu::quitGame);

    m_aboutButton = new QPushButton("About");
    gridLayout->addWidget(m_aboutButton, 4, 2);
    connect(m_aboutButton, &QPushButton::clicked, this, &GameMenu::onAboutClicked);

    // Logging Window Setup
    MessagesWindow *loggerWindow = new MessagesWindow();
    QObject::connect(this, &GameMenu::logMessageTriggered,
                     loggerWindow, &MessagesWindow::logMessage);
    loggerWindow->show();
    emit logMessageTriggered("GameMenu has successfully initialized the Messages Log.");

    toggleMenuState(false); 
    
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_player->setSource(QUrl::fromLocalFile("mordor.mp3"));
    // Connect signals to handle different states
    QObject::connect(m_player, &QMediaPlayer::mediaStatusChanged, [](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia) {
            qDebug() << "Media loaded successfully.";
        } else if (status == QMediaPlayer::EndOfMedia) {
            qDebug() << "Playback finished.";
        }
    });

    QObject::connect(m_player, &QMediaPlayer::errorOccurred, 
                     [](QMediaPlayer::Error /*error*/, const QString &errorString) {
        qDebug() << "Error playing audio:" << errorString;
    });

    m_audioOutput->setVolume(0.5); // Set volume (0.0 to 1.0 in modern Qt)
    m_player->play();
    qDebug() << "Playing audio from GameMenu constructor...";
    
    if (GameStateManager::instance()->areResourcesLoaded()) {
        qDebug() << "Initial resource check passed via GameStateManager. Player Gold:" << GameStateManager::instance()->getGameValue("PlayerGold").toULongLong();
    }
}

void GameMenu::onEventTriggered(const GameEvent& event)
{
    qDebug() << "Triggered event:" << event.id << event.description;
/*
    if (event.effect == "SHOW_MESSAGE")
    {
        QMessageBox::information(this, tr("Game Message"), event.description);
    }
*/
}
// Override for proper background scaling on window resize
void GameMenu::resizeEvent(QResizeEvent *event) 
{
    if (!m_backgroundPixmap.isNull()) {
        QPixmap scaledPixmap = m_backgroundPixmap.scaled(size(), 
                                                         Qt::IgnoreAspectRatio, 
                                                         Qt::SmoothTransformation);
        QPalette palette;
        palette.setBrush(QPalette::Window, scaledPixmap);
        setPalette(palette);
    }
    QWidget::resizeEvent(event);
}
// Centralized function to control button visibility
void GameMenu::toggleMenuState(bool characterIsLoaded) 
{
    // Character Loaded State: Show 'Run Character'
    if (characterIsLoaded) {
        m_runButton->setVisible(true);
        m_loadButton->setVisible(false);
        m_newButton->setVisible(false);
    } 
    // Initial State: Show 'New Character' and 'Load Character'
    else { 
        m_runButton->setVisible(false);
        m_loadButton->setVisible(true);
        m_newButton->setVisible(true);
    }
}

/**
 * @brief Handles the signal that a character was successfully created and saved.
 * Switches the main menu buttons to show the 'Run Character' option.
 * @param characterName The name of the newly created character.
 */
void GameMenu::onCharacterCreated(const QString &characterName) 
{
    emit logMessageTriggered(QString("New character **%1** created successfully!").arg(characterName));
    // --- UPDATED: Use GameStateManager to save the character data ---
    // Assuming the newly created character is at Party Index 0
    if (GameStateManager::instance()->saveCharacterToFile(0)) {
        qDebug() << "Character" << characterName << "saved to disk via GameStateManager.";
    } else {
        qWarning() << "Failed to save character" << characterName << "to disk.";
    }
    QMessageBox::information(this, tr("Creation Successful"), 
                             tr("Character **%1** created successfully. Ready to run.").arg(characterName));
    toggleMenuState(true);
// Start the 10-second loop
    GameStateManager::instance()->startAutosave(10000);
}

void GameMenu::onRunClicked() 
{
    m_Player->stop();
    // 1. Instantiate the City dialog
    TheCity *cityDialog = new TheCity(this);  
    cityDialog->setAttribute(Qt::WA_DeleteOnClose);    
    // 2. Hide the GameMenu while the player is in the city
    this->hide(); 
    // 3. Connect the finished signal to handle returning to the menu
    // This lambda triggers when TheCity is closed or the Exit button is clicked
    connect(cityDialog, &QDialog::finished, this, [this](int result){
	Q_UNUSED(result);
        // Check the current state of "ResourcesLoaded" from the GameStateManager
        // If the City's Exit button reset this flag, toggleMenuState will hide "Run Character"
        bool isLoaded = GameStateManager::instance()->getGameValue("ResourcesLoaded").toBool();
        this->toggleMenuState(isLoaded); 
        // Show the GameMenu again
        this->show();
    });
    // 4. Launch the city dialog
    cityDialog->show();
    qDebug() << "Run Character clicked - GameMenu hidden, launching TheCity";
}


void GameMenu::onCharacterListClicked() 
{
    CharacterListDialog *dialog = new CharacterListDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    qDebug() << "Character list clicked";
}

void GameMenu::startNewGame() 
{
    m_Player->stop();
    // 1. Load data required by the new CreateCharacterDialog constructor.
    QVector<RaceStats> raceData = loadRaceData();
    QVector<QString> guildData = loadGuildData();    
    // 2. Instantiate the dialog using the new constructor signature.
    CreateCharacterDialog *dialog = new CreateCharacterDialog(raceData, guildData, this);
    // 3. CRITICAL STEP: Connect the dialog's success signal to the new slot.
    // This connects the event from the dialog to the menu state logic.
    QObject::connect(dialog, SIGNAL(characterCreated(const QString &)), 
                     this, SLOT(onCharacterCreated(const QString &)));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    qDebug() << "Start New Game button clicked";
}

void GameMenu::loadGame() 
{
// Start the 10-second loop
    GameStateManager::instance()->startAutosave(10000);
    // 1. Setup file path and selection dialog 
    QString basePath = QCoreApplication::applicationDirPath();
    QString fullPath = QDir::cleanPath(basePath + QDir::separator() + m_subfolderName);
    QDir subfolderDir(fullPath);

    if (!subfolderDir.exists()) {
        subfolderDir.mkpath("."); //
        emit logMessageTriggered("No character folder found. Created one.");
        return;
    }
    // Filter for .txt files
    QStringList characterFiles = subfolderDir.entryList({"*.txt"}, QDir::Files);

    if (characterFiles.isEmpty()) {
        emit logMessageTriggered("No character (.txt) files found to load.");
        QMessageBox::warning(this, tr("No Characters Found"), 
                             tr("No character files were found in: %1").arg(m_subfolderName));
        return;
    }
    // 2. Create Selection Dialog
    QDialog selectionDialog(this);
    selectionDialog.setWindowTitle("Load Character");
    QVBoxLayout *layout = new QVBoxLayout(&selectionDialog);
    
    QComboBox *charComboBox = new QComboBox();
    charComboBox->addItems(characterFiles);
    layout->addWidget(new QLabel("Select a character:"));
    layout->addWidget(charComboBox);
    
    QPushButton *loadButton = new QPushButton("Load");
    layout->addWidget(loadButton);
    connect(loadButton, &QPushButton::clicked, &selectionDialog, &QDialog::accept);

    if (selectionDialog.exec() == QDialog::Accepted) {
        QString selectedFileName = charComboBox->currentText();
        // Remove .txt extension to pass the "name" to the manager 
        QString characterName = QFileInfo(selectedFileName).baseName();
        emit logMessageTriggered(QString("Attempting to load: %1").arg(characterName));
        // 3. Perform the Load and check Vitality
        if (GameStateManager::instance()->loadCharacterFromFile(characterName)) {
            // We check the "isAlive" flag from the newly loaded state
            int isAlive = GameStateManager::instance()->getGameValue("isAlive").toInt();
            qDebug() << "DEBUG: Character" << characterName << "isAlive status is:" << isAlive;
            if (isAlive == 0) {
                emit logMessageTriggered(QString("Load failed: %1 is dead.").arg(characterName));
                QMessageBox::critical(this, tr("Character Deceased"), 
                                     tr("The character **%1** is dead and cannot be loaded.").arg(characterName));
                return; // Stop the loading process here
            }
            // Post-load initialization (only if alive)
            GameStateManager::instance()->setGameValue("CurrentCharacterStatPointsLeft", 0);
            
            emit logMessageTriggered(QString("Success: %1 is ready.").arg(characterName));
            QMessageBox::information(this, tr("Load Successful"), 
                                     tr("Character **%1** loaded successfully.").arg(characterName));
            // Switch UI state to show 'Run Character'
            toggleMenuState(true);
        } else {
            emit logMessageTriggered(QString("Error: Failed to load %1.").arg(selectedFileName));
            QMessageBox::critical(this, tr("Load Failed"), 
                                 tr("The file **%1** could not be parsed.").arg(selectedFileName));
        }
    }
}

void GameMenu::showRecords() 
{
    emit logMessageTriggered("User entered hall of records");
    HallOfRecordsDialog *recordsDialog = new HallOfRecordsDialog(this);
    recordsDialog->setAttribute(Qt::WA_DeleteOnClose);
    recordsDialog->show();
    qDebug() << "User entered hall of records";
}

void GameMenu::quitGame() 
{
    QApplication::quit();
    qDebug() << "Exit button clicked";
}

void GameMenu::showCredits() 
{
    qDebug() << "ShowCredits (placeholder) clicked";
}

void GameMenu::onInventoryClicked() 
{
    InventoryDialog *inventoryDialog = new InventoryDialog(this);
    inventoryDialog->setAttribute(Qt::WA_DeleteOnClose);
    inventoryDialog->show();
    qDebug() << "Inventory button clicked";
}

void GameMenu::onMarlithClicked() 
{
    qDebug() << "Marlith (placeholder) clicked";
}

void GameMenu::onOptionsClicked() 
{
    OptionsDialog *optionsDialog = new OptionsDialog(this);
    optionsDialog->setAttribute(Qt::WA_DeleteOnClose);
    optionsDialog->show();
    qDebug() << "Options button clicked";
}

void GameMenu::onAboutClicked() 
{
    AboutDialog *aboutDialog = new AboutDialog(this);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
    aboutDialog->show();
    qDebug() << "About button clicked";
}

void GameMenu::onHelpClicked() 
{
    HelpLessonDialog *helpDialog = new HelpLessonDialog(this);
    helpDialog->setAttribute(Qt::WA_DeleteOnClose);
    helpDialog->show(); 
    emit logMessageTriggered("User viewed the Help/Lesson dialog.");
    qDebug() << "Help/Lesson dialog closed.";
}

void GameMenu::onEditMonsterClicked() 
{
    MonsterEditorDialog editor(this);
    // Show the dialog modally and check the result
    if (editor.exec() == QDialog::Accepted) {
        // If the user clicked "Save Monster"
        QString monsterData = editor.getMonsterData();
        qDebug() << "New Monster Data Saved:\n" << monsterData;
        // Logic to process/save the data goes here
    } else {
        qDebug() << "Monster editing cancelled.";
    }
}

void GameMenu::onEditSpellbookClicked() 
{
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

void GameMenu::onShowStatisticsClicked() 
{
    emit logMessageTriggered("User opened Statistics dialog.");
    MordorStatistics *statsDialog = new MordorStatistics(this);
    statsDialog->setAttribute(Qt::WA_DeleteOnClose);
    statsDialog->show();
    qDebug() << "Statistics dialog opened.";
}

GameMenu::~GameMenu() 
{
    qDebug() << "GameMenu Destructor.";
}

int main(int argc, char *argv[]) 
{
    // FORCE X11/XCB Platform
    qputenv("QT_QPA_PLATFORM", "xcb");
    QApplication a(argc, argv);
    qDebug() << "Launching LoadingScreen dialog...";
    LoadingScreen loadingScreen; 
    loadingScreen.exec(); 
    qDebug() << "LoadingScreen dialog closed. Starting main application...";
    qDebug() << "Launching Starting Story...";
    StoryDialog story;
    story.exec();
    GameMenu w;
    // 2. INSTANTIATE THE GAME CONTROLLER
    // Use the main window (w) as its parent, although it's not strictly necessary.
    //  GameController *controller = new GameController(&w);
    // 3. INSTALL THE GAME CONTROLLER AS AN EVENT FILTER
    // This is the key step. It tells Qt that *before* the key event is delivered 
    // to the main window (w) or any of its children, the controller gets a chance
    // to inspect and handle it.
    //    w.installEventFilter(controller);
    //    qDebug() << "GameController installed as event filter on GameMenu.";
    w.show();
    return a.exec();
}

