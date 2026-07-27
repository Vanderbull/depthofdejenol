#include "GeneralStore.h"
#include <QDebug>

GeneralStore::GeneralStore(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("General Store");
    resize(850, 580);
    setupUi();
    setupStyling();
    loadItemsFromCsv("src/general_store/items.csv");
    populateShopItems();
    populatePlayerInventory();
    updateCharacterHeader();

    // Reward player 100 gold
    gameStateManager::instance()->addGold(1000);

    // Take 50 gold from member slot 1
    gameStateManager::instance()->addGold(-50, 1);

    // Spend shared party gold for purchases
    if (gameStateManager::instance()->spendPartyGold(250)) {
        // Purchase successful
    }

    gameStateManager::instance()->getCurrentCharacter();

    // --- Frame Timer Setup (~60 FPS update cycle) ---
    m_frameTimer = new QTimer(this);
    connect(m_frameTimer, &QTimer::timeout, this, &GeneralStore::updateFrame);
    m_frameTimer->start(16); // ~60 FPS (1000ms / 60 ≈ 16.6ms)
}

void GeneralStore::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    // --- Top Bar: Character Status & Gold ---
    QHBoxLayout *topHeaderLayout = new QHBoxLayout();
    m_charInfoLabel = new QLabel("Character: -", this);
    m_charInfoLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    m_goldLabel = new QLabel("Gold: 0 GP", this);
    m_goldLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #000000;");

    topHeaderLayout->addWidget(m_charInfoLabel);
    topHeaderLayout->addStretch();
    topHeaderLayout->addWidget(m_goldLabel);
    mainLayout->addLayout(topHeaderLayout);
    // --- Main Content Area: Shop (Left) vs Player Inventory (Right) ---
    QHBoxLayout *contentLayout = new QHBoxLayout();
    // Shop Section
    QVBoxLayout *shopLayout = new QVBoxLayout();
    QLabel *shopTitle = new QLabel("<b>Store Items</b>", this);
    m_shopTable = new QTableWidget(0, 4, this);
    m_shopTable->setHorizontalHeaderLabels({"Item", "Type", "Price", "Stats / Effect"});
    m_shopTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_shopTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_shopTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_shopTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    m_shopTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_shopTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_shopTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    shopLayout->addWidget(shopTitle);
    shopLayout->addWidget(m_shopTable);
    // Player Inventory Section
    QVBoxLayout *inventoryLayout = new QVBoxLayout();
    QLabel *inventoryTitle = new QLabel("<b>Your Inventory</b>", this);
    m_playerInventoryList = new QListWidget(this);
    //    m_playerInventoryList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_playerInventoryList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    inventoryLayout->addWidget(inventoryTitle);
    inventoryLayout->addWidget(m_playerInventoryList);

    contentLayout->addLayout(shopLayout, 3);
    contentLayout->addLayout(inventoryLayout, 2);
    mainLayout->addLayout(contentLayout, 3);
    // --- Item Details / Inspection Box ---
    m_itemDetailsText = new QTextEdit(this);
    m_itemDetailsText->setReadOnly(true);
    m_itemDetailsText->setMaximumHeight(80);
    m_itemDetailsText->setPlaceholderText("Select an item from store or inventory to inspect its properties...");
    mainLayout->addWidget(m_itemDetailsText);
    // --- Action Button Row ---
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_buyButton = new QPushButton("Buy Item", this);
    m_sellButton = new QPushButton("Sell Item", this);
    m_identifyButton = new QPushButton("Identify (50 GP)", this);
    m_uncurseButton = new QPushButton("Uncurse (100 GP)", this);
    m_combineButton = new QPushButton("Combine Items", this);

    buttonLayout->addWidget(m_buyButton);
    buttonLayout->addWidget(m_sellButton);
    buttonLayout->addWidget(m_identifyButton);
    buttonLayout->addWidget(m_uncurseButton);
    buttonLayout->addWidget(m_combineButton);
    mainLayout->addLayout(buttonLayout);

    // --- Signal Connections ---
    connect(m_shopTable, &QTableWidget::itemSelectionChanged, this, &GeneralStore::onShopSelectionChanged);
    connect(m_shopTable, &QTableWidget::cellDoubleClicked, this, &GeneralStore::buySelectedItem);

    connect(m_playerInventoryList, &QListWidget::itemSelectionChanged, this, &GeneralStore::onPlayerInventorySelectionChanged);
    connect(m_playerInventoryList, &QListWidget::itemDoubleClicked, this, &GeneralStore::sellSelectedItem);

    connect(m_buyButton, &QPushButton::clicked, this, &GeneralStore::buySelectedItem);
    connect(m_sellButton, &QPushButton::clicked, this, &GeneralStore::sellSelectedItem);
    connect(m_identifyButton, &QPushButton::clicked, this, &GeneralStore::identifySelectedItem);
    connect(m_uncurseButton, &QPushButton::clicked, this, &GeneralStore::uncurseSelectedItem);
    connect(m_combineButton, &QPushButton::clicked, this, &GeneralStore::combineSelectedItems);

    gameStateManager::instance()->getCurrentCharacter();
}

