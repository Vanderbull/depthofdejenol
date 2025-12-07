// BankDialog.cpp
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
    // Fetch and convert PlayerGold from GSM (qulonglong)
    return GameStateManager::instance()->getGameValue("PlayerGold").toULongLong();
}

long long BankDialog::getBankedGold()
{
    // Fetch and convert BankedGold from GSM (qulonglong)
    return GameStateManager::instance()->getGameValue("BankedGold").toULongLong();
}

// --- NEW Helper Methods to Read FreeSlots from GSM ---
int BankDialog::getFreeSlots()
{
    // Fetch and convert BankSlotsFree from GSM (int or QVariant conversion)
    // Assuming the key is "BankSlotsFree" and returns an int.
    return GameStateManager::instance()->getGameValue("BankSlotsFree").toInt();
}
// ---------------------------------------------

// --- Constructor ---
BankDialog::BankDialog(QWidget *parent) :
    QDialog(parent)
    // REMOVED: currentGold and bankedGold local variables
    // REMOVED: freeSlots(24)      // Example data
{
    // Initialize item models
    itemModel = new QStandardItemModel(this); // Bank Vault Items
    playerItemModel = new QStandardItemModel(this); // Player Inventory Items

    // --- TEMPORARY: Ensure "BankSlotsFree" is initialized in GSM if not already. ---
    // In a real app, this should be done elsewhere, but for context completeness:
    if (!GameStateManager::instance()->getGameValue("BankSlotsFree").isValid()) {
         GameStateManager::instance()->setGameValue("BankSlotsFree", QVariant::fromValue(24));
    }
    // -------------------------------------------------------------------------------

    // 1. Create and position all widgets (Layout)
    setupUi(); 

    // 2. Connect signals to slots
    createConnections();

    // 3. Set initial data
    // Use the values retrieved from GameStateManager, including the new getFreeSlots()
    updateAccountStatus(getPlayerGold(), getBankedGold(), getFreeSlots());

    // Populate item list with example data (These items start in the bank)
    QStringList initialBankItems;
    initialBankItems << "Sword of Alton (g)" << "Prism of Sanctuary [1]" << "Potion of Agility [2]"
                 << "Bells of Kwalish [6]" << "Silver Cross (g)" << "Twisted Bracers (n) [2]"
                 << "Tome of Agility (n) [1]" << "War Hammer (g)";

    for (const QString& item : initialBankItems) {
        itemModel->appendRow(new QStandardItem(item));
    }

    // Populate player inventory with example data (These items start with the player)
    QStringList initialPlayerItems;
    initialPlayerItems << "Health Potion [5]" << "Mana Scroll [3]" << "Rusty Sword" 
                       << "Leather Armor" << "Magic Ring";
    for (const QString& item : initialPlayerItems) {
        playerItemModel->appendRow(new QStandardItem(item));
    }
}

// --- Destructor (REQUIRED for Linker Fix) ---
BankDialog::~BankDialog()
{
    // Qt's parent-child hierarchy handles the deletion of widgets
}

// --- Setup Methods (REQUIRED for Linker Fix) ---
// ... (setupUi remains the same)

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

// --- Private Logic Methods ---

void BankDialog::updateAccountStatus(long long playerGold, long long bankGold, int freeSlots)
{
    QString statusText = QString("Your Wallet: **%L1 gold** | Bank Vault: **%L2 gold** | Slots Free: **%3**")
                             .arg(playerGold)
                             .arg(bankGold)
                             .arg(freeSlots);
    statusLabel->setText(statusText); 
}

// --- Slot Implementations ---

void BankDialog::on_depositAllButton_clicked()
{
    long long playerGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    int freeSlots = getFreeSlots(); // Use GSM value
    
    // Deposit all player gold into the bank
    if (playerGold > 0) {
        long long amountToDeposit = playerGold; 

        // Update GSM: Increase BankedGold by playerGold, set PlayerGold to 0
        GameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(bankedGold + playerGold));
        // Use qulonglong for explicit type safety with large numbers
        GameStateManager::instance()->setGameValue("PlayerGold", QVariant::fromValue((qulonglong)0)); 
        
        // Use GSM value in the update call
        updateAccountStatus(0, bankedGold + playerGold, freeSlots); 
        depositLineEdit->clear();
        emit depositGold(amountToDeposit); 
        QMessageBox::information(this, "Deposit", "Deposited ALL your gold into the bank.");
    } else {
        QMessageBox::warning(this, "Deposit", "You have no gold to deposit.");
    }
}

