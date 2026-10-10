#include "inventorydialog.h"
#include "src/items/ItemDatabase.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>

InventoryDialog::InventoryDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Inventory");
    setFixedSize(700, 500);
    initializeItemData();
    setupUi();
    loadInventoryData();

    connect(gameStateManager::instance(), &gameStateManager::gameValueChanged,
            this, [this](const QString& key, const QVariant& /*value*/){
        if (key == "party_data") {
            loadInventoryData();
        }
    });
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
    unequipButton = new QPushButton("Unequip");
    useButton = new QPushButton("Use");
    dropButton = new QPushButton("Drop");
    infoButton = new QPushButton("Info");
    identifyButton = new QPushButton("Identify");
    buttonsLayout->addWidget(equipButton);
    buttonsLayout->addWidget(unequipButton);
    buttonsLayout->addWidget(useButton);
    buttonsLayout->addWidget(dropButton);
    buttonsLayout->addWidget(infoButton);
    buttonsLayout->addWidget(identifyButton);

    // Effective stats panel
    effectiveStatsLabel = new QLabel(this);
    effectiveStatsLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    effectiveStatsLabel->setMinimumHeight(100);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(new QLabel("<b>Effective Stats</b>", this));
    buttonsLayout->addWidget(effectiveStatsLabel);

    buttonsLayout->addStretch();
    mainLayout->addLayout(buttonsLayout);

    // Connect logic
    connect(equipButton, &QPushButton::clicked, this, &InventoryDialog::onEquipButtonClicked);
    connect(unequipButton, &QPushButton::clicked, this, &InventoryDialog::onUnequipButtonClicked);
    connect(useButton, &QPushButton::clicked, this, &InventoryDialog::onUseButtonClicked);
    connect(dropButton, &QPushButton::clicked, this, &InventoryDialog::onDropButtonClicked);
    connect(infoButton, &QPushButton::clicked, this, &InventoryDialog::onInfoButtonClicked);
    connect(identifyButton, &QPushButton::clicked, this, &InventoryDialog::onIdentifyButtonClicked);
}

void InventoryDialog::loadInventoryData() {
    inventoryList->clear();
    equippedList->clear();

    Character current = gameStateManager::instance()->getCurrentCharacter();

    for (const HeldItem& item : current.inventory) {
        QString display = item.identified ? item.name : "Unknown " + item.name;
        inventoryList->addItem(display);
    }

    for (const HeldItem& item : current.equipped) {
        equippedList->addItem(item.name);
    }

    updateEffectiveStatsPanel();
}

void InventoryDialog::updateEffectiveStatsPanel() {
    Character c = gameStateManager::instance()->getCurrentCharacter();
    QString text = QString(
        "STR: %1 (base %2)\n"
        "INT: %3 (base %4)\n"
        "WIS: %5 (base %6)\n"
        "CON: %7 (base %8)\n"
        "CHA: %9 (base %10)\n"
        "DEX: %11 (base %12)"
    ).arg(c.effectiveStrength()).arg(c.strength)
     .arg(c.effectiveIntelligence()).arg(c.intelligence)
     .arg(c.effectiveWisdom()).arg(c.wisdom)
     .arg(c.effectiveConstitution()).arg(c.constitution)
     .arg(c.effectiveCharisma()).arg(c.charisma)
     .arg(c.effectiveDexterity()).arg(c.dexterity);
    effectiveStatsLabel->setText(text);
}

QString InventoryDialog::itemTooltip(const QString& itemName) const
{
    const ItemDef* def = ItemDatabase::instance().byName(itemName);
    if (!def) return itemName;

    QString tip = QString("<b>%1</b><br>").arg(def->name);
    tip += QString("Type: %1<br>").arg(def->typeName());
    tip += QString("Slot: %1<br>").arg(ItemSlot::name(def->slot()));
    tip += QString("ATT: %1  DEF: %2<br>").arg(def->att).arg(def->def);
    tip += QString("Price: %1 GP<br>").arg(def->price);

    if (def->strMod != 0) tip += QString("STR %+1<br>").arg(def->strMod);
    if (def->intMod != 0) tip += QString("INT %+1<br>").arg(def->intMod);
    if (def->wisMod != 0) tip += QString("WIS %+1<br>").arg(def->wisMod);
    if (def->conMod != 0) tip += QString("CON %+1<br>").arg(def->conMod);
    if (def->chaMod != 0) tip += QString("CHA %+1<br>").arg(def->chaMod);
    if (def->dexMod != 0) tip += QString("DEX %+1<br>").arg(def->dexMod);

    if (def->strReq > 0) tip += QString("Requires STR %1<br>").arg(def->strReq);
    if (def->intReq > 0) tip += QString("Requires INT %1<br>").arg(def->intReq);
    if (def->wisReq > 0) tip += QString("Requires WIS %1<br>").arg(def->wisReq);
    if (def->conReq > 0) tip += QString("Requires CON %1<br>").arg(def->conReq);
    if (def->chaReq > 0) tip += QString("Requires CHA %1<br>").arg(def->chaReq);
    if (def->dexReq > 0) tip += QString("Requires DEX %1<br>").arg(def->dexReq);

    if (def->cursed) tip += "<b><font color='red'>CURSED</font></b><br>";
    if (def->nHands == 2) tip += "Two-handed<br>";

    return tip;
}

