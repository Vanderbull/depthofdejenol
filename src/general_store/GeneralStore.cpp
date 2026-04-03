#include "GeneralStore.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDebug>
#include <QStringList>
#include <QLabel>
#include <QVariant>
#include <QMap>
#include <QMessageBox> // Included for the feedback dialog

void GeneralStore::refreshInventory()
{
    // Fetch latest inventory from GameStateManager
    Character current = GameStateManager::instance()->getCurrentCharacter();
    QStringList items = current.inventory;

    // Prevent recursive signal loops while clearing/adding
    uncurseItemComboBox->blockSignals(true);
    combineItemAComboBox->blockSignals(true);
    combineItemBComboBox->blockSignals(true);
    identifySellItemComboBox->blockSignals(true);

    // Update all dropdowns
    QList<QComboBox*> dropdowns = {
        uncurseItemComboBox,
        combineItemAComboBox,
        combineItemBComboBox,
        identifySellItemComboBox
    };

    for(QComboBox* cb : dropdowns) {
        cb->clear();
        cb->addItems(items);
    }

    // Restore signals
    uncurseItemComboBox->blockSignals(false);
    combineItemAComboBox->blockSignals(false);
    combineItemBComboBox->blockSignals(false);
    identifySellItemComboBox->blockSignals(false);
}

GeneralStore::GeneralStore(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("General Store");
    setupUi();
    loadItemsFromCsv("src/general_store/items.csv");
    // NEW: Populate the Uncurse dropdown with the current character's items
    Character current = GameStateManager::instance()->getCurrentCharacter();
    uncurseItemComboBox->clear();
    uncurseItemComboBox->addItems(current.inventory);

    combineItemAComboBox->clear();
    combineItemAComboBox->addItems(current.inventory);

    combineItemBComboBox->clear();
    combineItemBComboBox->addItems(current.inventory);

    identifySellItemComboBox->clear();
    identifySellItemComboBox->addItems(current.inventory);

    //populateBuyItemsList();
    // Connect signals and slots
    connect(uncurseButton, &QPushButton::clicked, this, &GeneralStore::uncurseItem);
    connect(combineButton, &QPushButton::clicked, this, &GeneralStore::combineItems);
    connect(sellButton, &QPushButton::clicked, this, &GeneralStore::identifySellItem); // Using one slot for ID/Sell
    connect(idButton, &QPushButton::clicked, this, &GeneralStore::identifySellItem);
    connect(infoButton, &QPushButton::clicked, this, &GeneralStore::showItemInfo);
    connect(buyButton, &QPushButton::clicked, this, &GeneralStore::buyItem);
    connect(buyInfoButton, &QPushButton::clicked, this, &GeneralStore::showItemInfo);
    connect(searchItemsLineEdit, &QLineEdit::textChanged, this, &GeneralStore::searchItems);
    connect(exitButton, &QPushButton::clicked, this, &GeneralStore::exitStore);
    // MODIFIED LOGIC: Update cost by extracting 'price' from the QVariantMap
    connect(buyItemsListWidget, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *current, QListWidgetItem *previous) {
        Q_UNUSED(previous);
        if (current) {
            // Retrieve the QVariantMap stored in Qt::UserRole
            QVariantMap itemMap = current->data(Qt::UserRole).toMap();

            // Extract the 'price' value using the map key
            QString priceString = itemMap.value("price").toString();

            // Update the cost label and line edit
            buyItemCostLabel->setText(QString("Cost: %1").arg(priceString));
            buyItemLineEdit->setText(current->text());
        } else {
            buyItemCostLabel->setText("Cost: 0");
            buyItemLineEdit->clear();
        }
    });
}

