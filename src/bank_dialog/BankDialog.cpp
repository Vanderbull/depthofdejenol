#include "BankDialog.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QStandardItem>

// --- Constructor ---
BankDialog::BankDialog(QWidget *parent) :
    QDialog(parent),
    currentGold(1000), // Player's gold, set to 1000 for demonstration.
    bankedGold(0),     // **Bank Gold is now 0 as requested.**
    freeSlots(24)      // Example data
{
    // 1. Create and position all widgets (Layout)
    setupUi(); 

    // 2. Connect signals to slots
    createConnections();

    // 3. Set initial data
    // Updated to pass both player gold and banked gold
    updateAccountStatus(currentGold, bankedGold, freeSlots);

    // Populate item list with example data
    QStringList initialItems;
    initialItems << "Sword of Alton (g)" << "Prism of Sanctuary [1]" << "Potion of Agility [2]"
                 << "Bells of Kwalish [6]" << "Silver Cross (g)" << "Twisted Bracers (n) [2]"
                 << "Tome of Agility (n) [1]" << "War Hammer (g)";

    for (const QString& item : initialItems) {
        itemModel->appendRow(new QStandardItem(item));
    }
}

// --- Destructor ---
BankDialog::~BankDialog()
{
    // Note: Qt's parent-child hierarchy handles the deletion of widgets 
    // that have 'this' (the dialog) as a parent.
}

// --- Setup Methods ---

