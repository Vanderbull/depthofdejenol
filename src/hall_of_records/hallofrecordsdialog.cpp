// hallofrecordsdialog.cpp
#include "hallofrecordsdialog.h"
#include "GameStateManager.h" // Include the GameStateManager header

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVariantList> // Added for guild leader list access
#include <QVariantMap>  // Added for guild leader map access
#include <QLocale>      // Added for number formatting (e.g., thousands separator for Gold)
#include <QDebug>       // Added for warning logs

HallOfRecordsDialog::HallOfRecordsDialog(QWidget *parent) : QDialog(parent) {
    // Removed setFixedSize() to allow resizing
    setMinimumSize(1200, 480); // Set a minimum size to ensure content readability
    setWindowTitle("Hall of Records"); 

    // Main layout is a vertical box to stack the title, records, and the exit button
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setSpacing(10);
    rootLayout->setContentsMargins(15, 15, 15, 15);

    // --- Title Label ---
    QLabel *recordsTitle = new QLabel("Records & Masters of Abilities");
    QFont titleFont = recordsTitle->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2); 
    recordsTitle->setFont(titleFont);
    recordsTitle->setAlignment(Qt::AlignCenter);
    
    // Add a horizontal line separator (QLabel used as a line)
    QLabel *separator = new QLabel();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    
    rootLayout->addWidget(recordsTitle);
    rootLayout->addWidget(separator);

    // --- Records Container Layout (Two columns side-by-side) ---
    QHBoxLayout *recordsContainerLayout = new QHBoxLayout();
    recordsContainerLayout->setSpacing(35); // Space between the two main columns

    // Left Column Layout
    QGridLayout *leftRecordsLayout = new QGridLayout();
    leftRecordsLayout->setVerticalSpacing(8);
    // Set column stretch: 1 for Achievement (Name), 4 for Details (Value/Unit/Master)
    leftRecordsLayout->setColumnStretch(0, 1); 
    leftRecordsLayout->setColumnStretch(1, 4); 

    // Right Column Layout
    QGridLayout *rightRecordsLayout = new QGridLayout();
    rightRecordsLayout->setVerticalSpacing(8);
    // Set column stretch for the right column as well
    rightRecordsLayout->setColumnStretch(0, 1); 
    rightRecordsLayout->setColumnStretch(1, 4); 

    // --- GAME STATE MANAGER INTEGRATION ---
    GameStateManager* gsm = GameStateManager::instance();
    QVariantList guildLeadersList = gsm->getGameValue("GuildLeaders").toList();
    
    int numRecords = guildLeadersList.size();
    // Split the records roughly in half for two columns
    int leftColumnSize = (numRecords + 1) / 2; 
    
    int leftRowIndex = 0;
    int rightRowIndex = 0;

    for (int i = 0; i < numRecords; ++i) {
        const QVariant& variant = guildLeadersList.at(i);
        if (!variant.canConvert<QVariantMap>()) {
            qWarning() << "HallOfRecordsDialog: Skipped invalid QVariant entry in GuildLeaders list.";
            continue;
        }

        QVariantMap leaderData = variant.toMap();

        // Extract data
        QString achievement = leaderData.value("Achievement").toString();
        QString leaderName = leaderData.value("Name").toString();
        QVariant recordValue = leaderData.value("RecordValue");
        QString recordUnit = leaderData.value("RecordUnit").toString();
        
        // Use QLocale for number formatting (e.g., thousands separator)
        QString recordValueString = "";
        if (recordValue.typeId() == QMetaType::ULongLong) {
            recordValueString = QLocale().toString(recordValue.toULongLong());
        } else {
            recordValueString = recordValue.toString();
        }

        // 1. Achievement Title 
        QLabel *achievementLabel = new QLabel(achievement);
        QFont achievementFont = achievementLabel->font();
        achievementFont.setBold(true); 
        achievementLabel->setFont(achievementFont);
        
        // 2. Record Details 
        QString recordValueDisplay = recordValueString;
        if (!recordUnit.isEmpty()) {
            recordValueDisplay = QString("%1 %2").arg(recordValueString).arg(recordUnit);
        }

        // Format: Set by "Name" of "Guild" <br> Record: [Value] [Unit]
        // NOTE: The "Guild" key is not always present in the current GameStateManager initialization, 
        // but keeping it here for a standard format, or using a placeholder if empty.
        QString guildName = leaderData.value("Guild").toString();
        if (guildName.isEmpty()) {
            guildName = "The Explorers"; // Placeholder/Default
        }
        
        QString detailString = QString("Set by \"<span style='font-weight:bold;'>%1</span>\" of \"<span style='font-weight:bold;'>%2</span>\"<br>Record Value: %3")
                       .arg(leaderName)
                       .arg(guildName)
                       .arg(recordValueDisplay);

        QLabel *detailLabel = new QLabel(detailString);
        detailLabel->setTextFormat(Qt::RichText); // Keep RichText to handle the <br> and basic bolding

        // --- Add to appropriate column ---
        if (i < leftColumnSize) { // First half records go to the left column
            leftRecordsLayout->addWidget(achievementLabel, leftRowIndex, 0, Qt::AlignLeft | Qt::AlignTop);
            leftRecordsLayout->addWidget(detailLabel, leftRowIndex, 1, Qt::AlignLeft | Qt::AlignTop);
            leftRowIndex++;
        } else { // Remaining records go to the right column
            rightRecordsLayout->addWidget(achievementLabel, rightRowIndex, 0, Qt::AlignLeft | Qt::AlignTop);
            rightRecordsLayout->addWidget(detailLabel, rightRowIndex, 1, Qt::AlignLeft | Qt::AlignTop);
            rightRowIndex++;
        }
    }

    // Add layouts to the container
    recordsContainerLayout->addLayout(leftRecordsLayout);
    recordsContainerLayout->addLayout(rightRecordsLayout);
    
    rootLayout->addLayout(recordsContainerLayout);

    // Add a spacer to push the close button to the bottom
    rootLayout->addStretch(1);

    // --- Exit Button ---
    QPushButton *exitButton = new QPushButton("Exit"); 
    // INCREASED SIZE HERE: 150 wide, 35 high.
    exitButton->setFixedSize(150, 35); 
    
    // Create a horizontal layout just for the close button to center it
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(exitButton);
    bottomLayout->addStretch(1);

    rootLayout->addLayout(bottomLayout);

    connect(exitButton, &QPushButton::clicked, this, &QDialog::close);
}

HallOfRecordsDialog::~HallOfRecordsDialog() {}
