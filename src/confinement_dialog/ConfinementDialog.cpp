#include "ConfinementDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug>
#include <QFontDatabase>
#include <QSpacerItem>
#include <QRegularExpressionMatch>

ConfinementAndHoldingDialog::ConfinementAndHoldingDialog(QWidget *parent)
    : QDialog(parent)
{
    applyThemeAndWindowSettings();
    setupUi();
    initializeGameState();
    populateStockList();
    connectSignalsAndSlots();
}

void ConfinementAndHoldingDialog::applyThemeAndWindowSettings()
{
    setWindowTitle("Confinement & Holding");
    resize(850, 650);

    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fixedFont.setPointSize(10);
    setFont(fixedFont);
}

void ConfinementAndHoldingDialog::initializeGameState()
{
    gameStateManager* gsm = gameStateManager::instance();

    // Move pending dungeon exits into persistent stock
    if (gsm->getGameValue("GhostHoundPending").toBool()) {
        gsm->incrementStock("Ghost hound");
        gsm->setGameValue("GhostHoundPending", false);
    }
}

void ConfinementAndHoldingDialog::populateStockList()
{
    gameStateManager* gsm = gameStateManager::instance();
    buyCreatureListWidget->clear();

    QMap<QString, int> currentStock = gsm->getConfinementStock();
    for (auto it = currentStock.begin(); it != currentStock.end(); ++it) {
        buyCreatureListWidget->addItem(it.key());
    }
}

void ConfinementAndHoldingDialog::connectSignalsAndSlots()
{
    // Left Column Connections
    connect(bindButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::bindCompanion);
    connect(identifyInfoButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::identifyCompanion);
    connect(identifyGneButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::identifyCompanionGNE);
    connect(identifySellButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::sellCompanion);
    connect(identifyIdButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::realignCompanionID);

    // Right Column Connections
    connect(buyButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::buyCompanion);
    connect(buyInfoButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::showCompanionInfo);

    // Search & List Selection
    connect(searchLineEdit, &QLineEdit::textChanged, this, &ConfinementAndHoldingDialog::searchCompanion);
    connect(buyCreatureListWidget, &QListWidget::itemSelectionChanged, this, &ConfinementAndHoldingDialog::updateBuyFieldsFromList);

    // Exit
    connect(exitButton, &QPushButton::clicked, this, &QDialog::accept);
}

void ConfinementAndHoldingDialog::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(20);

    mainLayout->addWidget(createLeftColumn());
    mainLayout->addWidget(createRightColumn());
}

