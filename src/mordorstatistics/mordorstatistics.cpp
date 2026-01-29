#include "mordorstatistics.h"

MordorStatistics::MordorStatistics(QWidget *parent) :
    QDialog(parent)
{
    setupUi();
    populateTable();
}

void MordorStatistics::setupUi()
{
    setWindowTitle("Mordor Statistics");
    setMinimumSize(600, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Guild Statistics", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    QLabel *instructionLabel = new QLabel("Below is a list of the guilds and information on each one.", this);
    mainLayout->addWidget(instructionLabel);

    guildTable = new QTableWidget(this);
    guildTable->setColumnCount(15); // Guild, AH, ML, MH, EP, QP, Str, Int, Wis, Con, Cha, Dex, G, N, E
    guildTable->setHorizontalHeaderLabels({"Guild", "AH", "ML", "MH", "EP", "QP", "Str", "Int", "Wis", "Con", "Cha", "Dex", "G", "N", "E"});
    guildTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make table read-only
    guildTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    guildTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(guildTable);

    descriptionLabel = new QLabel(this);
    descriptionLabel->setText(
        "<b>AH</b>: Average hit points received when making a level before reaching the level of ML.<br>"
        "<b>ML</b>: The Maximum level up to which AH hit points are received. After reaching ML, MH hit points are received.<br>"
        "<b>MH</b>: K points received when making new levels after reaching ML.<br>"
        "<b>EP</b>: Experience Parameter. The higher the number, the more experience required to make a level.<br>"
        "<b>QP</b>: Quest Parameter. The percentage chance that a character will be assigned a quest after making a level, requiring the quest to be completed before making the next level."
    );
    descriptionLabel->setTextFormat(Qt::RichText);
    descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(descriptionLabel);

    setLayout(mainLayout);
}

void MordorStatistics::populateTable()
{
    // Example data based on the provided image (15 columns: Guild + 11 stats + G, N, E)
    QList<QStringList> data = {
        {"Nomad", "5", "30", "1", "8", "20", "1", "1", "1", "1", "1", "1", "X", "X", "X"},
        {"Warrior", "6", "28", "2", "16", "20", "14", "7", "5", "10", "3", "8", "X", "X", "X"},
        {"Paladin", "5", "29", "2", "19", "25", "14", "9", "9", "10", "16", "15", "X", "", ""},
        {"Ninja", "4", "25", "2", "21", "35", "13", "11", "7", "9", "3", "15", "X", "", "X"},
        {"Villain", "4", "26", "2", "21", "25", "15", "14", "14", "11", "6", "16", "", "", "X"},
        {"Seeker", "4", "27", "2", "18", "20", "10", "13", "13", "11", "5", "13", "X", "X", ""},
        {"Thief", "3", "21", "2", "15", "15", "8", "12", "8", "6", "5", "17", "X", "", ""},
        {"Scavenger", "4", "24", "2", "18", "20", "11", "8", "8", "9", "4", "14", "X", "X", "X"},
        {"Mage", "3", "27", "2", "27", "15", "6", "11", "13", "8", "10", "14", "X", "", ""},
        {"Sorcerer", "2", "29", "2", "27", "10", "6", "14", "13", "11", "5", "8", "X", "X", "X"},
        {"Wizard", "2", "41", "2", "30", "15", "8", "18", "18", "13", "6", "14", "X", "X", "X"},
        {"Healer", "2", "28", "2", "32", "15", "8", "14", "14", "8", "7", "14", "X", "", ""}
    };

    guildTable->setRowCount(data.size());

    for (int row = 0; row < data.size(); ++row) {
        const QStringList &rowData = data.at(row);
        for (int col = 0; col < rowData.size(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(rowData.at(col));
            guildTable->setItem(row, col, item);
        }
    }
    // Adjust column widths to fit content
    guildTable->resizeColumnsToContents();
}

MordorStatistics::~MordorStatistics(){}