void GeneralStore::setupUi()
{
    // Initialize Identify/Sell UI element as ComboBox
    identifySellItemComboBox = new QComboBox();

    combineItemAComboBox = new QComboBox();
    combineItemBComboBox = new QComboBox();

    uncurseItemComboBox = new QComboBox(); // Replace QLineEdit
    uncurseButton = new QPushButton("UNCURSE");
    // Initialize UI elements
    //uncurseItemLineEdit = new QLineEdit();
    uncurseButton = new QPushButton("UNCURSE");
    //combineItemLineEdit = new QLineEdit();
    //combineItemsLineEdit = new QLineEdit();
    combineButton = new QPushButton("COMBINE");
    //identifySellItemLineEdit = new QLineEdit();
    itemValueLabel = new QLabel("Value: 0");
    infoButton = new QPushButton("INFO");
    sellButton = new QPushButton("SELL");
    idButton = new QPushButton("ID");
    idCostLabel = new QLabel("ID Cost: 100");
    buyItemsListWidget = new QListWidget();
    searchItemsLineEdit = new QLineEdit();
    searchItemsLineEdit->setPlaceholderText("Search items...");
    buyItemLineEdit = new QLineEdit();
    buyItemLineEdit->setReadOnly(true); // Typically read-only, shows selected item
    buyItemCostLabel = new QLabel("Cost: 0");
    buyButton = new QPushButton("BUY");
    buyInfoButton = new QPushButton("INFO");
    exitButton = new QPushButton("EXIT");
    // Layout for "Uncurse Items"
    QGroupBox *uncurseBox = new QGroupBox("Uncurse Items");
    QHBoxLayout *uncurseLayout = new QHBoxLayout();
    uncurseLayout->addWidget(new QLabel("Item:"));
    uncurseLayout->addWidget(uncurseItemComboBox); // Add the combo box here
    uncurseLayout->addWidget(uncurseButton);
    uncurseBox->setLayout(uncurseLayout);
    //QGroupBox *uncurseBox = new QGroupBox("Uncurse Items");
    //QHBoxLayout *uncurseLayout = new QHBoxLayout();
    //uncurseLayout->addWidget(new QLabel("Item:"));
    //uncurseLayout->addWidget(uncurseItemLineEdit);
    //uncurseLayout->addWidget(uncurseButton);
    //uncurseBox->setLayout(uncurseLayout);
    // Layout for "Combine Items"
    QGroupBox *combineBox = new QGroupBox("Combine Items");
    QGridLayout *combineGridLayout = new QGridLayout();
    combineGridLayout->addWidget(new QLabel("Item A:"), 0, 0);
    combineGridLayout->addWidget(combineItemAComboBox, 0, 1); // Use ComboBox
    combineGridLayout->addWidget(new QLabel("Item B:"), 1, 0);
    combineGridLayout->addWidget(combineItemBComboBox, 1, 1); // Use ComboBox
    combineGridLayout->addWidget(combineButton, 0, 2, 2, 1);
    combineBox->setLayout(combineGridLayout);

    //QGroupBox *combineBox = new QGroupBox("Combine Items");
    //QGridLayout *combineGridLayout = new QGridLayout();
    //combineGridLayout->addWidget(new QLabel("Item A:"), 0, 0);
    //combineGridLayout->addWidget(combineItemLineEdit, 0, 1);
    //combineGridLayout->addWidget(new QLabel("Item B:"), 1, 0);
    //combineGridLayout->addWidget(combineItemsLineEdit, 1, 1);
    //combineGridLayout->addWidget(combineButton, 0, 2, 2, 1); // Span 2 rows
    //combineBox->setLayout(combineGridLayout);
    // Layout for "Identify, Realign & Sell Items"
    QGroupBox *idSellBox = new QGroupBox("Identify, Realign & Sell Items");
    QGridLayout *idSellGridLayout = new QGridLayout();
    idSellGridLayout->addWidget(new QLabel("Item Name/ID:"), 0, 0);
    idSellGridLayout->addWidget(identifySellItemComboBox, 0, 1, 1, 3); // Now adding the ComboBox
    idSellGridLayout->addWidget(itemValueLabel, 1, 0, 1, 2);

    //QGroupBox *idSellBox = new QGroupBox("Identify, Realign & Sell Items");
    //QGridLayout *idSellGridLayout = new QGridLayout();
    //idSellGridLayout->addWidget(new QLabel("Item Name/ID:"), 0, 0);
    //idSellGridLayout->addWidget(identifySellItemLineEdit, 0, 1, 1, 3); // Span 3 columns
    //idSellGridLayout->addWidget(itemValueLabel, 1, 0, 1, 2); // Span 2 columns
    idSellGridLayout->addWidget(idCostLabel, 1, 2, 1, 2); // Span 2 columns
    idSellGridLayout->addWidget(infoButton, 2, 0);
    idSellGridLayout->addWidget(idButton, 2, 1);
    idSellGridLayout->addWidget(sellButton, 2, 2);
    idSellBox->setLayout(idSellGridLayout);
    // Layout for "Buy Items"
    QGroupBox *buyBox = new QGroupBox("Buy Items");
    QVBoxLayout *buyLayout = new QVBoxLayout();
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Search:"));
    searchLayout->addWidget(searchItemsLineEdit);
    buyLayout->addLayout(searchLayout);
    buyLayout->addWidget(buyItemsListWidget);
    QHBoxLayout *buyDetailsLayout = new QHBoxLayout();
    buyDetailsLayout->addWidget(buyItemLineEdit);
    buyDetailsLayout->addWidget(buyItemCostLabel);
    buyLayout->addLayout(buyDetailsLayout);
    QHBoxLayout *buyControlsLayout = new QHBoxLayout();
    buyControlsLayout->addWidget(buyInfoButton);
    buyControlsLayout->addWidget(buyButton);
    buyLayout->addLayout(buyControlsLayout);
    buyBox->setLayout(buyLayout);
    // Main Layout (Top half for all services, Bottom for Buy/Exit)
    QHBoxLayout *topLayout = new QHBoxLayout();
    QVBoxLayout *leftColumn = new QVBoxLayout();
    leftColumn->addWidget(uncurseBox);
    leftColumn->addWidget(combineBox);
    leftColumn->addWidget(idSellBox);
    topLayout->addLayout(leftColumn);
    topLayout->addWidget(buyBox);
    // Final main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(exitButton);
    setLayout(mainLayout);
    resize(800, 600); // Set a reasonable default size
}