void GeneralStore::setupStyling()
{
    // Apply cohesive dark fantasy store styling
    setStyleSheet(
        "QDialog { background-color: #2B2B2B; color: #E0E0E0; }"
        "QTableWidget, QListWidget, QTextEdit { background-color: #1E1E1E; color: #DCDCDC; border: 1px solid #444; gridline-color: #333; }"
        "QHeaderView::section { background-color: #3C3F41; color: #FFFFFF; padding: 4px; border: 1px solid #444; }"
        "QPushButton { background-color: #3C3F41; color: #FFFFFF; border: 1px solid #555; padding: 6px 12px; border-radius: 3px; font-weight: bold; }"
        "QPushButton:hover { background-color: #4C5052; border-color: #FFD700; }"
        "QPushButton:disabled { background-color: #2A2A2A; color: #666; border-color: #333; }"
    );
}

void GeneralStore::updateFrame()
{
    // 1. Update the UI text elements with the latest game state values
    updateCharacterHeader();

    // Debug log to trace frame-by-frame updates
    Character current = gameStateManager::instance()->getCurrentCharacter();
    int sharedGold = gameStateManager::instance()->getPartyGold();

//    qDebug() << "[GeneralStore::updateFrame] Frame tick | Active Char:" 
//             << (current.name.isEmpty() ? "Hero" : current.name)
//             << "| Gold:" << current.gold 
//             << "| Party Gold:" << sharedGold;
}

