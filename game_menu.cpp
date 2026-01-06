// game_menu.cpp
#include "game_menu.h"
// Include all necessary class definitions for dialogs and Qt components
#include "src/characterlist_dialog/characterlistdialog.h"
#include "src/hall_of_records/hallofrecordsdialog.h"
#include "src/create_character/createcharacterdialog.h"
#include "src/inventory_dialog/inventorydialog.h"
#include "src/options_dialog/optionsdialog.h"
#include "src/about_dialog/AboutDialog.h"
#include "tools/monster_editor/MonsterEditorDialog.h"
#include "tools/spellbook_editor/SpellbookEditorDialog.h"
#include "src/character_dialog/CharacterDialog.h"
#include "src/message_window/MessageWindow.h"
#include "src/sender_window/SenderWindow.h"
#include "src/library_dialog/library_dialog.h"
#include "src/automap/automap_dialog.h"
#include "src/game_controller/game_controller.h"
#include "src/helplesson/helplesson.h"
#include "src/mordorstatistics/mordorstatistics.h"
#include "src/loadingscreen/LoadingScreen.h"
#include "TheCity.h"
#include "StoryDialog.h"
// --- NEW INCLUDE ADDED FOR DATA LOADING ---
#include "src/race_data/RaceData.h"
#include "src/event/EventManager.h"
// --- GAME STATE INTEGRATION ---
#include "GameStateManager.h"
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
// The global MenuSwitch and QSettings variables are removed (now members)
// QString subfolderName is now a member variable (m_subfolderName)

// Global function to launch Automap (unchanged)
void launchAutomapDialog() {
    // Using nullptr as the parent widget, as 'this' is not available here.
    // If you are inside a class like MainGameWindow, use AutomapDialog mapDialog(this);
    AutomapDialog mapDialog(nullptr);
    // Execute the dialog modally
    //mapDialog.exec(); 
    mapDialog.show(); 
    qDebug() << "Automap dialog triggered";
}

GameMenu::GameMenu(QWidget *parent)
    : QWidget(parent)
    // Initialize QSettings and subfolderName in the initializer list
    , m_settings("MyCompany", "MyApp")
    , m_subfolderName(m_settings.value("Paths/SubfolderName", "data/characters").toString()) 
{
    // --- GAME STATE INTEGRATION START ---
    // LOG INITIAL GAME STATE INFO
    qDebug() << "Current Game Version:" << GameStateManager::instance()->getGameValue("GameVersion").toString();
    // --- GAME STATE INTEGRATION END ---

    //EventManager::instance()->loadEvents("./data/events-json");
    // Connect event system to this menu
    connect(EventManager::instance(), &EventManager::eventTriggered,
            this, &GameMenu::onEventTriggered);

    // Immediately trigger GAME_START event(s)
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
        
        // --- GAME STATE INTEGRATION EXAMPLE 1: Set a state value ---
        GameStateManager::instance()->setGameValue("ResourcesLoaded", true); 

    } else {
        qDebug() << "FATAL: Could not load background image from:" << imagePath;
        // Ensure state reflects failure if needed
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
//    m_runButton->setFixedWidth(250);
    connect(m_runButton, &QPushButton::clicked, this, &GameMenu::onRunClicked);

    // FIRST MENU BUTTONS

    m_newButton = new QPushButton("Create a Character");
    gridLayout->addWidget(m_newButton, 1, 1, 1, 2, Qt::AlignBottom | Qt::AlignCenter);
//    m_newButton->setFixedWidth(250);
    connect(m_newButton, &QPushButton::clicked, this, &GameMenu::startNewGame);

    m_loadButton = new QPushButton("Load Character");
    gridLayout->addWidget(m_loadButton, 2, 1, 1, 2, Qt::AlignTop | Qt::AlignCenter);
//    m_loadButton->setFixedWidth(250);
    connect(m_loadButton, &QPushButton::clicked, this, &GameMenu::loadGame);
    
    m_helpButton = new QPushButton("Help/Lesson");
    gridLayout->addWidget(m_helpButton, 3, 1);
//    m_helpButton->setFixedWidth(250);
    connect(m_helpButton, &QPushButton::clicked, this, &GameMenu::onHelpClicked);

    m_recordsButton = new QPushButton("Hall of Records");
    gridLayout->addWidget(m_recordsButton, 2, 3);
    connect(m_recordsButton, &QPushButton::clicked, this, &GameMenu::showRecords);

    m_characterListButton = new QPushButton("Character List");
//    m_characterListButton->setFixedWidth(250);
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

    // Set initial menu state
    // Use the new centralized function instead of global MenuSwitch
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
 
    if (event.effect == "SHOW_MESSAGE")
    {
        QMessageBox::information(this, tr("Game Message"), event.description);
    }
}