void GeneralStore::populateBuyItemsList()
{
    // The QStringList array to hold all item data from MDATA3.csv.
    // Each string represents one item/row.
    QStringList buyItemData;
    buyItemData << "Helm of Astral Projection,78,0,3,5213451,5,2,0,0,0,80,66,10,4095,10,1.6,14,0,13,0,14,16,16,0,0,14,0,0,0,0,0,0,0,75,-1";
    // Define the headers to be used as keys
    QStringList headers;
    headers << "name" << "ID" << "att" << "def" << "price" << "floor" << "rarity" << "abilities" << "swings" << "specialType" << "spellIndex" << "spellID" << "charges" << "guilds" << "levelScale" << "damageMod" << "alignmentFlags" << "nHands" << "type" << "resistanceFlags" << "StrReq" << "IntReq" << "WisReq" << "ConReq" << "ChaReq" << "DexReq" << "StrMod" << "IntMod" << "WisMod" << "ConMod" << "ChaMod" << "DexMod" << "cursed" << "spellLvl" << "classRestricted";
    // Loop through the data array to populate the QListWidget
    for (const QString &itemData : buyItemData) {
        QStringList values = itemData.split(",");
        // Safety check for data integrity
        if (values.size() != headers.size()) {
            qDebug() << "Warning: Item data does not match header count for:" << itemData;
            continue; // Skip malformed rows
        }
        QVariantMap itemMap;
        for (int i = 0; i < headers.size(); ++i) {
            // Store all values as QVariants (QString) in the map
            itemMap.insert(headers.at(i), values.at(i));
        }
        // Extract the item name
        QString name = itemMap.value("name").toString();
        // Create a new list widget item with the name
        QListWidgetItem *item = new QListWidgetItem(name);
        // Store the QVariantMap in Qt::UserRole
        item->setData(Qt::UserRole, itemMap);
        buyItemsListWidget->addItem(item);
    }
    refreshInventory();
}
// --- Feedback Helper ---
void GeneralStore::showFeedbackDialog(const QString &title, const QString &message, QMessageBox::Icon icon)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.exec();
}

