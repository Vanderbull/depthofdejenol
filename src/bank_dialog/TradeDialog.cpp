#include "TradeDialog.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStandardItem>
#include <QModelIndex>

TradeDialog::TradeDialog(QStandardItemModel *playerModel, QStandardItemModel *bankModel, QWidget *parent) :
    QDialog(parent),
    playerItemModel(playerModel),
    bankItemModel(bankModel)
{
    // Safety check: ensure models were passed correctly
    if (!playerItemModel || !bankItemModel) {
        return; 
    }

    GameStateManager* gsm = GameStateManager::instance();

    // 1. Fetch Active Character Inventory
    int activeIdx = gsm->getGameValue("ActiveCharacterIndex").toInt();
    QVariantList party = gsm->getGameValue("Party").toList();
    
    playerItemModel->clear();
    if (activeIdx >= 0 && activeIdx < party.size()) {
        QVariantMap character = party[activeIdx].toMap();
        if (character.contains("Inventory")) {
            QStringList inventory = character["Inventory"].toStringList();
            for (const QString& item : inventory) {
                playerItemModel->appendRow(new QStandardItem(item));
            }
        }
    }

    // 2. Fetch Bank Inventory
    bankItemModel->clear();
    QStringList bankItems = gsm->getBankInventory();
    for (const QString& item : bankItems) {
        bankItemModel->appendRow(new QStandardItem(item));
    }

    setupUi(); 
    createConnections();
}

TradeDialog::~TradeDialog() {}

void TradeDialog::setupUi()
{
    setWindowTitle("Item Trade: Player <-> Bank");
    setMinimumSize(600, 400); 

    QVBoxLayout *mainVLayout = new QVBoxLayout(this);
    QLabel *titleLabel = new QLabel("Transfer Items Between Inventory and Bank", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #007bff;");
    mainVLayout->addWidget(titleLabel);

    QHBoxLayout *centralHLayout = new QHBoxLayout();
    
    // Player Column
    QVBoxLayout *playerVLayout = new QVBoxLayout();
    playerVLayout->addWidget(new QLabel("Your Inventory", this));
    playerListView = new QListView(this);
    playerListView->setModel(playerItemModel);
    playerVLayout->addWidget(playerListView);
    centralHLayout->addLayout(playerVLayout);

    // Buttons
    QVBoxLayout *buttonVLayout = new QVBoxLayout();
    buttonVLayout->addStretch(1);
    depositItemButton = new QPushButton(">> Deposit >>", this);
    withdrawItemButton = new QPushButton("<< Withdraw <<", this);
    buttonVLayout->addWidget(depositItemButton);
    buttonVLayout->addWidget(withdrawItemButton);
    buttonVLayout->addStretch(1);
    centralHLayout->addLayout(buttonVLayout);

    // Bank Column
    QVBoxLayout *bankVLayout = new QVBoxLayout();
    bankVLayout->addWidget(new QLabel("Bank Vault", this));
    bankListView = new QListView(this);
    bankListView->setModel(bankItemModel);
    bankVLayout->addWidget(bankListView);
    centralHLayout->addLayout(bankVLayout);

    mainVLayout->addLayout(centralHLayout);

    closeButton = new QPushButton("Close & Save Trade", this);
    mainVLayout->addWidget(closeButton, 0, Qt::AlignCenter);
    setLayout(mainVLayout);
}

void TradeDialog::createConnections()
{
    connect(depositItemButton, &QPushButton::clicked, this, &TradeDialog::on_depositItemButton_clicked);
    connect(withdrawItemButton, &QPushButton::clicked, this, &TradeDialog::on_withdrawItemButton_clicked);
    connect(closeButton, &QPushButton::clicked, this, &TradeDialog::on_closeButton_clicked);
}

void TradeDialog::moveSelectedItem(QListView *sourceView, QStandardItemModel *sourceModel, QStandardItemModel *destModel)
{
    if (!sourceView || !sourceModel || !destModel) return;

    QModelIndex index = sourceView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Trade Error", "Please select an item to move.");
        return;
    }

    QStandardItem *item = sourceModel->takeItem(index.row());
    if (item) {
        destModel->appendRow(item);
        sourceView->clearSelection();
    }
}

void TradeDialog::on_depositItemButton_clicked() {
    moveSelectedItem(playerListView, playerItemModel, bankItemModel);
}

void TradeDialog::on_withdrawItemButton_clicked() {
    moveSelectedItem(bankListView, bankItemModel, playerItemModel);
}

void TradeDialog::on_closeButton_clicked()
{
    GameStateManager* gsm = GameStateManager::instance();

    // 1. Sync Player Inventory
    if (playerItemModel) {
        int activeIdx = gsm->getGameValue("ActiveCharacterIndex").toInt();
        QStringList updatedPlayerItems;
        for(int i = 0; i < playerItemModel->rowCount(); ++i) {
            QStandardItem* item = playerItemModel->item(i);
            if (item) {
                updatedPlayerItems << item->text();
            }
        }
        gsm->setCharacterInventory(activeIdx, updatedPlayerItems);
    }

    // 2. Sync Bank Inventory
    if (bankItemModel) {
        QStringList updatedBankItems;
        for(int i = 0; i < bankItemModel->rowCount(); ++i) {
            QStandardItem* item = bankItemModel->item(i);
            if (item) {
                updatedBankItems << item->text();
            }
        }
        gsm->setBankInventory(updatedBankItems);
    }

    accept();
}
