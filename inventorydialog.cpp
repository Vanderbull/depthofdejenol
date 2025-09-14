#include "inventorydialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>

InventoryDialog::InventoryDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Inventory");
    setFixedSize(600, 400);
    initializeItemData();
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

    QHBoxLayout *inventoryLayout = new QHBoxLayout(inventoryTab);
    inventoryList = new QListWidget();
    inventoryLayout->addWidget(inventoryList);

    inventoryList->addItem("Short Sword");
    inventoryList->addItem("Leather Armor");
    inventoryList->addItem("Health Potion (x3)");
    inventoryList->addItem("Gold (125)");

    QHBoxLayout *equippedLayout = new QHBoxLayout(equippedTab);
    equippedList = new QListWidget();
    equippedLayout->addWidget(equippedList);

    equippedList->addItem("Battle Axe");
    equippedList->addItem("Plate Mail");
    equippedList->addItem("Shield of Fire Resistance");

    QHBoxLayout *spellsLayout = new QHBoxLayout(spellsTab);
    spellsList = new QListWidget();
    spellsLayout->addWidget(spellsList);

    spellsList->addItem("Magic Missile");
    spellsList->addItem("Fireball");
    spellsList->addItem("Cure Light Wounds");

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

    connect(equipButton, &QPushButton::clicked, this, &InventoryDialog::onEquipButtonClicked);
    connect(dropButton, &QPushButton::clicked, this, &InventoryDialog::onDropButtonClicked);
    connect(infoButton, &QPushButton::clicked, this, &InventoryDialog::onInfoButtonClicked); // Connect the new info button slot
}

void InventoryDialog::initializeItemData() {
    itemInfoMap["Short Sword"] = "A standard short sword.\nStats: 5 Damage, 1 Weight";
    itemInfoMap["Leather Armor"] = "Light leather armor.\nStats: 10 Defense, 5 Weight";
    itemInfoMap["Health Potion (x3)"] = "Restores 50 HP.\nType: Potion";
    itemInfoMap["Gold (125)"] = "A pouch with 125 gold pieces.";
    itemInfoMap["Battle Axe"] = "A heavy battle axe.\nStats: 12 Damage, 10 Weight";
    itemInfoMap["Plate Mail"] = "Heavy full plate armor.\nStats: 30 Defense, 25 Weight";
    itemInfoMap["Shield of Fire Resistance"] = "A shield that protects against fire.\nStats: 15 Defense, 50% Fire Resistance";
    itemInfoMap["Magic Missile"] = "A basic damaging spell.\nStats: 1-4 Damage";
    itemInfoMap["Fireball"] = "An area of effect spell.\nStats: 10-20 Damage, Area of Effect";
    itemInfoMap["Cure Light Wounds"] = "Restores 10 HP.\nType: Healing Spell";
}

void InventoryDialog::onEquipButtonClicked() {
    if (inventoryList->currentItem()) {
        QListWidgetItem *selectedItem = inventoryList->currentItem();
        if (selectedItem->text().contains("Potion") || selectedItem->text().contains("Gold")) {
            qWarning() << "Cannot equip this item.";
            return;
        }

        QListWidgetItem *equippedItem = selectedItem->clone();
        equippedList->addItem(equippedItem);
        delete selectedItem;
    }
}

void InventoryDialog::onDropButtonClicked() {
    int currentIndex = tabWidget->currentIndex();

    if (currentIndex == 2) {
        qWarning() << "Cannot drop spells.";
        return;
    }

    QListWidget* currentList = nullptr;
    if (currentIndex == 0) {
        currentList = inventoryList;
    } else if (currentIndex == 1) {
        currentList = equippedList;
    }

    if (currentList && currentList->currentItem()) {
        QListWidgetItem *selectedItem = currentList->currentItem();
        delete selectedItem;
        qDebug() << "Item dropped.";
    }
}

void InventoryDialog::onInfoButtonClicked() {
    QListWidget* currentList = nullptr;
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex == 0) {
        currentList = inventoryList;
    } else if (currentIndex == 1) {
        currentList = equippedList;
    } else if (currentIndex == 2) {
        currentList = spellsList;
    }

    if (currentList && currentList->currentItem()) {
        QString itemName = currentList->currentItem()->text();
        QString itemInfo = itemInfoMap.value(itemName, "No information available for this item.");

        QMessageBox msgBox;
        msgBox.setWindowTitle(itemName);
        msgBox.setText(itemInfo);
        msgBox.exec();
    }
}