// GeneralStore.cpp
void GeneralStore::uncurseItem() {
    QString selectedItem = uncurseItemComboBox->currentText(); // Get from dropdown

    if (selectedItem.isEmpty()) {
        showFeedbackDialog("No Item", "Please select an item to uncurse.", QMessageBox::Warning);
        return;
    }

    // Logic to check if item is actually cursed and remove gold would go here
    qDebug() << "Attempting to uncurse:" << selectedItem;
}
// --- Slot Implementations (Updated to use QMessageBox) --
/*
void GeneralStore::uncurseItem()
{
    QString itemName = uncurseItemLineEdit->text().trimmed();
    if (itemName.isEmpty()) {
        showFeedbackDialog("Uncurse Error", "Please enter the name of the item you wish to uncurse.", QMessageBox::Warning);
        return;
    }
    GameStateManager* gsm = GameStateManager::instance();
    int activeCharIndex = gsm->getGameValue("ActiveCharacterIndex").toInt();
    // Retrieve character inventory
    QVariantList party = gsm->getGameValue("Party").toList();
    QVariantMap character = party[activeCharIndex].toMap();
    QStringList inventory = character["Inventory"].toStringList();
    // 1. Check if the player owns the item
    int invIndex = -1;
    for (int i = 0; i < inventory.size(); ++i) {
        if (inventory[i].compare(itemName, Qt::CaseInsensitive) == 0) {
            invIndex = i;
            break;
        }
    }
    if (invIndex == -1) {
        showFeedbackDialog("Uncurse Error", "This item is not in your inventory.", QMessageBox::Warning);
        return;
    }
    // 2. Check the item database if it is cursed
    bool isCursed = false;
    const QList<QVariantMap>& db = gsm->itemData();
    for (const QVariantMap& item : db) {
        if (item["name"].toString().compare(itemName, Qt::CaseInsensitive) == 0) {
            // Check the "cursed" column (In your data, column index 32 often tracks special flags like cursed)
            if (item["cursed"].toInt() == 1 || item["cursed"].toString() == "1") {
                isCursed = true;
            }
            break;
        }
    }
    if (isCursed) {
        // 3. Perform the Uncursing
        QString oldName = inventory[invIndex];
        QString newName = oldName + "-uncursed";
        inventory[invIndex] = newName;
        // 4. Persist the changes
        gsm->setCharacterInventory(activeCharIndex, inventory);

        showFeedbackDialog("Uncurse Success",
            QString("The curse has been lifted! '%1' is now '%2'.").arg(oldName, newName),
            QMessageBox::Information);

        uncurseItemLineEdit->clear();
    } else {
        showFeedbackDialog("Uncurse Status",
            QString("'%1' is not cursed and does not require uncursing.").arg(itemName),
            QMessageBox::Information);
    }
}
*/
void GeneralStore::combineItems()
{
    QString itemA = combineItemAComboBox->currentText();
    QString itemB = combineItemBComboBox->currentText();

    if (itemA.isEmpty() || itemB.isEmpty()) {
        showFeedbackDialog("Combine Error", "Please select two items to attempt a combination.", QMessageBox::Warning);
        return;
    }

    // Logic remains the same, just using dropdown strings
    if ((itemA.toLower().contains("steel sword") && itemB.toLower().contains("crystal")) ||
        (itemB.toLower().contains("steel sword") && itemA.toLower().contains("crystal"))) {
        showFeedbackDialog("Combine Success",
                           QString("The combination created a magnificent Crystal Steel Blade!"),
                           QMessageBox::Information);
    } else {
        showFeedbackDialog("Combine Failure", "These items did not combine.", QMessageBox::Warning);
    }
    refreshInventory();
}
/*
void GeneralStore::combineItems()
{
    QString itemA = combineItemLineEdit->text().trimmed();
    QString itemB = combineItemsLineEdit->text().trimmed();
    if (itemA.isEmpty() || itemB.isEmpty()) {
        showFeedbackDialog("Combine Error", "Please enter two item names to attempt a combination.", QMessageBox::Warning);
        return;
    }
    // Placeholder Logic:
    // 1. Check if the player has both 'itemA' and 'itemB'.
    // 2. Check a combination recipe database.
    // Simulate combination based on specific inputs
    if ((itemA.toLower().contains("steel sword") && itemB.toLower().contains("crystal")) ||
        (itemB.toLower().contains("steel sword") && itemA.toLower().contains("crystal"))) {
        showFeedbackDialog("Combine Success",
                           QString("The combination of '%1' and '%2' created a magnificent Crystal Steel Blade!").arg(itemA).arg(itemB),
                           QMessageBox::Information);
    } else {
        showFeedbackDialog("Combine Failure",
                           QString("The items '%1' and '%2' did not combine. No known recipe exists or they are incompatible.").arg(itemA).arg(itemB),
                           QMessageBox::Warning);
    }
}
*/

