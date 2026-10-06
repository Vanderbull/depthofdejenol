#include "BankDialog.h"
#include "TradeDialog.h"
#include "src/items/ItemDatabase.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QStandardItem>

// --- Helper Methods to Read Gold from GSM ---
long long BankDialog::getPlayerGold()
{
    return gameStateManager::instance()->getPartyGold();
}

long long BankDialog::getBankedGold()
{
    return gameStateManager::instance()->getGameValue("BankedGold").toULongLong();
}

int BankDialog::getFreeSlots()
{
    return gameStateManager::instance()->getGameValue("BankSlotsFree").toInt();
}

// --- Constructor ---
BankDialog::BankDialog(QWidget *parent) :
    QDialog(parent)
{
    itemModel = new QStandardItemModel(this);
    playerItemModel = new QStandardItemModel(this);

    if (!gameStateManager::instance()->getGameValue("BankSlotsFree").isValid()) {
         gameStateManager::instance()->setGameValue("BankSlotsFree", QVariant::fromValue(24));
    }

    setupUi();
    createConnections();
    // Keep the account status in sync whenever gold values change in the GSM
    connect(gameStateManager::instance(), &gameStateManager::gameValueChanged,
            this, [this](const QString& key, const QVariant&) {
                if (key == "BankedGold" || key == "CurrentCharacterGold" || key == "PlayerGold") {
                    updateAccountStatus(getPlayerGold(), getBankedGold(), getFreeSlots());
                }
            });
    populatePlayerInventory();
    populateBankInventory();
    updateAccountStatus(getPlayerGold(), getBankedGold(), getFreeSlots());
}

