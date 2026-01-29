#include "characterlistdialog.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFile>

CharacterListDialog::CharacterListDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Black Lands: Character List");
    setFixedSize(800, 600);
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
    loadCharactersFromFiles();
}
/**
 * @brief Dynamically loads character names from .txt files in the "characters" folder.
 */
void CharacterListDialog::loadCharactersFromFiles()
{
    QString charactersDir = "./data/characters";
    QDir dir(charactersDir);
    if (!dir.exists()) {
        QMessageBox::warning(this, "Load Error", "Character directory not found: " + charactersDir);
        return;
    }
    QStringList filters;
    filters << "*.txt";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    if (fileList.isEmpty()) {
        QMessageBox::information(this, "No Characters", "No character save files (.txt) found in the 'characters' folder.");
    }
    foreach (const QFileInfo &fileInfo, fileList) {
        QString characterName = fileInfo.baseName(); 
        characterListWidget->addItem(characterName);
    }
}

void CharacterListDialog::onSelectClicked()
{
    QListWidgetItem *selectedItem = characterListWidget->currentItem();
    if (selectedItem) {
        QString characterName = selectedItem->text();
        QMessageBox::information(this, "Character Selected",
                                 "You have selected the character: " + characterName +
                                 "\n(The game would proceed to load the character data from 'characters/" + characterName + ".txt' here)");
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
            // Try removing the actual file
            QString filePath = "characters/" + characterName + ".txt";
            if (QFile::remove(filePath)) {
                // Remove from UI if file removed
                delete characterListWidget->takeItem(characterListWidget->currentRow());
                QMessageBox::information(this, "Deleted", characterName + " and its save file have been deleted.");
            } else {
                QMessageBox::critical(this, "Error", "Could not delete the file: " + filePath + 
                                       "\n(File may not exist or permissions are wrong.)");
            }
        }
    } else {
        QMessageBox::warning(this, "No Selection", "Please select a character to delete.");
    }
}

CharacterListDialog::~CharacterListDialog(){}
