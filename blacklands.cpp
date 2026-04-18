#include "blacklands.h"
#include "GameStateManager.h"
#include "AudioManager.h"
#include "TheCity.h"
#include "StoryDialog.h"

// Dialog and UI Includes
#include "src/characterlist_dialog/characterlistdialog.h"
#include "src/hall_of_records/hallofrecordsdialog.h"
#include "src/create_character/createcharacterdialog.h"
#include "src/options_dialog/optionsdialog.h"
#include "src/about_dialog/AboutDialog.h"
#include "src/message_window/MessageWindow.h"
#include "src/helplesson/helplesson.h"
#include "src/loadingscreen/LoadingScreen.h"
#include "src/race_data/RaceData.h"

// Qt Includes
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDir>
#include <QPainter>

GameMenu::GameMenu(QWidget *parent)
    : QWidget(parent)
    , m_settings("MyCompany", "MyApp")
    , m_subfolderName(m_settings.value("Paths/SubfolderName", "data/characters").toString()) 
{
    // 1. Core Engine Initialization
    GameStateManager::instance()->loadFontSprite("resources/images/font_spritesheet_transparent.png");
    GameStateManager::instance()->incrementPartyAge(1);
    EventManager::instance()->loadEvents("./data/events-json");
    
    // 2. Window Styling and Palette
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
    setAutoFillBackground(true);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | 
                   Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    setWindowTitle("The Depths of Dejenol: Black Lands");

    // 3. Modular Setup
    setupUI();
    setupConnections();
    loadStyleSheet();
    loadBackgroundImage();
    
    // 4. Initial State
    toggleMenuState(false); 
    EventManager::instance()->update("GAME_START");
    emit logMessageTriggered("GameMenu has successfully initialized.");
}

void GameMenu::setupUI() {
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(50, 50, 50, 50);
    gridLayout->setSpacing(20);

    // Initialize Buttons
    m_runButton = new QPushButton("Run Character");
    m_newButton = new QPushButton("Create a Character");
    m_loadButton = new QPushButton("Load Character");
    m_helpButton = new QPushButton("Help/Lesson");
    m_recordsButton = new QPushButton("Hall of Records");
    m_characterListButton = new QPushButton("Character List");
    m_optionsButton = new QPushButton("Options...");
    m_exitButton = new QPushButton("Exit");
    m_aboutButton = new QPushButton("About");

    // Grid Layout Mapping
    gridLayout->addWidget(m_runButton, 1, 1, 1, 2, Qt::AlignBottom | Qt::AlignCenter);
    gridLayout->addWidget(m_newButton, 1, 1, 1, 2, Qt::AlignBottom | Qt::AlignCenter);
    gridLayout->addWidget(m_loadButton, 2, 1, 1, 2, Qt::AlignTop | Qt::AlignCenter);
    gridLayout->addWidget(m_characterListButton, 2, 0);
    gridLayout->addWidget(m_recordsButton, 2, 3);
    gridLayout->addWidget(m_helpButton, 3, 1);
    gridLayout->addWidget(m_optionsButton, 3, 2);
    gridLayout->addWidget(m_aboutButton, 4, 2);
    gridLayout->addWidget(m_exitButton, 4, 1);

    // Screen-based Resizing
    setMinimumSize(800, 600);
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        resize(screen->geometry().width() * 0.7, screen->geometry().height() * 0.65);
    }
}

void GameMenu::setupConnections() {
    // Button Signals
    connect(m_runButton, &QPushButton::clicked, this, &GameMenu::onRunClicked);
    connect(m_newButton, &QPushButton::clicked, this, &GameMenu::startNewGame);
    connect(m_loadButton, &QPushButton::clicked, this, &GameMenu::loadGame);
    connect(m_helpButton, &QPushButton::clicked, this, &GameMenu::onHelpClicked);
    connect(m_recordsButton, &QPushButton::clicked, this, &GameMenu::showRecords);
    connect(m_characterListButton, &QPushButton::clicked, this, &GameMenu::onCharacterListClicked);
    connect(m_optionsButton, &QPushButton::clicked, this, &GameMenu::onOptionsClicked);
    connect(m_aboutButton, &QPushButton::clicked, this, &GameMenu::onAboutClicked);
    connect(m_exitButton, &QPushButton::clicked, this, &GameMenu::quitGame);

    // Engine Signals
    connect(EventManager::instance(), &EventManager::eventTriggered, this, &GameMenu::onEventTriggered);
    
    // Logging Window Setup
    MessagesWindow *loggerWindow = new MessagesWindow(); 
    connect(this, &GameMenu::logMessageTriggered, loggerWindow, &MessagesWindow::logMessage);
    loggerWindow->show();
}