void BankDialog::setupUi()
{
    setWindowTitle("Bank Deposit/Withdrawal");
    setMinimumSize(700, 520);

    QGridLayout *mainLayout = new QGridLayout(this);

    // Row 0: Title
    QLabel *titleLabel = new QLabel("Welcome to the Bank!", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16pt; font-weight: bold; color: #4CAF50;");
    mainLayout->addWidget(titleLabel, 0, 0, 1, 5);

    // Row 1: Status
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("font-weight: bold; color: darkblue;");
    mainLayout->addWidget(statusLabel, 1, 0, 1, 5);

    // Row 2-4: Deposit/Withdraw Gold (left)
    QLabel *depositLabel = new QLabel("Deposit Gold:", this);
    mainLayout->addWidget(depositLabel, 2, 0, Qt::AlignLeft);
    depositLineEdit = new QLineEdit(this);
    depositLineEdit->setPlaceholderText("Enter amount...");
    mainLayout->addWidget(depositLineEdit, 3, 0);
    depositAllButton = new QPushButton("All", this);
    depositAllButton->setAutoDefault(false);
    depositAllButton->setDefault(false);
    mainLayout->addWidget(depositAllButton, 3, 1);

    QLabel *withdrawLabel = new QLabel("Withdraw Gold:", this);
    mainLayout->addWidget(withdrawLabel, 4, 0, Qt::AlignLeft);
    withdrawLineEdit = new QLineEdit(this);
    withdrawLineEdit->setPlaceholderText("Enter amount...");
    mainLayout->addWidget(withdrawLineEdit, 5, 0);
    withdrawAllButton = new QPushButton("All", this);
    withdrawAllButton->setAutoDefault(false);
    withdrawAllButton->setDefault(false);
    mainLayout->addWidget(withdrawAllButton, 5, 1);

    // --- NEW: Item Transfer Section (rows 2-9, cols 2-3) ---
    QLabel *itemTransferLabel = new QLabel("Item Transfer:", this);
    itemTransferLabel->setStyleSheet("font-weight: bold; color: darkblue; font-size: 12pt;");
    mainLayout->addWidget(itemTransferLabel, 2, 2, 1, 2);

    // Player inventory list (left side of transfer)
    QLabel *playerInvLabel = new QLabel("Your Inventory", this);
    playerInvLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(playerInvLabel, 3, 2, 1, 1);
    playerListView = new QListView(this);
    playerListView->setModel(playerItemModel);
    playerListView->setSelectionMode(QAbstractItemView::SingleSelection);
    playerListView->setMinimumHeight(180);
    mainLayout->addWidget(playerListView, 4, 2, 4, 1);

    // Deposit / Withdraw buttons (middle)
    depositItemButton = new QPushButton(">> Deposit >>", this);
    depositItemButton->setAutoDefault(false);
    depositItemButton->setMinimumWidth(120);
    withdrawItemButton = new QPushButton("<< Withdraw <<", this);
    withdrawItemButton->setAutoDefault(false);
    withdrawItemButton->setMinimumWidth(120);

    QHBoxLayout *transferBtnLayout = new QHBoxLayout();
    transferBtnLayout->addStretch(1);
    transferBtnLayout->addWidget(depositItemButton);
    transferBtnLayout->addStretch(1);
    transferBtnLayout->addWidget(withdrawItemButton);
    transferBtnLayout->addStretch(1);
    mainLayout->addLayout(transferBtnLayout, 4, 3, 1, 1);

    // Bank vault list (right side of transfer)
    QLabel *bankInvLabel = new QLabel("Bank Vault", this);
    bankInvLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(bankInvLabel, 5, 3, 1, 1);
    itemListView = new QListView(this);
    itemListView->setModel(itemModel);
    itemListView->setSelectionMode(QAbstractItemView::SingleSelection);
    itemListView->setMinimumHeight(180);
    mainLayout->addWidget(itemListView, 6, 3, 4, 1);

    // Row 8-9: Party Options (left)
    QLabel *partyOptionsLabel = new QLabel("Party Actions:", this);
    mainLayout->addWidget(partyOptionsLabel, 7, 0, 1, 2);

    QHBoxLayout *partyHLayout1 = new QHBoxLayout();
    poolAndDepositButton = new QPushButton("Pool & Deposit", this);
    poolAndDepositButton->setAutoDefault(false);
    partyDepositButton = new QPushButton("Party Deposit", this);
    partyDepositButton->setAutoDefault(false);
    partyHLayout1->addWidget(poolAndDepositButton);
    partyHLayout1->addWidget(partyDepositButton);
    mainLayout->addLayout(partyHLayout1, 8, 0, 1, 2);

    partyPoolAndDepositButton = new QPushButton("Party Pool & Deposit", this);
    partyPoolAndDepositButton->setAutoDefault(false);
    mainLayout->addWidget(partyPoolAndDepositButton, 9, 0, 1, 2);

    // Row 9: Info and Exit (right)
    QHBoxLayout *bottomRightLayout = new QHBoxLayout();
    infoButton = new QPushButton("Trade Items", this);
    infoButton->setAutoDefault(false);
    exitButton = new QPushButton("Exit", this);
    exitButton->setAutoDefault(false);
    exitButton->setDefault(false);
    bottomRightLayout->addStretch(1);
    bottomRightLayout->addWidget(infoButton);
    bottomRightLayout->addWidget(exitButton);
    mainLayout->addLayout(bottomRightLayout, 9, 4);

    setLayout(mainLayout);
}

void BankDialog::createConnections()
{
    connect(depositAllButton, &QPushButton::clicked, this, &BankDialog::on_depositAllButton_clicked);
    connect(withdrawAllButton, &QPushButton::clicked, this, &BankDialog::on_withdrawAllButton_clicked);
    connect(depositLineEdit, &QLineEdit::editingFinished, this,
            [this](){ updateDepositValue(depositLineEdit->text()); });
    connect(withdrawLineEdit, &QLineEdit::editingFinished, this,
            [this](){ updateWithdrawValue(withdrawLineEdit->text()); });

    // Item transfer connections
    connect(depositItemButton, &QPushButton::clicked, this, &BankDialog::on_depositItemButton_clicked);
    connect(withdrawItemButton, &QPushButton::clicked, this, &BankDialog::on_withdrawItemButton_clicked);
    connect(itemListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &BankDialog::handleItemSelectionChanged);

    connect(poolAndDepositButton, &QPushButton::clicked, this, &BankDialog::on_poolAndDepositButton_clicked);
    connect(partyDepositButton, &QPushButton::clicked, this, &BankDialog::on_partyDepositButton_clicked);
    connect(partyPoolAndDepositButton, &QPushButton::clicked, this, &BankDialog::on_partyPoolAndDepositButton_clicked);
    connect(infoButton, &QPushButton::clicked, this, &BankDialog::on_infoButton_clicked);
    connect(exitButton, &QPushButton::clicked, this, &BankDialog::on_exitButton_clicked);
}

void BankDialog::updateAccountStatus(long long playerGold, long long bankGold, int freeSlots)
{
    QString statusText = QString("Your Wallet: **%L1 gold** | Bank Vault: **%L2 gold** | Slots Free: **%3**")
                             .arg(playerGold)
                             .arg(bankGold)
                             .arg(freeSlots);
    statusLabel->setText(statusText);
}

void BankDialog::populatePlayerInventory()
{
    playerItemModel->clear();
    gameStateManager* gsm = gameStateManager::instance();
    int activeIdx = gsm->getGameValue("ActiveCharacterIndex").toInt();
    QVariantList party = gsm->getGameValue("Party").toList();
    if (activeIdx >= 0 && activeIdx < party.size()) {
        QVariantMap character = party[activeIdx].toMap();
        if (character.contains("Inventory")) {
            QStringList inventory = character["Inventory"].toStringList();
            for (const QString& item : inventory) {
                playerItemModel->appendRow(new QStandardItem(item));
            }
        }
    }
}

void BankDialog::populateBankInventory()
{
    itemModel->clear();
    QList<HeldItem> bankItems = gameStateManager::instance()->getBankInventory();
    for (const HeldItem& item : bankItems) {
        itemModel->appendRow(new QStandardItem(item.name));
    }
}

void BankDialog::on_depositAllButton_clicked()
{
    long long partyGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    long long amount = partyGold;
    if (amount > 0) {
        gameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(bankedGold + amount));
        gameStateManager::instance()->spendPartyGold(static_cast<int>(amount));
        depositLineEdit->clear();
        QMessageBox::information(this, "Deposit All", QString("Deposited %L1 gold.").arg(amount));
        emit depositGold(amount);
        updateAccountStatus(getPlayerGold(), getBankedGold(), getFreeSlots());
    } else {
        QMessageBox::warning(this, "Deposit All", "The party has no gold to deposit.");
    }
}

