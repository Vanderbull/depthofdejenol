#include "game_menu.h"
#include "src/characterlist_dialog/characterlistdialog.h"
#include "hallofrecordsdialog.h"
#include "createcharacterdialog.h"
//#include "dungeondialog.h"
//#include "DungeonDialog.h"
#include "inventorydialog.h"
//#include "marlith_dialog.h"
#include "optionsdialog.h"
#include "src/about_dialog/AboutDialog.h"
#include "MonsterEditorDialog.h"
#include "SpellbookEditorDialog.h"
#include "src/character_dialog/CharacterDialog.h"
#include "MessageWindow.h"
#include "SenderWindow.h"
#include "library_dialog.h"
#include "src/automap/automap_dialog.h"
#include "game_controller.h"
#include "helplesson.h"
#include "mordorstatistics.h"
#include "LoadingScreen.h"
//#include "GuildsDialog.h"
#include "TheCity.h"

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
#include <QFileDialog>
#include <QDir>
#include <QSettings>

bool MenuSwitch = false;
QSettings settings("MyCompany", "MyApp");
QString subfolderName = settings.value("Paths/SubfolderName", "data").toString();


void launchAutomapDialog() {
    // Using nullptr as the parent widget, as 'this' is not available here.
    // If you are inside a class like MainGameWindow, use AutomapDialog mapDialog(this);
    AutomapDialog mapDialog(nullptr);
    // Execute the dialog modally
    //mapDialog.exec(); 
    mapDialog.show(); 
    qDebug() << "Automap dialog closed.";
}

