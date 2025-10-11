#include "BankPane.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDebug>
#include <QMessageBox>

// --- Stubbade Klasser (Definierade i ett namespace) ---
namespace mordorData {
    class ItemInstance {
    public:
        QString name;
        ItemInstance(const QString& n) : name(n) {}
    };

    class BankAccount {
    public:
        static const int MAXITEMSINBANK = 50;
        long gold = 1000;
        QList<ItemInstance*> items;
        int getItemsSize() const { return items.size(); }
        long getGold() const { return gold; }
        void changeGold(long amount) { gold += amount; }
        void setGold(long amount) { gold = amount; }
        void addItem(ItemInstance* item) { items.append(item); }
        void removeItem(ItemInstance* item) { items.removeOne(item); }
        QList<ItemInstance*> getItems() const { return items; }
    };

    class Player {
    public:
        static const int MAXITEMSONHAND = 20;
        long goldOnHand = 500;
        BankAccount bankAccount;
        BankAccount* getBankAccount() { return &bankAccount; }
        long getGoldOnHand() const { return goldOnHand; }
        void setGoldOnHand(long amount) { goldOnHand = amount; }
        void changeGoldOnHand(long amount) { goldOnHand += amount; }
        int getItemCount() const { return 10; }
        void addItem(ItemInstance* item) { Q_UNUSED(item); /* Stub */ }
        void removeItem(ItemInstance* item) { Q_UNUSED(item); /* Stub */ }
    };
}

class Mordor {};
class InformationPanel { public: void updatePanes(mordorData::Player* p) { Q_UNUSED(p); /* Stub */ } };
class SICPane { public: mordorData::ItemInstance* getItemSelected() { return nullptr; } bool isSelectedEquipped() { return false; } void updateItems() { /* Stub */ } };
class DataBank {
    public:
    class Messenger {
    public:
        enum Destination { ItemInfo };
        void postThing(Destination dest, QVariant thing) { Q_UNUSED(dest); Q_UNUSED(thing); /* Stub */ }
    };
    Messenger messenger;
    Messenger* getMessenger() { return &messenger; }
};

using namespace mordorData;

// --- BANKPANE IMPLEMENTATION ---

BankPane::BankPane(Mordor* nParent, InformationPanel* nInfoPane, mordorData::Player* nPlayer, SICPane* nSicPane, DataBank* nDataBank, QWidget* parent)
    : QWidget(parent),
      parentApp(nParent),
      infoPane(nInfoPane),
      player(nPlayer),
      dataBank(nDataBank),
      sicPane(nSicPane)
{
    setupUI();
    updatePane();
}

