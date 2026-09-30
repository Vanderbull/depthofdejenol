#include "BankDialog.h"
#include "TradeDialog.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QStandardItem>
// --- Helper Methods to Read Gold from GSM ---
long long BankDialog::getPlayerGold()
{
    // Party gold is the single source of truth for on-hand gold.
    return gameStateManager::instance()->getPartyGold();
}

long long BankDialog::getBankedGold()
{
    // Fetch and convert BankedGold from GSM (qulonglong)
    return gameStateManager::instance()->getGameValue("BankedGold").toULongLong();
}

int BankDialog::getFreeSlots()
{
    // Fetch and convert BankSlotsFree from GSM (int or QVariant conversion)
    // Assuming the key is "BankSlotsFree" and returns an int.
    return gameStateManager::instance()->getGameValue("BankSlotsFree").toInt();
}
// --- Constructor ---
BankDialog::BankDialog(QWidget *parent) :
    QDialog(parent)
{
    // Initialize item models
    itemModel = new QStandardItemModel(this); // Bank Vault Items
    playerItemModel = new QStandardItemModel(this); // Player Inventory Items
    // --- TEMPORARY: Ensure "BankSlotsFree" is initialized in GSM if not already. ---
    // In a real app, this should be done elsewhere, but for context completeness:
    if (!gameStateManager::instance()->getGameValue("BankSlotsFree").isValid()) {
         gameStateManager::instance()->setGameValue("BankSlotsFree", QVariant::fromValue(24));
    }
    // -------------------------------------------------------------------------------
    // 1. Create and position all widgets (Layout)
    setupUi(); 
    // 2. Connect signals to slots
    createConnections();
    // 3. Set initial data
    // Use the values retrieved from gameStateManager, including the new getFreeSlots()
    updateAccountStatus(getPlayerGold(), getBankedGold(), getFreeSlots());
}

