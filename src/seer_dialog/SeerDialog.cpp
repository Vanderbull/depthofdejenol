// SeerDialog.cpp
#include "SeerDialog.h"
#include <QMessageBox>
#include <QRandomGenerator> // Include for random number generation

SeerDialog::SeerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("The Seer");
    setFixedSize(350, 200); // Adjust size as needed

    // --- Widgets ---
    welcomeLabel = new QLabel("<b><font color='blue' size='5'>Welcome to the City Seer!</font></b>", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);

    optionsLabel = new QLabel("<font size='4'>Seer Options</font>", this);
    optionsLabel->setContentsMargins(10, 20, 0, 10); // Adjust margins as needed

    characterButton = new QPushButton("Character", this);
    monsterButton = new QPushButton("Monster", this);
    itemButton = new QPushButton("Item", this);
    exitButton = new QPushButton("Exit", this);

    // --- Layouts ---
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(characterButton);
    buttonLayout->addWidget(monsterButton);
    buttonLayout->addWidget(itemButton);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addWidget(optionsLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(); // Pushes buttons to the top
    mainLayout->addWidget(exitButton, 0, Qt::AlignRight); // Align exit button to the bottom right

    // --- Connections ---
    connect(characterButton, &QPushButton::clicked, this, &SeerDialog::on_characterButton_clicked);
    connect(monsterButton, &QPushButton::clicked, this, &SeerDialog::on_monsterButton_clicked);
    connect(itemButton, &QPushButton::clicked, this, &SeerDialog::on_itemButton_clicked);
    connect(exitButton, &QPushButton::clicked, this, &SeerDialog::on_exitButton_clicked);

    // Set layout for the dialog
    setLayout(mainLayout);
}

SeerDialog::~SeerDialog()
{
    // No explicit deletion of child widgets created with 'new' and parented to 'this'
    // Qt's object tree handles their deletion.
}

// --- Slot implementations ---

void SeerDialog::on_characterButton_clicked()
{
    // High chance to find a character/clue
    int randomChance = QRandomGenerator::global()->bounded(100);
    int successThreshold = 60; // 60% chance of success

    if (randomChance < successThreshold) {
        // Success: Found a character clue
        QString characterName = "Elara, The Sun's Blade";
        QString message = QString("Success! You sense the spirit of a forgotten hero.\nSeek out **%1**.").arg(characterName);

        QMessageBox::information(this, "Seer Option - Character Insight", message);
    } else {
        // Failure: No character insight this time
        QMessageBox::information(this, "Seer Option - Search Failed", "The fates are silent. No notable character can be found at this time.");
    }
}

void SeerDialog::on_monsterButton_clicked()
{
    // Implementation from the previous step (40% chance)
    int randomChance = QRandomGenerator::global()->bounded(100);
    int successThreshold = 40; // 40% chance of success

    if (randomChance < successThreshold) {
        // Success: The monster is found.
        QString monsterName = "Gargoyle";
        QString location = "Old Watchtower (x=15, y=42)";
        QString message = QString("Success! You found a **%1**.\nIt's located at the **%2**.").arg(monsterName, location);

        QMessageBox::information(this, "Seer Option - Monster Found!", message);
    } else {
        // Failure: The monster is not found this time.
        QMessageBox::information(this, "Seer Option - Search Failed", "The city's shadows are thick. You couldn't locate a monster this time.");
    }
}

void SeerDialog::on_itemButton_clicked()
{
    // Low chance to find an item
    int randomChance = QRandomGenerator::global()->bounded(100);
    int successThreshold = 30; // 30% chance of success

    if (randomChance < successThreshold) {
        // Success: Found an item
        QString itemName = "Amulet of Whispers";
        QString location = "Hidden crypt under the Merchant's Guild";
        QString message = QString("Success! A vision of an artifact appears.\nIt is the **%1**, hidden in the **%2**.").arg(itemName, location);

        QMessageBox::information(this, "Seer Option - Item Found!", message);
    } else {
        // Failure: No item found
        QMessageBox::information(this, "Seer Option - Search Failed", "The veil is too thick. No item of note could be discerned.");
    }
}

void SeerDialog::on_exitButton_clicked()
{
    QMessageBox::information(this, "Exit", "Exiting the Seer!");
    accept(); // Closes the dialog with QDialog::Accepted result
}
