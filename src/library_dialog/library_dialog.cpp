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
    onCategoryChanged(CATEGORY_MAGIC);
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
    headerLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFD700; padding: 10px;");
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);
    // --- Category and Search Layout ---
    QHBoxLayout *topControlsLayout = new QHBoxLayout();
    // Category Selection
    QHBoxLayout *categoryLayout = new QHBoxLayout();
    QLabel *categoryLabel = new QLabel("Knowledge Category:", this);
    categoryLabel->setStyleSheet("color: #AFAFAF; font-weight: bold;");
    categoryComboBox = new QComboBox(this);
    categoryComboBox->setStyleSheet("background-color: #3e2723; color: #FFFFFF; border: 1px solid #5D4037; padding: 5px;");
    categoryComboBox->addItem(CATEGORY_MAGIC);
    categoryComboBox->addItem(CATEGORY_MONSTERS);
    categoryComboBox->addItem(CATEGORY_ITEMS);
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(categoryComboBox);
    topControlsLayout->addLayout(categoryLayout);
    // Search Field (New)
    searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Search for Item, Spell, or Monster...");
    searchField->setStyleSheet("background-color: #1a1a1a; color: #FFFFFF; border: 1px solid #5D4037; padding: 5px; border-radius: 4px;");
    searchField->setMaximumWidth(250);
    topControlsLayout->addStretch(1); // Push category left and search right
    topControlsLayout->addWidget(searchField); 
    mainLayout->addLayout(topControlsLayout);
    // --- Content Area (List and Description) ---
    QHBoxLayout *contentLayout = new QHBoxLayout();
    // 1. List (Left side)
    bookList = new QListWidget(this);
    bookList->setStyleSheet("background-color: #3e2723; color: #FFFFFF; border: 2px solid #5D4037; selection-background-color: #5D4037; font-size: 14px;");
    bookList->setMaximumWidth(250);
    contentLayout->addWidget(bookList);
    // 2. Description Text (Right side)
    descriptionText = new QTextEdit(this);
    descriptionText->setReadOnly(true);
    descriptionText->setStyleSheet("background-color: #121212; color: #AFAFAF; font-family: 'Consolas', monospace; border: 2px solid #5D4037; padding: 10px;");
    contentLayout->addWidget(descriptionText);
    mainLayout->addLayout(contentLayout);
    // --- Bottom Control Layout (Buttons and Count) ---
    QVBoxLayout *bottomLayout = new QVBoxLayout();
    // 1. Action Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    // Initialize member variable addItemButton
    addItemButton = new QPushButton("Add New Entry", this);
    QString addButton = "QPushButton { background-color: #388E3C; color: #FFFFFF; border: 2px solid #2E7D32; border-radius: 8px; padding: 10px; }"
                          "QPushButton:hover { background-color: #4CAF50; }";
    addItemButton->setStyleSheet(addButton);
    addItemButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(addItemButton);
    // Initialize member variable closeButton
    closeButton = new QPushButton("Close Library", this);
    // FIX: Renamed local variable to avoid shadowing the member variable 'closeButton'
    QString closeButtonStyle = "QPushButton { background-color: #3e2723; color: #FFFFFF; border: 2px solid #5D4037; border-radius: 8px; padding: 10px; }"
                          "QPushButton:hover { background-color: #5d4037; }";
    // Use the member pointer and the renamed string
    closeButton->setStyleSheet(closeButtonStyle);
    closeButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(closeButton); 
    bottomLayout->addLayout(buttonLayout);
    // 2. Count Label
    // Initialize member variable countLabel
    countLabel = new QLabel(this); 
    countLabel->setStyleSheet("color: #7FFF00; font-size: 13px; padding-top: 5px;");
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
    setStyleSheet("QDialog { background-color: #212121; }");
    setLayout(mainLayout);
}
/**
 * @brief Populates the main knowledge base with all categories,
 * including loading data from a CSV file.
 */