void GeneralStore::updateCharacterHeader()
{
    Character current = gameStateManager::instance()->getCurrentCharacter();
    // Check party gold or single character gold dynamically
    int sharedGold = gameStateManager::instance()->getPartyGold();


    m_charInfoLabel->setText(QString("Hero: %1 (%2 Lvl %3)")
                                 .arg(current.name.isEmpty() ? "Hero" : current.name)
                                 .arg(current.race)
                                 .arg(current.level));
    // Show character individual gold and shared party gold
    m_goldLabel->setText(QString("Gold: %1 GP (Party: %2 GP)")
                             .arg(current.gold)
                             .arg(sharedGold));
}
void GeneralStore::loadItemsFromCsv(const QString& filePath)
{
    m_availableShopItems.clear();

    // 1. Try loading from global game state manager first
    QList<QVariantMap> globalItems = gameStateManager::instance()->itemData();
    if (!globalItems.isEmpty()) {
        m_availableShopItems = globalItems;
        return;
    }

    // 2. Read from CSV file directly
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV file:" << filePath << "- Loading fallback items.";
        
        m_availableShopItems = {
            {{"name", "Healing Potion"}, {"type", "Consumable"}, {"cost", 25}, {"desc", "Restores 20 HP."}, {"stats", "+20 HP"}},
            {{"name", "Iron Longsword"}, {"type", "Weapon"},     {"cost", 150}, {"desc", "Standard issue blade."}, {"stats", "+5 Atk"}},
            {{"name", "Leather Armor"},  {"type", "Armor"},      {"cost", 100}, {"desc", "Light protection."},     {"stats", "+2 AC"}},
            {{"name", "Rations"},        {"type", "Food"},       {"cost", 5},   {"desc", "Sustained food supply."}, {"stats", "None"}}
        };
        return;
    }

    QTextStream in(&file);

    auto parseCsvLine = [](const QString& line) -> QStringList {
        QStringList fields;
        QString currentField;
        bool inQuotes = false;

        for (int i = 0; i < line.length(); ++i) {
            QChar ch = line.at(i);
            if (ch == '"') {
                inQuotes = !inQuotes;
            } else if (ch == ',' && !inQuotes) {
                fields.append(currentField.trimmed());
                currentField.clear();
            } else {
                currentField.append(ch);
            }
        }
        fields.append(currentField.trimmed());
        return fields;
    };

    if (in.atEnd()) return;
    QString headerLine = in.readLine();
    QStringList headers = parseCsvLine(headerLine);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList values = parseCsvLine(line);
        QVariantMap itemMap;

        for (int i = 0; i < headers.size() && i < values.size(); ++i) {
            // Convert headers to lowercase to avoid case-sensitivity bugs ("Cost" vs "cost")
            QString key = headers[i].trimmed().toLower();
            QString val = values[i].trimmed();

            // Handle price / cost column variations
            if (key == "cost" || key == "price" || key == "gp" || key.contains("cost") || key.contains("price")) {
                // Strip non-digit characters (e.g. "150 GP" or "$150" -> "150")
                QString digitsOnly = val;
                digitsOnly.remove(QRegularExpression("[^0-9]"));

                bool ok;
                int costVal = digitsOnly.toInt(&ok);
                
                // Store under both "cost" and "price" to ensure compatibility
                itemMap["cost"] = ok ? costVal : 0;
                itemMap["price"] = ok ? costVal : 0;
            } else {
                itemMap[key] = val;
            }
        }

        if (!itemMap.isEmpty()) {
            m_availableShopItems.append(itemMap);
        }
    }

    file.close();
}
/*
void GeneralStore::loadItemsFromCsv(const QString& filePath)
{
    // Load item catalogue via gameStateManager standard item registry or local fallback
    QList<QVariantMap> globalItems = gameStateManager::instance()->itemData();
    if (!globalItems.isEmpty()) {
        m_availableShopItems = globalItems;
    } else {
        // Fallback default stock items if CSV is missing or loading dynamically
        m_availableShopItems = {
            {{"name", "Healing Potion"}, {"type", "Consumable"}, {"cost", 25}, {"desc", "Restores 20 HP."}},
            {{"name", "Iron Longsword"}, {"type", "Weapon"},     {"cost", 150}, {"desc", "Atk +5. Standard issue blade."}},
            {{"name", "Leather Armor"},  {"type", "Armor"},      {"cost", 100}, {"desc", "AC +2. Light protection."}},
            {{"name", "Rations"},        {"type", "Food"},       {"cost", 5},   {"desc", "Sustained food supply for dungeon travel."}}
        };
    }
}
*/
void GeneralStore::populateShopItems()
{
    m_shopTable->setRowCount(0);

    // Standard header keys to filter out from the stats display
    static const QSet<QString> nonStatKeys = {
        "name", "type", "cost", "price", "gp", "desc"
    };

    for (int i = 0; i < m_availableShopItems.size(); ++i) {
        const QVariantMap& item = m_availableShopItems[i];
        m_shopTable->insertRow(i);

        QString name = item.value("name").toString();
        QString type = item.value("type").toString();
        
        // Flexible price lookup
        int cost = 0;
        if (item.contains("cost")) {
            cost = item.value("cost").toInt();
        } else if (item.contains("price")) {
            cost = item.value("price").toInt();
        } else if (item.contains("gp")) {
            cost = item.value("gp").toInt();
        }

        // Dynamically collect all additional stats from the CSV
        QStringList statsList;
        for (auto it = item.constBegin(); it != item.constEnd(); ++it) {
            QString key = it.key();

            if (nonStatKeys.contains(key)) continue;

            QString val = it.value().toString().trimmed();
            if (val.isEmpty()) continue;

            // Capitalize key name for UI formatting (e.g., "atk" -> "Atk")
            QString formattedKey = key;
            if (!formattedKey.isEmpty()) {
                formattedKey[0] = formattedKey[0].toUpper();
            }

            statsList.append(QString("%1: %2").arg(formattedKey, val));
        }

        // Build summary string: show collected stats, or fall back to description if no extra stats exist
        QString statsDisplayStr = statsList.join(" | ");
        if (statsDisplayStr.isEmpty() && item.contains("desc")) {
            statsDisplayStr = item.value("desc").toString();
        }

        // Create table items
        QTableWidgetItem *nameItem  = new QTableWidgetItem(name);
        QTableWidgetItem *typeItem  = new QTableWidgetItem(type);
        QTableWidgetItem *costItem  = new QTableWidgetItem(QString("%1 GP").arg(cost));
        QTableWidgetItem *statsItem = new QTableWidgetItem(statsDisplayStr);

        costItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        // Store entire QVariantMap in UserRole on the primary item
        nameItem->setData(Qt::UserRole, item);

        m_shopTable->setItem(i, 0, nameItem);
        m_shopTable->setItem(i, 1, typeItem);
        m_shopTable->setItem(i, 2, costItem);
        m_shopTable->setItem(i, 3, statsItem);
    }
}
/*
void GeneralStore::populateShopItems()
{
    m_shopTable->setRowCount(0);
    for (int i = 0; i < m_availableShopItems.size(); ++i) {
        const QVariantMap& item = m_availableShopItems[i];
        m_shopTable->insertRow(i);

        QTableWidgetItem *nameItem = new QTableWidgetItem(item["name"].toString());
        QTableWidgetItem *typeItem = new QTableWidgetItem(item["type"].toString());
        QTableWidgetItem *costItem = new QTableWidgetItem(QString("%1 GP").arg(item["cost"].toInt()));

        m_shopTable->setItem(i, 0, nameItem);
        m_shopTable->setItem(i, 1, typeItem);
        m_shopTable->setItem(i, 2, costItem);
    }
}
*/
/*
void GeneralStore::populateShopItems()
{
    m_shopTable->setRowCount(0);

    for (int i = 0; i < m_availableShopItems.size(); ++i) {
        const QVariantMap& item = m_availableShopItems[i];
        m_shopTable->insertRow(i);

        QString name = item.value("name").toString();
        QString type = item.value("type").toString();
        int cost = item.value("cost").toInt();

        // Check for 'stats' key, falling back to 'desc' if 'stats' is empty
        QString statsStr = item.value("stats").toString();
        if (statsStr.isEmpty()) {
            statsStr = item.value("desc").toString();
        }

        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        QTableWidgetItem *typeItem = new QTableWidgetItem(type);
        QTableWidgetItem *costItem = new QTableWidgetItem(QString("%1 GP").arg(cost));
        QTableWidgetItem *statsItem = new QTableWidgetItem(statsStr);

        // Right-align the cost column
        costItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        // Store full item metadata into UserRole for easy retrieval on selection
        nameItem->setData(Qt::UserRole, item);

        m_shopTable->setItem(i, 0, nameItem);
        m_shopTable->setItem(i, 1, typeItem);
        m_shopTable->setItem(i, 2, costItem);
        m_shopTable->setItem(i, 3, statsItem);
    }
}
*/
void GeneralStore::populatePlayerInventory()
{
    m_playerInventoryList->clear();
    Character current = gameStateManager::instance()->getCurrentCharacter();
    for (const QString& itemName : current.inventory) {
        m_playerInventoryList->addItem(itemName);
    }
}
void GeneralStore::onShopSelectionChanged()
{
    int row = m_shopTable->currentRow();
    if (row < 0 || row >= m_shopTable->rowCount()) return;

    m_playerInventoryList->clearSelection();

    // Retrieve the item map stored in Qt::UserRole from column 0
    QTableWidgetItem *nameItemWidget = m_shopTable->item(row, 0);
    if (!nameItemWidget) return;

    QVariantMap item = nameItemWidget->data(Qt::UserRole).toMap();
    if (item.isEmpty()) return;

    // Standard headers to exclude from dynamic stat listing
    static const QSet<QString> headerKeys = {
        "name", "type", "cost", "price", "gp"
    };

    QString name = item.value("name").toString();
    QString type = item.value("type").toString();
    int cost = item.contains("cost") ? item.value("cost").toInt() : item.value("price").toInt();

    // Build html string for description and stats
    QString html = QString("<b>%1</b> (%2) — <b>Price: %3 GP</b><br>").arg(name, type).arg(cost);

    // Add description first if present
    if (item.contains("desc") && !item["desc"].toString().isEmpty()) {
        html += QString("<i>%1</i><br>").arg(item["desc"].toString());
    }

    // Collect all remaining stats from the CSV
    QStringList extraStats;
    for (auto it = item.constBegin(); it != item.constEnd(); ++it) {
        QString key = it.key();

        // Skip standard header fields and desc (handled above)
        if (headerKeys.contains(key) || key == "desc") continue;

        QString valueStr = it.value().toString().trimmed();
        if (valueStr.isEmpty()) continue;

        // Capitalize the stat key for clean display (e.g. "atk" -> "Atk")
        QString capitalizedKey = key;
        if (!capitalizedKey.isEmpty()) {
            capitalizedKey[0] = capitalizedKey[0].toUpper();
        }

        extraStats.append(QString("<b>%1:</b> %2").arg(capitalizedKey, valueStr));
    }

    if (!extraStats.isEmpty()) {
        html += "<br><b>Stats:</b> " + extraStats.join(" | ");
    }

    m_itemDetailsText->setText(html);
}
/*
void GeneralStore::onShopSelectionChanged()
{
    int row = m_shopTable->currentRow();
    if (row < 0 || row >= m_availableShopItems.size()) return;

    m_playerInventoryList->clearSelection();
    const QVariantMap& item = m_availableShopItems[row];

    m_itemDetailsText->setText(QString("<b>%1</b> (%2)<br>Price: %3 GP<br>%4")
                                   .arg(item["name"].toString())
                                   .arg(item["type"].toString())
                                   .arg(item["cost"].toInt())
                                   .arg(item["desc"].toString()));
}
*/