void BankDialog::on_withdrawAllButton_clicked()
{
    long long playerGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    int freeSlots = getFreeSlots(); // Use GSM value
    
    // Withdraw all bank gold into the player's inventory
    if (bankedGold > 0) {
        long long amountToWithdraw = bankedGold;
        
        // Update GSM: Increase PlayerGold by bankedGold, set BankedGold to 0
        GameStateManager::instance()->setGameValue("PlayerGold", QVariant::fromValue(playerGold + bankedGold));
        // Use qulonglong for explicit type safety with large numbers
        GameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue((qulonglong)0));
        
        // Use GSM value in the update call
        updateAccountStatus(playerGold + bankedGold, 0, freeSlots);
        withdrawLineEdit->clear();
        emit withdrawGold(amountToWithdraw); 
        QMessageBox::information(this, "Withdraw", "Withdrew ALL gold from the bank.");
    } else {
        QMessageBox::warning(this, "Withdraw", "The bank vault is empty.");
    }
}

void BankDialog::on_poolAndDepositButton_clicked()
{
    QMessageBox::information(this, "Action", "Pooling Party Gold and Items for Deposit.");
}

void BankDialog::on_partyDepositButton_clicked()
{
    QMessageBox::information(this, "Action", "Depositing only Party-tagged items.");
}

void BankDialog::on_partyPoolAndDepositButton_clicked()
{
    QMessageBox::information(this, "Action", "Pooling and Depositing all Party assets.");
}

void BankDialog::on_exitButton_clicked()
{
    // Use the standard reject() slot to close the dialog
    reject();
}

void BankDialog::on_infoButton_clicked()
{
    // Launch the Item Trade Dialog
    TradeDialog tradeDialog(playerItemModel, itemModel, this);
    tradeDialog.exec();
}

void BankDialog::updateDepositValue(const QString &text)
{
    bool ok;
    long long amount = text.toLongLong(&ok);
    long long playerGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    int freeSlots = getFreeSlots(); // Use GSM value
    
    depositLineEdit->clear(); // Clear the field after transaction attempt

    if (ok && amount > 0) {
        if (amount <= playerGold) {
            // Successful deposit
            // Update GSM: Decrease PlayerGold, Increase BankedGold
            GameStateManager::instance()->setGameValue("PlayerGold", QVariant::fromValue(playerGold - amount));
            GameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(bankedGold + amount));
            
            QMessageBox::information(this, "Deposit Complete", QString("Successfully deposited %L1 gold.").arg(amount));
            emit depositGold(amount);
        } else {
            // Not enough gold
            QMessageBox::warning(this, "Deposit Failed", "You do not have that much gold in your wallet.");
        }
        // Update status with new values from GSM, including freeSlots
        updateAccountStatus(getPlayerGold(), getBankedGold(), freeSlots);
    } else if (!text.isEmpty()) {
        // Invalid input
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid positive number for the deposit amount.");
    }
}

void BankDialog::updateWithdrawValue(const QString &text)
{
    bool ok;
    long long amount = text.toLongLong(&ok);
    long long playerGold = getPlayerGold();
    long long bankedGold = getBankedGold();
    int freeSlots = getFreeSlots(); // Use GSM value
    
    withdrawLineEdit->clear(); // Clear the field after transaction attempt

    if (ok && amount > 0) {
        if (amount <= bankedGold) {
            // Successful withdrawal
            // Update GSM: Decrease BankedGold, Increase PlayerGold
            GameStateManager::instance()->setGameValue("BankedGold", QVariant::fromValue(bankedGold - amount));
            GameStateManager::instance()->setGameValue("PlayerGold", QVariant::fromValue(playerGold + amount));
            
            QMessageBox::information(this, "Withdrawal Complete", QString("Successfully withdrew %L1 gold.").arg(amount));
            emit withdrawGold(amount);
        } else {
            // Not enough gold in the bank
            QMessageBox::warning(this, "Withdrawal Failed", "The bank does not hold that much gold.");
        }
        // Update status with new values from GSM, including freeSlots
        updateAccountStatus(getPlayerGold(), getBankedGold(), freeSlots);
    } else if (!text.isEmpty()) {
        // Invalid input
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