void GeneralStore::identifySellItem()
{
    // 1. Get current selection from the dropdown
    QString itemName = identifySellItemComboBox->currentText();

    if (itemName.isEmpty()) {
        showFeedbackDialog("Input Required", "Please select an item from your inventory.", QMessageBox::Warning);
        return;
    }

    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    bool isIdentifyAction = (senderButton == idButton);

    GameStateManager* gsm = GameStateManager::instance();
    int activeCharIndex = gsm->getGameValue("ActiveCharacterIndex").toInt();

    // Fetch the character and their inventory
    Character current = gsm->getCurrentCharacter();
    QStringList inventory = current.inventory;

    // --- DEBUG: Before Action ---
    qDebug() << "--- [DEBUG] Inventory Before Action ---";
    qDebug() << "Character:" << current.name;
    qDebug() << "Target Item:" << itemName;
    qDebug() << "Current Items:" << inventory;

    // 2. Find the exact index
    int invIndex = inventory.indexOf(itemName);

    if (invIndex == -1) {
        showFeedbackDialog("Error", "Item not found in inventory.", QMessageBox::Warning);
        refreshInventory();
        return;
    }

    // 3. Fetch price from Database
    const QList<QVariantMap>& db = gsm->itemData();
    qulonglong basePrice = 0;
    for (const QVariantMap& item : db) {
        if (item["name"].toString().compare(itemName, Qt::CaseInsensitive) == 0) {
            basePrice = item["price"].toULongLong();
            break;
        }
    }

    if (isIdentifyAction) {
        // --- IDENTIFY LOGIC ---
        if (itemName.contains("(ID)")) {
            showFeedbackDialog("Info", "Already identified.", QMessageBox::Information);
            return;
        }

        qulonglong idCost = qMax(static_cast<qulonglong>(basePrice * 0.15), (qulonglong)10);
        if (current.gold < idCost) {
            showFeedbackDialog("Identify Failed", "Insufficient gold.", QMessageBox::Critical);
            return;
        }

        inventory[invIndex] = itemName + " (ID)";
        gsm->updateCharacterGold(activeCharIndex, idCost, false);
        gsm->setCharacterInventory(activeCharIndex, inventory); // SAVE UPDATE
    }
    else {
        // --- SELL LOGIC (REMOVAL) ---
        bool alreadyIdentified = itemName.contains("(ID)");
        qulonglong finalSellPrice = alreadyIdentified ? (basePrice / 2) : (basePrice / 4);

        // Remove the item from the local list
        inventory.removeAt(invIndex);
        
        // Push the updated list and gold back to the GameStateManager
        gsm->updateCharacterGold(activeCharIndex, finalSellPrice, true);
        gsm->setCharacterInventory(activeCharIndex, inventory); // SAVE UPDATE
        
        showFeedbackDialog("Sold", QString("Sold %1 for %2 gold.").arg(itemName).arg(finalSellPrice));
    }

    // --- DEBUG: After Action ---
    // Fetch directly from manager to prove it saved
    Character updatedChar = gsm->getCurrentCharacter();
    qDebug() << "--- [DEBUG] Inventory After Action ---";
    qDebug() << "Items:" << updatedChar.inventory;
    qDebug() << "Gold:" << updatedChar.gold;
    qDebug() << "--------------------------------------";

    // Update the UI dropdowns
    refreshInventory();
}
/*
void GeneralStore::identifySellItem()
{
    QString itemName = identifySellItemLineEdit->text().trimmed();
    if (itemName.isEmpty()) {
        showFeedbackDialog("Input Required", "Please enter the name of the item.", QMessageBox::Warning);
        return;
    }
    // Identify which button triggered the slot
    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    bool isIdentifyAction = (senderButton == idButton);
    GameStateManager* gsm = GameStateManager::instance();
    int activeCharIndex = gsm->getGameValue("ActiveCharacterIndex").toInt();
    // Retrieve character inventory
    QVariantList party = gsm->getGameValue("Party").toList();
    QVariantMap character = party[activeCharIndex].toMap();
    QStringList inventory = character["Inventory"].toStringList();
    // 1. Verify item is in Character Inventory
    int invIndex = -1;
    for (int i = 0; i < inventory.size(); ++i) {
        if (inventory[i].compare(itemName, Qt::CaseInsensitive) == 0) {
            invIndex = i;
            break;
        }
    }
    if (invIndex == -1) {
        showFeedbackDialog("Error", "This item is not in your inventory.", QMessageBox::Warning);
        return;
    }
    // 2. Fetch Item Data from Database
    bool alreadyIdentified = inventory[invIndex].contains("(ID)");
    const QList<QVariantMap>& db = gsm->itemData();
    qulonglong basePrice = 0;
    for (const QVariantMap& item : db) {
        if (item["name"].toString().compare(itemName, Qt::CaseInsensitive) == 0) {
            basePrice = item["price"].toULongLong();
            break;
        }
    }
    // 3. Branch Logic: Identify vs Sell
    if (isIdentifyAction) {
        // IDENTIFY LOGIC
        // Calculate dynamic ID cost (e.g., 15% of base price)
        qulonglong idCost = static_cast<qulonglong>(basePrice * 0.15);
        if (idCost < 10) idCost = 10; // Minimum flat fee for low-value items
        qulonglong currentGold = character["Gold"].toULongLong();
        if (alreadyIdentified) {
            showFeedbackDialog("Info", "This item is already identified.", QMessageBox::Information);
            return;
        }
        if (currentGold < (qulonglong)idCost) {
            showFeedbackDialog("Identify Failed", "You do not have enough gold to identify this item.", QMessageBox::Critical);
            return;
        }
        // Deduct gold and update name
        gsm->updateCharacterGold(activeCharIndex, idCost, false);
        inventory[invIndex] = itemName + " (ID)";
        gsm->setCharacterInventory(activeCharIndex, inventory);
        showFeedbackDialog("Identify Success", QString("You have identified %1!").arg(itemName));
    } else {
        // SELL LOGIC
        // Usually, selling unidentified items yields less gold (e.g., 25% value)
        qulonglong finalSellPrice = alreadyIdentified ? (basePrice / 2) : (basePrice / 4);
        inventory.removeAt(invIndex);
        gsm->setCharacterInventory(activeCharIndex, inventory);
        gsm->updateCharacterGold(activeCharIndex, finalSellPrice, true);
        showFeedbackDialog("Sold", QString("Sold %1 for %2 gold.").arg(itemName).arg(finalSellPrice));
    }
    identifySellItemLineEdit->clear();
}
*/

