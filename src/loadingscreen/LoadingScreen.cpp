#include "include/game_resources.h"
#include "LoadingScreen.h"
#include "../../GameStateManager.h" // ADDED: Include the new GameStateManager
#include <QPixmap>
#include <QFont>
#include <QColor>
#include <QFile>
#include <QDir>
#include <QSettings>

LoadingScreen::LoadingScreen(QWidget *parent) :
    QDialog(parent),
    m_currentFileIndex(0)
{
    // Load resources and update Game State Manager
    GameResources::loadAllResources();
    GameStateManager::instance()->setGameValue("ResourcesLoaded", true); // NEW: Update resource status

    // --- Basic Window Setup ---
    setWindowTitle("Mordor");
    setFixedSize(350, 480);

    // --- Widget Creation ---
    m_gameTitleLabel = new QLabel("MORDOR", this);
    
    const QString versionString = "Lurked Version 1.1.7.6.450";
    m_versionLabel = new QLabel(versionString, this);
    // NEW: Store version in the manager
    GameStateManager::instance()->setGameValue("GameVersion", versionString); 

    m_copyrightLabel = new QLabel("© Copyright 1995, MakeItSo Software", this);
    m_imageLabel = new QLabel(this);
    m_loadingMessageLabel = new QLabel("Initializing system...", this);

    // --- Styling: Text ---

    // 1. Game Title ("MORDOR")
    QFont titleFont = m_gameTitleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_gameTitleLabel->setFont(titleFont);
    m_gameTitleLabel->setAlignment(Qt::AlignCenter);
    m_gameTitleLabel->setStyleSheet("color: #C00000;"); // Dark Red

    // 2. Version Label
    QFont versionFont = m_versionLabel->font();
    versionFont.setPointSize(10);
    m_versionLabel->setFont(versionFont);
    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_versionLabel->setStyleSheet("color: gold;");

    // 3. Copyright Label
    QFont copyrightFont = m_copyrightLabel->font();
    copyrightFont.setPointSize(8);
    m_copyrightLabel->setFont(copyrightFont);
    m_copyrightLabel->setAlignment(Qt::AlignCenter);
    m_copyrightLabel->setStyleSheet("color: gold;");


    // --- Styling: Image ---
    QPixmap originalPixmap = GameResources::getPixmap("mordor_art");

    if (originalPixmap.isNull()) {
        originalPixmap = QPixmap(300, 300);
        originalPixmap.fill(Qt::darkGray);
        m_imageLabel->setText("IMAGE NOT LOADED");
        m_imageLabel->setAlignment(Qt::AlignCenter);
    } else {
        QPixmap scaledPixmap = originalPixmap.scaled(
            QSize(300, 300), 
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        m_imageLabel->setPixmap(scaledPixmap);
        m_imageLabel->setAlignment(Qt::AlignCenter);
    }


    // --- Layout ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15); // Padding
    mainLayout->setSpacing(5); // Tight spacing

    // Add widgets in the order they appear
    mainLayout->addWidget(m_gameTitleLabel);
    mainLayout->addWidget(m_versionLabel);
    
    mainLayout->addSpacing(15); 
    
    mainLayout->addWidget(m_imageLabel, 0, Qt::AlignCenter); // Center the image container

    mainLayout->addSpacing(15); 
    mainLayout->addWidget(m_copyrightLabel);
    
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_loadingMessageLabel);

    mainLayout->addStretch(); 

    setLayout(mainLayout);

    // ------------------------------------------------------------------
    // Dynamically load files from the current folder
    // ------------------------------------------------------------------
    
    QDir currentDir = QDir::current();
    QStringList fileList = currentDir.entryList(
        QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs, 
        QDir::Name 
    );

    // Populate m_loadingFiles with actual file names and status messages
    if (fileList.isEmpty()) {
        m_loadingFiles << "No files found in: " + currentDir.path();
        m_loadingFiles << "Falling back to default initialization sequence...";
    } else {
        m_loadingFiles << "Scanning directory: " + currentDir.path();
        
        for (const QString &itemName : fileList) {
            QString status = "Loading: ";
            if (currentDir.exists(itemName) && currentDir.entryInfoList({itemName}, QDir::Dirs).count() > 0) {
                 status = "Scanning directory: ";
            }
            m_loadingFiles << status + itemName;
        }
    }

    // Check for the settings file integrity
    checkSettingsFile();
    
    m_loadingFiles << "Scanning resources complete."
                   << "Game data integrity check complete.";

    // ------------------------------------------------------------------
    
    // --- Timer Setup: Message Cycle ---
    m_messageTimer = new QTimer(this);
    connect(m_messageTimer, &QTimer::timeout, this, &LoadingScreen::updateLoadingMessage);
    m_messageTimer->start(100);

    // --- QTimer Setup: Auto-Close ---
    m_closeTimer = new QTimer(this);
    connect(m_closeTimer, &QTimer::timeout, this, &LoadingScreen::closeDialogAutomatically);
    m_closeTimer->start(5000);
}

LoadingScreen::~LoadingScreen()
{
    // Destructor is empty
}

// --- NEW METHOD IMPLEMENTATION ---
void LoadingScreen::checkSettingsFile()
{
    QSettings settings("game_settings.ini", QSettings::IniFormat);

    QStringList requiredKeys = {
        "Graphics/ResolutionWidth",
        "Graphics/ResolutionHeight",
        "Audio/MasterVolume",
        "Gameplay/Difficulty"
    };

    bool allKeysPresent = true;
    for (const QString &key : requiredKeys) {
        if (!settings.contains(key)) {
            allKeysPresent = false;
            break;
        }
    }
    
    bool configOK = false; // NEW: Local flag to track success for the manager
    
    // Add a loading message based on the check result
    if (QFile::exists("game_settings.ini")) {
        if (allKeysPresent) {
            m_loadingFiles << "Configuration check: game_settings.ini loaded successfully.";
            configOK = true; // Success!
        } else {
            m_loadingFiles << "Configuration WARNING: game_settings.ini is incomplete or corrupted!";
        }
    } else {
        m_loadingFiles << "Configuration ERROR: game_settings.ini not found. Using default values.";
    }

    // NEW: Update the central game state manager
    GameStateManager::instance()->setGameValue("ConfigIntegrityOK", configOK);
}
// --- END NEW METHOD IMPLEMENTATION ---

void LoadingScreen::updateLoadingMessage()
{
    // Stop cycling messages just before the dialog closes
    if (m_closeTimer->remainingTime() < 500) { 
        m_messageTimer->stop();
        m_loadingMessageLabel->setText("Initialization complete. Starting game...");
        return;
    }
    
    // Ensure the index is within the bounds of the list
    if (m_currentFileIndex < m_loadingFiles.size()) {
        m_loadingMessageLabel->setText(m_loadingFiles.at(m_currentFileIndex));
        m_currentFileIndex++;
    } else {
        // If we reach the end of the list, loop back to the beginning
        m_currentFileIndex = 0; 
    }
}

// --- Slot Implementation ---
void LoadingScreen::closeDialogAutomatically()
{
    // Stop the timer to prevent it from firing again
    m_closeTimer->stop();
    
    // Close the dialog.
    this->accept(); 
}
