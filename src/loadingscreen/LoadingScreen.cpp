#include "src/core/game_resources.h"
#include "LoadingScreen.h"
#include "../../GameStateManager.h"
#include <QPixmap>
#include <QFont>
#include <QColor>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QEasingCurve> // Added for smooth animation curves
#include <QPainter>
LoadingScreen::LoadingScreen(QWidget *parent) :
    QDialog(parent),
    m_currentFileIndex(0)
{
    // Load your uploaded sprite sheet
    m_fontSpriteSheet.load("resources/images/font_spritesheet_transparent.png");
    // Load resources and update Game State Manager
    GameResources::loadAllResources();
    GameStateManager::instance()->setGameValue("ResourcesLoaded", true);
    // --- Basic Window Setup ---
    setWindowTitle("Black land");
    setFixedSize(350, 480);
    // --- Widget Creation ---
    m_gameTitleLabel = new QLabel("BLACK LAND", this);
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
    const QString versionString = "Dejenol Legacy: 0.0.0.0.001";
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
    m_closeTimer->start(100);
}

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
void LoadingScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // Configuration based on your uploaded image
    const int charWidth = 32;  
    const int charHeight = 42; 
    const int charsPerRow = 9; // A-I is 9 chars

    QString textToDraw = "ABC 123";
    textToDraw = textToDraw.toUpper(); // The sheet only has Uppercase

    int drawX = 20; // Starting X position on the dialog
    int drawY = 400; // Starting Y position (near the bottom)

    for (QChar c : textToDraw) {
        int index = -1;
        
        // Map Char to index in your specific sprite sheet
        if (c >= 'A' && c <= 'I') index = c.unicode() - 'A';
        else if (c >= 'J' && c <= 'M') index = 9 + (c.unicode() - 'J'); // Row 2 starts at J
        else if (c >= 'N' && c <= 'V') index = 13 + (c.unicode() - 'N'); // Row 3 starts at N
        else if (c >= 'W' && c <= 'Z') index = 22 + (c.unicode() - 'W'); // Row 4 starts at W
        else if (c >= '1' && c <= '9') index = 26 + (c.unicode() - '1'); // Row 5 starts at 1
        else if (c == '0') index = 35; // 0 is after 9

        if (index != -1) {
            // Calculate source coordinates in the sprite sheet
            int sx = (index % charsPerRow) * charWidth;
            int sy = (index / charsPerRow) * charHeight;

            // Draw the specific letter
            painter.drawPixmap(drawX, drawY, m_fontSpriteSheet, sx, sy, charWidth, charHeight);
        }

        drawX += charWidth - 5; // Move right for next char (with slight kerning)
        if (c == ' ') drawX += 15; // Space handling
    }
}
LoadingScreen::~LoadingScreen() {}