void GeneralStore::onPlayerInventorySelectionChanged()
{
    QListWidgetItem *selected = m_playerInventoryList->currentItem();
    if (!selected) return;

    m_shopTable->clearSelection();
    QString itemName = selected->text();
    QVariantMap stats = gameStateManager::instance()->getItemStats(itemName);

    if (!stats.isEmpty()) {
        m_itemDetailsText->setText(QString("<b>%1</b><br>Estimated Value: %2 GP")
                                       .arg(itemName)
                                       .arg(stats.value("cost", 10).toInt() / 2));
    } else {
        m_itemDetailsText->setText(QString("<b>%1</b><br>Standard inventory item.").arg(itemName));
    }
}

void GeneralStore::buySelectedItem()
{
    int row = m_shopTable->currentRow();
    if (row < 0 || row >= m_availableShopItems.size()) return;

    const QVariantMap& item = m_availableShopItems[row];
    int cost = 0;
    if (item.contains("cost")) {
        cost = item.value("cost").toInt();
    } else if (item.contains("price")) {
        cost = item.value("price").toInt();
    } else if (item.contains("gp")) {
        cost = item.value("gp").toInt();
    }

    QString itemName = item["name"].toString();

    Character current = gameStateManager::instance()->getCurrentCharacter();
    if (current.gold < static_cast<qulonglong>(cost)) {
        QMessageBox::warning(this, "Insufficient Gold", "You do not have enough gold to purchase this item.");
        return;
    }
    else
    {
        QMessageBox::warning(this, "sufficient Gold", QString("gold...\n\nIt is a %1!").arg(cost) );
    }
    int activeIdx = gameStateManager::instance()->getCurrentCharacterIndex();
    gameStateManager::instance()->updateCharacterGold(activeIdx, cost, false);
    gameStateManager::instance()->addItemToCharacter(activeIdx, itemName);

    updateCharacterHeader();
    populatePlayerInventory();
}

