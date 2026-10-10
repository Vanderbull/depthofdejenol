#include "blacklands.h"
#include "gameStateManager.h"
#include "audioManager.h"
#include "theCity.h"
#include "storyDialog.h"

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
#include "src/core/DungeonLevelState.h"
#include "test/selftest.h"
#include "src/dungeon_dialog/DungeonDialog.h"
#include <QPushButton>

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
#include <algorithm>
#include <QPainter>

GameMenu::GameMenu(QWidget *parent)
    : QWidget(parent)
    , m_settings("MyCompany", "MyApp")
    , m_subfolderName(m_settings.value("Paths/SubfolderName", "data/characters").toString()) 
{
    // 1. Core Engine Initialization
    gameStateManager::instance()->loadFontSprite("resources/images/font_spritesheet_transparent.png");
    gameStateManager::instance()->incrementPartyAge(1);
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
        gameStateManager::instance()->setGameValue("ResourcesLoaded", true); 
    } else {
        qDebug() << "FATAL: Could not load background image from:" << imagePath;
    }
}

void GameMenu::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPainter painter(this);
    // Draw branding or version text using custom sprite font
    gameStateManager::instance()->drawCustomText(&painter, "ABCDEFGH", QPoint(0, 0));
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
    audioManager::instance()->stopAllAudio();
    theCity *cityDialog = new theCity(this);  
    cityDialog->setAttribute(Qt::WA_DeleteOnClose);    
    hide(); 
    
    // Handle return from theCity
    connect(cityDialog, &QDialog::finished, this, [this](int){
        bool isLoaded = gameStateManager::instance()->getGameValue("ResourcesLoaded").toBool();
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

    if (gameStateManager::instance()->loadCharacterFromFile(fileName)) {
        if (gameStateManager::instance()->hasLivingCharacters()) {
            toggleMenuState(true); 
            emit logMessageTriggered("Character loaded successfully.");
        } else {
            QMessageBox::warning(this, tr("Load Error"), tr("Character is deceased or invalid."));
        }
    }
}

void GameMenu::onCharacterCreated(const QString &characterName) {
    if (gameStateManager::instance()->saveCharacterToFile(0)) {
        emit logMessageTriggered(QString("Character %1 saved.").arg(characterName));
        toggleMenuState(true);
        gameStateManager::instance()->startAutosave(10000); // 10s intervals
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
    audioManager::instance()->stopAllAudio();

    // Clear any leftover dungeon / placed-item state from a previous game so
    // a fresh start never inherits stale floors or items.
    DungeonLevelRegistry::instance().clear();

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

    // Headless verification suite (`make check`). Runs without any GUI so it
    // can gate every change; see test/selftest.cpp.
    if (a.arguments().contains("--selftest")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
        return runSelfTest();
    }

    // Layout probe: build the real DungeonDialog through the normal xcb path,
    // show it, and dump the on-screen geometry of every action button. Used to
    // verify the button grid against what the user actually sees.
    if (a.arguments().contains("--probe-dungeon")) {
        LoadingScreen loadingScreen;
        loadingScreen.exec();
        DungeonDialog dlg;
        dlg.show();
        // Probe at several sizes: the default resize, the declared minimum
        // (900x640), and the size the user's screen actually gives it.
        QList<QSize> sizes = {QSize(1280, 800), QSize(900, 640)};
        for (const QSize& sz : sizes) {
            dlg.resize(sz);
            for (int i = 0; i < 40; ++i) QApplication::processEvents();
            qInfo() << "=== DUNGEON PROBE" << sz << "===";
            qInfo() << "actual dialog size:" << dlg.size();
            QWidget* sidebar = dlg.findChild<QWidget*>("dungeonSidebar");
            if (!sidebar) { qInfo() << "no sidebar!"; continue; }
            qInfo() << "sidebar:" << sidebar->geometry();
            QList<QWidget*> kids = sidebar->findChildren<QWidget*>(
                QString(), Qt::FindDirectChildrenOnly);
            QList<QPair<QString,QRect>> rects;
            for (QWidget* w : kids) {
                if (!w->isVisible() || w->geometry().isEmpty()) continue;
                const QRect r = w->geometry();
                QString label = w->objectName().isEmpty()
                    ? QString(w->metaObject()->className()) : w->objectName();
                if (auto* gb = qobject_cast<QGroupBox*>(w)) label += "(" + gb->title() + ")";
                if (auto* btn = qobject_cast<QPushButton*>(w)) label += "(" + btn->text() + ")";
                if (auto* lbl = qobject_cast<QLabel*>(w)) label += "(" + lbl->text().left(20) + ")";
                rects.append({label, r});
                qInfo().noquote() << QString("  %1  geo=%2,%3 %4x%5  bottom=%6")
                    .arg(label, -40)
                    .arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height())
                    .arg(r.y() + r.height());
            }
            int overlaps = 0;
            for (int i = 0; i < rects.size(); ++i)
                for (int j = i + 1; j < rects.size(); ++j)
                    if (rects[i].second.intersects(rects[j].second)) {
                        ++overlaps;
                        qInfo().noquote() << QString("  OVERLAP: %1 <-> %2")
                            .arg(rects[i].first, rects[j].first);
                    }
            qInfo() << "sidebar direct-child overlaps:" << overlaps;
            int bottom = 0;
            for (auto& p : rects) bottom = qMax(bottom, p.second.bottom());
            qInfo() << "sidebar height:" << sidebar->height()
                    << "content bottom:" << bottom
                    << (bottom > sidebar->height() ? "  <<< CONTENT OVERFLOWS" : "");
            qInfo() << "=== END PROBE" << sz << "===";
        }
        dlg.resize(1280, 800);
        for (int i = 0; i < 40; ++i) QApplication::processEvents();
        dlg.grab().save("/tmp/dungeon_probe.png");
        qInfo() << "saved grab to /tmp/dungeon_probe.png";
        return 0;
    }

    // Initial sequence
    LoadingScreen loadingScreen; 
    loadingScreen.exec(); 

    storyDialog story;
    story.exec();

    GameMenu w;
    w.show();
    return a.exec();
}
