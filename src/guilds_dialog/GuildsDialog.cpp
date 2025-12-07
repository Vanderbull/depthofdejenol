#include "GuildsDialog.h"
#include <QApplication>
#include "src/library_dialog/library_dialog.h" // Include the LibraryDialog header

// Assuming GameStateManager is a singleton with an instance() method and a getGameValue() method.

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
    guildsListWidget->addItem("Nomad");
    guildsListWidget->addItem("Paladin");
    guildsListWidget->addItem("Warrior");
    guildsListWidget->addItem("Villain");
    guildsListWidget->addItem("Seeker");
    guildsListWidget->addItem("Thief");
    guildsListWidget->addItem("Scavenger");
    guildsListWidget->addItem("Mage");
    guildsListWidget->addItem("Sorcerer");
    guildsListWidget->addItem("Wizard");
    guildsListWidget->addItem("Healer");
    guildsListWidget->addItem("Ninja");

    // Removed hardcoded setCurrentRow(5) to allow setInitialGuildSelection() to take effect

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

// --- Set Initial Guild Selection based on Game State ---
    setInitialGuildSelection();
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

/**
 * @brief Retrieves the current character's guild from GameStateManager and selects it in the list widget.
 */
void GuildsDialog::setInitialGuildSelection()
{
    // Retrieve the current character's guild from GameStateManager
    // GameStateManager::instance() returns a pointer, so we use '->' to call its method.
    QString currentGuildName = GameStateManager::instance()->getGameValue("CurrentCharacterGuild").toString();

    if (currentGuildName.isEmpty()) {
        return; // Nothing to select if the guild name is empty
    }

    // Iterate through the QListWidget items to find a match
    for (int i = 0; i < guildsListWidget->count(); ++i) {
        QListWidgetItem *item = guildsListWidget->item(i);
        
        // We extract the guild name from the item text, removing potential level parentheses like "(1)"
        QString itemText = item->text();
        int parenthesisIndex = itemText.indexOf('(');
        if (parenthesisIndex != -1) {
            itemText = itemText.left(parenthesisIndex).trimmed();
        }

        // Match the cleaned item text against the guild name from the game state
        if (itemText == currentGuildName) {
            guildsListWidget->setCurrentItem(item); // Set the item as current/selected
            break; // Stop searching once found
        }
    }
}

// --- Implementation of Slots ---

void GuildsDialog::on_makeLevelButton_clicked()
{
    // 1. Get Game State Manager instance
    GameStateManager* gsm = GameStateManager::instance();

    // 2. Retrieve current level (using the key you recently added)
    int currentLevel = gsm->getGameValue("CurrentCharacterLevel").toInt();

    // 3. (Placeholder for XP Check) Check if the player has enough XP to level up.
    // NOTE: This assumes you have keys like "CurrentCharacterExperience" and a "LevelUpXPTable" somewhere.
    // For now, we'll simply increment the level for demonstration.
    
    // Check required resources (simplified check for demonstration)
    bool hasEnoughXP = true; // Replace with actual XP check logic

    if (hasEnoughXP) {
        // 4. Increment the level
        int newLevel = currentLevel + 1;
        
        // 5. Update the game state
        gsm->setGameValue("CurrentCharacterLevel", newLevel);
        
        // 6. Provide feedback to the user
        QMessageBox::information(this, "Level Up!", 
            QString("Congratulations! You have advanced to Level %1!").arg(newLevel));

        // 7. (Optional) Refresh the GuildMaster line edit if it was displaying the old level
        // Currently, it's hardcoded to "Requnix (Level 61)", but in a real game, 
        // this line edit might update to show the player's level:
        // guildMasterLineEdit->setText(QString("%1 (Level %2)").arg(currentGuildMasterName).arg(newLevel));
        
    } else {
        // 8. If they don't have enough resources
        QMessageBox::warning(this, "Cannot Level Up", 
            "You do not have enough experience or gold to advance to the next level.");
    }
}

void GuildsDialog::on_reAcquaintButton_clicked()
{
    QMessageBox::information(this, "Action", "ReAcquaint button clicked (Requires implementation).");
}

void GuildsDialog::on_visitLibraryButton_clicked()
{
    // Implementation to open the LibraryDialog
    LibraryDialog library(this);
    library.exec(); // Execute as a modal dialog
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
