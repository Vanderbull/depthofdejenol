#include "hallofrecordsdialog.h"
#include "GameStateManager.h" // Include the GameStateManager header

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScreen>
#include <QGuiApplication>
#include <QFile>
#include <QPushButton> 
#include <QDate> 

// FIX: Corrected constructor name capitalization from HallofRecordsDialog to HallOfRecordsDialog
HallOfRecordsDialog::HallOfRecordsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Records of Masters and Abilities");

    // Load external style sheet
    QFile styleSheetFile("style.qss");
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }

    // Get screen geometry to make the window scale with screen resolution
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int windowWidth = screenGeometry.width() * 0.75;
    int windowHeight = screenGeometry.height() * 0.70;

    // Set a minimum size to prevent the window from becoming too small
    setMinimumSize(750, 600);
    resize(windowWidth, windowHeight);

    // Main layout is a vertical box to stack the records layout and the close button
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setSpacing(20);
    rootLayout->setContentsMargins(40, 40, 40, 40);

    // -----------------------------------------------------------------
    // --- GAME STATE MANAGER INTEGRATION: Displaying System State ---
    // -----------------------------------------------------------------
    
    QGridLayout *stateLayout = new QGridLayout();
    stateLayout->setHorizontalSpacing(20);
    stateLayout->setContentsMargins(0, 0, 0, 10); // Margin below the state box
    
    QLabel *stateTitle = new QLabel("System State (from GameStateManager)");
    stateTitle->setStyleSheet("font-weight: bold; font-size: 14px; color: #444;");
    stateLayout->addWidget(stateTitle, 0, 0, 1, 4, Qt::AlignLeft);

    // Helper function to format boolean status
    auto formatStatus = [](bool status) -> QString {
        return status ? "<span style='color: green; font-weight: bold;'>OK</span>" : "<span style='color: red; font-weight: bold;'>ERROR</span>";
    };

    // Row 1: Game Version and Player Score
    // Get values from the manager
    GameStateManager* gsm = GameStateManager::instance();
    QString gameVersion = gsm->getGameValue("GameVersion").toString();
    int playerScore = gsm->getGameValue("PlayerScore").toInt();
    
    stateLayout->addWidget(new QLabel("<b>Version:</b>"), 1, 0, Qt::AlignLeft);
    stateLayout->addWidget(new QLabel(gameVersion), 1, 1, Qt::AlignLeft);
    
    stateLayout->addWidget(new QLabel("<b>Player Score:</b>"), 1, 2, Qt::AlignLeft);
    stateLayout->addWidget(new QLabel(QString::number(playerScore)), 1, 3, Qt::AlignLeft);

    // Row 2: Resources and Configuration Status
    bool resourcesLoaded = gsm->getGameValue("ResourcesLoaded").toBool();
    bool configOK = gsm->getGameValue("ConfigIntegrityOK").toBool();
    
    stateLayout->addWidget(new QLabel("<b>Resources Loaded:</b>"), 2, 0, Qt::AlignLeft);
    stateLayout->addWidget(new QLabel(formatStatus(resourcesLoaded)), 2, 1, Qt::AlignLeft);

    stateLayout->addWidget(new QLabel("<b>Config Integrity:</b>"), 2, 2, Qt::AlignLeft);
    stateLayout->addWidget(new QLabel(formatStatus(configOK)), 2, 3, Qt::AlignLeft);

    rootLayout->addLayout(stateLayout);
    // -----------------------------------------------------------------
    // --- END GAME STATE MANAGER INTEGRATION ---
    // -----------------------------------------------------------------


    // Records layout
    QGridLayout *recordsLayout = new QGridLayout();
    recordsLayout->setSpacing(10);
    // Setting column stretches for better visual appeal: Category, Master, Date, Stat
    recordsLayout->setColumnStretch(0, 1);
    recordsLayout->setColumnStretch(1, 1);
    recordsLayout->setColumnStretch(2, 0);
    recordsLayout->setColumnStretch(3, 0);

    QLabel *recordsTitle = new QLabel("Records of Masters and Abilities");
    recordsTitle->setStyleSheet("font-size: 20px; font-weight: bold; margin-bottom: 10px;");
    // Span title across all 4 columns
    recordsLayout->addWidget(recordsTitle, 0, 0, 1, 4, Qt::AlignCenter);

    // Define the new categories
    QStringList categories = {
        "Strongest", "Smartest", "Wisest", "Healthiest", "Most Attractive","Deadliest (Defeated)", 
	"Most Experienced Explorer", "Wealthiest Explorer", "Master of Fighting", "Master of Magic", "Master of Thieving"
    };    
    // Companion list for sample stat names and values
    QStringList masterNames = {
	"Crashland","Mager","Theshal","Orgal","Alaya","Nimblefingers",
	"Crashland","Nimlefingers","Morgul","Ge'nal","Nimblefingers"
    };
    QStringList statNames = {
        "Strength", "Intelligence", "Wisdom", "Constitution", "Charisma","Dexterity",
        "Kobold", "Experience", "Gold", "Fight Skill", "Knowledge & Power", "Thieving Skill"
    };

    QList<int> statValues = {
        14, 15, 18, 14, 15, 17, 0, 1322451, 5466453, 17, 6, 15
    };


    // Headers for the record table
    int headerRow = 1;
    
    QLabel *categoryHeader = new QLabel("Category");
    categoryHeader->setStyleSheet("font-weight: bold;");
    recordsLayout->addWidget(categoryHeader, headerRow, 0, Qt::AlignLeft);

    QLabel *masterHeader = new QLabel("Master");
    masterHeader->setStyleSheet("font-weight: bold;");
    recordsLayout->addWidget(masterHeader, headerRow, 1, Qt::AlignLeft);
    
    //QLabel *dateHeader = new QLabel("Date Set");
    //dateHeader->setStyleSheet("font-weight: bold;");
    //recordsLayout->addWidget(dateHeader, headerRow, 2, Qt::AlignCenter);

    QLabel *statHeader = new QLabel("Record Value");
    statHeader->setStyleSheet("font-weight: bold;");
    recordsLayout->addWidget(statHeader, headerRow, 2, Qt::AlignCenter);


    // Populate the grid with the new categories and sample data
    for (int i = 0; i < categories.size(); ++i) {
        int row = i + 2;
        
        // --- Column 1: Category Name ---
        QLabel *categoryLabel = new QLabel(categories.at(i));
        recordsLayout->addWidget(categoryLabel, row, 0, Qt::AlignLeft);

        // --- Column 2: Sample Master Name ---
        QString masterName = masterNames.at(i);
        QLabel *masterNameLabel = new QLabel(masterName);
        recordsLayout->addWidget(masterNameLabel, row, 1, Qt::AlignLeft);

        // --- Column 3: Record Value ---
        QString statString = QString("%1 %2").arg(statValues.at(i)).arg(statNames.at(i));
        QLabel *statLabel = new QLabel(statString);
        recordsLayout->addWidget(statLabel, row, 2, Qt::AlignCenter);
    }

    // Add the records layout to the root layout
    rootLayout->addLayout(recordsLayout);

    // Add a spacer to push the close button to the bottom
    rootLayout->addStretch(1);

    // Close button at the bottom center
    QPushButton *closeButton = new QPushButton("Close");
    closeButton->setFixedSize(150, 40);
    
    // Create a horizontal layout just for the close button to center it
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(closeButton);
    bottomLayout->addStretch(1);

    rootLayout->addLayout(bottomLayout);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
}

HallOfRecordsDialog::~HallOfRecordsDialog() {}
