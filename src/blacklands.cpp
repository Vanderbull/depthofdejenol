#include "src/blacklands.h"
#include "src/core/gameStateManager.h"
#include "src/audio/audioManager.h"
#include "src/theCity.h"


// Dialog and UI Includes
#include "src/hall_of_records/hallofrecordsdialog.h"
#include "src/create_character/createcharacterdialog.h"
#include "src/options_dialog/optionsdialog.h"

#include "src/loadingscreen/LoadingScreen.h"
#include "src/load_party_dialog/LoadPartyDialog.h"
#include "src/assemble_party_dialog/AssemblePartyDialog.h"
#include "src/multiplayer_dialog/MultiplayerLobbyDialog.h"
#include <QInputDialog>
#include <QRegularExpression>
#include "src/race_data/RaceData.h"
#include "src/core/DungeonLevelState.h"
#include "src/core/GameSettings.h"
#include "test/selftest.h"
#include "src/dungeon_dialog/DungeonDialog.h"
#include <QPushButton>

// Qt Includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDateTime>
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
    refreshContinueState();
    EventManager::instance()->update("GAME_START");

}

void GameMenu::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(12);

    // --- Title Section ---
    m_titleLabel = new QLabel(tr("THE DEPTHS OF DEJENOL"), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);

    m_versionLabel = new QLabel(tr("BLACK LANDS — v5.0.0"), this);
    QFont versionFont = m_versionLabel->font();
    versionFont.setPointSize(12);
    m_versionLabel->setFont(versionFont);
    m_versionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_versionLabel);

    mainLayout->addSpacing(20);

    // --- Continue Section (primary, only when autosave exists) ---
    QHBoxLayout *continueLayout = new QHBoxLayout();
    continueLayout->addStretch();
    m_continueButton = new QPushButton(tr("CONTINUE"), this);
    m_continueButton->setObjectName("continueButton");
    m_continueButton->setMinimumHeight(50);
    QFont continueFont = m_continueButton->font();
    continueFont.setPointSize(16);
    continueFont.setBold(true);
    m_continueButton->setFont(continueFont);
    continueLayout->addWidget(m_continueButton, 0, Qt::AlignCenter);
    continueLayout->addStretch();
    mainLayout->addLayout(continueLayout);

    QHBoxLayout *previewLayout = new QHBoxLayout();
    previewLayout->addStretch();
    m_partyPreviewLabel = new QLabel(tr("No saved game"), this);
    m_partyPreviewLabel->setObjectName("partyPreviewLabel");
    m_partyPreviewLabel->setWordWrap(true);
    m_partyPreviewLabel->setAlignment(Qt::AlignCenter);
    m_partyPreviewLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_partyPreviewLabel->setMinimumHeight(40);
    m_partyPreviewLabel->setMaximumWidth(400);
    previewLayout->addWidget(m_partyPreviewLabel);
    previewLayout->addStretch();
    mainLayout->addLayout(previewLayout);

    mainLayout->addSpacing(10);

    // --- Main Actions (2x3 grid, centered) ---
    QGridLayout *actionsGrid = new QGridLayout();
    actionsGrid->setSpacing(10);
    actionsGrid->setColumnStretch(0, 1);
    actionsGrid->setColumnStretch(1, 1);
    actionsGrid->setColumnStretch(2, 1);

    m_newButton = new QPushButton(tr("New Game"), this);
    m_newButton->setObjectName("newGameButton");
    m_newButton->setMinimumHeight(55);
    QFont actionFont = m_newButton->font();
    actionFont.setPointSize(13);
    actionFont.setBold(true);
    m_newButton->setFont(actionFont);
    actionsGrid->addWidget(m_newButton, 0, 0, Qt::AlignCenter);

    m_createCharacterButton = new QPushButton(tr("Create Character"), this);
    m_createCharacterButton->setObjectName("createCharacterButton");
    m_createCharacterButton->setMinimumHeight(55);
    m_createCharacterButton->setFont(actionFont);
    actionsGrid->addWidget(m_createCharacterButton, 0, 1, Qt::AlignCenter);

    m_assemblePartyButton = new QPushButton(tr("Assemble Party"), this);
    m_assemblePartyButton->setObjectName("assemblePartyButton");
    m_assemblePartyButton->setMinimumHeight(55);
    m_assemblePartyButton->setFont(actionFont);
    actionsGrid->addWidget(m_assemblePartyButton, 0, 2, Qt::AlignCenter);

    m_saveButton = new QPushButton(tr("Save Game"), this);
    m_saveButton->setObjectName("saveGameButton");
    m_saveButton->setMinimumHeight(55);
    m_saveButton->setFont(actionFont);
    actionsGrid->addWidget(m_saveButton, 1, 0, Qt::AlignCenter);

    m_loadButton = new QPushButton(tr("Load Game"), this);
    m_loadButton->setObjectName("loadGameButton");
    m_loadButton->setMinimumHeight(55);
    m_loadButton->setFont(actionFont);
    actionsGrid->addWidget(m_loadButton, 1, 1, Qt::AlignCenter);

    m_recordsButton = new QPushButton(tr("Hall of Records"), this);
    m_recordsButton->setObjectName("recordsButton");
    m_recordsButton->setMinimumHeight(55);
    QFont secondaryFont = m_recordsButton->font();
    secondaryFont.setPointSize(11);
    m_recordsButton->setFont(secondaryFont);
    actionsGrid->addWidget(m_recordsButton, 1, 2, Qt::AlignCenter);

    mainLayout->addLayout(actionsGrid);

    mainLayout->addStretch();

    // --- Footer ---
    QHBoxLayout *footerLayout = new QHBoxLayout();
    footerLayout->setSpacing(8);

    footerLayout->addStretch(1);

    m_optionsButton = new QPushButton(tr("Options"), this);
    m_optionsButton->setObjectName("optionsButton");
    m_optionsButton->setMinimumHeight(55);
    footerLayout->addWidget(m_optionsButton);

    m_multiplayerButton = new QPushButton(tr("Multiplayer"), this);
    m_multiplayerButton->setObjectName("multiplayerButton");
    m_multiplayerButton->setMinimumHeight(55);
    footerLayout->addWidget(m_multiplayerButton);

    m_quitButton = new QPushButton(tr("Quit"), this);
    m_quitButton->setObjectName("quitButton");
    m_quitButton->setMinimumHeight(55);
    footerLayout->addWidget(m_quitButton);

    footerLayout->addStretch(1);

    mainLayout->addLayout(footerLayout);

    // The menu's sizes and weights are its own typography; the family follows
    // the font the player chose in Options.
    connect(fontManager::instance(), &fontManager::fontsChanged,
            this, &GameMenu::syncFontsToAppFont);
    syncFontsToAppFont();

    // Fullscreen is the player's choice (Options dialog), not a hard-coded mode.
    if (GameSettings::instance().fullscreen()) {
        showFullScreen();
    } else {
        resize(1280, 800);
        showNormal();
    }
}