void GeneralStore::sellSelectedItem()
{
    QListWidgetItem *selected = m_playerInventoryList->currentItem();
    if (!selected) return;

    QString itemName = selected->text();
    int activeIdx = gameStateManager::instance()->getCurrentCharacterIndex();
    Character current = gameStateManager::instance()->getCurrentCharacter();

    int itemIdx = current.inventory.indexOf(itemName);
    if (itemIdx >= 0) {
        current.inventory.removeAt(itemIdx);
        gameStateManager::instance()->setCharacterInventory(activeIdx, current.inventory);

        int sellValue = gameStateManager::instance()->getItemStats(itemName).value("cost", 10).toInt() / 2;
        if (sellValue <= 0) sellValue = 5;

        gameStateManager::instance()->updateCharacterGold(activeIdx, sellValue, true);

        updateCharacterHeader();
        populatePlayerInventory();
        m_itemDetailsText->clear();
    }
}

void GeneralStore::identifySelectedItem()
{
    // 1. Ensure an item from the player's inventory is selected
    QListWidgetItem *selectedItem = m_playerInventoryList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "Identify Item", "Please select an item from your inventory to identify.");
        return;
    }
    QString rawItemName = selectedItem->text();
    int activeIdx = gameStateManager::instance()->getCurrentCharacterIndex();
    Character current = gameStateManager::instance()->getCurrentCharacter();
    // 2. Check if the item is already identified
    // (Assuming unidentified items follow a naming convention or meta-property)
    if (!rawItemName.startsWith("Unidentified") && !rawItemName.contains("?")) {
        QMessageBox::information(this, "Identify Item", "This item has already been identified!");
        return;
    }
    // 3. Gold Verification (e.g., standard fee of 50 GP)
    const int identifyFee = 50;
    if (current.gold < static_cast<qulonglong>(identifyFee)) {
        QMessageBox::warning(this, "Insufficient Gold", 
                             QString("You need %1 GP to identify an item. You only have %2 GP.")
                             .arg(identifyFee)
                             .arg(current.gold));
        return;
    }
    // 4. Resolve the true item identity
    // Lookup full stats/true name via gameStateManager or internal registry
    QVariantMap itemStats = gameStateManager::instance()->getItemStats(rawItemName);
    QString identifiedName = itemStats.value("trueName").toString();
    // Fallback resolution logic if trueName key isn't present in metadata
    if (identifiedName.isEmpty()) {
        identifiedName = rawItemName;
        identifiedName.remove("Unidentified ").remove("?");
    }
    // 5. Deduct Gold and Update Inventory List in Game State
    gameStateManager::instance()->updateCharacterGold(activeIdx, identifyFee, false /* subtract */);

    int itemIdx = current.inventory.indexOf(rawItemName);
    if (itemIdx >= 0) {
        current.inventory[itemIdx] = identifiedName;
        gameStateManager::instance()->setCharacterInventory(activeIdx, current.inventory);
    }
    // 6. Refresh UI Views & Show Outcome
    updateCharacterHeader();
    populatePlayerInventory();
    // Reselect the newly named item in the list
    QList<QListWidgetItem*> found = m_playerInventoryList->findItems(identifiedName, Qt::MatchExactly);
    if (!found.isEmpty()) {
        m_playerInventoryList->setCurrentItem(found.first());
    }
    // Update inspection pane and notify player
    m_itemDetailsText->setText(QString("<b>Identified: %1</b><br>%2")
                                   .arg(identifiedName)
                                   .arg(itemStats.value("desc", "The true nature of this item has been revealed.").toString()));

    QMessageBox::information(this, "Item Identified", 
                             QString("The shopkeeper inspects the item...\n\nIt is a %1!").arg(identifiedName));
}
void GeneralStore::uncurseSelectedItem()
{
    // 1. Ensure an item from the player's inventory is selected
    QListWidgetItem *selectedItem = m_playerInventoryList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "Uncurse Item", "Please select an item from your inventory to uncurse.");
        return;
    }
    QString rawItemName = selectedItem->text();
    int activeIdx = gameStateManager::instance()->getCurrentCharacterIndex();
    Character current = gameStateManager::instance()->getCurrentCharacter();
    // 2. Query item metadata to check for cursed status
    QVariantMap itemStats = gameStateManager::instance()->getItemStats(rawItemName);
    bool isCursed = itemStats.value("isCursed", false).toBool() || rawItemName.contains("Cursed", Qt::CaseInsensitive);

    if (!isCursed) {
        QMessageBox::information(this, "Uncurse Item", "This item does not appear to be cursed!");
        return;
    }
    // 3. Gold Verification ( standard uncurse fee of 100 GP)
    const int uncurseFee = 100;
    if (current.gold < static_cast<qulonglong>(uncurseFee)) {
        QMessageBox::warning(this, "Insufficient Gold", 
                             QString("The uncurse ritual costs %1 GP. You only have %2 GP.")
                             .arg(uncurseFee)
                             .arg(current.gold));
        return;
    }
    // 4. Resolve uncursed item name
    // Check if itemStats defines a clean name, otherwise strip "Cursed " prefix
    QString uncursedName = itemStats.value("uncursedName").toString();
    if (uncursedName.isEmpty()) {
        uncursedName = rawItemName;
        uncursedName.remove("Cursed ", Qt::CaseInsensitive).remove("(Cursed)", Qt::CaseInsensitive).trimmed();
    }
    // 5. Deduct Gold and Update Inventory in Game State
    gameStateManager::instance()->updateCharacterGold(activeIdx, uncurseFee, false /* subtract */);

    int itemIdx = current.inventory.indexOf(rawItemName);
    if (itemIdx >= 0) {
        current.inventory[itemIdx] = uncursedName;
        gameStateManager::instance()->setCharacterInventory(activeIdx, current.inventory);
    }
    // 6. Refresh UI Views & Show Outcome
    updateCharacterHeader();
    populatePlayerInventory();
    // Reselect the cleansed item in the inventory list
    QList<QListWidgetItem*> found = m_playerInventoryList->findItems(uncursedName, Qt::MatchExactly);
    if (!found.isEmpty()) {
        m_playerInventoryList->setCurrentItem(found.first());
    }
    // Update details pane and notify player
    m_itemDetailsText->setText(QString("<b>Cleansed: %1</b><br>The dark aura surrounding this item has dissipated.")
                                   .arg(uncursedName));

    QMessageBox::information(this, "Curse Lifted", 
                             QString("The shopkeeper recites an incantation...\n\nThe dark magic binding %1 has been lifted!")
                             .arg(rawItemName));
}
void GeneralStore::combineSelectedItems()
{
    // 1. Get all currently selected items from player inventory
    QList<QListWidgetItem*> selectedItems = m_playerInventoryList->selectedItems();
    // If fewer than 2 items are selected, inform the player
    if (selectedItems.size() < 2) {
        QMessageBox::information(this, "Combine Items",
            "Please select at least two items from your inventory to combine (hold Ctrl or Shift to select multiple).");
        return;
    }
    // 2. Extract item names
    QString item1Name = selectedItems[0]->text();
    QString item2Name = selectedItems[1]->text();

    int activeIdx = gameStateManager::instance()->getCurrentCharacterIndex();
    Character current = gameStateManager::instance()->getCurrentCharacter();
    // 3. Query recipe / combination result via gameStateManager
    // Checks for recipes (e.g. "Herb" + "Water" -> "Healing Potion")
    QString combinedResult = gameStateManager::instance()->getCraftingRecipeResult(item1Name, item2Name);
    // Fallback/Sample recipe check if no manager endpoint exists yet
    if (combinedResult.isEmpty()) {
        if ((item1Name.contains("Herb", Qt::CaseInsensitive) && item2Name.contains("Flask", Qt::CaseInsensitive)) ||
            (item2Name.contains("Herb", Qt::CaseInsensitive) && item1Name.contains("Flask", Qt::CaseInsensitive))) {
            combinedResult = "Healing Potion";
        } else if (item1Name.contains("Iron Ore", Qt::CaseInsensitive) && item2Name.contains("Iron Ore", Qt::CaseInsensitive)) {
            combinedResult = "Iron Bar";
        }
    }
    // 4. Handle invalid combinations
    if (combinedResult.isEmpty()) {
        QMessageBox::warning(this, "Combination Failed",
            QString("Combining '%1' and '%2' yields no usable result.").arg(item1Name, item2Name));
        return;
    }
    // 5. Remove ingredients from inventory
    int idx1 = current.inventory.indexOf(item1Name);
    if (idx1 >= 0) current.inventory.removeAt(idx1);

    int idx2 = current.inventory.indexOf(item2Name);
    if (idx2 >= 0) current.inventory.removeAt(idx2);
    // 6. Add new crafted item
    current.inventory.append(combinedResult);
    gameStateManager::instance()->setCharacterInventory(activeIdx, current.inventory);
    // 7. Refresh UI Views & Show Outcome
    populatePlayerInventory();
    // Highlight the new item in inventory
    QList<QListWidgetItem*> found = m_playerInventoryList->findItems(combinedResult, Qt::MatchExactly);
    if (!found.isEmpty()) {
        m_playerInventoryList->setCurrentItem(found.first());
    }
    // Update inspection panel and alert player
    m_itemDetailsText->setText(QString("<b>Crafted: %1</b><br>Successfully combined %2 and %3.")
                                   .arg(combinedResult, item1Name, item2Name));

    QMessageBox::information(this, "Crafting Success",
        QString("You combined %1 and %2 to create:\n\n✨ %3!")
        .arg(item1Name, item2Name, combinedResult));
}