void BankDialog::setupUi()
{
    setWindowTitle("Bank Deposit/Withdrawal");
    setMinimumSize(400, 500); 

    QGridLayout *mainLayout = new QGridLayout(this);
    // --- Row 0: Title ---
    QLabel *titleLabel = new QLabel("Welcome to the Bank!", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16pt; font-weight: bold; color: #4CAF50;");
    mainLayout->addWidget(titleLabel, 0, 0, 1, 3); 
    // --- Row 1: Status ---
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("font-weight: bold; color: darkblue;");
    mainLayout->addWidget(statusLabel, 1, 0, 1, 3);
    // --- Row 2-4: Deposit/Withdraw Gold (Left) ---
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
    // --- Row 2-7: Item List (Right) ---
    QLabel *itemListLabel = new QLabel("Items in Bank:", this);
    mainLayout->addWidget(itemListLabel, 2, 2, Qt::AlignLeft);
    itemListView = new QListView(this);
    itemListView->setModel(itemModel); // Bank Vault Items
    itemListView->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(itemListView, 3, 2, 5, 1); // Span multiple rows
    // --- Row 8-9: Party Options (Left) ---
    QLabel *partyOptionsLabel = new QLabel("Party Actions:", this);
    mainLayout->addWidget(partyOptionsLabel, 7, 0, 1, 2);
    // Horizontal layout for two side-by-side buttons
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
    // --- Row 9: Info and Exit (Right) ---
    QHBoxLayout *bottomRightLayout = new QHBoxLayout();
    infoButton = new QPushButton("Trade Items", this); 
    infoButton->setAutoDefault(false);
    exitButton = new QPushButton("Exit", this);
    exitButton->setAutoDefault(false);
    exitButton->setDefault(false);
    bottomRightLayout->addStretch(1); 
    bottomRightLayout->addWidget(infoButton);
    bottomRightLayout->addWidget(exitButton);
    mainLayout->addLayout(bottomRightLayout, 9, 2);
    setLayout(mainLayout);
}

void BankDialog::createConnections()
{
    // Gold/Amount Connections
    connect(depositAllButton, &QPushButton::clicked, this, &BankDialog::on_depositAllButton_clicked);
    connect(withdrawAllButton, &QPushButton::clicked, this, &BankDialog::on_withdrawAllButton_clicked);   
    // IMPORTANT: Connect editingFinished so that pressing Enter in the line edit triggers the transaction
    connect(depositLineEdit, &QLineEdit::editingFinished, this, 
            [this](){ updateDepositValue(depositLineEdit->text()); });
    connect(withdrawLineEdit, &QLineEdit::editingFinished, this, 
            [this](){ updateWithdrawValue(withdrawLineEdit->text()); });
    // Item List Connections
    connect(itemListView->selectionModel(), &QItemSelectionModel::selectionChanged, 
            this, &BankDialog::handleItemSelectionChanged);
    // Party/Action Connections
    connect(poolAndDepositButton, &QPushButton::clicked, this, &BankDialog::on_poolAndDepositButton_clicked);
    connect(partyDepositButton, &QPushButton::clicked, this, &BankDialog::on_partyDepositButton_clicked);
    connect(partyPoolAndDepositButton, &QPushButton::clicked, this, &BankDialog::on_partyPoolAndDepositButton_clicked);
    // Info and Exit Connections
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

void BankDialog::on_depositAllButton_clicked()
{
    // 1. Get the current party gold and bank balance
    long long partyGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    // 2. We are depositing EVERYTHING, so the amount is the party gold
    long long amount = partyGold;
    if (amount > 0) {
        // Update the Bank Total
        gameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(bankedGold + amount));
        // Deduct from party gold
        gameStateManager::instance()->spendPartyGold(static_cast<int>(amount));
        depositLineEdit->clear();
        QMessageBox::information(this, "Deposit All", QString("Deposited %L1 gold.").arg(amount));
        emit depositGold(amount);
        // Refresh the UI labels
        updateAccountStatus(getPlayerGold(), getBankedGold(), getFreeSlots());
    } else {
        QMessageBox::warning(this, "Deposit All", "The party has no gold to deposit.");
    }
}

void BankDialog::on_withdrawAllButton_clicked()
{
    // 1. Get values
    long long partyGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    // 2. We are withdrawing EVERYTHING from the bank
    long long amount = bankedGold;
    if (amount > 0) {
        // Update Bank Total to 0
        gameStateManager::instance()->setGameValue("BankedGold", 0);
        // Add to party gold
        gameStateManager::instance()->addPartyGold(static_cast<int>(amount));
        withdrawLineEdit->clear();
        QMessageBox::information(this, "Withdraw All", QString("Withdrew %L1 gold.").arg(amount));
        emit withdrawGold(amount);
        // Refresh the UI labels
        updateAccountStatus(getPlayerGold(), getBankedGold(), getFreeSlots());
    } else {
        QMessageBox::warning(this, "Withdraw All", "There is no gold in the bank to withdraw.");
    }
}

void BankDialog::on_poolAndDepositButton_clicked()
{
    // 1. Get current party gold and bank balance
    long long partyGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    int freeSlots = getFreeSlots();
    // 2. Check if there is anything to deposit
    if (partyGold > 0) {
        long long newBankBalance = bankedGold + partyGold;
        // 3. Deduct all party gold and increase bank balance
        gameStateManager::instance()->spendPartyGold(static_cast<int>(partyGold));
        gameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(newBankBalance));
        // 4. Sync the UI labels immediately
        updateAccountStatus(getPlayerGold(), newBankBalance, freeSlots);
        // 5. Provide feedback to the player
        QMessageBox::information(this, "Deposit Successful", 
            QString("Pooled all party gold. Successfully deposited %L1 gold.").arg(partyGold));
        // Emit signal if other parts of your app need to know
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
    // 1. Retrieve current party gold and bank balance.
    //    Party gold IS the single source of truth for on-hand gold.
    long long partyGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    // 2. Pool all party gold into the bank
    if (partyGold > 0) {
        // 2. Calculate the new bank total
        long long newBankBalance = bankedGold + partyGold;
        // 3. Spend all party gold and update bank
        gameStateManager::instance()->spendPartyGold(static_cast<int>(partyGold));
        gameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(newBankBalance));
        // 4. Update the UI
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
    tradeDialog.exec();
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
            // Successful deposit: deduct from party gold, add to bank
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
    long long partyGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    int freeSlots = getFreeSlots();
    withdrawLineEdit->clear();
    if (ok && amount > 0) {
        if (amount <= bankedGold) {
            // Successful withdrawal: deduct from bank, add to party gold
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
    // Logic for when the selection in the list changes
    QModelIndex index = itemListView->currentIndex();
    if (index.isValid()) {
        qDebug() << "Selected item:" << itemModel->data(index, Qt::DisplayRole).toString();
    }
} 

BankDialog::~BankDialog(){}