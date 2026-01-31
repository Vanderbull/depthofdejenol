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
    //GameStateManager::instance()->stopMusic();
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
/**
 * @brief Populates the main knowledge base with all categories,
 * including loading data from a CSV file.
 */
void LibraryDialog::loadKnowledge() 
{
    knowledgeBase.clear();

    // Helper lambda to load and parse CSV files into categories
    auto loadCSV = [this](const QString& fileName, const QString& category, 
                         std::function<QString(const QStringList&, const QMap<QString, int>&)> descFormatter) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Could not open" << fileName;
            return;
        }

        QTextStream in(&file);
        if (in.atEnd()) return;

        // Parse Header
        QStringList header = in.readLine().trimmed().split(',');
        QMap<QString, int> columnMap;
        for (int i = 0; i < header.size(); ++i) {
            columnMap[header[i].trimmed()] = i;
        }

        int nameIdx = columnMap.value("name", -1);
        if (nameIdx == -1) return;

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;
            
            // Note: Simple split used; if names have commas, use QRegularExpression
            QStringList parts = line.split(',');
            if (parts.size() > nameIdx) {
                QString name = parts[nameIdx].trimmed();
                if (!name.isEmpty()) {
                    knowledgeBase[category].insert(name, descFormatter(parts, columnMap));
                }
            }
        }
        file.close();
    };

    // 1. Load Monsters (MDATA5.csv)
    loadCSV("tools/monsterconverter/data/MDATA5.csv", GameConstants::CATEGORY_MONSTERS, [](const QStringList& p, const QMap<QString, int>& m) {
        return QString( "<br>att:</b> %1 | <b>def:</b> %2 | <b>id:</b> %3<br>"
                        "<b>hits:</b> %4 | <b>numGroups:</b> %5 | <b>picID:</b> %6<br>"
                        "<b>lockedChance:</b> %7 | <b>levelFound:</b> %8<br>"
                        "<b>Resistances:</b> Fire:%9, Cold:%10, Electric:%11 Mind:%11 Poison:%12 Disease:%13 Magic:%14 Physical:%15 Weapon:%16 Spell:%17 Special:%18<br>"
                        "<b>specialPropertyFlags: </b>%19<b>specialAttackFlags: </b>%20<b>spellFlags: </b>%21<b>chance: </b>%22<br>"
                        "<b>boxChance0: </b>%23<b>boxChance1: </b>%24<b>boxChance2: </b>%25<br>"
                        "<b>alignment: </b>%26<b>ingroup: </b>%27<b>goldFactor: </b>%28<b>trapFlags: </b>%29<b>guildlevel: </b>%30<br>"
                        "<b>StatStr: </b>%31<b>StatInt: </b>%32<b>StatWis: </b>%33<b>StatCon: </b>%34<b>StatCha: </b>%35<b>StatDex: </b>%36<br>"
                        "<b>type: </b>%37<b>damageMod: </b>%38<b>companionType: </b>%39<b>companionSpawnMode: </b>%40<b>companionID: </b>%41<br>"
                        "<b>Item0: </b>%42<b>Item1: </b>%43<b>Item2: </b>%44<b>Item3: </b>%45<b>Item4: </b>%46<b>Item5: </b>%47<b>Item6: </b>%48<b>Item7: </b>%49<b>Item8: </b>%50<b>Item9: </b>%51<b>subtype: </b>%52<b>companionSubtype: </b>%53<b>deleted: </b>%54<b>unknown: </b>%55"
                        )
                .arg(p.value(m["att"]), p.value(m["def"]), p.value(m["id"]))
                .arg(p.value(m["hits"]), p.value(m["numGroups"]), p.value(m["picID"]))
                .arg(p.value(m["lockedChance"]), p.value(m["levelFound"]))
                .arg(p.value(m["ResFire"]), p.value(m["ResCold"]), p.value(m["ResElectric"]), p.value(m["ResMind"]), p.value(m["ResPoison"]), p.value(m["ResDisease"]), p.value(m["ResMagic"]), p.value(m["ResPhysical"]), p.value(m["ResWeapon"]), p.value(m["ResSpell"]), p.value(m["ResSpecial"]))
                .arg(p.value(m["specialPropertyFlags"]), p.value(m["specialAttackFlags"]), p.value(m["spellFlags"]), p.value(m["chance"]))
                .arg(p.value(m["boxChance0"]), p.value(m["boxChance1"]), p.value(m["boxChance2"]))
                .arg(p.value(m["alignment"]), p.value(m["ingroup"]), p.value(m["goldFactor"]), p.value(m["trapFlags"]), p.value(m["guildlevel"]))
                .arg(p.value(m["type"]), p.value(m["damageMod"]), p.value(m["companionType"]), p.value(m["companionSpawnMode"]), p.value(m["companionID"]))
                .arg(p.value(m["Item0"]), p.value(m["Item1"]), p.value(m["Item2"]), p.value(m["Item3"]), p.value(m["Item4"]), p.value(m["Item5"]), p.value(m["Item6"]), p.value(m["Item7"]), p.value(m["Item8"]), p.value(m["Item9"]))
                .arg(p.value(m["subType"]), p.value(m["companionSubType"]), p.value(m["deleted"]), p.value(m["unknown"]));
    });

    // 2. Load Spells (MDATA2.csv)
    // Note: Column names in your CSV have leading spaces, e.g., " Level"
    loadCSV("tools/spellconverter/data/MDATA2.csv", GameConstants::CATEGORY_MAGIC, [](const QStringList& p, const QMap<QString, int>& m) {
        return QString("<b>Spell Level:</b> %1 | <b>Class:</b> %2<br>"
                       "<b>Damage:</b> %3-%4<br>"
                       "<b>Req:</b> Int:%5, Wis:%6")
                .arg(p.value(m["Level"]), p.value(m["Class"]))
                .arg(p.value(m["damage1"]), p.value(m["damage2"]))
                .arg(p.value(m["ReqInt"]), p.value(m["ReqWis"]));
    });

    // 3. Load Items (MDATA3.csv)
    loadCSV("tools/itemconverter/data/MDATA3.csv", GameConstants::CATEGORY_ITEMS, [](const QStringList& p, const QMap<QString, int>& m) {
        QString cursed = p.value(m["cursed"]) == "1" ? "<span style='color:red;'>CURSED</span>" : "Normal";
        return QString("<b>Price:</b> %1 gold | <b>Status:</b> %2<br>"
                       "<b>Stats:</b> Att:%3, Def:%4, Swings:%5<br>"
                       "<b>Min Requirements:</b> Str:%6, Dex:%7")
                .arg(p.value(m["price"]), cursed)
                .arg(p.value(m["att"]), p.value(m["def"]), p.value(m["swings"]))
                .arg(p.value(m["StrReq"]), p.value(m["DexReq"]));
    });
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