void GeneralStore::buyItem()
{
    if (buyItemsListWidget->currentItem()) {
        QString selectedItemName = buyItemsListWidget->currentItem()->text();
        QVariantMap itemMap = buyItemsListWidget->currentItem()->data(Qt::UserRole).toMap();
        QString priceString = itemMap.value("price").toString();
        // Placeholder Logic: Assume player has enough money.
        showFeedbackDialog("Purchase Confirmation",
                           QString("You have purchased the '%1' for %2 gold. It has been added to your inventory!").arg(selectedItemName).arg(priceString),
                           QMessageBox::Information);
    } else {
        showFeedbackDialog("Purchase Error", "Please select an item from the list to buy.", QMessageBox::Warning);
    }
    refreshInventory();
}

void GeneralStore::showItemInfo()
{
    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    QString infoMessage;
    QString itemName;
    if (senderButton == infoButton) {
        //itemName = identifySellItemLineEdit->text().trimmed();
        itemName = identifySellItemComboBox->currentText();
        if (itemName.isEmpty()) {
            showFeedbackDialog("Info Request Error", "Please enter an item name/ID to view its details.", QMessageBox::Warning);
            return;
        }
        infoMessage = QString("Displaying full known details for item '%1'.\n\n[Placeholder: All item stats, effects, and requirements would be listed here.]").arg(itemName);
    } else if (senderButton == buyInfoButton) {
        if (buyItemsListWidget->currentItem()) {
            itemName = buyItemsListWidget->currentItem()->text();
            QVariantMap itemMap = buyItemsListWidget->currentItem()->data(Qt::UserRole).toMap();
            // Generate a simple, formatted info string from the map
            infoMessage = QString("--- %1 Details ---\n").arg(itemName);
            for (auto it = itemMap.constBegin(); it != itemMap.constEnd(); ++it) {
                // Skip the name since it's in the title
                if (it.key() == "name") continue;
                infoMessage += QString("%1: %2\n").arg(it.key()).arg(it.value().toString());
            }
        } else {
            showFeedbackDialog("Info Request Error", "Please select an item from the list to view its details.", QMessageBox::Warning);
            return;
        }
    }
    if (!infoMessage.isEmpty()) {
        showFeedbackDialog(QString("%1 Information").arg(itemName), infoMessage, QMessageBox::NoIcon);
    }
}