void BankPane::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // ... (Layout code for T-Pane, C-Pane, B-Pane) ...
    // (Layout setup details are omitted for brevity but remain the same)
    
    // T-Pane
    QLabel* titleLabel = new QLabel("Welcome to the Bank!");
    QLabel* statusLabel = new QLabel("Account status:");
    jlAccountSummary = new QLabel();

    QVBoxLayout* tLayout = new QVBoxLayout();
    tLayout->addWidget(titleLabel);
    tLayout->addWidget(statusLabel);
    tLayout->addWidget(jlAccountSummary);

    QGroupBox* tPane = new QGroupBox();
    tPane->setLayout(tLayout);
    
    // C-Pane (Gold and Items)
    QHBoxLayout* cLayout = new QHBoxLayout();
    
    // LC-Pane (Gold transactions)
    QVBoxLayout* lcLayout = new QVBoxLayout();
    lcLayout->addWidget(new QLabel("Deposit"));
    QHBoxLayout* depositLayout = new QHBoxLayout();
    tfDeposit = new QLineEdit();
    jbDepAll = new QPushButton("All");
    depositLayout->addWidget(tfDeposit);
    depositLayout->addWidget(jbDepAll);
    depositLayout->addStretch();
    lcLayout->addLayout(depositLayout);
    
    lcLayout->addWidget(new QLabel("Withdraw"));
    QHBoxLayout* withdrawLayout = new QHBoxLayout();
    tfWithdraw = new QLineEdit();
    jbWdAll = new QPushButton("All");
    withdrawLayout->addWidget(tfWithdraw);
    withdrawLayout->addWidget(jbWdAll);
    withdrawLayout->addStretch();
    lcLayout->addLayout(withdrawLayout);
    
    lcLayout->addWidget(new QLabel("Party"));
    QHBoxLayout* partyLayout = new QHBoxLayout();
    jbPartyPool = new QPushButton("Party Pool");
    jbPartyDeposit = new QPushButton("Party Deposit");
    jbPartyPoolDeposit = new QPushButton("Party Pool & Deposit");
    partyLayout->addWidget(jbPartyPool);
    partyLayout->addWidget(jbPartyDeposit);
    partyLayout->addStretch();
    lcLayout->addLayout(partyLayout);
    lcLayout->addWidget(jbPartyPoolDeposit);

    lcLayout->addStretch(1);
    QWidget* lcPane = new QWidget();
    lcPane->setLayout(lcLayout);

    // RC-Pane (Item transactions)
    QVBoxLayout* rcLayout = new QVBoxLayout();
    rcLayout->addWidget(new QLabel("Deposit/Withdraw Items"));
    liItemList = new QListWidget();
    rcLayout->addWidget(liItemList);

    // Item Buttons
    QHBoxLayout* itemButtonsLayout = new QHBoxLayout();
    jbInfo = new QPushButton("Info");
    jbRemoveItem = new QPushButton("-");
    jbAddItem = new QPushButton("+");
    itemButtonsLayout->addWidget(jbInfo);
    itemButtonsLayout->addWidget(jbRemoveItem);
    itemButtonsLayout->addWidget(jbAddItem);
    QWidget* itemButtons = new QWidget();
    itemButtons->setLayout(itemButtonsLayout);
    rcLayout->addWidget(itemButtons);

    QWidget* rcPane = new QWidget();
    rcPane->setLayout(rcLayout);

    cLayout->addWidget(lcPane);
    cLayout->addWidget(rcPane);
    QWidget* cPane = new QWidget();
    cPane->setLayout(cLayout);


    // B-Pane (Exit)
    QHBoxLayout* bLayout = new QHBoxLayout();
    jbExit = new QPushButton("Exit");
    bLayout->addStretch();
    bLayout->addWidget(jbExit);
    bLayout->addStretch();
    QWidget* bPane = new QWidget();
    bPane->setLayout(bLayout);

    mainLayout->addWidget(tPane);
    mainLayout->addWidget(cPane);
    mainLayout->addWidget(bPane);

    // Connect signals to slots
    connect(jbExit, &QPushButton::clicked, this, &BankPane::handleExit);
    connect(tfDeposit, &QLineEdit::returnPressed, this, &BankPane::handleDepositTextEntry);
    connect(jbDepAll, &QPushButton::clicked, this, &BankPane::handleDepositAll);
    connect(tfWithdraw, &QLineEdit::returnPressed, this, &BankPane::handleWithdrawTextEntry);
    connect(jbWdAll, &QPushButton::clicked, this, &BankPane::handleWithdrawAll);
    connect(jbPartyPool, &QPushButton::clicked, this, &BankPane::handlePartyPool);
    connect(jbPartyDeposit, &QPushButton::clicked, this, &BankPane::handlePartyDeposit);
    connect(jbPartyPoolDeposit, &QPushButton::clicked, this, &BankPane::handlePartyPoolDeposit);
    connect(jbInfo, &QPushButton::clicked, this, &BankPane::handleItemInfo);
    connect(jbRemoveItem, &QPushButton::clicked, this, &BankPane::handleRemoveItem);
    connect(jbAddItem, &QPushButton::clicked, this, &BankPane::handleAddItem);
}

void BankPane::updateSummary(mordorData::BankAccount* account)
{
    if (!account) return;
    QString accSummary = QString("You have %1 and %2 free Bank slots available.")
                             .arg(account->getGold())
                             .arg(BankAccount::MAXITEMSINBANK - account->getItemsSize());
    jlAccountSummary->setText(accSummary);
}

void BankPane::updatePane()
{
    if (!player) return;
    BankAccount* account = player->getBankAccount();
    updateSummary(account);
    liItemList->clear();
    for (ItemInstance* item : account->getItems())
    {
        QListWidgetItem* listItem = new QListWidgetItem(item->name, liItemList);
        listItem->setData(Qt::UserRole, QVariant::fromValue((void*)item));
    }
}

// =========================================================
// --- SLOT IMPLEMENTATIONS (FRISTÅENDE METODER) ---
// Dessa metoder måste vara definierade i det globala BankPane-scopet
// och inte inuti någon annan metod.
// =========================================================

void BankPane::handleExit()
{
    // Fixat: Implementation är nu garanterad att finnas.
    qDebug() << "Exit Bank action triggered.";
}