void LibraryDialog::loadKnowledge() 
{    
    // Clear existing knowledge base
    knowledgeBase.clear();   
    // --- 1. Hardcoded Knowledge (Base Data) ---
    // Magic Books (Based on "The Teachings of Magic")
    QMap<QString, QString> magicMap;
    magicMap["Fire Magic"] = "Teaches spells focusing on destructive heat and flame. Essential for destroying creatures vulnerable to fire.";
    magicMap["Cold Magic"] = "Focuses on chilling and freezing enemies. Used to slow opponents and deal devastating cold damage.";
    magicMap["Electrical Magic"] = "Harnesses the power of lightning and shock. Effective against metal-clad or water-based enemies.";
    magicMap["Mind Magic"] = "Involves psychic attacks and illusions. Used to confuse, frighten, and influence the minds of creatures.";
    magicMap["Damage Magic"] = "General offensive spells that deal raw physical or magical damage to a single target.";
    magicMap["Element Magic"] = "Spells that call upon the basic elements (Earth, Air, Water, Fire) to affect the environment or enemies.";
    magicMap["Kill Magic"] = "Potentially devastating spells designed to instantly slay weak or powerful foes, often resisted by high-level creatures.";
    magicMap["Charm Magic"] = "Spells used to pacify or temporarily control creatures, turning them into allies or stopping them from attacking.";
    magicMap["Bind Magic"] = "Focuses on rooting or binding targets, preventing them from moving or attacking for a duration.";
    magicMap["Heal Magic"] = "Essential spells for restoring health and curing status ailments like poison and disease.";
    magicMap["Movement Magic"] = "Allows for teleportation or rapid travel within the depths, bypassing dangerous areas.";
    magicMap["Banish Magic"] = "Used to forcibly remove certain types of enemies from the area or send them back to their origin.";
    magicMap["Dispell Magic"] = "Spells designed to counter and nullify hostile magical effects and enchantments on the party or the environment.";
    magicMap["Resistant Magic"] = "Teaches defensive spells that provide temporary resistance or immunity to various damage types.";
    magicMap["Visual Magic"] = "Spells for improving sight in darkness, revealing hidden objects, or creating light sources.";
    magicMap["Protection Magic"] = "Focuses on enhancing armor class and providing a magical barrier against incoming physical and magical attacks.";
    knowledgeBase[CATEGORY_MAGIC] = magicMap;
    // Creatures (Monsters)
    QMap<QString, QString> monsterMap;
    monsterMap["Goblin Shaman"] = "Weak but clever. Uses minor elemental spells. Avoid low-level encounters if injured.";
    monsterMap["Stone Golem"] = "Immune to physical damage from non-magical weapons. Highly resistant to elemental magic, weak to critical hits.";
    monsterMap["Giant Spider"] = "Fast and deadly. Its bite injects a powerful poison that causes rapid health loss. Requires a high-level cure.";
    monsterMap["Dark Imp"] = "A flying demon that relies on hit-and-run tactics. Highly resistant to Mind and Charm magic.";
    knowledgeBase[CATEGORY_MONSTERS] = monsterMap;
    // Items (Initial/Placeholder Data - will be overwritten/appended by CSV)
    QMap<QString, QString> itemMap;
    itemMap["Bronze Sword"] = "A simple, low-level weapon.";
    itemMap["Mithril Sword"] = "Lightweight and magical, grants quick attacks.";
    knowledgeBase[CATEGORY_ITEMS] = itemMap;
    // --- 2. Load Item Knowledge from CSV File (MDATA3.csv) ---
    QFile file("MDATA3.csv"); // **Updated filename here**
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        // Read and parse the header line
        if (in.atEnd()) {
            qDebug() << "Error: CSV file is empty.";
            file.close();
            return;
        }
        // Read header and map columns
        QStringList header = in.readLine().trimmed().split(','); 
        QMap<QString, int> columnMap;
        for (int i = 0; i < header.size(); ++i) {
            columnMap[header[i].trimmed()] = i;
        }
        // Define required indices for description formatting
        int nameIndex = columnMap.value("name", -1);
        int IDIndex = columnMap.value("ID", -1);
        int attIndex = columnMap.value("att", -1);
        int defIndex = columnMap.value("def", -1);
        int damageModIndex = columnMap.value("damageMod", -1);
        //int typeIndex = columnMap.value("type", -1);
        int StrReqIndex = columnMap.value("StrReq", -1);
        int IntReqIndex = columnMap.value("IntReq", -1);
        int WisReqIndex = columnMap.value("WisReq", -1);
        int ConReqIndex = columnMap.value("ConReq", -1);
        int ChaReqIndex = columnMap.value("ChaReq", -1);
        int DexReqIndex = columnMap.value("DexReq", -1);
        int priceIndex = columnMap.value("price", -1);
        int rarityIndex = columnMap.value("rarity", -1);
        int cursedIndex = columnMap.value("cursed", -1);
        int typeNameIndex = columnMap.value("type", -1); // Use a cleaner name for type column
        // We need 'name' and 'ID' at minimum
        if (nameIndex == -1 || IDIndex == -1) {
            qDebug() << "Error: CSV is missing required 'name' or 'ID' columns. Cannot load item data.";
            file.close();
            return;
        }
        int rowCount = 0;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue; 
            rowCount++;
            // Use simple comma-separated split. 
            QStringList parts = line.split(',');
            if (parts.size() > nameIndex) {
                QString itemName = parts.value(nameIndex).trimmed();
                if (itemName.isEmpty()) {
                    qDebug() << "Warning: Skipping item with empty name on line" << rowCount;
                    continue;
                }
                // --- Build the detailed description string ---
                QString description;
                description += QString("<b>ID:</b> %1 | ").arg(parts.value(IDIndex));
                description += QString("<b>Type:</b> %1 | ").arg(parts.value(typeNameIndex));
                description += QString("<b>Rarity:</b> %1<br>").arg(parts.value(rarityIndex));
                description += QString("<b>Attack:</b> %1 | ").arg(parts.value(attIndex, "0"));
                description += QString("<b>Defense:</b> %1 | ").arg(parts.value(defIndex, "0"));
                description += QString("<b>Damage Mod:</b> %1<br>").arg(parts.value(damageModIndex, "0"));
                // Requirements
                QString requirements;
                if (parts.value(StrReqIndex).toInt() > 0) requirements += QString("Str: %1 ").arg(parts.value(StrReqIndex));
                if (parts.value(IntReqIndex).toInt() > 0) requirements += QString("Int: %1 ").arg(parts.value(IntReqIndex));
                if (parts.value(WisReqIndex).toInt() > 0) requirements += QString("Wis: %1 ").arg(parts.value(WisReqIndex));
                if (parts.value(ConReqIndex).toInt() > 0) requirements += QString("Con: %1 ").arg(parts.value(ConReqIndex));
                if (parts.value(ChaReqIndex).toInt() > 0) requirements += QString("Cha: %1 ").arg(parts.value(ChaReqIndex));
                if (parts.value(DexReqIndex).toInt() > 0) requirements += QString("Dex: %1 ").arg(parts.value(DexReqIndex));
                if (!requirements.isEmpty()) {
                    description += QString("<b>Requirements:</b> %1<br>").arg(requirements.trimmed());
                } else {
                    description += QString("<b>Requirements:</b> None<br>");
                }
                // Price and Cursed status
                description += QString("<b>Price:</b> %1 gold | ").arg(parts.value(priceIndex, "0"));
                if (parts.value(cursedIndex).toInt() == 1) {
                    description += "<b><span style='color:red;'>CURSED!</span></b>";
                } else {
                    description += "Not cursed.";
                }
                // Insert the new entry.
                knowledgeBase[CATEGORY_ITEMS].insert(itemName, description);
            } else {
                qDebug() << "Warning: Corrupt line in CSV (too few columns) on line" << rowCount << ":" << line;
            }
        }
        file.close();
        qDebug() << "Successfully loaded" << rowCount << "items from MDATA3.csv";
    } else {
        qDebug() << "Could not open MDATA3.csv. Using hardcoded item knowledge only.";
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
    int spellCount = knowledgeBase.value(CATEGORY_MAGIC).count();
    int monsterCount = knowledgeBase.value(CATEGORY_MONSTERS).count();
    int itemCount = knowledgeBase.value(CATEGORY_ITEMS).count();
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
    QString formattedText = QString("<h2>%1</h2><p style='color:#E0E0E0;'>%2</p>")
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
    QStringList categories = {CATEGORY_MAGIC, CATEGORY_MONSTERS, CATEGORY_ITEMS};
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
    this->accept(); // Closes the modal dialog
}