QWidget* ConfinementAndHoldingDialog::createLeftColumn()
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *leftColLayout = new QVBoxLayout(container);
    leftColLayout->setContentsMargins(0, 0, 0, 0);

    // --- Group Box: Bind Companions ---
    QGroupBox *bindGroup = new QGroupBox("Bind Companions", container);
    QVBoxLayout *bindLayout = new QVBoxLayout(bindGroup);
    bindCompLineEdit = new QLineEdit(bindGroup);
    bindCostLineEdit = new QLineEdit(bindGroup);
    bindButton = new QPushButton("Bind", bindGroup);

    bindLayout->addWidget(new QLabel("Comp.", bindGroup));
    bindLayout->addWidget(bindCompLineEdit);
    bindLayout->addWidget(new QLabel("Cost", bindGroup));
    bindLayout->addWidget(bindCostLineEdit);
    bindLayout->addWidget(bindButton);
    leftColLayout->addWidget(bindGroup);

    // --- Group Box: Identify, Realign & Sell Companions ---
    QGroupBox *identifyGroup = new QGroupBox("Identify, Realign & Sell Companions", container);
    QVBoxLayout *identifyLayout = new QVBoxLayout(identifyGroup);

    identifyCompLineEdit = new QLineEdit(identifyGroup);
    identifyValueLineEdit = new QLineEdit(identifyGroup);
    identifyValueLineEdit->setReadOnly(true);

    identifyLayout->addWidget(new QLabel("Comp.", identifyGroup));
    identifyLayout->addWidget(identifyCompLineEdit);
    identifyLayout->addWidget(new QLabel("Value", identifyGroup));
    identifyLayout->addWidget(identifyValueLineEdit);

    // Buttons layout
    QHBoxLayout *identifyButtonsLayout = new QHBoxLayout();
    identifyGneButton = new QPushButton("GNE", identifyGroup);
    identifyInfoButton = new QPushButton("Info", identifyGroup);
    identifySellButton = new QPushButton("Sell", identifyGroup);
    identifyIdButton = new QPushButton("ID", identifyGroup);

    int buttonWidth = 60;
    identifyGneButton->setFixedWidth(buttonWidth);
    identifyInfoButton->setFixedWidth(buttonWidth);
    identifySellButton->setFixedWidth(buttonWidth);
    identifyIdButton->setFixedWidth(buttonWidth);

    identifyButtonsLayout->addStretch();
    identifyButtonsLayout->addWidget(identifyGneButton);
    identifyButtonsLayout->addWidget(identifyInfoButton);
    identifyButtonsLayout->addWidget(identifySellButton);
    identifyButtonsLayout->addWidget(identifyIdButton);
    identifyButtonsLayout->addStretch();

    identifyLayout->addLayout(identifyButtonsLayout);
    identifyLayout->addWidget(new QLabel("ID Cost", identifyGroup));

    identifyIdCostLineEdit = new QLineEdit(identifyGroup);
    identifyLayout->addWidget(identifyIdCostLineEdit);

    leftColLayout->addWidget(identifyGroup);
    leftColLayout->addStretch();

    return container;
}

QWidget* ConfinementAndHoldingDialog::createRightColumn()
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *rightColLayout = new QVBoxLayout(container);
    rightColLayout->setContentsMargins(0, 0, 0, 0);

    QGroupBox *buyGroup = new QGroupBox("Buy Companions", container);
    QVBoxLayout *buyLayout = new QVBoxLayout(buyGroup);

    // G N E Header
    QHBoxLayout *gneHeaderLayout = new QHBoxLayout();
    gneHeaderLayout->addItem(new QSpacerItem(300, 1, QSizePolicy::Fixed, QSizePolicy::Minimum));
    gneLabel = new QLabel("G N E", buyGroup);
    QFont boldFont = gneLabel->font();
    boldFont.setBold(true);
    gneLabel->setFont(boldFont);
    gneHeaderLayout->addWidget(gneLabel);
    buyLayout->addLayout(gneHeaderLayout);

    // List & Fields
    buyCreatureListWidget = new QListWidget(buyGroup);
    buyCreatureListWidget->setFont(font());
    buyCompanionLineEdit = new QLineEdit(buyGroup);
    buyCompanionLineEdit->setReadOnly(true);
    buyCostLineEdit = new QLineEdit(buyGroup);
    buyCostLineEdit->setReadOnly(true);

    buyLayout->addWidget(buyCreatureListWidget);
    buyLayout->addWidget(new QLabel("Companion", buyGroup));
    buyLayout->addWidget(buyCompanionLineEdit);
    buyLayout->addWidget(new QLabel("Cost", buyGroup));
    buyLayout->addWidget(buyCostLineEdit);

    // Buy Buttons
    QHBoxLayout *buyButtonsLayout = new QHBoxLayout();
    buyButton = new QPushButton("BUY", buyGroup);
    buyButton->setEnabled(false);
    buyInfoButton = new QPushButton("INFO", buyGroup);
    buyInfoButton->setEnabled(false);

    buyButtonsLayout->addStretch();
    buyButtonsLayout->addWidget(buyButton);
    buyButtonsLayout->addWidget(buyInfoButton);
    buyLayout->addLayout(buyButtonsLayout);

    buyLayout->addWidget(new QLabel("Search for What?", buyGroup));
    searchLineEdit = new QLineEdit(buyGroup);
    buyLayout->addWidget(searchLineEdit);
    rightColLayout->addWidget(buyGroup);
    rightColLayout->addStretch();

    // Exit Button
    exitButton = new QPushButton("Exit", container);
    QHBoxLayout *exitButtonLayout = new QHBoxLayout();
    exitButtonLayout->addStretch();
    exitButtonLayout->addWidget(exitButton);
    rightColLayout->addLayout(exitButtonLayout);

    return container;
}