void BankDialog::setupUi()
{
    setWindowTitle("Bank Deposit/Withdrawal");
    setMinimumSize(400, 500); // Set a reasonable default size

    // Main layout uses QGridLayout for structured positioning
    QGridLayout *mainLayout = new QGridLayout(this);
    
    // --- Row 0: Title ---
    QLabel *titleLabel = new QLabel("Welcome to the Bank!", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16pt; font-weight: bold; color: #4CAF50;");
    mainLayout->addWidget(titleLabel, 0, 0, 1, 3); // Spanning all columns

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
    // Explicitly prevent this button from becoming the default button
    depositAllButton->setAutoDefault(false); 
    depositAllButton->setDefault(false);
    mainLayout->addWidget(depositAllButton, 3, 1);

    QLabel *withdrawLabel = new QLabel("Withdraw Gold:", this);
    mainLayout->addWidget(withdrawLabel, 4, 0, Qt::AlignLeft);

    withdrawLineEdit = new QLineEdit(this);
    withdrawLineEdit->setPlaceholderText("Enter amount...");
    mainLayout->addWidget(withdrawLineEdit, 5, 0);

    withdrawAllButton = new QPushButton("All", this);
    // Explicitly prevent this button from becoming the default button
    withdrawAllButton->setAutoDefault(false);
    withdrawAllButton->setDefault(false);
    mainLayout->addWidget(withdrawAllButton, 5, 1);

    // --- Row 2-7: Item List (Right) ---
    QLabel *itemListLabel = new QLabel("Items in Bank:", this);
    mainLayout->addWidget(itemListLabel, 2, 2, Qt::AlignLeft);
    
    itemListView = new QListView(this);
    itemModel = new QStandardItemModel(this);
    itemListView->setModel(itemModel);
    itemListView->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(itemListView, 3, 2, 5, 1); // Span multiple rows

    // --- Row 8-9: Party Options (Left) ---
    QLabel *partyOptionsLabel = new QLabel("Party Actions:", this);
    mainLayout->addWidget(partyOptionsLabel, 7, 0, 1, 2);

    // Horizontal layout for two side-by-side buttons
    QHBoxLayout *partyHLayout1 = new QHBoxLayout();
    poolAndDepositButton = new QPushButton("Pool & Deposit", this);
    poolAndDepositButton->setAutoDefault(false); // Also set non-default for other buttons
    partyDepositButton = new QPushButton("Party Deposit", this);
    partyDepositButton->setAutoDefault(false);
    partyHLayout1->addWidget(poolAndDepositButton);
    partyHLayout1->addWidget(partyDepositButton);
    mainLayout->addLayout(partyHLayout1, 8, 0, 1, 2); // Spanning two columns

    partyPoolAndDepositButton = new QPushButton("Party Pool & Deposit", this);
    partyPoolAndDepositButton->setAutoDefault(false);
    mainLayout->addWidget(partyPoolAndDepositButton, 9, 0, 1, 2);

    // --- Row 9: Info and Exit (Right) ---
    QHBoxLayout *bottomRightLayout = new QHBoxLayout();
    infoButton = new QPushButton("Info", this);
    infoButton->setAutoDefault(false);
    exitButton = new QPushButton("Exit", this);
    // Setting the Exit button as default or auto default is fine, as it's the typical action. 
    // But setting it to false removes all potential conflicts.
    exitButton->setAutoDefault(false);
    exitButton->setDefault(false);
    
    bottomRightLayout->addStretch(1); // Push buttons to the right
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
    // This is the intended behavior and is now safely decoupled from the "Deposit All" button.
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
    // Deposit all player gold into the bank
    if (currentGold > 0) {
        // Since we are clearing the line edit later, we need to pass the currentGold amount to the signal
        // before setting currentGold to 0.
        long long amountToDeposit = currentGold; 

        bankedGold += currentGold;
        currentGold = 0;
        updateAccountStatus(currentGold, bankedGold, freeSlots);
        depositLineEdit->clear();
        // Emits the amount that was deposited
        emit depositGold(amountToDeposit); 
        QMessageBox::information(this, "Deposit", "Deposited ALL your gold into the bank.");
    } else {
        QMessageBox::warning(this, "Deposit", "You have no gold to deposit.");
    }
}

void BankDialog::on_withdrawAllButton_clicked()
{
    // Withdraw all bank gold into the player's inventory
    if (bankedGold > 0) {
        long long amountToWithdraw = bankedGold;
        
        currentGold += bankedGold;
        bankedGold = 0;
        updateAccountStatus(currentGold, bankedGold, freeSlots);
        withdrawLineEdit->clear();
        // Emits the amount that was withdrawn
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
    QModelIndex index = itemListView->currentIndex();
    if (index.isValid()) {
        QString itemName = itemModel->data(index, Qt::DisplayRole).toString();
        QMessageBox::information(this, "Item Info", QString("Displaying information for: %1").arg(itemName));
    } else {
        QMessageBox::warning(this, "Item Info", "Please select an item first.");
    }
}

void BankDialog::updateDepositValue(const QString &text)
{
    bool ok;
    long long amount = text.toLongLong(&ok);
    depositLineEdit->clear(); // Clear the field after transaction attempt

    if (ok && amount > 0) {
        if (amount <= currentGold) {
            // Successful deposit
            currentGold -= amount;
            bankedGold += amount;
            QMessageBox::information(this, "Deposit Complete", QString("Successfully deposited %L1 gold.").arg(amount));
            emit depositGold(amount);
        } else {
            // Not enough gold
            QMessageBox::warning(this, "Deposit Failed", "You do not have that much gold in your wallet.");
        }
        updateAccountStatus(currentGold, bankedGold, freeSlots);
    } else if (!text.isEmpty()) {
        // Invalid input
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid positive number for the deposit amount.");
    }
}

void BankDialog::updateWithdrawValue(const QString &text)
{
    bool ok;
    long long amount = text.toLongLong(&ok);
    withdrawLineEdit->clear(); // Clear the field after transaction attempt

    if (ok && amount > 0) {
        if (amount <= bankedGold) {
            // Successful withdrawal
            bankedGold -= amount;
            currentGold += amount;
            QMessageBox::information(this, "Withdrawal Complete", QString("Successfully withdrew %L1 gold.").arg(amount));
            emit withdrawGold(amount);
        } else {
            // Not enough gold in the bank
            QMessageBox::warning(this, "Withdrawal Failed", "The bank does not hold that much gold.");
        }
        updateAccountStatus(currentGold, bankedGold, freeSlots);
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
        // You could enable/disable other buttons here
    }
}