void GameMenu::loadStyleSheet() {
    QString qssPath = QFileInfo(__FILE__).absolutePath() + "/MainMenu.qss";
    QFile styleFile(qssPath);
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(QTextStream(&styleFile).readAll());
    } else {
        qDebug() << "Style file not found at:" << qssPath;
    }
}

void GameMenu::loadBackgroundImage() {
    QString imagePath = QDir::cleanPath(qApp->applicationDirPath() + "/introtitle.png");
    if (m_backgroundPixmap.load(imagePath)) {
        resizeEvent(nullptr); // Force initial scale
        GameStateManager::instance()->setGameValue("ResourcesLoaded", true); 
    } else {
        qDebug() << "FATAL: Could not load background image from:" << imagePath;
    }
}

void GameMenu::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPainter painter(this);
    // Draw branding or version text using custom sprite font
    GameStateManager::instance()->drawCustomText(&painter, "ABCDEFGH", QPoint(0, 0));
}

void GameMenu::resizeEvent(QResizeEvent *event) {
    if (!m_backgroundPixmap.isNull()) {
        QPalette palette;
        palette.setBrush(QPalette::Window, m_backgroundPixmap.scaled(size(), 
                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        setPalette(palette);
    }
    QWidget::resizeEvent(event);
}

void GameMenu::toggleMenuState(bool characterIsLoaded) {
    // Toggle visibility based on whether a character is loaded
    m_runButton->setVisible(characterIsLoaded);
    m_loadButton->setVisible(!characterIsLoaded);
    m_newButton->setVisible(!characterIsLoaded);
}

void GameMenu::onRunClicked() {
    AudioManager::instance()->stopAllAudio();
    TheCity *cityDialog = new TheCity(this);  
    cityDialog->setAttribute(Qt::WA_DeleteOnClose);    
    hide(); 
    
    // Handle return from TheCity
    connect(cityDialog, &QDialog::finished, this, [this](int){
        bool isLoaded = GameStateManager::instance()->getGameValue("ResourcesLoaded").toBool();
        this->toggleMenuState(isLoaded); 
        this->show();
    });
    cityDialog->show();
}

void GameMenu::loadGame() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Character"), 
                                                    "data/characters", 
                                                    tr("Character Files (*.json *.lua)"));
    if (fileName.isEmpty()) return;

    if (GameStateManager::instance()->loadCharacterFromFile(fileName)) {
        if (GameStateManager::instance()->hasLivingCharacters()) {
            toggleMenuState(true); 
            emit logMessageTriggered("Character loaded successfully.");
        } else {
            QMessageBox::warning(this, tr("Load Error"), tr("Character is deceased or invalid."));
        }
    }
}

void GameMenu::onCharacterCreated(const QString &characterName) {
    if (GameStateManager::instance()->saveCharacterToFile(0)) {
        emit logMessageTriggered(QString("Character %1 saved.").arg(characterName));
        toggleMenuState(true);
        GameStateManager::instance()->startAutosave(10000); // 10s intervals
    }
}

void GameMenu::quitGame() { 
    qApp->quit();
}

// Dialog Launchers
void GameMenu::showRecords() { 
    (new HallOfRecordsDialog(this))->show();
}

void GameMenu::onCharacterListClicked() { 
    (new CharacterListDialog(this))->show();
}

void GameMenu::onOptionsClicked() { 
    (new OptionsDialog(this))->show();
}

void GameMenu::onAboutClicked() { 
    (new AboutDialog(this))->show();
}

void GameMenu::onHelpClicked() { 
    (new HelpLessonDialog(this))->show();
}

void GameMenu::startNewGame() {
    AudioManager::instance()->stopAllAudio();
    auto *dialog = new CreateCharacterDialog(loadRaceData(), loadGuildData(), this);
    connect(dialog, &CreateCharacterDialog::characterCreated, this, &GameMenu::onCharacterCreated);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void GameMenu::onEventTriggered(const GameEvent& event) {
    qDebug() << "Triggered event:" << event.id << event.description;
}

GameMenu::~GameMenu() { 
    qDebug() << "GameMenu Destructor.";
}

int main(int argc, char *argv[]) {
    // Force X11 for Wayland compatibility
    qputenv("QT_QPA_PLATFORM", "xcb");
    QApplication a(argc, argv);

    // Initial sequence
    LoadingScreen loadingScreen; 
    loadingScreen.exec(); 

    StoryDialog story;
    story.exec();

    GameMenu w;
    w.show();
    return a.exec();
}
