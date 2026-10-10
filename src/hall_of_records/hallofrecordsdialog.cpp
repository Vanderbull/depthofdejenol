#include "hallofrecordsdialog.h"
#include "gameStateManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
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
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollContainer = new QWidget();
    auto *recordsLayout = new QHBoxLayout(scrollContainer);
    recordsLayout->setSpacing(40);
    recordsLayout->setContentsMargins(10, 10, 10, 10);

    auto *leftCol = new QVBoxLayout();
    auto *rightCol = new QVBoxLayout();
    leftCol->setAlignment(Qt::AlignTop);
    rightCol->setAlignment(Qt::AlignTop);

    // --- Guild Records (existing) ---
    gameStateManager* gsm = gameStateManager::instance();
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

        auto *entry = new QLabel(scrollContainer);
        entry->setWordWrap(true);
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

    // --- Ranked Records (new: from Endgame::ranked) ---
    // Read persisted records from game state.
    const QVariantList recordsList = gsm->getGameValue("HallOfRecords").toList();
    QList<GameRecord> records;
    for (const QVariant& v : recordsList) {
        GameRecord rec;
        rec.loadFromMap(v.toMap());
        records.append(rec);
    }

    // Build ranked sections for each category.
    buildRankedSection(leftCol, tr("Highest Level"), Endgame::Category::HighestLevel);
    buildRankedSection(leftCol, tr("Most Gold"), Endgame::Category::MostGold);
    buildRankedSection(rightCol, tr("Deepest Floor"), Endgame::Category::DeepestFloor);
    buildRankedSection(rightCol, tr("Fastest Completion"), Endgame::Category::FastestCompletion);

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

void HallOfRecordsDialog::buildRankedSection(QVBoxLayout* parentLayout, const QString& header,
                                             Endgame::Category category)
{
    gameStateManager* gsm = gameStateManager::instance();
    const QVariantList recordsList = gsm->getGameValue("HallOfRecords").toList();
    QList<GameRecord> records;
    for (const QVariant& v : recordsList) {
        GameRecord rec;
        rec.loadFromMap(v.toMap());
        records.append(rec);
    }

    QList<GameRecord> ranked = Endgame::ranked(records, category);

    auto *sectionLabel = new QLabel(QString("<b style='font-size: 14pt; color: #8e44ad;'>%1</b>").arg(header), parentLayout->parentWidget());
    sectionLabel->setWordWrap(true);
    parentLayout->addWidget(sectionLabel);

    if (ranked.isEmpty()) {
        auto *emptyLabel = new QLabel(tr("(no records yet)"), parentLayout->parentWidget());
        emptyLabel->setStyleSheet("color: #888; font-style: italic;");
        parentLayout->addWidget(emptyLabel);
        return;
    }

    for (int i = 0; i < ranked.size() && i < 5; ++i) {
        const GameRecord& rec = ranked[i];
        QString value;
        switch (category) {
        case Endgame::Category::HighestLevel:     value = QString("Level %1").arg(rec.highestLevel); break;
        case Endgame::Category::MostGold:         value = QString("%1 gold").arg(rec.mostGold); break;
        case Endgame::Category::DeepestFloor:     value = QString("Floor %1").arg(rec.deepestFloor); break;
        case Endgame::Category::FastestCompletion: value = rec.formattedTime(); break;
        }

        auto *entry = new QLabel(parentLayout->parentWidget());
        entry->setWordWrap(true);
        entry->setTextFormat(Qt::RichText);
        entry->setText(QString(
            "<div style='margin-bottom: 8px;'>"
            "<b>%1</b> — %2 %3"
            "</div>"
        ).arg(rec.heroName, value, rec.won ? "🏆" : ""));
        parentLayout->addWidget(entry);
    }
}

void HallOfRecordsDialog::refresh()
{
    // Rebuild the dialog content. For now, just close and reopen.
    accept();
}
