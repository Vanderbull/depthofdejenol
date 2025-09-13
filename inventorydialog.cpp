#include "inventorydialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug> // For debugging purposes

InventoryDialog::InventoryDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Inventory");
    setFixedSize(600, 400);
    setupUi();
}

InventoryDialog::~InventoryDialog() {}

void InventoryDialog::setupUi() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    QWidget *inventoryTab = new QWidget();
    QWidget *equippedTab = new QWidget();
    QWidget *spellsTab = new QWidget();

    tabWidget->addTab(inventoryTab, "Inventory");
    tabWidget->addTab(equippedTab, "Equipped");
    tabWidget->addTab(spellsTab, "Spells");

    // Layout for the main Inventory tab
    QHBoxLayout *inventoryLayout = new QHBoxLayout(inventoryTab);
    inventoryList = new QListWidget();
    inventoryLayout->addWidget(inventoryList);

    // Add example items to the Inventory tab
    inventoryList->addItem("Short Sword");
    inventoryList->addItem("Leather Armor");
    inventoryList->addItem("Health Potion (x3)");
    inventoryList->addItem("Gold (125)");

    // Layout for the Equipped tab
    QHBoxLayout *equippedLayout = new QHBoxLayout(equippedTab);
    equippedList = new QListWidget(); // Now a member variable
    equippedLayout->addWidget(equippedList);

    // Add example items to the Equipped tab
    equippedList->addItem("Battle Axe");
    equippedList->addItem("Plate Mail");
    equippedList->addItem("Shield of Fire Resistance");

    // Layout for the Spells tab
    QHBoxLayout *spellsLayout = new QHBoxLayout(spellsTab);
    spellsList = new QListWidget(); // Now a member variable
    spellsLayout->addWidget(spellsList);

    // Add example items to the Spells tab
    spellsList->addItem("Magic Missile");
    spellsList->addItem("Fireball");
    spellsList->addItem("Cure Light Wounds");

    // Right side: Buttons layout
    QVBoxLayout *buttonsLayout = new QVBoxLayout();
    buttonsLayout->setSpacing(5);

    equipButton = new QPushButton("Equip");
    useButton = new QPushButton("Use");
    dropButton = new QPushButton("Drop");
    infoButton = new QPushButton("Info");

    buttonsLayout->addWidget(equipButton);
    buttonsLayout->addWidget(useButton);
    buttonsLayout->addWidget(dropButton);
    buttonsLayout->addWidget(infoButton);

    mainLayout->addLayout(buttonsLayout);

    // Connect the equip button to the new slot
    connect(equipButton, &QPushButton::clicked, this, &InventoryDialog::onEquipButtonClicked);
}

void InventoryDialog::onEquipButtonClicked() {
    // Check if an item is selected in the inventory list
    if (inventoryList->currentItem()) {
        // Get the selected item
        QListWidgetItem *selectedItem = inventoryList->currentItem();

        // Check if the item is a single-use item (e.g., a potion) that can't be equipped
        if (selectedItem->text().contains("Potion") || selectedItem->text().contains("Gold")) {
            qWarning() << "Cannot equip this item.";
            return;
        }

        // Clone the item and add it to the equipped list
        QListWidgetItem *equippedItem = selectedItem->clone();
        equippedList->addItem(equippedItem);

        // Remove the item from the inventory list
        delete selectedItem;
    }
}
