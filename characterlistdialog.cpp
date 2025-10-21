#include "characterlistdialog.h"
#include <QHBoxLayout>
#include <QMessageBox>

CharacterListDialog::CharacterListDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Mordor: Character List");
    setFixedSize(400, 500); // Fixed size, adjust as needed

    // 1. Create UI elements
    titleLabel = new QLabel("Select a Character", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18pt; font-weight: bold; margin-bottom: 10px;");

    characterListWidget = new QListWidget(this);
    // Set list widget to allow only single selection
    characterListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    selectButton = new QPushButton("Select", this);
    deleteButton = new QPushButton("Delete", this);
    closeButton = new QPushButton("Close", this);

    // 2. Create layout for buttons (horizontal)
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(selectButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(closeButton);

    // 3. Create main layout (vertical)
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(characterListWidget);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // 4. Connect signals and slots
    connect(selectButton, &QPushButton::clicked, this, &CharacterListDialog::onSelectClicked);
    connect(deleteButton, &QPushButton::clicked, this, &CharacterListDialog::onDeleteClicked);
    connect(closeButton, &QPushButton::clicked, this, &CharacterListDialog::close);
    // Double-click in the list should also select the character
    connect(characterListWidget, &QListWidget::itemDoubleClicked, this, &CharacterListDialog::onSelectClicked);

    // Example Data (should be loaded from a save file in the real game)
    addCharacter("Fighter McSword");
    addCharacter("Mage Fireball");
    addCharacter("Cleric Heal");
    addCharacter("Thief Backstab");
}

CharacterListDialog::~CharacterListDialog()
{
    // Qt handles memory management for child widgets
}

void CharacterListDialog::addCharacter(const QString &characterName)
{
    characterListWidget->addItem(characterName);
}

void CharacterListDialog::onSelectClicked()
{
    QListWidgetItem *selectedItem = characterListWidget->currentItem();
    if (selectedItem) {
        QString characterName = selectedItem->text();
        QMessageBox::information(this, "Character Selected",
                                 "You have selected the character: " + characterName +
                                 "\n(The game would proceed to load the character here)");
        // In a real implementation, you would use accept() or emit a signal
        // with the character's data to close the dialog and load the character.
        accept();
    } else {
        QMessageBox::warning(this, "No Selection", "Please select a character from the list.");
    }
}

void CharacterListDialog::onDeleteClicked()
{
    QListWidgetItem *selectedItem = characterListWidget->currentItem();
    if (selectedItem) {
        QString characterName = selectedItem->text();
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Confirm Deletion",
                                      "Are you sure you want to delete the character: " + characterName + "?",
                                      QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            // Remove from UI
            delete characterListWidget->takeItem(characterListWidget->currentRow());
            // In a real implementation: call function to remove from save file/database
            QMessageBox::information(this, "Deleted", characterName + " has been deleted.");
        }
    } else {
        QMessageBox::warning(this, "No Selection", "Please select a character to delete.");
    }
}