void BankDialog::on_withdrawAllButton_clicked()
{
    long long bankedGold = getBankedGold();
    long long amount = bankedGold;
    if (amount > 0) {
        gameStateManager::instance()->setGameValue("BankedGold", 0);
        gameStateManager::instance()->addPartyGold(static_cast<int>(amount));
        withdrawLineEdit->clear();
        QMessageBox::information(this, "Withdraw All", QString("Withdrew %L1 gold.").arg(amount));
        emit withdrawGold(amount);
        updateAccountStatus(getPlayerGold(), getBankedGold(), getFreeSlots());
    } else {
        QMessageBox::warning(this, "Withdraw All", "There is no gold in the bank to withdraw.");
    }
}

void BankDialog::on_poolAndDepositButton_clicked()
{
    long long partyGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    int freeSlots = getFreeSlots();
    if (partyGold > 0) {
        long long newBankBalance = bankedGold + partyGold;
        gameStateManager::instance()->spendPartyGold(static_cast<int>(partyGold));
        gameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(newBankBalance));
        updateAccountStatus(getPlayerGold(), newBankBalance, freeSlots);
        QMessageBox::information(this, "Deposit Successful",
            QString("Pooled all party gold. Successfully deposited %L1 gold.").arg(partyGold));
        emit depositGold(partyGold);
    } else {
        QMessageBox::warning(this, "Deposit Failed", "The party has no gold to pool.");
    }
}

void BankDialog::on_partyDepositButton_clicked()
{
    QMessageBox::information(this, "Action", "Depositing only Party-tagged items.");
}

void BankDialog::on_partyPoolAndDepositButton_clicked()
{
    long long partyGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    if (partyGold > 0) {
        long long newBankBalance = bankedGold + partyGold;
        gameStateManager::instance()->spendPartyGold(static_cast<int>(partyGold));
        gameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(newBankBalance));
        updateAccountStatus(getPlayerGold(), newBankBalance, getFreeSlots());
        QMessageBox::information(this, "Party Deposit",
            QString("Pooled and deposited %L1 gold.").arg(partyGold));
    } else {
        QMessageBox::warning(this, "Party Deposit", "The party has no gold to pool.");
    }
}

void BankDialog::on_exitButton_clicked()
{
    reject();
}

void BankDialog::on_infoButton_clicked()
{
    TradeDialog tradeDialog(playerItemModel, itemModel, this);
    if (tradeDialog.exec() == QDialog::Accepted) {
        populatePlayerInventory();
        populateBankInventory();
        updateAccountStatus(getPlayerGold(), getBankedGold(), getFreeSlots());
    }
}

void BankDialog::updateDepositValue(const QString &text)
{
    bool ok;
    long long amount = text.toLongLong(&ok);
    long long partyGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    int freeSlots = getFreeSlots();
    depositLineEdit->clear();
    if (ok && amount > 0) {
        if (amount <= partyGold) {
            gameStateManager::instance()->spendPartyGold(static_cast<int>(amount));
            gameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(bankedGold + amount));
            QMessageBox::information(this, "Deposit Complete", QString("Successfully deposited %L1 gold.").arg(amount));
            emit depositGold(amount);
        } else {
            QMessageBox::warning(this, "Deposit Failed", "The party does not have that much gold.");
        }
        updateAccountStatus(getPlayerGold(), getBankedGold(), freeSlots);
    } else if (!text.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid positive number for the deposit amount.");
    }
}

