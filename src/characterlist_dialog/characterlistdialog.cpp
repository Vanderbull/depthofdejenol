#include "characterlistdialog.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QFileDialog>

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
    // Multi-select so a whole party can be loaded at once.
    characterListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectButton = new QPushButton("Select", this);
    deleteButton = new QPushButton("Delete", this);
    closeButton = new QPushButton("Close", this);
    savePartyButton = new QPushButton("Save Party", this);
    loadPartyButton = new QPushButton("Load Party", this);
    // Party size indicator
    partySizeLabel = new QLabel(tr("Party: 0 / %1").arg(GameConstants::MAX_PARTY_SIZE), this);
    partySizeLabel->setObjectName("partySizeLabel");
    partySizeLabel->setAlignment(Qt::AlignCenter);

    // Selection preview
    selectionPreviewLabel = new QLabel(tr("Select characters to build your party"), this);
    selectionPreviewLabel->setObjectName("selectionPreviewLabel");
    selectionPreviewLabel->setWordWrap(true);
    selectionPreviewLabel->setAlignment(Qt::AlignCenter);
    selectionPreviewLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    selectionPreviewLabel->setMinimumHeight(50);

    // Clear selection button
    clearSelectionButton = new QPushButton(tr("Clear Selection"), this);
    clearSelectionButton->setObjectName("clearSelectionButton");

    // 2. Create layout for buttons (horizontal)
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(selectButton);
    buttonLayout->addWidget(clearSelectionButton);

    buttonLayout->addWidget(savePartyButton);
    buttonLayout->addWidget(loadPartyButton);
    buttonLayout->addWidget(closeButton);
    // 3. Create main layout (vertical)
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(partySizeLabel);
    mainLayout->addWidget(characterListWidget);
    mainLayout->addWidget(selectionPreviewLabel);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
    // 4. Connect signals and slots
    connect(selectButton, &QPushButton::clicked, this, &CharacterListDialog::onSelectClicked);
    connect(clearSelectionButton, &QPushButton::clicked, this, &CharacterListDialog::onClearSelectionClicked);
    connect(deleteButton, &QPushButton::clicked, this, &CharacterListDialog::onDeleteClicked);
    connect(savePartyButton, &QPushButton::clicked, this, &CharacterListDialog::onSavePartyClicked);
    connect(loadPartyButton, &QPushButton::clicked, this, &CharacterListDialog::onLoadPartyClicked);
    connect(closeButton, &QPushButton::clicked, this, &CharacterListDialog::close);
    connect(characterListWidget, &QListWidget::itemSelectionChanged, this, &CharacterListDialog::updateSelectionPreview);
    // Double-click in the list loads the character(s)
    connect(characterListWidget, &QListWidget::itemDoubleClicked, this, [this]() {
        onSelectClicked();
    });
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
    filters << "*.json";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    if (fileList.isEmpty()) {
        QMessageBox::information(this, "No Characters", "No character save files (.json) found in the 'characters' folder.");
    }
    foreach (const QFileInfo &fileInfo, fileList) {
        QString characterName = fileInfo.baseName(); 
        characterListWidget->addItem(characterName);
    }
}

void CharacterListDialog::onSelectClicked()
{
    auto selectedItems = characterListWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "No Selection", "Please select a character from the list.");
        return;
    }

    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();
    members.clear();

    int loaded = 0;
    for (auto* item : selectedItems) {
        QString characterName = item->text();
        QString filePath = "./data/characters/" + characterName + ".json";
        if (gsm->addCharacterFromFile(filePath)) {
            loaded++;
        } else {
            qWarning() << "Failed to load character:" << filePath;
        }
    }

    if (loaded == 0) {
        QMessageBox::warning(this, "Load Error",
                             "None of the selected characters could be loaded.");
        return;
    }

    if (loaded < selectedItems.size()) {
        QMessageBox::information(this, "Partial Load",
                                 QString("Loaded %1 of %2 selected characters.")
                                     .arg(loaded).arg(selectedItems.size()));
    }

    accept();
}

void CharacterListDialog::onSavePartyClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Party"),
                                                    "./data/characters",
                                                    tr("Party Files (*.json)"));
    if (fileName.isEmpty()) return;

    if (!fileName.endsWith(".json"))
        fileName += ".json";

    if (gameStateManager::instance()->savePartyToFile(fileName)) {
        QMessageBox::information(this, "Saved",
                                 tr("The party was saved to:\n") + fileName);
    } else {
        QMessageBox::critical(this, "Error",
                              tr("Could not save the party to:\n") + fileName);
    }
}

void CharacterListDialog::onLoadPartyClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Party"),
                                                    "./data/characters",
                                                    tr("Party Files (*.json)"));
    if (fileName.isEmpty()) return;

    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();

    if (gsm->loadPartyFromFile(fileName)) {
        QMessageBox::information(this, "Loaded",
                                 tr("Loaded %1 characters from:\n%2")
                                     .arg(members.size()).arg(fileName));
        accept();
    } else {
        QMessageBox::critical(this, "Error",
                              tr("Could not load the party from:\n") + fileName);
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
            QString filePath = "./data/characters/" + characterName + ".json";
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

void CharacterListDialog::onClearSelectionClicked()
{
    characterListWidget->clearSelection();
    updateSelectionPreview();
}

void CharacterListDialog::updateSelectionPreview()
{
    int selectedCount = characterListWidget->selectedItems().size();
    partySizeLabel->setText(tr("Party: %1 / %2").arg(selectedCount).arg(GameConstants::MAX_PARTY_SIZE));

    if (selectedCount == 0) {
        selectionPreviewLabel->setText(tr("Select characters to build your party"));
        return;
    }

    QStringList names;
    int totalLevel = 0;
    for (QListWidgetItem *item : characterListWidget->selectedItems()) {
        names.append(item->text());
        // Try to read level from file
        QString path = "./data/characters/" + item->text() + ".json";
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.startsWith("Level: ")) {
                    totalLevel += line.mid(7).toInt();
                    break;
                }
            }
            file.close();
        }
    }

    QString preview = tr("Selected: %1\nTotal level: %2")
        .arg(names.join(", "))
        .arg(totalLevel);
    selectionPreviewLabel->setText(preview);
}

CharacterListDialog::~CharacterListDialog(){}