void ConfinementAndHoldingDialog::bindCompanion()
{
    QString name = bindCompLineEdit->text().trimmed();
    if (name.isEmpty()) return;

    gameStateManager* gsm = gameStateManager::instance();
    if (!gsm) return;

    // Cost (default 100 gold if the field is empty).
    int cost = 100;
    bool costOk = false;
    if (!bindCostLineEdit->text().trimmed().isEmpty()) {
        cost = bindCostLineEdit->text().trimmed().toInt(&costOk);
        if (!costOk || cost <= 0) cost = 100;
    }

    // Add a level-1 companion to the first party member.
    QList<Character>& members = gsm->getPartyMembers();
    if (members.isEmpty()) return;
    members[0].addCompanion(name, 1);

    // Charge the cost from the party.
    gsm->spendPartyGold(cost);

    populateStockList();
    QMessageBox::information(this, tr("Bind Companion"),
        tr("Bound %1 at a cost of %2 gold.").arg(name).arg(cost));
}

void ConfinementAndHoldingDialog::identifyCompanion()
{
    QString compName = identifyCompLineEdit->text().trimmed();
    if (compName.isEmpty()) return;

    gameStateManager* gsm = gameStateManager::instance();
    if (!gsm) return;

    // Identification fee (fixed, low).
    const int fee = 25;

    // Charge the identification fee.
    if (!gsm->spendPartyGold(fee)) {
        QMessageBox::warning(this, tr("Identify Companion"),
            tr("Not enough gold to identify %1.").arg(compName));
        return;
    }

    identifyValueLineEdit->setText(tr("Identified value: %1 gold.").arg(fee));
    QMessageBox::information(this, tr("Identify Companion"),
        tr("Identified %1 for a fee of %2 gold.").arg(compName).arg(fee));
}

void ConfinementAndHoldingDialog::identifyCompanionGNE()
{
    QMessageBox::information(this, "Identify G N E", "Calculating G N E for: " + identifyCompLineEdit->text());
}

void ConfinementAndHoldingDialog::sellCompanion() {
    QString compName = identifyCompLineEdit->text().trimmed();
    if (compName.isEmpty()) return;

    gameStateManager* gsm = gameStateManager::instance();
    if (!gsm) return;
    QList<Character>& members = gsm->getPartyMembers();

    for (int mi = 0; mi < members.size(); ++mi) {
        for (int i = 0; i < members[mi].companionCount(); ++i) {
            if (members[mi].companionName(i).toLower()
                    == compName.toLower()) {
                // v6.23.0: selling price based on companion level
                int basePrice = 100;
                int level = members[mi].companionLevel(i); // returns M4ED4
                int price = basePrice * level;
                // Optionally add bonus for abilities/resistances
                QString ability = members[mi].companionAbility(i);
                if (!ability.isEmpty()) {
                    price += 50; // example bonus
                }
                // Cap price to avoid insane values
                if (price > 10000) price = 10000;

                members[mi].clearCompanion(i);
                gsm->addGold(price);
                gsm->saveCharacterToFile(mi);
                populateStockList();
                QMessageBox::information(this, tr("Sell Companion"),
                                         tr("Sold companion %1. You received %2 gold.")
                                             .arg(compName).arg(price));
                return;
            }
        }
    }

    QMessageBox::warning(this, tr("Sell Companion"),
                       tr("Companion not found in your party."));
}

