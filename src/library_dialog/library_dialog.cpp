#include "library_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMap>
#include <QDebug>
#include <QLineEdit>
#include <QInputDialog> // Added for user input
#include <QMessageBox>  // Added for user feedback
#include <QFile>        // Added for file handling
#include <QTextStream>  // Added for reading text streams
#include <QStringList>  // Added for string splitting
#include <QRegularExpression> // Added for robust CSV splitting
// Constructor
LibraryDialog::LibraryDialog(QWidget *parent) : QDialog(parent) 
{
    setWindowTitle("The Library of Knowledge");
    loadKnowledge();
    setupUI();
    // Set default category to "Magic Books" and update the list
    //gameStateManager::instance()->stopMusic();
    // Access using Namespace::ConstantName
    QString category = GameConstants::CATEGORY_MAGIC;
    
    qDebug() << "Selected Category:" << category;
    onCategoryChanged(GameConstants::CATEGORY_MAGIC);
    // Select the first item in the default list
    if (bookList->count() > 0) {
        bookList->setCurrentRow(0);
        onItemSelected(bookList->item(0));
    }
}
// Sets up the main UI layout
void LibraryDialog::setupUI() 
{
    setFixedSize(650, 500);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // --- Header ---
    headerLabel = new QLabel("Mordor: The Library of Knowledge", this);
    //headerLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFD700; padding: 10px;");
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);
    // --- Category and Search Layout ---
    QHBoxLayout *topControlsLayout = new QHBoxLayout();
    // Category Selection
    QHBoxLayout *categoryLayout = new QHBoxLayout();
    QLabel *categoryLabel = new QLabel("Knowledge Category:", this);
    //categoryLabel->setStyleSheet("color: #AFAFAF; font-weight: bold;");
    categoryComboBox = new QComboBox(this);
    //categoryComboBox->setStyleSheet("background-color: #3e2723; color: #FFFFFF; border: 1px solid #5D4037; padding: 5px;");
    categoryComboBox->addItem(GameConstants::CATEGORY_MAGIC);
    categoryComboBox->addItem(GameConstants::CATEGORY_MONSTERS);
    categoryComboBox->addItem(GameConstants::CATEGORY_ITEMS);
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(categoryComboBox);
    topControlsLayout->addLayout(categoryLayout);
    // Search Field (New)
    searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Search for Item, Spell, or Monster...");
    //searchField->setStyleSheet("background-color: #1a1a1a; color: #FFFFFF; border: 1px solid #5D4037; padding: 5px; border-radius: 4px;");
    searchField->setMaximumWidth(250);
    topControlsLayout->addStretch(1); // Push category left and search right
    topControlsLayout->addWidget(searchField);
    mainLayout->addLayout(topControlsLayout);
    // --- Content Area (List and Description) ---
    QHBoxLayout *contentLayout = new QHBoxLayout();
    // 1. List (Left side)
    bookList = new QListWidget(this);
    //bookList->setStyleSheet("background-color: #3e2723; color: #FFFFFF; border: 2px solid #5D4037; selection-background-color: #5D4037; font-size: 14px;");
    bookList->setMaximumWidth(250);
    contentLayout->addWidget(bookList);
    // 2. Description Text (Right side)
    descriptionText = new QTextEdit(this);
    descriptionText->setReadOnly(true);
    //descriptionText->setStyleSheet("background-color: #121212; color: #AFAFAF; font-family: 'Consolas', monospace; border: 2px solid #5D4037; padding: 10px;");
    contentLayout->addWidget(descriptionText);
    mainLayout->addLayout(contentLayout);
    // --- Bottom Control Layout (Buttons and Count) ---
    QVBoxLayout *bottomLayout = new QVBoxLayout();
    // 1. Action Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    // Initialize member variable addItemButton
    addItemButton = new QPushButton("Add New Entry", this);
    //QString addButton = "QPushButton { background-color: #388E3C; color: #FFFFFF; border: 2px solid #2E7D32; border-radius: 8px; padding: 10px; }"
    //                      "QPushButton:hover { background-color: #4CAF50; }";
    //addItemButton->setStyleSheet(addButton);
    addItemButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(addItemButton);
    // Initialize member variable closeButton
    closeButton = new QPushButton("Close Library", this);
    // FIX: Renamed local variable to avoid shadowing the member variable 'closeButton'
    //QString closeButtonStyle = "QPushButton { background-color: #3e2723; color: #FFFFFF; border: 2px solid #5D4037; border-radius: 8px; padding: 10px; }"
    //                      "QPushButton:hover { background-color: #5d4037; }";
    // Use the member pointer and the renamed string
    //closeButton->setStyleSheet(closeButtonStyle);
    closeButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(closeButton);
    bottomLayout->addLayout(buttonLayout);
    // 2. Count Label
    // Initialize member variable countLabel
    countLabel = new QLabel(this);
    //countLabel->setStyleSheet("color: #7FFF00; font-size: 13px; padding-top: 5px;");
    countLabel->setAlignment(Qt::AlignCenter);
    bottomLayout->addWidget(countLabel);
    mainLayout->addLayout(bottomLayout);
    // Call update count once during setup (Assuming updateCountLabel is declared in header)
    updateCountLabel();
    // --- Connections ---
    //connect(bookList, &QListWidget::itemClicked, this, &LibraryDialog::onItemSelected);
    connect(bookList, &QListWidget::currentItemChanged, this,
        [this](QListWidgetItem *current, QListWidgetItem *previous) {
            Q_UNUSED(previous);
            this->onItemSelected(current);
        });
    // Use the member pointer closeButton
    connect(closeButton, &QPushButton::clicked, this, &LibraryDialog::onCloseClicked);
    // Use the member pointer addItemButton
    connect(addItemButton, &QPushButton::clicked, this, &LibraryDialog::onAddItemClicked);
    // Connect Category Combo Box
    connect(categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this](int index) {
            // Clear search when category changes
            this->searchField->clear();
            this->onCategoryChanged(this->categoryComboBox->itemText(index));
        });
    // Connect Search Field (New)
    connect(searchField, &QLineEdit::textChanged, this, &LibraryDialog::onSearchTextChanged);
    // Set dialog background style
    //setStyleSheet("QDialog { background-color: #212121; }");
    setLayout(mainLayout);
}