void BankDialog::updateWithdrawValue(const QString &text)
{
    bool ok;
    long long amount = text.toLongLong(&ok);
    long long bankedGold = getBankedGold();
    int freeSlots = getFreeSlots();
    withdrawLineEdit->clear();
    if (ok && amount > 0) {
        if (amount <= bankedGold) {
            gameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(bankedGold - amount));
            gameStateManager::instance()->addPartyGold(static_cast<int>(amount));
            QMessageBox::information(this, "Withdrawal Complete", QString("Successfully withdrew %L1 gold.").arg(amount));
            emit withdrawGold(amount);
        } else {
            QMessageBox::warning(this, "Withdrawal Failed", "The bank does not hold that much gold.");
        }
        updateAccountStatus(getPlayerGold(), getBankedGold(), freeSlots);
    } else if (!text.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid positive number for the withdrawal amount.");
    }
}

void BankDialog::handleItemSelectionChanged()
{
    QModelIndex index = itemListView->currentIndex();
    if (index.isValid()) {
        qDebug() << "Selected bank item:" << itemModel->data(index, Qt::DisplayRole).toString();
    }
}

// --- NEW: Item deposit ---
void BankDialog::on_depositItemButton_clicked()
{
    if (!playerListView || !playerItemModel || !itemModel) return;
    QModelIndex index = playerListView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Deposit Item", "Please select an item in your inventory to deposit.");
        return;
    }
    QStandardItem *item = playerItemModel->takeItem(index.row());
    if (!item) return;
    itemModel->appendRow(item);

    // Persist: write updated bank inventory to GSM
    QList<HeldItem> bankItems;
    for (int i = 0; i < itemModel->rowCount(); ++i) {
        QStandardItem *bi = itemModel->item(i);
        if (bi) {
            HeldItem hi;
            hi.name = bi->text();
            if (const ItemDef* def = ItemDatabase::instance().byName(hi.name)) {
                hi.M4E97 = static_cast<int16_t>(def->id);
            }
            bankItems << hi;
        }
    }
    gameStateManager::instance()->setBankInventory(bankItems);

    // Persist: write updated player inventory back to GSM
    QList<HeldItem> playerItems;
    for (int i = 0; i < playerItemModel->rowCount(); ++i) {
        QStandardItem *pi = playerItemModel->item(i);
        if (pi) {
            HeldItem hi;
            hi.name = pi->text();
            if (const ItemDef* def = ItemDatabase::instance().byName(hi.name)) {
                hi.M4E97 = static_cast<int16_t>(def->id);
            }
            playerItems << hi;
        }
    }
    int activeIdx = gameStateManager::instance()->getGameValue("ActiveCharacterIndex").toInt();
    gameStateManager::instance()->setCharacterInventory(activeIdx, playerItems);

    QString name = item->text();
    delete item;
    emit itemDeposited(name);
    QMessageBox::information(this, "Item Deposited", QString("Deposited '%1' to bank.").arg(name));
}

// --- NEW: Item withdraw ---
void BankDialog::on_withdrawItemButton_clicked()
{
    if (!itemListView || !itemModel || !playerItemModel) return;
    QModelIndex index = itemListView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Withdraw Item", "Please select an item in the bank to withdraw.");
        return;
    }
    QStandardItem *item = itemModel->takeItem(index.row());
    if (!item) return;
    playerItemModel->appendRow(item);

    // Persist
    QList<HeldItem> bankItems;
    for (int i = 0; i < itemModel->rowCount(); ++i) {
        QStandardItem *bi = itemModel->item(i);
        if (bi) {
            HeldItem hi;
            hi.name = bi->text();
            if (const ItemDef* def = ItemDatabase::instance().byName(hi.name)) {
                hi.M4E97 = static_cast<int16_t>(def->id);
            }
            bankItems << hi;
        }
    }
    gameStateManager::instance()->setBankInventory(bankItems);

    QList<HeldItem> playerItems;
    for (int i = 0; i < playerItemModel->rowCount(); ++i) {
        QStandardItem *pi = playerItemModel->item(i);
        if (pi) {
            HeldItem hi;
            hi.name = pi->text();
            if (const ItemDef* def = ItemDatabase::instance().byName(hi.name)) {
                hi.M4E97 = static_cast<int16_t>(def->id);
            }
            playerItems << hi;
        }
    }
    int activeIdx = gameStateManager::instance()->getGameValue("ActiveCharacterIndex").toInt();
    gameStateManager::instance()->setCharacterInventory(activeIdx, playerItems);

    QString name = item->text();
    delete item;
    emit itemWithdrawn(name);
    QMessageBox::information(this, "Item Withdrawn", QString("Withdrew '%1' from bank.").arg(name));
}

BankDialog::~BankDialog()
{
}