// Override for proper background scaling on window resize
void GameMenu::resizeEvent(QResizeEvent *event) {
    if (!m_backgroundPixmap.isNull()) {
        QPixmap scaledPixmap = m_backgroundPixmap.scaled(size(), 
                                                         Qt::IgnoreAspectRatio, 
                                                         Qt::SmoothTransformation);
        QPalette palette;
        palette.setBrush(QPalette::Window, scaledPixmap);
        setPalette(palette);
    }
    // Call the base class implementation
    QWidget::resizeEvent(event);
}

// Centralized function to control button visibility
void GameMenu::toggleMenuState(bool characterIsLoaded) {
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
 * Switches the main menu buttons to show the '

 Run Character' option.
 * @param characterName The name of the newly created character.
 */
void GameMenu::onCharacterCreated(const QString &characterName) {
    emit logMessageTriggered(QString("New character **%1** created successfully!").arg(characterName));
    QMessageBox::information(this, tr("Creation Successful"), 
                             tr("Character **%1** created successfully. Ready to run.").arg(characterName));
    toggleMenuState(true);
}
// -------------------------------

// Function definitions
void GameMenu::onRunClicked() {
    TheCity *cityDialog = new TheCity(this);  
    cityDialog->setAttribute(Qt::WA_DeleteOnClose);
    cityDialog->show();
    //CharacterDialog *charDialog = new CharacterDialog(this);
    //charDialog->setAttribute(Qt::WA_DeleteOnClose);
    //charDialog->show();
    qDebug() << "Run Character clicked - launching TheCity";
}

void GameMenu::onCharacterListClicked() {
    CharacterListDialog *dialog = new CharacterListDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    qDebug() << "Character list clicked";
}

void GameMenu::startNewGame() {
    
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

// ----------------------------------------------------------------------
// MODIFIED: Load Game Function with File Parsing and Debug Logging
// ----------------------------------------------------------------------

void GameMenu::loadGame() {
    // 1. Setup file path and selection dialog 
    QString basePath = QCoreApplication::applicationDirPath();
    QString fullPath = QDir::cleanPath(basePath + QDir::separator() + m_subfolderName);
    QDir subfolderDir(fullPath);

    qDebug() << "Full Subfolder Path:" << subfolderDir.absolutePath();

    // Optional: Ensure the directory exists
    if (!subfolderDir.exists()) {
        if (subfolderDir.mkpath(".")) { 
            qDebug() << "Subfolder created successfully!";
        } else {
            qDebug() << "Failed to create subfolder!";
            emit logMessageTriggered("Error: Failed to ensure character folder exists.");
            return;
        }
    }
    QDir currentDir(subfolderDir.absolutePath());
    // Filter for .txt files
    QStringList nameFilters;
    nameFilters << "*.txt";
    
    // Get all files matching the filter
    QFileInfoList fileList = currentDir.entryInfoList(nameFilters, QDir::Files);
    
    QStringList characterFiles;
    for (const QFileInfo &fileInfo : fileList) {
        characterFiles << fileInfo.fileName();
    }

    if (characterFiles.isEmpty()) {
        emit logMessageTriggered("No character (.txt) files found to load.");
        QMessageBox::warning(this, tr("No Characters Found"), 
                             tr("No character files (*.txt) were found in the current directory."));
        return;
    }
    
    // --- Create Temporary Selection Dialog (omitted for brevity) ---
    QDialog selectionDialog(this);
    selectionDialog.setWindowTitle("Load Character");
    
    QVBoxLayout *layout = new QVBoxLayout(&selectionDialog);
    
    QLabel *promptLabel = new QLabel("Select a character file to load:");
    layout->addWidget(promptLabel);
    
    QComboBox *charComboBox = new QComboBox();
    charComboBox->addItems(characterFiles);
    layout->addWidget(charComboBox);
    
    QPushButton *loadButton_Dialog = new QPushButton("Load Character");
    loadButton_Dialog->setObjectName("loadButton_Dialog"); // Give it a unique name for connection
    layout->addWidget(loadButton_Dialog);
    
    QPushButton *cancelButton_Dialog = new QPushButton("Cancel");
    layout->addWidget(cancelButton_Dialog);

    // Connect dialog buttons
    QObject::connect(loadButton_Dialog, &QPushButton::clicked, &selectionDialog, &QDialog::accept);
    QObject::connect(cancelButton_Dialog, &QPushButton::clicked, &selectionDialog, &QDialog::reject);

    // Execute the dialog modally
    int result = selectionDialog.exec();
    
    if (result == QDialog::Accepted) {
        // User clicked "Load Character"
        QString selectedFileName = charComboBox->currentText();
        QString filePath = currentDir.absoluteFilePath(selectedFileName);

        emit logMessageTriggered(QString("Attempting to load character: %1").arg(selectedFileName));
        qDebug() << "Attempting to load character from:" << filePath;
        
        // -----------------------------------------------------------
        // --- REAL FILE LOADING AND PARSING LOGIC ---
        // -----------------------------------------------------------
        GameStateManager *gsm = GameStateManager::instance();
        QFile file(filePath);
        bool loadSuccess = false;

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            
            // Map the field names from the file to their corresponding GameState keys
            QMap<QString, QString> keyMap;
            
            // Map File Keys to GameState Keys (String and Integer values)
            keyMap["Name"] = "CurrentCharacterName";
            keyMap["Race"] = "CurrentCharacterRace";
            keyMap["Sex"] = "CurrentCharacterSex";
            keyMap["Alignment"] = "CurrentCharacterAlignment";
            keyMap["Guild"] = "CurrentCharacterGuild"; 

            keyMap["Charisma"] = "CurrentCharacterCharisma";
            keyMap["Constitution"] = "CurrentCharacterConstitution";
            keyMap["Dexterity"] = "CurrentCharacterDexterity";
            keyMap["Intelligence"] = "CurrentCharacterIntelligence";
            keyMap["Strength"] = "CurrentCharacterStrength";
            keyMap["Wisdom"] = "CurrentCharacterWisdom";

            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty() || line.startsWith("[") || line.startsWith("---") || line.contains("CHARACTER_FILE_VERSION")) {
                    continue; // Skip comments, sections, and version info
                }

                // Check for "Key: Value" format
                if (line.contains(':')) {
                    QStringList parts = line.split(':', Qt::SkipEmptyParts);
                    if (parts.size() == 2) {
                        QString fileKey = parts[0].trimmed();
                        QString fileValue = parts[1].trimmed();

                        if (keyMap.contains(fileKey)) {
                            QString gsmKey = keyMap[fileKey];
                            QVariant gsmValue;
                            
                            // Determine value type based on the key
                            if (gsmKey.startsWith("CurrentCharacterCharisma") || gsmKey.startsWith("CurrentCharacterConstitution") || 
                                gsmKey.startsWith("CurrentCharacterDexterity") || gsmKey.startsWith("CurrentCharacterIntelligence") || 
                                gsmKey.startsWith("CurrentCharacterStrength") || gsmKey.startsWith("CurrentCharacterWisdom")) 
                            {
                                // Stats are integers
                                bool ok;
                                int statValue = fileValue.toInt(&ok);
                                if (ok) {
                                    gsmValue = statValue;
                                } else {
                                    // Log error for bad stat data
                                    qDebug() << "Warning: Could not convert stat value to int for key:" << fileKey;
                                    continue; 
                                }
                            } else {
                                // All other keys are strings
                                gsmValue = fileValue;
                            }
                            
                            // DEBUG: Log the parsed key/value before setting state
                            qDebug() << "LOAD PARSE: File Key/Value:" << fileKey << "=" << fileValue << "| Mapped to GSM Key:" << gsmKey << "Final Value:" << gsmValue << "(Type:" << gsmValue.typeName() << ")";
                            
                            gsm->setGameValue(gsmKey, gsmValue);
                            loadSuccess = true;
                        }
                    }
                }
            }
            file.close();
        } else {
            // File failed to open
            emit logMessageTriggered(QString("Error: Could not open file %1 for reading.").arg(selectedFileName));
            QMessageBox::critical(this, tr("Load Failed"), 
                                 tr("Could not open character file **%1**.").arg(selectedFileName));
            return; // Exit on failure
        }
        
        // Finalize state if data was successfully parsed
        if (loadSuccess) {
            // This is a common state value set after loading any character
            gsm->setGameValue("CurrentCharacterStatPointsLeft", 0);
            
            qDebug() << "Character data loaded. Dumping GameState:";
            gsm->printAllGameState(); 
            
            QMessageBox::information(this, tr("Load Successful"), 
                                     tr("Character **%1** loaded successfully.").arg(selectedFileName));
            
            // Switch Menu State
            toggleMenuState(true);
        } else {
            // If the file opened but no valid data was parsed
            emit logMessageTriggered(QString("Error: Character file %1 loaded but was empty or invalid.").arg(selectedFileName));
            QMessageBox::critical(this, tr("Load Failed"), 
                                 tr("Character file **%1** loaded but contained no recognizable data.").arg(selectedFileName));
        }

    } else {
        // User cancelled the dialog (rejected)
        qDebug() << "Character loading cancelled by user via dropdown dialog.";
        emit logMessageTriggered("Character selection cancelled.");
    }
}