void LibraryDialog::loadKnowledge()
{
    knowledgeBase.clear();
    gameStateManager* gsm = gameStateManager::instance();

    // 1. Load Monsters from GSM
    for (const QVariantMap& monster : gsm->monsterData()) {
        QString name = monster.value("name").toString();
        if (name.isEmpty()) continue;

        QString description = QString(
            "<b>ID:</b> %1 | <b>Level Found:</b> %2<br>"
            "<b>Stats:</b> Att:%3, Def:%4, Hits:%5<br>"
            "<b>Resistances:</b> Fire:%6, Cold:%7, Elec:%8, Mag:%9<br>"
            "<b>Alignment:</b> %10 | <b>Gold Factor:</b> %11")
            .arg(monster.value("id").toString())
            .arg(monster.value("levelFound").toString())
            .arg(monster.value("att").toString())
            .arg(monster.value("def").toString())
            .arg(monster.value("hits").toString())
            .arg(monster.value("ResFire").toString())
            .arg(monster.value("ResCold").toString())
            .arg(monster.value("ResElectric").toString())
            .arg(monster.value("ResMagic").toString())
            .arg(monster.value("alignment").toString())
            .arg(monster.value("goldFactor").toString());

        knowledgeBase[GameConstants::CATEGORY_MONSTERS].insert(name, description);
    }

    // 2. Load Spells from GSM
    for (const QVariantMap& spell : gsm->spellData()) {
        QString name = spell.value("name").toString();
        if (name.isEmpty()) continue;

        QString description = QString(
            "<b>Level:</b> %1 | <b>Class:</b> %2<br>"
            "<b>Damage:</b> %3-%4<br>"
            "<b>Requirements:</b> Int:%5, Wis:%6")
            .arg(spell.value("Level").toString())
            .arg(spell.value("Class").toString())
            .arg(spell.value("damage1").toString())
            .arg(spell.value("damage2").toString())
            .arg(spell.value("ReqInt").toString())
            .arg(spell.value("ReqWis").toString());

        knowledgeBase[GameConstants::CATEGORY_MAGIC].insert(name, description);
    }
    // 3. Load Items from GSM
    for (const QVariantMap& item : gsm->itemData()) {
        QString name = item.value("name").toString();
        if (name.isEmpty()) continue;

        QString cursed = item.value("cursed").toInt() == 1 
            ? "<span style='color:red;'>CURSED</span>" : "Normal";

        QString description = QString(
            "<b>Price:</b> %1 gold | <b>Status:</b> %2<br>"
            "<b>Stats:</b> Att:%3, Def:%4, Swings:%5<br>"
            "<b>Requirements:</b> Str:%6, Dex:%7")
            .arg(item.value("price").toString())
            .arg(cursed)
            .arg(item.value("att").toString())
            .arg(item.value("def").toString())
            .arg(item.value("swings").toString())
            .arg(item.value("StrReq").toString())
            .arg(item.value("DexReq").toString());

        knowledgeBase[GameConstants::CATEGORY_ITEMS].insert(name, description);
    }
}
/**
 * @brief Updates the QListWidget based on the selected category.
 */
void LibraryDialog::updateList(const QString &category)
{
    bookList->clear();
    descriptionText->clear();
    // Store the full list of names for the current category before filtering
    if (knowledgeBase.contains(category)) {
        const QMap<QString, QString>& currentMap = knowledgeBase.value(category);
        for (const QString& item : currentMap.keys()) {
            new QListWidgetItem(item, bookList);
        }
        // Apply any existing search filter
        onSearchTextChanged(searchField->text());
        // Select the first visible item if the new list is not empty
        QListWidgetItem *firstVisible = nullptr;
        for(int i = 0; i < bookList->count(); ++i) {
            if (!bookList->item(i)->isHidden()) {
                firstVisible = bookList->item(i);
                break;
            }
        }
        if (firstVisible) {
            bookList->setCurrentItem(firstVisible);
            onItemSelected(firstVisible);
        } else {
            // Clear description if no items are visible
            descriptionText->clear();
        }
    } else {
        descriptionText->setHtml("<h2>Error</h2><p>Knowledge Base for this category is corrupt or missing.</p>");
    }
}
/**
 * @brief Recalculates and updates the count label at the bottom of the dialog.
 */