void ConfinementAndHoldingDialog::realignCompanionID()
{
    QMessageBox::information(this, "Realign ID", "Attempting to realign ID for: " + identifyCompLineEdit->text() + " at cost: " + identifyIdCostLineEdit->text());
}

void ConfinementAndHoldingDialog::buyCompanion()
{
    QString selectedCreature = buyCompanionLineEdit->text().trimmed();
    if (selectedCreature.isEmpty()) {
        return;
    }

    gameStateManager* gsm = gameStateManager::instance();
    if (!gsm) return;

    // Cost parsed from the buy list item (e.g. "Ghost hound 75000 ...").
    int cost = 100;
    bool costOk = false;
    if (!buyCostLineEdit->text().trimmed().isEmpty()) {
        cost = buyCostLineEdit->text().trimmed().toInt(&costOk);
        if (!costOk || cost <= 0) cost = 100;
    }

    // Charge the cost from the party.
    if (!gsm->spendPartyGold(cost)) {
        QMessageBox::warning(this, tr("Buy Companion"),
            tr("Not enough gold to buy %1.").arg(selectedCreature));
        return;
    }

    // Add the companion to the first party member.
    QList<Character>& members = gsm->getPartyMembers();
    if (!members.isEmpty()) {
        members[0].addCompanion(selectedCreature, 1);
    }

    // Deduct the stock entry.
    gsm->decrementStock(selectedCreature);

    // Remove the purchased item from the buy list.
    if (buyCreatureListWidget->currentItem()) {
        delete buyCreatureListWidget->currentItem();
    }
    buyCompanionLineEdit->clear();
    buyCostLineEdit->clear();
    buyButton->setEnabled(false);
    buyInfoButton->setEnabled(false);

    QMessageBox::information(this, tr("Buy Companion"),
        tr("Bought companion %1 for %2 gold.").arg(selectedCreature).arg(cost));
}

void ConfinementAndHoldingDialog::showCompanionInfo()
{
    QMessageBox::information(this, "Companion Info", "Showing info for companion: " + buyCompanionLineEdit->text());
    qDebug() << "Show Companion Info: " << buyCompanionLineEdit->text();
}

void ConfinementAndHoldingDialog::searchCompanion()
{
    QString searchText = searchLineEdit->text().trimmed();
    qDebug() << "Searching for: " << searchText;

    for (int i = 0; i < buyCreatureListWidget->count(); ++i) {
        QListWidgetItem *item = buyCreatureListWidget->item(i);
        bool matches = item->text().contains(searchText, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

void ConfinementAndHoldingDialog::updateBuyFieldsFromList()
{
    QList<QListWidgetItem*> selectedItems = buyCreatureListWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        buyCompanionLineEdit->clear();
        buyCostLineEdit->clear();
        buyButton->setEnabled(false);
        buyInfoButton->setEnabled(false);
        return;
    }

    QListWidgetItem *selectedItem = selectedItems.first();
    QString fullText = selectedItem->text().trimmed();

    QRegularExpression rx("^(.+?)\\s+(\\d+)\\s+");
    QRegularExpressionMatch match = rx.match(fullText);
    QString companionName;
    QString companionCost;

    if (match.hasMatch()) {
        companionName = match.captured(1).trimmed();
        companionCost = match.captured(2);
    } else {
        QStringList parts = fullText.split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            companionName = parts.first();
            companionCost = parts.at(1);
        } else {
            companionName = fullText;
            companionCost = "";
        }
    }

    buyCompanionLineEdit->setText(companionName);
    buyCostLineEdit->setText(companionCost);
    qDebug() << "Selected: " << companionName << ", Cost: " << companionCost;
    buyButton->setEnabled(true);
    buyInfoButton->setEnabled(true);
}

void ConfinementAndHoldingDialog::addGhostHoundOnExit()
{
    buyCreatureListWidget->addItem("Ghost hound 75000     0  2  0");
    qDebug() << "Character exited dungeon: Ghost Hound added to Confinement stock.";
}
