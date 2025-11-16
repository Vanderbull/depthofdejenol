#include "TradeDialog.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStandardItem>
#include <QModelIndex>

// --- Constructor ---
TradeDialog::TradeDialog(QStandardItemModel *playerModel, QStandardItemModel *bankModel, QWidget *parent) :
    QDialog(parent),
    playerItemModel(playerModel),
    bankItemModel(bankModel)
{
    // 1. Set up initial player inventory model with example data
    if (playerItemModel->rowCount() == 0) {
        QStringList initialPlayerItems;
        initialPlayerItems << "Health Potion [5]" << "Mana Scroll [3]" << "Rusty Sword" 
                           << "Leather Armor" << "Magic Ring";
        for (const QString& item : initialPlayerItems) {
            playerItemModel->appendRow(new QStandardItem(item));
        }
    }

    // 2. Create and position all widgets (Layout)
    setupUi(); 

    // 3. Connect signals to slots
    createConnections();
}

// --- Destructor ---
TradeDialog::~TradeDialog()
{
    // The models are managed by the parent (or BankDialog in a real app) and are not deleted here.
}

// --- Setup Methods ---

void TradeDialog::setupUi()
{
    setWindowTitle("Item Trade: Player <-> Bank");
    setMinimumSize(600, 400); 

    QVBoxLayout *mainVLayout = new QVBoxLayout(this);
    
    // Top Label
    QLabel *titleLabel = new QLabel("Transfer Items Between Inventory and Bank", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #007bff;");
    mainVLayout->addWidget(titleLabel);

    // Main central layout (Side-by-side lists and buttons)
    QHBoxLayout *centralHLayout = new QHBoxLayout();
    
    // --- Left Section: Player Inventory ---
    QVBoxLayout *playerVLayout = new QVBoxLayout();
    playerLabel = new QLabel("Your Inventory", this);
    playerLabel->setStyleSheet("font-weight: bold; color: #dc3545;");
    playerVLayout->addWidget(playerLabel);
    
    playerListView = new QListView(this);
    playerListView->setModel(playerItemModel);
    playerListView->setSelectionMode(QAbstractItemView::SingleSelection);
    playerVLayout->addWidget(playerListView);
    
    centralHLayout->addLayout(playerVLayout);

    // --- Center Section: Action Buttons ---
    QVBoxLayout *buttonVLayout = new QVBoxLayout();
    buttonVLayout->addStretch(1);
    
    depositItemButton = new QPushButton(">> Deposit >>", this);
    depositItemButton->setToolTip("Move selected item from Inventory to Bank");
    buttonVLayout->addWidget(depositItemButton);

    withdrawItemButton = new QPushButton("<< Withdraw <<", this);
    withdrawItemButton->setToolTip("Move selected item from Bank to Inventory");
    buttonVLayout->addWidget(withdrawItemButton);

    buttonVLayout->addStretch(1);
    centralHLayout->addLayout(buttonVLayout);

    // --- Right Section: Bank Vault ---
    QVBoxLayout *bankVLayout = new QVBoxLayout();
    bankLabel = new QLabel("Bank Vault", this);
    bankLabel->setStyleSheet("font-weight: bold; color: #28a745;");
    bankVLayout->addWidget(bankLabel);
    
    bankListView = new QListView(this);
    bankListView->setModel(bankItemModel);
    bankListView->setSelectionMode(QAbstractItemView::SingleSelection);
    bankVLayout->addWidget(bankListView);
    
    centralHLayout->addLayout(bankVLayout);

    mainVLayout->addLayout(centralHLayout);

    // Bottom Button
    closeButton = new QPushButton("Close Trade Window", this);
    mainVLayout->addWidget(closeButton, 0, Qt::AlignCenter);
    
    setLayout(mainVLayout);
}

void TradeDialog::createConnections()
{
    connect(depositItemButton, &QPushButton::clicked, this, &TradeDialog::on_depositItemButton_clicked);
    connect(withdrawItemButton, &QPushButton::clicked, this, &TradeDialog::on_withdrawItemButton_clicked);
    connect(closeButton, &QPushButton::clicked, this, &TradeDialog::on_closeButton_clicked);
}

// --- Private Logic Methods ---

void TradeDialog::moveSelectedItem(QListView *sourceView, QStandardItemModel *sourceModel, QStandardItemModel *destModel)
{
    QModelIndex index = sourceView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Trade Error", "Please select an item to move.");
        return;
    }

    // Get the item from the source model
    QStandardItem *item = sourceModel->takeItem(index.row());
    if (item) {
        // Append the item to the destination model
        destModel->appendRow(item);
        sourceView->clearSelection();
        
        // Success message
        QMessageBox::information(this, "Trade Success", QString("'%1' transferred successfully!").arg(item->text()));
    } else {
        // This case should ideally not happen if index.isValid() is true
        QMessageBox::critical(this, "Internal Error", "Could not retrieve item for transfer.");
    }
}

// --- Slot Implementations ---

void TradeDialog::on_depositItemButton_clicked()
{
    // Move item from player inventory (source) to bank vault (destination)
    moveSelectedItem(playerListView, playerItemModel, bankItemModel);
}

void TradeDialog::on_withdrawItemButton_clicked()
{
    // Move item from bank vault (source) to player inventory (destination)
    moveSelectedItem(bankListView, bankItemModel, playerItemModel);
}

void TradeDialog::on_closeButton_clicked()
{
    // Close the trade dialog
    accept();
}