/*
void GameMenu::loadGame() {
    // 1. Access the GameStateManager instance
    GameStateManager *gsm = GameStateManager::instance();
    
    // 2. Retrieve character/guild data already loaded from MDATA1
    const QList<QVariantMap>& characters = gsm->gameData();

    if (characters.isEmpty()) {
        emit logMessageTriggered("Error: MDATA1 character data is empty or not loaded.");
        QMessageBox::warning(this, tr("No Data"), tr("No game data found in MDATA1."));
        return;
    }

    // 3. Create a selection dialog for the user
    QDialog selectionDialog(this);
    selectionDialog.setWindowTitle("Load Character from MDATA1");
    
    QVBoxLayout *layout = new QVBoxLayout(&selectionDialog);
    layout->addWidget(new QLabel("Select an entry to load:"));
    
    QComboBox *charComboBox = new QComboBox();
    
    // 4. Populate the dropdown with names from the MDATA1 memory storage
    for (const QVariantMap& entry : characters) {
        // MDATA1.js uses "Name" for guild/character entries based on your JSON parser
        QString name = entry.value("Name").toString();
        charComboBox->addItem(name.isEmpty() ? "Unknown Entry" : name);
    }
    layout->addWidget(charComboBox);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *loadBtn = new QPushButton("Load");
    QPushButton *cancelBtn = new QPushButton("Cancel");
    btnLayout->addWidget(loadBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(loadBtn, &QPushButton::clicked, &selectionDialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &selectionDialog, &QDialog::reject);

    // 5. Process the selection and update the Game State
    if (selectionDialog.exec() == QDialog::Accepted) {
        int index = charComboBox->currentIndex();
        QVariantMap selectedData = characters.at(index);

        // Map the selected data to current character values
        gsm->setGameValue("CurrentCharacterName", selectedData.value("Name"));
        gsm->setGameValue("CurrentCharacterGuild", selectedData.value("Name"));
        
        // Finalize state to enable 'Run Character'
        gsm->setGameValue("CurrentCharacterStatPointsLeft", 0);
        toggleMenuState(true);
        
        emit logMessageTriggered(QString("Loaded MDATA1 entry: %1").arg(charComboBox->currentText()));
        QMessageBox::information(this, tr("Success"), tr("Character **%1** loaded.").arg(charComboBox->currentText()));
    }
}
*/
// ----------------------------------------------------------------------

