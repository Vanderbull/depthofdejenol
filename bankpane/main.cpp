#include <QApplication>
#include "BankPane.h"
#include <QMainWindow>
#include <QVariant>

// --- Återanvänd stubbade klasser från BankPane.cpp för att tillfredsställa kompilatorn ---
// Normalt skulle dessa inkluderas från sina egna headerfiler.
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
        Player() {
             // Lägg till några testobjekt i banken
             bankAccount.addItem(new ItemInstance("Old Sword"));
             bankAccount.addItem(new ItemInstance("Shiny Shield"));
        }
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
class InformationPanel { public: void updatePanes(mordorData::Player* p) { Q_UNUSED(p); } };
class SICPane { 
public: 
    mordorData::ItemInstance* getItemSelected() { return new mordorData::ItemInstance("Deposit Test Item"); } 
    bool isSelectedEquipped() { return false; } 
    void updateItems() { /* Stub */ } 
};
class DataBank {
    public:
    class Messenger {
    public:
        enum Destination { ItemInfo };
        void postThing(Destination dest, QVariant thing) { Q_UNUSED(dest); Q_UNUSED(thing); }
    };
    Messenger messenger;
    Messenger* getMessenger() { return &messenger; }
};
// ----------------------------------------------------------------------------


int main(int argc, char *argv[])
{
    // 1. Skapa QApplication-objektet
    QApplication a(argc, argv);

    // 2. Skapa stubb-instanser av de nödvändiga beroendena
    // I en riktig app skulle dessa komma från din spelmotor eller datalager.
    Mordor mordorApp;
    InformationPanel infoPanel;
    mordorData::Player player;
    SICPane sicPane;
    DataBank dataBank;

    // 3. Skapa BankPane-widgeten
    BankPane* bankPane = new BankPane(&mordorApp, &infoPanel, &player, &sicPane, &dataBank);

    // 4. Placera BankPane i ett huvudfönster och visa det
    QMainWindow mainWindow;
    mainWindow.setCentralWidget(bankPane);
    mainWindow.setWindowTitle("Mordor Bank Interface");
    mainWindow.setMinimumSize(600, 400);

    // 5. Visa fönstret
    mainWindow.show();

    // 6. Starta Qt's händelseloop
    return a.exec();
}
