#include "include/game_resources.h"
#include "LoadingScreen.h"
#include "../../GameStateManager.h"
#include <QPixmap>
#include <QFont>
#include <QColor>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QEasingCurve> // Added for smooth animation curves

LoadingScreen::LoadingScreen(QWidget *parent) :
    QDialog(parent),
    m_currentFileIndex(0)
{
    // Load resources and update Game State Manager
    GameResources::loadAllResources();
    GameStateManager::instance()->setGameValue("ResourcesLoaded", true);

    // --- Basic Window Setup ---
    setWindowTitle("Mordor");
    setFixedSize(350, 480);

    // --- Widget Creation ---
    m_gameTitleLabel = new QLabel("MORDOR", this);
    
    // --- ANIMATION SYSTEM SETUP ---
    // 1. Create the effect and attach it to the label
    m_titleOpacityEffect = new QGraphicsOpacityEffect(this);
    m_gameTitleLabel->setGraphicsEffect(m_titleOpacityEffect);

    // 2. Setup the animation to target the "opacity" property of the effect
    m_titleFadeAnimation = new QPropertyAnimation(m_titleOpacityEffect, "opacity");
    m_titleFadeAnimation->setDuration(2500);       // 2.5 seconds
    m_titleFadeAnimation->setStartValue(0.0);      // Start invisible
    m_titleFadeAnimation->setEndValue(1.0);        // End fully visible
    m_titleFadeAnimation->setEasingCurve(QEasingCurve::InOutQuad); // Smooth start/stop
    
    // 3. Start the animation immediately
    m_titleFadeAnimation->start(QAbstractAnimation::DeleteWhenStopped); 
    // -------------------------------

    const QString versionString = "Lurked Version 1.1.7.6.450";
    m_versionLabel = new QLabel(versionString, this);
    GameStateManager::instance()->setGameValue("GameVersion", versionString); 

    m_copyrightLabel = new QLabel("© Copyright 1995, MakeItSo Software", this);
    m_imageLabel = new QLabel(this);
    m_loadingMessageLabel = new QLabel("Initializing system...", this);

    // Styling (unchanged)
    QFont titleFont = m_gameTitleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_gameTitleLabel->setFont(titleFont);
    m_gameTitleLabel->setAlignment(Qt::AlignCenter);
    m_gameTitleLabel->setStyleSheet("color: #C00000;");

    // Version/Copyright Styling
    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_versionLabel->setStyleSheet("color: gold;");
    m_copyrightLabel->setAlignment(Qt::AlignCenter);
    m_copyrightLabel->setStyleSheet("color: gold;");

    // Image Setup
    QPixmap originalPixmap = GameResources::getPixmap("mordor_art");
    if (originalPixmap.isNull()) {
        m_imageLabel->setText("IMAGE NOT LOADED");
    } else {
        m_imageLabel->setPixmap(originalPixmap.scaled(QSize(300, 300), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_gameTitleLabel);
    mainLayout->addWidget(m_versionLabel);
    mainLayout->addSpacing(15); 
    mainLayout->addWidget(m_imageLabel, 0, Qt::AlignCenter);
    mainLayout->addSpacing(15); 
    mainLayout->addWidget(m_copyrightLabel);
    mainLayout->addWidget(m_loadingMessageLabel);
    setLayout(mainLayout);

    // File Scanning Logic
    QDir currentDir = QDir::current();
    QStringList fileList = currentDir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name);
    for (const QString &itemName : fileList) {
        m_loadingFiles << "Loading: " + itemName;
    }

    checkSettingsFile();
    
    // Timer Setup
    m_messageTimer = new QTimer(this);
    connect(m_messageTimer, &QTimer::timeout, this, &LoadingScreen::updateLoadingMessage);
    m_messageTimer->start(100);

    m_closeTimer = new QTimer(this);
    connect(m_closeTimer, &QTimer::timeout, this, &LoadingScreen::closeDialogAutomatically);
    m_closeTimer->start(5000);
}

LoadingScreen::~LoadingScreen() {}

void LoadingScreen::checkSettingsFile()
{
    QSettings settings("game_settings.ini", QSettings::IniFormat);
    bool configOK = settings.contains("Graphics/ResolutionWidth");
    m_loadingFiles << (configOK ? "Config OK." : "Config Missing.");
    GameStateManager::instance()->setGameValue("ConfigIntegrityOK", configOK);
}

void LoadingScreen::updateLoadingMessage()
{
    if (m_closeTimer->remainingTime() < 500) { 
        m_messageTimer->stop();
        m_loadingMessageLabel->setText("Initialization complete. Starting game...");
        return;
    }
    
    if (m_currentFileIndex < m_loadingFiles.size()) {
        m_loadingMessageLabel->setText(m_loadingFiles.at(m_currentFileIndex++));
    } else {
        m_currentFileIndex = 0; 
    }
}

void LoadingScreen::closeDialogAutomatically()
{
    m_closeTimer->stop();
    this->accept(); 
}