void LibraryDialog::updateCountLabel()
{
    int spellCount = knowledgeBase.value(GameConstants::CATEGORY_MAGIC).count();
    int monsterCount = knowledgeBase.value(GameConstants::CATEGORY_MONSTERS).count();
    int itemCount = knowledgeBase.value(GameConstants::CATEGORY_ITEMS).count();
    QString text = QString("Library Contents: <b>%1</b> Spells | <b>%2</b> Creatures | <b>%3</b> Items")
        .arg(spellCount)
        .arg(monsterCount)
        .arg(itemCount);
    countLabel->setText(text);
}
// Slot called when a magic book is selected from the list
void LibraryDialog::onItemSelected(QListWidgetItem *item)
{
    if (!item) return;
    QString itemName = item->text();
    QString currentCategory = categoryComboBox->currentText();
    QString description = "No description found.";
    if (knowledgeBase.contains(currentCategory)) {
        description = knowledgeBase.value(currentCategory).value(itemName, "The details of this item are lost to time.");
    }
    // Format the text for the QTextEdit
    QString formattedText = QString("<h2>%1</h2><p>%2</p>")
                                .arg(itemName)
                                .arg(description);
    descriptionText->setHtml(formattedText);
}
// Slot called when the category combo box value changes
void LibraryDialog::onCategoryChanged(const QString &categoryName)
{
    updateList(categoryName);
}
// Slot called when the search text changes (New)
void LibraryDialog::onSearchTextChanged(const QString &searchText)
{
    QString filter = searchText.toLower();
    // Iterate through all items in the list widget
    for (int i = 0; i < bookList->count(); ++i) {
        QListWidgetItem *item = bookList->item(i);
        // Check if the item's text contains the filter text (case-insensitive)
        if (item->text().toLower().contains(filter)) {
            item->setHidden(false);
        } else {
            item->setHidden(true);
        }
    }
    // If the currently selected item is hidden, select the first visible item
    QListWidgetItem *currentItem = bookList->currentItem();
    if (currentItem && currentItem->isHidden()) {
        QListWidgetItem *firstVisible = nullptr;
        for(int i = 0; i < bookList->count(); ++i) {
            if (!bookList->item(i)->isHidden()) {
                firstVisible = bookList->item(i);
                break;
            }
        }
        if (firstVisible) {
            bookList->setCurrentItem(firstVisible);
            onItemSelected(firstVisible);
        } else {
            // Clear description if no items are visible
            descriptionText->clear();
        }
    }
}
// Slot called when the 'Add New Entry' button is clicked (New)
void LibraryDialog::onAddItemClicked()
{
    bool ok;
    // 1. Get Category
    QStringList categories = {GameConstants::CATEGORY_MAGIC, GameConstants
        ::CATEGORY_MONSTERS, GameConstants::CATEGORY_ITEMS};
    QString selectedCategory = QInputDialog::getItem(this,
        tr("Add New Entry"),
        tr("Select Category:"),
        categories,
        0,
        false,
        &ok);
    if (!ok) return; // User cancelled
    // 2. Get Name
    QString itemName = QInputDialog::getText(this,
        tr("Add New Entry"),
        tr("Enter Name for the %1:").arg(selectedCategory),
        QLineEdit::Normal,
        QString(),
        &ok);
    if (!ok || itemName.isEmpty()) return; // User cancelled or entered nothing
    // Prevent duplicates
    if (knowledgeBase.value(selectedCategory).contains(itemName)) {
        QMessageBox::warning(this, tr("Duplicate Entry"),
            tr("The entry '%1' already exists in the '%2' category.").arg(itemName).arg(selectedCategory));
        return;
    }
    // 3. Get Description
    QString itemDescription = QInputDialog::getMultiLineText(this,
        tr("Add New Entry"),
        tr("Enter Description/Details for '%1':").arg(itemName),
        QString(),
        &ok);
    if (!ok || itemDescription.isEmpty()) return; // User cancelled or entered nothing
    // 4. Add to knowledgeBase
    knowledgeBase[selectedCategory].insert(itemName, itemDescription);
    // 5. Update UI
    updateCountLabel();
    // Switch to the newly added category and select the new item
    categoryComboBox->setCurrentText(selectedCategory);
    updateList(selectedCategory);
    // Find the newly added item and select it
    QList<QListWidgetItem*> foundItems = bookList->findItems(itemName, Qt::MatchExactly);
    if (!foundItems.isEmpty()) {
        bookList->setCurrentItem(foundItems.first());
        onItemSelected(foundItems.first());
    }
    QMessageBox::information(this, tr("Success"),
        tr("'%1' has been added to the Library.").arg(itemName));
}
// Slot called when the 'Close' button is clicked
void LibraryDialog::onCloseClicked()
{
    this->accept();
}
