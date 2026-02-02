#include "hallofrecordsdialog.h"
#include "GameStateManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea> // Added for scroll support
#include <QVariantList>
#include <QVariantMap>
#include <QLocale>

HallOfRecordsDialog::HallOfRecordsDialog(QWidget *parent) : QDialog(parent) 
{
    setMinimumSize(1000, 600); 
    setWindowTitle(tr("Hall of Records")); 

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(15, 15, 15, 15);

    // --- Header ---
    auto *title = new QLabel(tr("Records & Masters of Abilities"), this);
    title->setStyleSheet("font-weight: bold; font-size: 18pt; margin-bottom: 10px;");
    title->setAlignment(Qt::AlignCenter);
    rootLayout->addWidget(title);

    // --- Scroll Area Setup ---
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame); // Cleaner look
    
    auto *scrollContainer = new QWidget();
    auto *recordsLayout = new QHBoxLayout(scrollContainer);
    recordsLayout->setSpacing(40);
    recordsLayout->setContentsMargins(10, 10, 10, 10);

    auto *leftCol = new QVBoxLayout();
    auto *rightCol = new QVBoxLayout();
    leftCol->setAlignment(Qt::AlignTop);
    rightCol->setAlignment(Qt::AlignTop);

    // --- Data Processing ---
    GameStateManager* gsm = GameStateManager::instance();
    const QVariantList guildLeadersList = gsm->getGameValue("GuildLeaders").toList();
    const int numRecords = guildLeadersList.size();
    const int leftColumnSize = (numRecords + 1) / 2;
    const QLocale locale;

    for (int i = 0; i < numRecords; ++i) {
        const QVariantMap leaderData = guildLeadersList.at(i).toMap();
        if (leaderData.isEmpty()) continue;

        QString achievement = leaderData.value("Achievement").toString();
        QString leaderName  = leaderData.value("Name").toString();
        QString guildName   = leaderData.value("Guild", tr("The Explorers")).toString();
        
        QVariant val = leaderData.value("RecordValue");
        QString valueStr = (val.typeId() == QMetaType::ULongLong) ? locale.toString(val.toULongLong()) : val.toString();
        QString unit = leaderData.value("RecordUnit").toString();
        if (!unit.isEmpty()) valueStr += " " + unit;

        // Rich Text Label with Word Wrap enabled
        auto *entry = new QLabel(scrollContainer);
        entry->setWordWrap(true); // CRITICAL: Allows text to expand vertically
        entry->setTextFormat(Qt::RichText);
        entry->setText(QString(
            "<div style='margin-bottom: 15px;'>"
            "<b style='font-size: 12pt; color: #2980b9;'>%1</b><br>"
            "Set by: <b>%2</b> (%3)<br>"
            "Record: <span style='color: #27ae60; font-weight: bold;'>%4</span>"
            "</div>"
        ).arg(achievement, leaderName, guildName, valueStr));

        if (i < leftColumnSize) leftCol->addWidget(entry);
        else rightCol->addWidget(entry);
    }

    recordsLayout->addLayout(leftCol);
    recordsLayout->addLayout(rightCol);
    
    scrollArea->setWidget(scrollContainer);
    rootLayout->addWidget(scrollArea);

    // --- Footer ---
    auto *exitButton = new QPushButton(tr("Close Hall"), this);
    exitButton->setFixedSize(160, 40);
    
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(exitButton);
    btnLayout->addStretch();
    rootLayout->addLayout(btnLayout);

    connect(exitButton, &QPushButton::clicked, this, &QDialog::accept);
}

HallOfRecordsDialog::~HallOfRecordsDialog() = default;