void BankPane::handleDepositTextEntry()
{
    // Fixat: Implementation är nu garanterad att finnas.
    bool ok;
    long gold = tfDeposit->text().toLong(&ok);

    if (!ok || gold <= 0 || gold > player->getGoldOnHand())
    {
        QMessageBox::warning(this, "Input Error", "Invalid amount to deposit or not enough gold on hand.");
        return;
    }

    player->changeGoldOnHand(-gold);
    player->getBankAccount()->changeGold(gold);
    updateSummary(player->getBankAccount());
    infoPane->updatePanes(player);
    tfDeposit->clear();
}

void BankPane::handleDepositAll()
{
    // Fixat: Implementation är nu garanterad att finnas.
    player->getBankAccount()->changeGold(player->getGoldOnHand());
    player->setGoldOnHand(0);
    updateSummary(player->getBankAccount());
    infoPane->updatePanes(player);
}

void BankPane::handleWithdrawTextEntry()
{
    // Fixat: Implementation är nu garanterad att finnas.
    bool ok;
    long gold = tfWithdraw->text().toLong(&ok);

    if (!ok || gold <= 0 || gold > player->getBankAccount()->getGold())
    {
        QMessageBox::warning(this, "Input Error", "Invalid amount to withdraw or not enough gold in bank.");
        return;
    }

    player->changeGoldOnHand(gold);
    player->getBankAccount()->changeGold(-gold);
    updateSummary(player->getBankAccount());
    infoPane->updatePanes(player);
    tfWithdraw->clear();
}

void BankPane::handleWithdrawAll()
{
    // Fixat: Implementation är nu garanterad att finnas.
    player->changeGoldOnHand(player->getBankAccount()->getGold());
    player->getBankAccount()->setGold(0);
    updateSummary(player->getBankAccount());
    infoPane->updatePanes(player);
}

void BankPane::handlePartyPool()
{
    // Fixat: Implementation är nu garanterad att finnas.
    qDebug() << "TODO: Party Pool command";
}

void BankPane::handlePartyDeposit()
{
    // Fixat: Implementation är nu garanterad att finnas.
    qDebug() << "TODO: Party Deposit command";
}

void BankPane::handlePartyPoolDeposit()
{
    // Fixat: Implementation är nu garanterad att finnas.
    qDebug() << "TODO: Party Pool & Deposit command";
}

void BankPane::handleItemInfo()
{
    QListWidgetItem* selectedItem = liItemList->currentItem();
    if (selectedItem)
    {
        QVariant itemData = selectedItem->data(Qt::UserRole);
        ItemInstance* tItem = static_cast<ItemInstance*>(itemData.value<void*>());

        if (tItem)
        {
            qDebug() << "Display Info for item:" << tItem->name;
        }
    }
}

void BankPane::handleRemoveItem()
{
    QListWidgetItem* selectedItem = liItemList->currentItem();

    if (selectedItem && player->getItemCount() < Player::MAXITEMSONHAND - 1)
    {
        QVariant itemData = selectedItem->data(Qt::UserRole);
        ItemInstance* tItem = static_cast<ItemInstance*>(itemData.value<void*>());

        if (tItem)
        {
            player->addItem(tItem);
            player->getBankAccount()->removeItem(tItem);

            delete liItemList->takeItem(liItemList->currentRow());

            sicPane->updateItems();
            updateSummary(player->getBankAccount());
        }
    }
    else if (selectedItem && player->getItemCount() >= Player::MAXITEMSONHAND - 1)
    {
        QMessageBox::warning(this, "Inventory Full", "Your on-hand inventory is full. Cannot remove item from bank.");
    }
}

void BankPane::handleAddItem()
{
    mordorData::BankAccount* account = player->getBankAccount();

    if (account->getItemsSize() < mordorData::BankAccount::MAXITEMSINBANK - 1)
    {
        mordorData::ItemInstance* tItem = sicPane->getItemSelected();

        if (tItem == nullptr || sicPane->isSelectedEquipped())
        {
            if(tItem == nullptr)
                QMessageBox::information(this, "No Item", "Please select an item to deposit from your inventory.");
            else if(sicPane->isSelectedEquipped())
                QMessageBox::information(this, "Equipped Item", "Cannot deposit an equipped item.");
            return;
        }

        account->addItem(tItem);
        player->removeItem(tItem);

        QListWidgetItem* listItem = new QListWidgetItem(tItem->name, liItemList);
        listItem->setData(Qt::UserRole, QVariant::fromValue((void*)tItem));

        sicPane->updateItems();
        updateSummary(account);
    }
    else
    {
        QMessageBox::warning(this, "Bank Full", "The bank account is full. Cannot deposit more items.");
    }
}