void GeneralStore::searchItems(const QString &searchText)
{
    // The filtering logic remains the same as it doesn't need a dialog on every key press.
    QString lowerSearchText = searchText.toLower();
    for (int i = 0; i < buyItemsListWidget->count(); ++i) {
        QListWidgetItem *item = buyItemsListWidget->item(i);
        bool matchFound = false;
        // Check against the visible name first
        if (item->text().contains(searchText, Qt::CaseInsensitive)) {
            matchFound = true;
        } else {
            // Retrieve the QVariantMap
            QVariantMap itemMap = item->data(Qt::UserRole).toMap();
            // Iterate over all values in the map for a match
            for (const QVariant &value : itemMap.values()) {
                if (value.toString().toLower().contains(lowerSearchText)) {
                    matchFound = true;
                    break;
                }
            }
        }
        item->setHidden(!matchFound);
    }
}

void GeneralStore::loadItemsFromCsv(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "FILE OPEN FAIL:" << fileName;
        return;
    }

    QTextStream in(&file);
    QString headerLine = in.readLine();
    QStringList headers = headerLine.split(",");

    buyItemsListWidget->clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = line.split(",");
        
        // Use the smaller of the two counts to prevent index crashes
        int count = qMin(headers.size(), fields.size());
        
        QVariantMap itemData;
        for (int i = 0; i < count; ++i) {
            itemData.insert(headers[i].trimmed(), fields[i].trimmed());
        }

        // Check if we actually got a name
        QString name = itemData.value("name").toString();
        if (!name.isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem(name);
            item->setData(Qt::UserRole, itemData);
            buyItemsListWidget->addItem(item);
        }
    }
    file.close();
}

void GeneralStore::exitStore()
{
    this->close();
}

GeneralStore::~GeneralStore(){}