void GameMenu::syncFontsToAppFont() {
    const QFont appFont = QApplication::font();
    if (appFont.family().isEmpty())
        return;

    // The buttons take the font Options selected - family and size - so the
    // menu follows the player's choice instead of a hard-coded size.
    const QList<QWidget*> synced = {
        m_continueButton, m_newButton, m_createCharacterButton,
        m_assemblePartyButton, m_saveButton, m_loadButton,
        m_recordsButton, m_optionsButton, m_multiplayerButton, m_quitButton
    };
    for (QWidget *w : synced) {
        if (!w)
            continue;
        w->setFont(appFont);
    }
}

void GameMenu::setupConnections() {
    // Button Signals
    connect(m_continueButton, &QPushButton::clicked, this, &GameMenu::onContinueClicked);
    connect(m_newButton, &QPushButton::clicked, this, &GameMenu::startNewGame);
    connect(m_createCharacterButton, &QPushButton::clicked, this, &GameMenu::onCreateCharacterClicked);
    connect(m_assemblePartyButton, &QPushButton::clicked, this, &GameMenu::onAssemblePartyClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &GameMenu::saveGame);
    connect(m_loadButton, &QPushButton::clicked, this, &GameMenu::loadGame);
    connect(m_recordsButton, &QPushButton::clicked, this, &GameMenu::showRecords);
    connect(m_optionsButton, &QPushButton::clicked, this, &GameMenu::onOptionsClicked);
    connect(m_multiplayerButton, &QPushButton::clicked, this, &GameMenu::onMultiplayerClicked);
    connect(m_quitButton, &QPushButton::clicked, this, &GameMenu::quitGame);

    // Engine Signals
    connect(EventManager::instance(), &EventManager::eventTriggered, this, &GameMenu::onEventTriggered);

    // Logging Window Setup
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
    // Background is drawn via palette brush in resizeEvent — no debug text.
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

void GameMenu::refreshContinueState() {
    QString autosavePath = "data/saves/autosave.json";
    QFile autosaveFile(autosavePath);

    if (!autosaveFile.exists()) {
        m_continueButton->setVisible(false);
        m_partyPreviewLabel->setVisible(false);
        return;
    }

    // Check if autosave is valid JSON with a living party
    if (!autosaveFile.open(QIODevice::ReadOnly)) {
        m_continueButton->setVisible(false);
        m_partyPreviewLabel->setVisible(false);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(autosaveFile.readAll());
    autosaveFile.close();

    if (doc.isNull() || doc.isEmpty() || !doc.isObject()) {
        m_continueButton->setVisible(false);
        m_partyPreviewLabel->setVisible(false);
        return;
    }

    QJsonObject root = doc.object();
    bool hasLiving = false;
    if (root.contains("Party") && root["Party"].isObject()) {
        QJsonObject party = root["Party"].toObject();
        QJsonArray members = party.value("Members").toArray();
        for (const QJsonValue &v : members) {
            QJsonObject member = v.toObject();
            QString name = member.value("Name").toString();
            bool alive = member.value("isAlive").toBool(true);
            int hp = member.value("HP").toInt(0);
            if (name != "Empty Slot" && !name.isEmpty() && alive && hp > 0) {
                hasLiving = true;
                break;
            }
        }
    }

    if (!hasLiving) {
        m_continueButton->setVisible(false);
        m_partyPreviewLabel->setVisible(false);
        return;
    }

    m_continueButton->setVisible(true);
    m_continueButton->setEnabled(true);
    m_partyPreviewLabel->setVisible(true);
    m_partyPreviewLabel->setText(partyPreviewText());
}

QString GameMenu::partyPreviewText() const {
    QString autosavePath = "data/saves/autosave.json";
    QFile file(autosavePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return tr("Cannot read save");
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject()) {
        return tr("Invalid save");
    }

    QJsonObject root = doc.object();
    QStringList memberNames;
    int totalLevel = 0;
    int memberCount = 0;

    if (root.contains("Party") && root["Party"].isObject()) {
        QJsonObject party = root["Party"].toObject();
        QJsonArray members = party.value("Members").toArray();
        for (const QJsonValue &v : members) {
            QJsonObject member = v.toObject();
            QString name = member.value("Name").toString();
            int level = member.value("Level").toInt(1);
            bool alive = member.value("isAlive").toBool(true);
            if (name != "Empty Slot" && !name.isEmpty()) {
                memberNames.append(name + (alive ? "" : " (deceased)"));
                totalLevel += level;
                memberCount++;
            }
        }
    }

    if (memberCount == 0) {
        return tr("No party members");
    }

    return tr("Party: %1 — %2 members — avg level %3")
        .arg(memberNames.join(", "))
        .arg(memberCount)
        .arg(totalLevel / memberCount);
}

void GameMenu::onContinueClicked() {
    audioManager::instance()->stopAllAudio();
    if (gameStateManager::instance()->loadFullGameState("autosave")) {
        if (gameStateManager::instance()->hasLivingCharacters()) {
            QString loc = gameStateManager::instance()->getGameValue("CurrentLocation").toString();
            if (loc == "Dungeon") {
                enterDungeon();
            } else {
                enterCity();
            }
        } else {
            QMessageBox::warning(this, tr("Load Error"),
                                 tr("The autosave party is deceased."));
        }
    } else {
        QMessageBox::warning(this, tr("Load Error"),
                             tr("Failed to load autosave."));
    }
}

void GameMenu::enterDungeon() {
    audioManager::instance()->stopAllAudio();
    DungeonDialog *dungeonDialog = new DungeonDialog(this);
    dungeonDialog->setAttribute(Qt::WA_DeleteOnClose);
    hide();

    connect(dungeonDialog, &QDialog::finished, this, [this](int){
        this->refreshContinueState();
        this->show();
    });
    dungeonDialog->showNormal();
    dungeonDialog->raise();
    dungeonDialog->activateWindow();
}

void GameMenu::enterCity() {
    audioManager::instance()->stopAllAudio();
    theCity *cityDialog = new theCity(this);
    cityDialog->setAttribute(Qt::WA_DeleteOnClose);
    hide();

    connect(cityDialog, &QDialog::finished, this, [this](int){
        this->refreshContinueState();
        this->show();
    });
    cityDialog->showNormal();
    cityDialog->raise();
    cityDialog->activateWindow();
}

void GameMenu::loadGame() {
    LoadPartyDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    QString filePath = dlg.selectedPartyPath();
    if (filePath.isEmpty()) {
        return;
    }

    // Extract the save name from the path (e.g. "data/saves/autosave.json" → "autosave")
    QFileInfo fi(filePath);
    QString saveName = fi.completeBaseName();

    if (gameStateManager::instance()->loadFullGameState(saveName)) {
        if (gameStateManager::instance()->hasLivingCharacters()) {

            enterCity();
        } else {
            QMessageBox::warning(this, tr("Load Error"),
                                 tr("The saved party is deceased."));
        }
    } else {
        QMessageBox::warning(this, tr("Load Error"),
                             tr("Failed to load the save file."));
    }
}

void GameMenu::saveGame() {
    bool ok;
    QString name = QInputDialog::getText(this, tr("Save Game"),
        tr("Save name:"), QLineEdit::Normal, tr("Party1"), &ok);
    if (!ok || name.isEmpty()) return;

    // Sanitize: no path separators, no ..
    name = name.replace(QRegularExpression("[^a-zA-Z0-9_\\-]"), "_");

    if (gameStateManager::instance()->saveFullGameState(name)) {
        QMessageBox::information(this, tr("Save Game"),
            tr("Game saved as '%1'.").arg(name));
    } else {
        QMessageBox::critical(this, tr("Save Error"),
            tr("Failed to save the game."));
    }
}

void GameMenu::onCharacterCreated(const QString &characterName) {
    // CreateCharacterDialog already saved the character to data/characters/.
    // Return to the main menu so the user can assemble a party and start a game.
    // Do NOT save the game here — creating a character must not overwrite autosave.
    refreshContinueState();
    show();
}

void GameMenu::quitGame() {
    qApp->quit();
}

// Dialog Launchers
void GameMenu::showRecords() {
    (new HallOfRecordsDialog(this))->show();
}

void GameMenu::onOptionsClicked() {
    (new OptionsDialog(this))->show();
}

void GameMenu::onMultiplayerClicked() {
    (new MultiplayerLobbyDialog(this))->show();
}

void GameMenu::startNewGame() {
    // Confirm before wiping any existing game state.
    if (gameStateManager::instance()->getPartyMembers().size() > 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Start New Game"),
            tr("This will clear your current party and start a new game. Continue?"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
    }

    audioManager::instance()->stopAllAudio();

    // Clear any leftover dungeon / placed-item state from a previous game so
    // a fresh start never inherits stale floors or items.
    DungeonLevelRegistry::instance().clear();

    // Clear the party and reset the save name so autosave starts fresh.
    gameStateManager::instance()->getPartyMembers().clear();
    gameStateManager::instance()->setCurrentSaveName("");

    // Always start with creating a new character.
    auto *createDialog = new CreateCharacterDialog(loadRaceData(), loadGuildData(), this);
    createDialog->setAttribute(Qt::WA_DeleteOnClose);
    hide();

    connect(createDialog, &CreateCharacterDialog::characterCreated, this, [this](const QString &) {
        // After character creation, open the party assembler.
        auto *assembleDialog = new AssemblePartyDialog(this);
        assembleDialog->setAttribute(Qt::WA_DeleteOnClose);

        // Start the game only when the user saves the assembled party.
        connect(assembleDialog, &AssemblePartyDialog::partySaved, this, [this]() {
            if (gameStateManager::instance()->getPartyMembers().isEmpty()) {
                QMessageBox::warning(this, tr("Empty Party"),
                    tr("You must add at least one character to the party."));
                return;
            }
            gameStateManager::instance()->startAutosave(10000);
            enterCity();
        });

        connect(assembleDialog, &QDialog::finished, this, [this](int) {
            refreshContinueState();
            show();
        });

        assembleDialog->show();
    });

    connect(createDialog, &QDialog::finished, this, [this](int) {
        refreshContinueState();
        show();
    });

    createDialog->show();
}

void GameMenu::onCreateCharacterClicked() {
    auto *dialog = new CreateCharacterDialog(loadRaceData(), loadGuildData(), this);
    connect(dialog, &CreateCharacterDialog::characterCreated, this, &GameMenu::onCharacterCreated);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void GameMenu::onAssemblePartyClicked() {
    auto *dialog = new AssemblePartyDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QDialog::finished, this, [this](int) {
        refreshContinueState();
        show();
    });
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
    QApplication::setQuitOnLastWindowClosed(false);

    // Keep every active window/dialog on top when focus changes.
    // Without this, dialogs launched from a terminal (or another app) end up
    // behind the window that had focus before.
    QObject::connect(&a, &QApplication::focusChanged, [](QWidget *old, QWidget *now) {
        if (now && old != now) {
            now->raise();
            now->activateWindow();
        }
    });

    // Headless verification suite (`make check`). Runs without any GUI so it
    // can gate every change; see test/selftest.cpp.
    if (a.arguments().contains("--selftest")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
        int rc = runSelfTest();
        if (rc != 0) return rc;
        return runEquipmentTest();
    }

    // Layout probe: build the real DungeonDialog through the normal xcb path,
    // show it, and dump the on-screen geometry of every action button. Used to
    // verify the button grid against what the user actually sees.
    if (a.arguments().contains("--probe-dungeon")) {
        LoadingScreen loadingScreen;
        loadingScreen.exec();
        // Reproduce the REAL style inheritance: in the game DungeonDialog is a
        // child of theCity, a child of GameMenu, and GameMenu applies
        // MainMenu.qss. The sheet must sit on an ANCESTOR, not on the dialog
        // itself, or the dialog's own override would be replaced instead of
        // layered on top. Without this the probe renders plain 32px buttons and
        // misses what the player sees.
        QWidget styleHost;
        {
            QFile qss(QFileInfo(__FILE__).absolutePath() + "/MainMenu.qss");
            if (qss.open(QFile::ReadOnly | QFile::Text))
                styleHost.setStyleSheet(QString::fromUtf8(qss.readAll()));
        }
        styleHost.show();
        DungeonDialog dlg(&styleHost);
        dlg.show();
        // Probe at several sizes: the default resize, the declared minimum
        // (900x640), and the size the user's screen actually gives it.
        QList<QSize> sizes = {QSize(1280, 800), QSize(900, 640), QSize(1636, 1070)};
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
            dlg.grab().save(QString("/tmp/probe_%1x%2.png").arg(sz.width()).arg(sz.height()));
        }
        dlg.resize(1280, 800);
        for (int i = 0; i < 40; ++i) QApplication::processEvents();
        dlg.grab().save("/tmp/dungeon_probe.png");
        qInfo() << "saved grab to /tmp/dungeon_probe.png";
        return 0;
    }

    // Menu probe: build the real GameMenu and report whether its background
    // pixmap loaded. Verifies the introtitle.png copy into DESTDIR.
    if (a.arguments().contains("--probe-menu")) {
        GameMenu w;
        w.resize(1280, 800);
        w.show();
        for (int i = 0; i < 40; ++i) QApplication::processEvents();
        const QString p = QDir::cleanPath(qApp->applicationDirPath() + "/introtitle.png");
        QPixmap probe;
        qInfo() << "=== MENU PROBE ===";
        qInfo() << "applicationDirPath:" << qApp->applicationDirPath();
        qInfo() << "introtitle path:" << p << "exists:" << QFile::exists(p);
        qInfo() << "loads as pixmap:" << probe.load(p)
                << "size:" << probe.size();
        w.grab().save("/tmp/menu_probe.png");
        qInfo() << "saved grab to /tmp/menu_probe.png";
        qInfo() << "=== END MENU PROBE ===";
        return 0;
    }

    // Options probe: run the font path the way the player does and print what
    // actually happens, so the Default button can be verified without driving
    // the GUI by hand.
    if (a.arguments().contains("--probe-options")) {
        GameSettings::instance().applyAll();

        // The real thing: GameMenu carries MainMenu.qss and sets its own fonts,
        // and the Options dialog is created as its child.
        GameMenu menu;
        QLabel *title = menu.findChildren<QLabel *>().first();

        qInfo() << "=== OPTIONS PROBE ===";
        qInfo() << "app font at startup:" << QApplication::font().family()
                << QApplication::font().pointSize();
        qInfo() << "title font (fresh):" << title->font().family()
                << title->font().pointSize();

        OptionsDialog dlg(&menu);
        auto *combo = dlg.findChild<QComboBox*>("combatSpeedComboBox");
        auto *propBtn = dlg.findChild<QPushButton*>("proportionalFontButton");
        qInfo() << "prop button (fresh):" << propBtn->text();

        GameSettings::instance().setProportionalFont("Courier New", 9);
        GameSettings::instance().applyAll();
        qInfo() << "app font after picking Courier New 9:"
                << QApplication::font().family() << QApplication::font().pointSize();
        qInfo() << "combo font after pick:" << combo->font().family()
                << combo->font().pointSize();
        qInfo() << "title font after pick:" << title->font().family()
                << title->font().pointSize();

        dlg.findChild<QPushButton*>("defaultButton")->click();
        qInfo() << "app font after Default:"
                << QApplication::font().family() << QApplication::font().pointSize();
        qInfo() << "combo font after Default:" << combo->font().family()
                << combo->font().pointSize();
        qInfo() << "title font after Default:" << title->font().family()
                << title->font().pointSize();
        qInfo() << "prop button after Default:" << propBtn->text();
        qInfo() << "=== END OPTIONS PROBE ===";
        return 0;
    }

    // Push the stored options into the running game before anything is shown,
    // so a choice made in a previous session is actually in effect.
    GameSettings::instance().applyAll();

    // Initial sequence
    LoadingScreen loadingScreen;
    loadingScreen.exec();

    GameMenu w;
    w.show();
    return a.exec();
}