GameMenu::GameMenu(QWidget *parent) : QWidget(parent) {

    qDebug() << "Configured Subfolder Name:" << subfolderName;


    setWindowTitle("Mordor: The Depths of Dejenol v1.0");
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
    //Second menu  

    runButton = new QPushButton("Run Character");
    gridLayout->addWidget(runButton, 1, 1, 1, 2, Qt::AlignBottom | Qt::AlignCenter);
    runButton->setFixedWidth(250);
    connect(runButton, &QPushButton::clicked, this, &GameMenu::onRunClicked);
    // First menu
    helpButton = new QPushButton("Help/Lesson");
    gridLayout->addWidget(helpButton, 3, 1);
    helpButton->setFixedWidth(250);
    connect(helpButton, &QPushButton::clicked, this, &GameMenu::onHelpClicked);

    newButton = new QPushButton("Create a Character");
    gridLayout->addWidget(newButton, 1, 1, 1, 2, Qt::AlignBottom | Qt::AlignCenter);
    newButton->setFixedWidth(250);
    connect(newButton, &QPushButton::clicked, this, &GameMenu::startNewGame);

    loadButton = new QPushButton("Load Character");
    gridLayout->addWidget(loadButton, 2, 1, 1, 2, Qt::AlignTop | Qt::AlignCenter);
    loadButton->setFixedWidth(250);
    connect(loadButton, &QPushButton::clicked, this, &GameMenu::loadGame);

    recordsButton = new QPushButton("Hall of Records");
    gridLayout->addWidget(recordsButton, 2, 3);
    connect(recordsButton, &QPushButton::clicked, this, &GameMenu::showRecords);

    characterListButton = new QPushButton("Character List");
    newButton->setFixedWidth(250);
    gridLayout->addWidget(characterListButton, 2, 0);
    connect(characterListButton, &QPushButton::clicked, this, &GameMenu::onCharacterListClicked);

//    QPushButton *helpButton = new QPushButton("Help/Lesson");
//    gridLayout->addWidget(helpButton, 3, 1);

    optionsButton = new QPushButton("Options...");
    gridLayout->addWidget(optionsButton, 3, 2);
    connect(optionsButton, &QPushButton::clicked, this, &GameMenu::onOptionsClicked);

    exitButton = new QPushButton("Exit");
    gridLayout->addWidget(exitButton, 4, 1);
    connect(exitButton, &QPushButton::clicked, this, &GameMenu::quitGame);

    aboutButton = new QPushButton("About");
    gridLayout->addWidget(aboutButton, 4, 2);
    connect(aboutButton, &QPushButton::clicked, this, &GameMenu::onAboutClicked);

//    creditsButton = new QPushButton("Charactersheet");
//    gridLayout->addWidget(creditsButton, 6, 0);
//    connect(creditsButton, &QPushButton::clicked, this, &GameMenu::showCredits);
    // Placeholder for bottom-left image
    QLabel *bottomLeftImage = new QLabel();
    gridLayout->addWidget(bottomLeftImage, 3, 0, 2, 1);
    // Placeholder for bottom-right image
    QLabel *bottomRightImage = new QLabel();
    gridLayout->addWidget(bottomRightImage, 3, 3, 2, 1);
    // Create it on the heap. We'll make it a top-level window (no parent).
    MessagesWindow *loggerWindow = new MessagesWindow();
    // If you made it a private member (m_loggerWindow) you would use:
    // m_loggerWindow = new MessagesWindow();
    // 2. ESTABLISH THE CONNECTION
    // Connect THIS object's signal (GameMenu::logMessageTriggered)
    // to the loggerWindow's slot (MessagesWindow::logMessage).
    QObject::connect(this, &GameMenu::logMessageTriggered,
                     loggerWindow, &MessagesWindow::logMessage);
    loggerWindow->show();
    // Use the signal you just connected to immediately log a message.
    emit logMessageTriggered("GameMenu has successfully initialized the Messages Log.");
//    connect(inventoryButton, &QPushButton::clicked, this, &GameMenu::onInventoryClicked);
runButton->setVisible(false);
}
// Function definitions
void GameMenu::onRunClicked() {
 MenuSwitch = false;
runButton->setVisible(false);
loadButton->setVisible(true);
newButton->setVisible(true);
//TheCity cityDialog;
//cityDialog.setAttribute(Qt::WA_DeleteOnClose);
//cityDialog.show();
//int result = cityDialog.exec();
// 1. Create the dialog on the HEAP using 'new'.
    // Use 'this' as the parent so the dialog is automatically
    // cleaned up if the GameMenu is ever destroyed.
    TheCity *cityDialog = new TheCity(this); 

    // 2. Set the Qt::WA_DeleteOnClose attribute. 
    // This tells Qt to automatically delete the object when the user closes it, 
    // preventing a memory leak since it's on the heap.
    cityDialog->setAttribute(Qt::WA_DeleteOnClose);

    // 3. Use show() to display the dialog non-modally (non-blocking).
    cityDialog->show();

    // The function now returns, but the cityDialog object remains alive
    // because it was created on the heap.


    // 1. Create the dialog on the heap.
    CharacterDialog *charDialog = new CharacterDialog("Goodie Gil'thrialle");
    // 2. IMPORTANT: Tell Qt to delete the object when the user closes the window.
    charDialog->setAttribute(Qt::WA_DeleteOnClose);
    // 3. Call show() to display the dialog non-modally.
    // Execution continues immediately, and the main Qt event loop handles the dialog.
    charDialog->show();
    qDebug() << "ShowCredits button clicked";

}
void GameMenu::onCharacterListClicked() {
    CharacterListDialog *dialog = new CharacterListDialog(this);
    dialog->show();
    qDebug() << "Character list clicked";
}
void GameMenu::startNewGame() {
    CreateCharacterDialog *dialog = new CreateCharacterDialog(this);
    dialog->show();
    qDebug() << "Start New Game button clicked";
}
void GameMenu::loadGame() {
// 3. Get the absolute path of the application's executable directory
QString basePath = QCoreApplication::applicationDirPath();

// 4. Combine the base path and the subfolder name
// QDir::cleanPath handles directory separators ('/' vs '\') automatically.
QString fullPath = QDir::cleanPath(basePath + QDir::separator() + subfolderName);

// 5. Create the QDir object for the calculated subfolder path
QDir subfolderDir(fullPath);

qDebug() << "Full Subfolder Path:" << subfolderDir.absolutePath();

// Optional: Ensure the directory exists
if (!subfolderDir.exists()) {
    if (subfolderDir.mkpath(".")) { // Use mkpath(".") to create the directory represented by subfolderDir
        qDebug() << "Subfolder created successfully!";
    } else {
        qDebug() << "Failed to create subfolder!";
    }
}
    //QDir currentDir(QDir::currentPath());
    QDir currentDir(subfolderDir.absolutePath());
    // Filter for .txt files
    QStringList nameFilters;
    nameFilters << "*.txt";
    
    // Get all files matching the filter
    QFileInfoList fileList = currentDir.entryInfoList(nameFilters, QDir::Files);
    
    QStringList characterFiles;
    for (const QFileInfo &fileInfo : fileList) {
        // Exclude files that might be game configuration files and not characters
        // For this example, we add all .txt files found.
        characterFiles << fileInfo.fileName();
    }

    if (characterFiles.isEmpty()) {
        emit logMessageTriggered("No character (.txt) files found to load.");
        QMessageBox::warning(this, tr("No Characters Found"), 
                             tr("No character files (*.txt) were found in the current directory."));
        return;
    }
    
    // --- Create Temporary Selection Dialog ---
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
    //selectionDialog.show();
    int result = selectionDialog.exec();
    
    if (result == QDialog::Accepted) {
        // User clicked "Load Character"
        QString selectedFileName = charComboBox->currentText();
        QString filePath = currentDir.absoluteFilePath(selectedFileName);

        // **LOAD DATA LOGIC:**
        // Simulate loading the character data based on the filePath here.
        // e.g., CharacterData loadedChar = loadCharacterFromFile(filePath);
        
        emit logMessageTriggered(QString("Loading character: %1").arg(selectedFileName));
        qDebug() << "Character selected from dropdown and attempting to load:" << filePath;
        
        // Simulate success message before menu switch
        QMessageBox::information(this, tr("Load Successful"), 
                                 tr("Character **%1** loaded successfully (Data load simulated).").arg(selectedFileName));
        
        // Exit back to the main menu / Switch Menu State
        MenuSwitch = true;
        runButton->setVisible(true);
        loadButton->setVisible(false);
        newButton->setVisible(false);

    } else {
        // User cancelled the dialog (rejected)
        qDebug() << "Character loading cancelled by user via dropdown dialog.";
        emit logMessageTriggered("Character selection cancelled.");
    }
}
void GameMenu::showRecords() {
    emit logMessageTriggered("User entered hall of records");
    HallOfRecordsDialog *recordsDialog = new HallOfRecordsDialog(this);
    recordsDialog->show();
    qDebug() << "User entered hall of records";
}
void GameMenu::quitGame() {
    QApplication::quit();
    qDebug() << "Exit button clicked";

}
void GameMenu::showCredits() {
    // 1. Create the dialog on the heap.
//    CharacterDialog *charDialog = new CharacterDialog("Goodie Gil'thrialle");
    // 2. IMPORTANT: Tell Qt to delete the object when the user closes the window.
//    charDialog->setAttribute(Qt::WA_DeleteOnClose);
    // 3. Call show() to display the dialog non-modally.
    // Execution continues immediately, and the main Qt event loop handles the dialog.
//    charDialog->show();
//    qDebug() << "ShowCredits button clicked";
}
void GameMenu::onInventoryClicked() {
    InventoryDialog *inventoryDialog = new InventoryDialog(this);
    inventoryDialog->show();
//    inventoryDialog->exec();
    qDebug() << "Inventory button clicked";
}
void GameMenu::onMarlithClicked() {
    //MarlithDialog *dialog = new MarlithDialog(this);
    //dialog->show();
    //qDebug() << "Marlith button clicked";
}
void GameMenu::onOptionsClicked() {
    OptionsDialog *optionsDialog = new OptionsDialog(this);
    optionsDialog->show();
//    optionsDialog->exec();
    qDebug() << "Options button clicked";
}
void GameMenu::onAboutClicked() {
    AboutDialog *aboutDialog = new AboutDialog(this);
    aboutDialog->show();
//    aboutDialog->exec();
    qDebug() << "About button clicked";
}
void GameMenu::onHelpClicked() {
    // 1. Instansiera dialogen. Använd 'this' som parent.
    HelpLessonDialog *helpDialog = new HelpLessonDialog(this);
// 1. Tell Qt to automatically delete this heap object when the user closes the window.
    helpDialog->setAttribute(Qt::WA_DeleteOnClose);
    // 2. Använd exec() för att köra dialogen modalt (blockerar tills den stängs)
    helpDialog->show(); 
//    helpDialog->exec(); 
    
    // 3. (Valfritt) Logga stängningen
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
void GameMenu::onShowStatisticsClicked() { // <--- NEW SLOT IMPLEMENTATION
    emit logMessageTriggered("User opened Mordor Statistics dialog.");
    MordorStatistics *statsDialog = new MordorStatistics(this);
    statsDialog->show();
//    statsDialog->exec();
    qDebug() << "Mordor Statistics dialog closed.";
    delete statsDialog;
}

// Conceptual private helper function GameMenu::toggleMenuState()
void GameMenu::toggleMenuState() {
    // List of buttons that belong to the MenuSwitch == false state (First Menu)
//    QList<QPushButton*> firstMenuButtons = {m_helpButton, m_newButton, m_loadButton, m_recordsButton, m_characterListButton, m_optionsButton, m_exitButton, m_aboutButton, m_creditsButton};
    
    // Switch between the two menus
//    if (MenuSwitch) {
        // MenuSwitch is true: Show 'Run Character', hide First Menu
//        m_runButton->setVisible(true);
//        for (QPushButton* btn : firstMenuButtons) {
//            btn->setVisible(false);
//        }
//    } else {
        // MenuSwitch is false: Hide 'Run Character', show First Menu
//        m_runButton->setVisible(false);
//        for (QPushButton* btn : firstMenuButtons) {
//            btn->setVisible(true);
//        }
//    }
//}
runButton->setVisible(true);
}

GameMenu::~GameMenu() {
    qDebug() << "Destructor.";
}
int main(int argc, char *argv[]) {
//    Q_INIT_RESOURCE(resources);
    QApplication a(argc, argv);

    qDebug() << "Launching LoadingScreen dialog...";
    // Create the dialog on the stack
    LoadingScreen loadingScreen; 
    // Show the dialog modally. Execution will block here until the user closes it.
    loadingScreen.exec(); 
    qDebug() << "LoadingScreen dialog closed. Starting main application...";
    // ------------------------------------------
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
    audioOutput->setVolume(0); // Set volume (0.0 to 100.0)
    player->play();
    qDebug() << "Playing audio...";
//    LibraryDialog library;
//    library.exec();
//    launchAutomapDialog();
// --- NEW CODE ---
//    MordorStatistics statsDialog; // Create the dialog on the stack
//    statsDialog.exec(); // Launch the dialog modally
//    qDebug() << "Mordor Statistics dialog launched and closed from main.";
    // --- END NEW CODE ---
    // 2. INSTANTIATE THE GAME CONTROLLER
    // Use the main window (w) as its parent, although it's not strictly necessary.
    // We create it here to manage its lifetime alongside the application.
    GameController *controller = new GameController(&w);
    // 3. INSTALL THE GAME CONTROLLER AS AN EVENT FILTER
    // This is the key step. It tells Qt that *before* the key event is delivered 
    // to the main window (w) or any of its children, the controller gets a chance
    // to inspect and handle it.
    w.installEventFilter(controller);
    qDebug() << "GameController installed as event filter on GameMenu.";
//    GuildsDialog g;
//    g.show();

//    GeneralStore store;
//    store.show();

// 2. Create an instance of your custom dialog, TheCity
//    TheCity cityDialog;

    // 3. Display the dialog and start the application's event loop
    // exec() is used to show a modal dialog
//    int result = cityDialog.exec();

// --- ADDED RESOURCE CLEANUP ---
//    Q_CLEANUP_RESOURCE(resources);
    // ----------------------------
    return a.exec();
}
