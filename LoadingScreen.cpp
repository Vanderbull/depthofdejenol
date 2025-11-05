#include "LoadingScreen.h"
#include <QPixmap>
#include <QFont>
#include <QColor> // For setting text/background colors
#include <QFile>  // Potentially needed to check for resource file loading

LoadingScreen::LoadingScreen(QWidget *parent) :
    QDialog(parent),
    m_currentFileIndex(0)
{
    // --- Basic Window Setup ---
    setWindowTitle("Mordor");
    // Set a fixed size that is proportionate to the image (e.g., 350x450)
    setFixedSize(350, 480);

    // Set the dialog background color to a dark blue/black to match the image's aesthetic
    // This is optional and might look different based on the user's OS style.
    // setStyleSheet("background-color: black;"); // Uncomment to enforce a black background

    // --- Widget Creation ---
    m_gameTitleLabel = new QLabel("MORDOR", this);
    m_versionLabel = new QLabel("Lurked Version 1.1.7.6.450", this);
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
    // Set color to Red/Dark Red
    m_gameTitleLabel->setStyleSheet("color: #C00000;"); // Dark Red

    // 2. Version Label
    QFont versionFont = m_versionLabel->font();
    versionFont.setPointSize(10);
    m_versionLabel->setFont(versionFont);
    m_versionLabel->setAlignment(Qt::AlignCenter);
    // Set color to Yellow/Gold
    m_versionLabel->setStyleSheet("color: gold;");

    // 3. Copyright Label
    QFont copyrightFont = m_copyrightLabel->font();
    copyrightFont.setPointSize(8);
    m_copyrightLabel->setFont(copyrightFont);
    m_copyrightLabel->setAlignment(Qt::AlignCenter);
    // Set color to White/Light Gray
    m_copyrightLabel->setStyleSheet("color: white;");


    // --- Styling: Image ---
    // You must replace "YOUR_IMAGE_PATH_HERE" with the actual path to your image
    // (e.g., ":/images/mordor_art.png" if using Qt resources)
    QPixmap originalPixmap("mordor_art.png");

    if (originalPixmap.isNull()) {
        // Fallback: Create a placeholder if the image fails to load
        originalPixmap = QPixmap(300, 300);
        originalPixmap.fill(Qt::darkGray);
        // Display a message indicating image load failure for debugging
        m_imageLabel->setText("IMAGE NOT LOADED");
        m_imageLabel->setAlignment(Qt::AlignCenter);
    } else {
        // Scale the image to fit nicely within the dialog
        QPixmap scaledPixmap = originalPixmap.scaled(
            QSize(300, 300), // Target size for the image area
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        m_imageLabel->setPixmap(scaledPixmap);
        m_imageLabel->setAlignment(Qt::AlignCenter);
    }
    m_imageLabel->setFixedSize(300, 300); // Enforce a specific size for the image area


    // --- Layout ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 25, 25, 25); // Padding
    mainLayout->setSpacing(5); // Tight spacing

    // Add widgets in the order they appear in the image
    mainLayout->addWidget(m_gameTitleLabel);
    mainLayout->addWidget(m_versionLabel);
    
    // Add stretch to push the image down a bit, similar to the screenshot
    mainLayout->addSpacing(15); 
    
    mainLayout->addWidget(m_imageLabel, 0, Qt::AlignCenter); // Center the image container

    mainLayout->addSpacing(15); 
    mainLayout->addWidget(m_copyrightLabel);
    
// Add the new loading message label at the bottom
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_loadingMessageLabel);

    // Add a final stretch to push content toward the top/center (less relevant with fixed size)
    mainLayout->addStretch(); 



    setLayout(mainLayout);

    m_loadingFiles << "Loading MOR.DAT" 
                   << "Loading SPRITES.IMG"
                   << "Loading AUDIO/MUSIC.MP3"
                   << "Loading CHAR/GOODIE.CHR"
                   << "Loading CHAR/MONSTER.LST"
                   << "Loading SYSTEM.INI"
                   << "Scanning resources..."
                   << "Game data integrity check complete.";

// --- Timer Setup: Message Cycle ---
    m_messageTimer = new QTimer(this);
    // Cycle the message every 100 milliseconds
    connect(m_messageTimer, &QTimer::timeout, this, &LoadingScreen::updateLoadingMessage);
    m_messageTimer->start(100);

    // --- NEW: QTimer Setup ---
    m_closeTimer = new QTimer(this);
    // Connect the timer's timeout signal to our private slot
    connect(m_closeTimer, &QTimer::timeout, this, &LoadingScreen::closeDialogAutomatically);
    
    // Start the timer for 5000 milliseconds (5 seconds)
    m_closeTimer->start(5000);
}

LoadingScreen::~LoadingScreen()
{
    // The destructor is empty as QDialog's parent-child mechanism manages
    // the deletion of the child widgets (labels, layouts) automatically.
}
void LoadingScreen::updateLoadingMessage()
{
    // Stop cycling messages just before the dialog closes (e.g., after 4.5 seconds)
    if (m_closeTimer->remainingTime() < 500) { 
        m_messageTimer->stop();
        m_loadingMessageLabel->setText("Initialization complete. Starting game...");
        return;
    }
    
    // Ensure the index is within the bounds of the list
    if (m_currentFileIndex < m_loadingFiles.size()) {
        m_loadingMessageLabel->setText("Loading: " + m_loadingFiles.at(m_currentFileIndex));
        m_currentFileIndex++;
    } else {
        // If we reach the end of the list, loop back to the beginning for continuous updates
        m_currentFileIndex = 0; 
    }
}
// --- NEW: Slot Implementation ---
void LoadingScreen::closeDialogAutomatically()
{
    // Stop the timer to prevent it from firing again
    m_closeTimer->stop();
    
    // Close the dialog. Since we used .exec() in main(), this will return control 
    // to the main function and allow the application to proceed.
    this->accept(); 
}
