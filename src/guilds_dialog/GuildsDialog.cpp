// GuildsDialog.cpp
#include "GuildsDialog.h"
#include "src/library_dialog/library_dialog.h" // Include the LibraryDialog header
#include <QApplication>
#include <QDebug>
#include <QListWidget> // Needed for QListWidgetItem
#include <QListWidgetItem> // Needed for QListWidgetItem
#include <QVariantList> // Needed for reading the log
#include <QRandomGenerator> // Needed for random number generation

// Assuming GameStateManager is a singleton with an instance() method and a getGameValue() method.

GuildsDialog::GuildsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Guilds");
    setFixedSize(600, 450);

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
    QStringList guilds = GameStateManager::guildNames();
    for (const QString& guild : guilds) {
        guildsListWidget->addItem(guild);
    }
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

GuildsDialog::~GuildsDialog()
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

    connect(guildsListWidget, &QListWidget::itemSelectionChanged, this, &GuildsDialog::on_guildsListWidget_itemSelectionChanged);
}

/**
 * @brief Retrieves the current character's guild from GameStateManager, marks it with a '*' and selects it.
 */
void GuildsDialog::setInitialGuildSelection()
{
    QString currentGuildName = GameStateManager::instance()->getGameValue("CurrentCharacterGuild").toString();
    
    // Only proceed if a guild name is stored
    if (currentGuildName.isEmpty()) {
        return; 
    }

    for (int i = 0; i < guildsListWidget->count(); ++i) {
        QListWidgetItem *item = guildsListWidget->item(i);
        QString itemText = item->text();
        
        // 1. Remove the mark if it exists (on all items) before checking
        if (itemText.startsWith("* ")) {
            itemText.remove(0, 2); // Remove "* "
        }

        // 2. Check for exact match against the stored name
        if (itemText == currentGuildName) {
            // Found a match: ensure it is marked and selected
            item->setText("* " + itemText); // Add the mark
            guildsListWidget->setCurrentItem(item);
        } else {
            // No match: ensure it is unmarked (set the cleaned text back)
            item->setText(itemText); 
        }
    }
}

// --- Implementation of Slots ---

void GuildsDialog::on_makeLevelButton_clicked()
{
    // 1. Get Game State Manager instance
    GameStateManager* gsm = GameStateManager::instance();

    // 2. Retrieve current level
    int currentLevel = gsm->getGameValue("CurrentCharacterLevel").toInt();

    // 3. (Placeholder for XP Check) Check if the player has enough XP to level up.
    bool hasEnoughXP = true; // Replace with actual XP check logic

    if (hasEnoughXP) {
        // 4. Increment the level
        int newLevel = currentLevel + 1;
        
        // 5. Update the game state
        gsm->setGameValue("CurrentCharacterLevel", newLevel);
        
        // 6. Log the successful action
        gsm->logGuildAction(QString("Leveled up to Level %1.").arg(newLevel));
        
        // 7. Provide feedback to the user
        QMessageBox::information(this, "Level Up!", 
            QString("Congratulations! You have advanced to Level %1!").arg(newLevel));
        
    } else {
        // 8. Log the failed action
        gsm->logGuildAction("Attempted to level up, but failed (not enough resources).");
        
        // 9. If they don't have enough resources
        QMessageBox::warning(this, "Cannot Level Up", 
            "You do not have enough experience or gold to advance to the next level.");
    }
}

void GuildsDialog::on_reAcquaintButton_clicked()
{
    QListWidgetItem *selectedItem = guildsListWidget->currentItem();
    GameStateManager* gsm = GameStateManager::instance();
    
    // 1. Check if a guild is selected
    if (!selectedItem) {
        // Log the failed action
        gsm->logGuildAction("Attempted to re-acquaint, but no guild was selected.");
        QMessageBox::warning(this, "Selection Required", "Please select a guild from the list first to re-acquaint.");
        return;
    }

    // 2. Get the full item text (which may be marked, e.g., "* Sorcerer")
    QString newGuildName = selectedItem->text();
    
    // 3. REMOVE THE VISUAL MARK (* ) before saving to game state
    if (newGuildName.startsWith("* ")) {
        newGuildName.remove(0, 2); // Remove "* "
    }
    
    // 4. Update the Game State Manager with the clean-of-mark string
    gsm->setGameValue("CurrentCharacterGuild", newGuildName);
    
    // 5. Log the successful action
    gsm->logGuildAction(QString("Re-acquainted with the %1 guild.").arg(newGuildName));
    
    // 6. Provide confirmation feedback
    QMessageBox::information(this, "Acquaintance Made", 
        QString("You have successfully re-acquainted yourself with the %1 guild.").arg(newGuildName));
        
    // 7. Refresh the list to move the visual mark to the newly selected guild
    setInitialGuildSelection();
}

void GuildsDialog::on_visitLibraryButton_clicked()
{
    // Implementation to open the LibraryDialog
    // Note: Assuming LibraryDialog is correctly defined and available via the include
    LibraryDialog library(this);
    library.exec(); // Execute as a modal dialog
}

