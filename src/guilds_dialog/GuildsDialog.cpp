#include "GuildsDialog.h"
#include <QApplication>

GuildsDialog::GuildsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Guilds");
    setFixedSize(600, 450); // Adjust size to roughly match the image

    // --- Left Side Widgets ---
    welcomeLabel = new QLabel("<b><font color='blue' size='4'>Welcome to the Sorcerer's guild!</font></b>");
    welcomeLabel->setAlignment(Qt::AlignCenter);

    guildMasterLabel = new QLabel("<b><font color='red'>GuildMaster</font></b>");
    guildMasterLineEdit = new QLineEdit("Requnix (Level 61)");
    guildMasterLineEdit->setReadOnly(true); // Make it read-only like in the image

    statsRequiredLabel = new QLabel("<b><font color='red'>Stats Required</font></b>");
    statsLabel = new QLabel("Str Int Wis Con Cha Dex\n6  14  13  11  5   8");
    statsLabel->setStyleSheet("border: 1px solid black; padding: 5px; background-color: lightgray;"); // Simulating the box
    statsLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    tooLowLabel = new QLabel("<font color='red'>Your natural stats are too low to join!</font>");
    tooLowLabel->setStyleSheet("font-style: italic;"); // Italicize as in the image

    makeLevelButton = new QPushButton("Make Level");
    reAcquaintButton = new QPushButton("ReAcquaint");
    visitLibraryButton = new QPushButton("Visit the Library");

    // Layout for left side buttons
    QVBoxLayout *leftButtonLayout = new QVBoxLayout();
    leftButtonLayout->addWidget(makeLevelButton);
    leftButtonLayout->addWidget(reAcquaintButton);
    leftButtonLayout->addWidget(visitLibraryButton);

    // Layout for the left side
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(welcomeLabel);
    leftLayout->addSpacing(20);
    leftLayout->addWidget(guildMasterLabel);
    leftLayout->addWidget(guildMasterLineEdit);
    leftLayout->addSpacing(10);
    leftLayout->addWidget(statsRequiredLabel);
    leftLayout->addWidget(statsLabel);
    leftLayout->addWidget(tooLowLabel);
    leftLayout->addStretch(1); // Push content upwards
    leftLayout->addLayout(leftButtonLayout);


    // --- Right Side Widgets ---
    guildsLabel = new QLabel("<b><font color='red'>Guilds</font></b>");
    guildsListWidget = new QListWidget();
    guildsListWidget->addItem("Nomad (1)");
    guildsListWidget->addItem("Paladin");
    guildsListWidget->addItem("Seeker");
    guildsListWidget->addItem("Scavenger");
    guildsListWidget->addItem("Mage");
    guildsListWidget->addItem("Sorcerer");
    guildsListWidget->addItem("Wizard");
    guildsListWidget->setCurrentRow(5); // Select "Sorcerer" as in the image

    expInfoButton = new QPushButton("Exp. Info");
    readGuildLogButton = new QPushButton("Read Guild Log");
    visitButton = new QPushButton("Visit");
    exitButton = new QPushButton("EXIT");

    // Layout for right side buttons
    QVBoxLayout *rightButtonLayout = new QVBoxLayout();
    rightButtonLayout->addWidget(expInfoButton);
    rightButtonLayout->addWidget(readGuildLogButton);
    rightButtonLayout->addSpacing(20); // Spacing between the first two and last two buttons
    rightButtonLayout->addWidget(visitButton);
    rightButtonLayout->addWidget(exitButton);

    // Layout for the right side
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(guildsLabel);
    rightLayout->addWidget(guildsListWidget);
    rightLayout->addLayout(rightButtonLayout);

    // --- Main Layout ---
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(leftLayout, 1); // Give left side more stretch
    mainLayout->addSpacing(20); // Space between left and right columns
    mainLayout->addLayout(rightLayout, 1);
// --- Connect Signals and Slots ---
    setupConnections();
}

GuildsDialog::~GuildsDialog() // Renamed destructor
{
}
void GuildsDialog::setupConnections()
{
    // Left side button connections
    connect(makeLevelButton, &QPushButton::clicked, this, &GuildsDialog::on_makeLevelButton_clicked);
    connect(reAcquaintButton, &QPushButton::clicked, this, &GuildsDialog::on_reAcquaintButton_clicked);
    connect(visitLibraryButton, &QPushButton::clicked, this, &GuildsDialog::on_visitLibraryButton_clicked);

    // Right side button connections
    connect(expInfoButton, &QPushButton::clicked, this, &GuildsDialog::on_expInfoButton_clicked);
    connect(readGuildLogButton, &QPushButton::clicked, this, &GuildsDialog::on_readGuildLogButton_clicked);
    connect(visitButton, &QPushButton::clicked, this, &GuildsDialog::on_visitButton_clicked);
    connect(exitButton, &QPushButton::clicked, this, &GuildsDialog::on_exitButton_clicked);
}

// --- Implementation of Slots (Placeholder Functions) ---

void GuildsDialog::on_makeLevelButton_clicked()
{
    QMessageBox::information(this, "Action", "Make Level button clicked (Requires implementation).");
}

void GuildsDialog::on_reAcquaintButton_clicked()
{
    QMessageBox::information(this, "Action", "ReAcquaint button clicked (Requires implementation).");
}

void GuildsDialog::on_visitLibraryButton_clicked()
{
    QMessageBox::information(this, "Action", "Visit the Library button clicked (Requires implementation).");
}

void GuildsDialog::on_expInfoButton_clicked()
{
    // Should likely show experience related to the selected guild
    QString currentGuild = guildsListWidget->currentItem() ? guildsListWidget->currentItem()->text() : "No Guild Selected";
    QMessageBox::information(this, "Action", "Exp. Info button clicked for: " + currentGuild + " (Requires implementation).");
}

void GuildsDialog::on_readGuildLogButton_clicked()
{
    // Should likely open a new dialog/window with the guild log
    QMessageBox::information(this, "Action", "Read Guild Log button clicked (Requires implementation).");
}

void GuildsDialog::on_visitButton_clicked()
{
    // Should likely change the user's current location to the selected guild
    QString currentGuild = guildsListWidget->currentItem() ? guildsListWidget->currentItem()->text() : "No Guild Selected";
    QMessageBox::information(this, "Action", "Visit button clicked for: " + currentGuild + " (Requires implementation).");
}

void GuildsDialog::on_exitButton_clicked()
{
    // Closes the dialog
    this->accept();
}