void GameMenu::showRecords() {
    emit logMessageTriggered("User entered hall of records");
    HallOfRecordsDialog *recordsDialog = new HallOfRecordsDialog(this);
    recordsDialog->setAttribute(Qt::WA_DeleteOnClose);
    recordsDialog->show();
    qDebug() << "User entered hall of records";
}
void GameMenu::quitGame() {
    QApplication::quit();
    qDebug() << "Exit button clicked";
}
void GameMenu::showCredits() {
    qDebug() << "ShowCredits (placeholder) clicked";
}
void GameMenu::onInventoryClicked() {
    InventoryDialog *inventoryDialog = new InventoryDialog(this);
    inventoryDialog->setAttribute(Qt::WA_DeleteOnClose);
    inventoryDialog->show();
    qDebug() << "Inventory button clicked";
}
void GameMenu::onMarlithClicked() {
    qDebug() << "Marlith (placeholder) clicked";
}
void GameMenu::onOptionsClicked() {
    OptionsDialog *optionsDialog = new OptionsDialog(this);
    optionsDialog->setAttribute(Qt::WA_DeleteOnClose);
    optionsDialog->show();
    qDebug() << "Options button clicked";
}
void GameMenu::onAboutClicked() {
    AboutDialog *aboutDialog = new AboutDialog(this);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
    aboutDialog->show();
    qDebug() << "About button clicked";
}
void GameMenu::onHelpClicked() {
    HelpLessonDialog *helpDialog = new HelpLessonDialog(this);
    helpDialog->setAttribute(Qt::WA_DeleteOnClose);
    helpDialog->show(); 
    qDebug() << "Help/Lesson dialog closed.";
    emit logMessageTriggered("User viewed the Help/Lesson dialog.");
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
void GameMenu::onShowStatisticsClicked() {
    emit logMessageTriggered("User opened Statistics dialog.");
    MordorStatistics *statsDialog = new MordorStatistics(this);
    statsDialog->setAttribute(Qt::WA_DeleteOnClose);
    statsDialog->show();
    qDebug() << "Statistics dialog opened.";
}

GameMenu::~GameMenu() {
    // m_player and m_audioOutput have 'this' as parent, so they are cleaned up by Qt.
    // Explicit deletion is not strictly needed but can be used if they were not parented.
    // Removing explicit delete calls to rely on Qt object tree cleanup, as they are parented.
    qDebug() << "GameMenu Destructor.";
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    qDebug() << "Launching LoadingScreen dialog...";
    LoadingScreen loadingScreen; 
    loadingScreen.exec(); 
    qDebug() << "LoadingScreen dialog closed. Starting main application...";
    // --- NEW STARTING STORY ---
    qDebug() << "Launching Starting Story...";
    StoryDialog story;
    story.exec(); // This blocks until the three parts are finished
    // --------------------------
    // ------------------------------------------
    GameMenu w;
    // Audio is initialized in GameMenu's constructor now.
    
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