void GuildsDialog::on_expInfoButton_clicked()
{
    // 1. Get Game State Manager instance
    GameStateManager* gsm = GameStateManager::instance();

    // 2. Retrieve the current character experience (stored as qulonglong)
    qulonglong currentExp = gsm->getGameValue("CurrentCharacterExperience").value<qulonglong>();
    
    // 3. Retrieve the current level for display context
    int currentLevel = gsm->getGameValue("CurrentCharacterLevel").toInt();

    // 4. Format the information string
    QString infoMessage = QString("Current Level: %1\nTotal Experience: %2").arg(currentLevel).arg(currentExp);
    
    // 5. Display the information
    QMessageBox::information(this, "Character Experience Info", infoMessage);
}

void GuildsDialog::on_readGuildLogButton_clicked()
{
    // 1. Get the log list from the Game State Manager
    QVariantList logList = GameStateManager::instance()->getGameValue("GuildActionLog").toList();

    QString logContent;
    if (logList.isEmpty()) {
        logContent = "The Guild Log is currently empty.";
    } else {
        // 2. Join the list entries into a single string, separated by newlines
        // Reverse the list to show the newest actions first
        for (int i = logList.count() - 1; i >= 0; --i) {
            logContent += logList.at(i).toString() + "\n";
        }
        
        // Remove trailing newline if it exists (not strictly needed with setDetailedText)
        if (!logContent.isEmpty()) {
            logContent.chop(1);
        }
    }
    
    // 3. Display the log content using a message box with detailed text
    QMessageBox logBox(this);
    logBox.setWindowTitle("Guild Action Log");
    logBox.setText("Recent Guild Activities (Scroll down for details):");
    logBox.setDetailedText(logContent);
    logBox.setIcon(QMessageBox::Information);
    logBox.exec();
}

void GuildsDialog::on_visitButton_clicked()
{
    // Generate 6 random stats (between 1 and 20)
    int str = QRandomGenerator::global()->bounded(1, 21);
    int intel = QRandomGenerator::global()->bounded(1, 21);
    int wis = QRandomGenerator::global()->bounded(1, 21);
    int con = QRandomGenerator::global()->bounded(1, 21);
    int cha = QRandomGenerator::global()->bounded(1, 21);
    int dex = QRandomGenerator::global()->bounded(1, 21);

    // *** FIX: Reverted to the simple one-argument form of QString::arg() ***
    // This ensures values are inserted without complex width failures, relying on spaces for alignment.
    QString newStatsText = QString("Str Int Wis Con Cha Dex\n%1  %2  %3  %4  %5   %6")
                               .arg(str)
                               .arg(intel)
                               .arg(wis)
                               .arg(con)
                               .arg(cha)
                               .arg(dex);


    // Update the stats label
    statsLabel->setText(newStatsText);

    // Log the action 
    GameStateManager::instance()->logGuildAction("Stats refreshed via Visit button.");

    // Original 'Visit' functionality placeholder/feedback
    QString currentGuild = guildsListWidget->currentItem() ? guildsListWidget->currentItem()->text() : "No Guild Selected";
    
    // REMOVE MARKER for display clarity in the message box
    if (currentGuild.startsWith("* ")) {
        currentGuild.remove(0, 2); 
    }
    
    QMessageBox::information(this, "Action", 
        "Visit button clicked. The **Stats Required** have been refreshed to random values, and you are visiting: **" + currentGuild + "**.");
}

void GuildsDialog::on_exitButton_clicked()
{
    // Closes the dialog
    this->accept();
}
void GuildsDialog::on_guildsListWidget_itemSelectionChanged()
{
    QListWidgetItem *item = guildsListWidget->currentItem();
    if (!item) return;

    QString guildName = item->text();
    // Clean selection marker if present
    if (guildName.startsWith("* ")) {
        guildName.remove(0, 2); 
    }

    // 1. Update the Welcome Label
    welcomeLabel->setText(QString("<b><font color='blue' size='4'>Welcome to the %1's guild!</font></b>").arg(guildName));

    // 2. Fetch the Master from GameStateManager
    QMap<QString, QString> masters = GameStateManager::guildMasters();
    QString masterName = masters.value(guildName, "Unknown Master");
// 3. Update Stats Required from GameStateManager (MDATA1)
    const QList<QVariantMap>& allGuilds = GameStateManager::instance()->gameData();
    QVariantMap selectedGuildData;

    // Search for the guild entry matching the selected name
    for (const QVariantMap& guild : allGuilds) {
        if (guild["name"].toString() == guildName) {
            selectedGuildData = guild;
            break;
        }
    }

    // 4. Update the stats label if the guild was found
    if (!selectedGuildData.isEmpty()) {
        // reqStats is a nested object in MDATA1.js
        QVariantMap reqStats = selectedGuildData["reqStats"].toMap();
        
        // Format the stats string based on the keys in MDATA1.js
        QString statsText = QString("Str Int Wis Con Cha Dex\n%-3  %-3  %-3  %-3  %-3  %1")
            .arg(reqStats["Str"].toInt())
            .arg(reqStats["Int"].toInt())
            .arg(reqStats["Wis"].toInt())
            .arg(reqStats["Con"].toInt())
            .arg(reqStats["Cha"].toInt())
            .arg(reqStats["Dex"].toInt());

        statsLabel->setText(statsText);
    } else {
        statsLabel->setText("Str Int Wis Con Cha Dex\n--  --  --  --  --  --");
    }

    // 3. Update the LineEdit
    guildMasterLineEdit->setText(masterName);
}
