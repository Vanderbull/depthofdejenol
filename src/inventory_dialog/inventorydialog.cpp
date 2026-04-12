#include "inventorydialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>

InventoryDialog::InventoryDialog(QWidget *parent) : QDialog(parent) 
{
    setWindowTitle("Inventory");
    setFixedSize(600, 400);
    initializeItemData(); // Loads your itemInfoMap descriptions
    setupUi();
    loadInventoryData(); // Pulls the real data from GameStateManager
}

void InventoryDialog::setupUi() 
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    // Create Tabs
    QWidget *inventoryTab = new QWidget();
    QWidget *equippedTab = new QWidget();
    QWidget *spellsTab = new QWidget();
    tabWidget->addTab(inventoryTab, "Inventory");
    tabWidget->addTab(equippedTab, "Equipped");
    tabWidget->addTab(spellsTab, "Spells");
    // Layouts for Lists
    QHBoxLayout *inventoryLayout = new QHBoxLayout(inventoryTab);
    inventoryList = new QListWidget();
    inventoryLayout->addWidget(inventoryList);
    QHBoxLayout *equippedLayout = new QHBoxLayout(equippedTab);
    equippedList = new QListWidget();
    equippedLayout->addWidget(equippedList);
    QHBoxLayout *spellsLayout = new QHBoxLayout(spellsTab);
    spellsList = new QListWidget();
    spellsLayout->addWidget(spellsList);
    // Sidebar Buttons
    QVBoxLayout *buttonsLayout = new QVBoxLayout();
    equipButton = new QPushButton("Equip");
    useButton = new QPushButton("Use");
    dropButton = new QPushButton("Drop");
    infoButton = new QPushButton("Info");
    buttonsLayout->addWidget(equipButton);
    buttonsLayout->addWidget(useButton);
    buttonsLayout->addWidget(dropButton);
    buttonsLayout->addWidget(infoButton);
    mainLayout->addLayout(buttonsLayout);
    // Connect logic
    connect(equipButton, &QPushButton::clicked, this, &InventoryDialog::onEquipButtonClicked);
    connect(dropButton, &QPushButton::clicked, this, &InventoryDialog::onDropButtonClicked);
    connect(infoButton, &QPushButton::clicked, this, &InventoryDialog::onInfoButtonClicked);
}

void InventoryDialog::loadInventoryData() 
{
    // 1. Get the singleton instance
    GameStateManager* gsm = GameStateManager::instance();    
    // 2. Identify which character to look at
    int activeIdx = gsm->getGameValue("ActiveCharacterIndex").toInt();
    QVariantList party = gsm->getGameValue("Party").toList();
    if (activeIdx >= 0 && activeIdx < party.size()) {
        QVariantMap character = party[activeIdx].toMap();
        // 3. Clear and Populate the Inventory List
        inventoryList->clear();
        QVariantList inventory = character["Inventory"].toList(); 
        for (const QVariant& item : inventory) {
            inventoryList->addItem(item.toString());
        }
        // 4. Repeat for Equipped and Spells
        equippedList->clear();
        QVariantList equipped = character["Equipped"].toList();
        for (const QVariant& item : equipped) {
            equippedList->addItem(item.toString());
        }
    }
}

void InventoryDialog::initializeItemData() 
{
    itemInfoMap["Short Sword"] = "A standard short sword.\nStats: 5 Damage";
    itemInfoMap["Leather Armor"] = "Light leather armor.\nStats: 10 Defense";
    // Add other treasure items here to provide descriptions
}

void InventoryDialog::onEquipButtonClicked() 
{
    if (inventoryList->currentItem()) {
        QListWidgetItem *selectedItem = inventoryList->currentItem();        
        // Example logic: Don't equip consumables
        if (selectedItem->text().contains("Potion") || selectedItem->text().contains("Gold")) {
            return;
        }
        // Add to equipped list and remove from inventory
        equippedList->addItem(selectedItem->text());
        delete selectedItem;
        qDebug() << "Item equipped locally.";
        // Note: In a full implementation, you would also update GameStateManager here
    }
}

void InventoryDialog::onDropButtonClicked() 
{
    int currentIndex = tabWidget->currentIndex();
    QListWidget* currentList = nullptr;
    if (currentIndex == 0) currentList = inventoryList;
    else if (currentIndex == 1) currentList = equippedList;
    if (currentList && currentList->currentItem()) {
        delete currentList->currentItem();
        qDebug() << "Item removed from UI.";
    }
}

void InventoryDialog::onInfoButtonClicked() 
{
    QListWidget* currentList = nullptr;
    int currentIndex = tabWidget->currentIndex();    
    if (currentIndex == 0) currentList = inventoryList;
    else if (currentIndex == 1) currentList = equippedList;
    else if (currentIndex == 2) currentList = spellsList;
    if (currentList && currentList->currentItem()) {
        QString itemName = currentList->currentItem()->text();
        GameStateManager* gsm = GameStateManager::instance();
        // Retrieve the full item list from GameStateManager
        const QList<QVariantMap>& allItems = gsm->itemData();
        QVariantMap foundItem;
        bool itemFound = false;
        // Search for the item by name in the database
        for (const QVariantMap& item : allItems) {
            // Adjust "name" key if your CSV header uses a different case (e.g., "Name")
            if (item.value("name").toString() == itemName) {
                foundItem = item;
                itemFound = true;
                break;
            }
        }
        if (itemFound) {
            QString details = "--- Item Statistics ---\n";
            QMapIterator<QString, QVariant> i(foundItem);
            while (i.hasNext()) {
                i.next();
                // Skip the internal DataType or empty values to keep it clean
                if (i.key() == "DataType" || i.value().toString().isEmpty()) continue;
                
                details += QString("%1: %2\n").arg(i.key()).arg(i.value().toString());
            }
            QMessageBox::information(this, itemName, details);
        } else {
            // Fallback to the local map if not found in the CSV database
            QString fallbackInfo = itemInfoMap.value(itemName, "No detailed stats found in database.");
            QMessageBox::information(this, itemName, fallbackInfo);
        }
    }
}

InventoryDialog::~InventoryDialog() {}