void InventoryDialog::initializeItemData()
{
    itemInfoMap["Short Sword"] = "A standard short sword.\nStats: 5 Damage";
    itemInfoMap["Leather Armor"] = "Light leather armor.\nStats: 10 Defense";
}

void InventoryDialog::onEquipButtonClicked()
{
    if (!inventoryList->currentItem()) return;

    int activeIdx = gameStateManager::instance()->getCurrentCharacterIndex();
    int invRow = inventoryList->currentRow();
    QString reason;
    if (gameStateManager::instance()->equipItem(activeIdx, invRow, reason)) {
        loadInventoryData();
    } else {
        QMessageBox::warning(this, "Cannot Equip", reason);
    }
}

void InventoryDialog::onUnequipButtonClicked()
{
    if (!equippedList->currentItem()) return;

    int activeIdx = gameStateManager::instance()->getCurrentCharacterIndex();
    int slotRow = equippedList->currentRow();
    QString reason;
    if (gameStateManager::instance()->unequipItem(activeIdx, slotRow, reason)) {
        loadInventoryData();
    } else {
        QMessageBox::warning(this, "Cannot Unequip", reason);
    }
}

void InventoryDialog::onDropButtonClicked()
{
    int currentIndex = tabWidget->currentIndex();
    // Only the inventory tab holds droppable items; equipped items must be
    // unequipped first (and cursed ones cannot be removed at all).
    if (currentIndex != 0) {
        QMessageBox::information(this, "Drop", "Unequip the item before dropping it.");
        return;
    }
    if (!inventoryList->currentItem()) return;

    gameStateManager* gsm = gameStateManager::instance();
    int activeIdx = gsm->getCurrentCharacterIndex();
    int invRow = inventoryList->currentRow();
    QString reason;
    if (gsm->removeItemFromInventory(activeIdx, invRow, reason)) {
        loadInventoryData();
    } else {
        QMessageBox::warning(this, "Cannot Drop", reason);
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
        gameStateManager* gsm = gameStateManager::instance();
        const QList<QVariantMap>& allItems = gsm->itemData();
        QVariantMap foundItem;
        bool itemFound = false;
        for (const QVariantMap& item : allItems) {
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
                if (i.key() == "DataType" || i.value().toString().isEmpty()) continue;
                details += QString("%1: %2\n").arg(i.key()).arg(i.value().toString());
            }
            QMessageBox::information(this, itemName, details);
        } else {
            QString fallbackInfo = itemInfoMap.value(itemName, "No detailed stats found in database.");
            QMessageBox::information(this, itemName, fallbackInfo);
        }
    }
}

void InventoryDialog::onUseButtonClicked()
{
    if (!inventoryList->currentItem()) return;

    int activeIdx = gameStateManager::instance()->getCurrentCharacterIndex();
    int invRow = inventoryList->currentRow();
    QString effect;
    if (gameStateManager::instance()->useConsumable(activeIdx, invRow, effect)) {
        QMessageBox::information(this, "Use Item", effect);
        loadInventoryData();
    } else {
        QMessageBox::warning(this, "Cannot Use", effect);
    }
}

void InventoryDialog::onIdentifyButtonClicked()
{
    if (!inventoryList->currentItem()) return;

    int activeIdx = gameStateManager::instance()->getCurrentCharacterIndex();
    int invRow = inventoryList->currentRow();

    auto& members = gameStateManager::instance()->getPartyMembers();
    if (activeIdx < 0 || activeIdx >= members.size()) return;
    if (invRow < 0 || invRow >= members[activeIdx].inventory.size()) return;

    HeldItem& item = members[activeIdx].inventory[invRow];
    if (item.identified) {
        QMessageBox::information(this, "Identify", QString("%1 is already identified.").arg(item.name));
        return;
    }

    // Identification cost: 10 GP per item.
    const int identifyCost = 10;
    int partyGold = gameStateManager::instance()->getPartyGold();
    if (partyGold < identifyCost) {
        QMessageBox::warning(this, "Cannot Identify",
            QString("Identifying an item costs %1 gold, but the party only has %2.")
            .arg(identifyCost).arg(partyGold));
        return;
    }

    gameStateManager::instance()->spendPartyGold(identifyCost);
    item.identified = true;

    QMessageBox::information(this, "Identify",
        QString("You identify the %1. (-%2 gold)").arg(item.name).arg(identifyCost));
    loadInventoryData();
}

InventoryDialog::~InventoryDialog() {}
