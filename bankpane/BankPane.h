#ifndef BANKPANE_H
#define BANKPANE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QVariant>

// Framåtförklara ALLA dataklasser inom deras namespace för att undvika tvetydighet
namespace mordorData {
    class Player;
    class BankAccount;
    class ItemInstance;
}

// Framåtförklaringar för externa klasser (som antas vara globala)
class Mordor;
class InformationPanel;
class SICPane;
class DataBank;

class BankPane : public QWidget
{
    Q_OBJECT

public:
    // Konstruktorn använder den fullt kvalificerade typen mordorData::Player*
    BankPane(Mordor* nParent, InformationPanel* nInfoPane, mordorData::Player* nPlayer, SICPane* nSicPane, DataBank* nDataBank, QWidget* parent = nullptr);

    void updatePane();

private slots:
    // Deklarationer av alla slots
    void handleExit();
    void handleDepositTextEntry();
    void handleDepositAll();
    void handleWithdrawTextEntry();
    void handleWithdrawAll();
    void handlePartyPool();
    void handlePartyDeposit();
    void handlePartyPoolDeposit();
    void handleItemInfo();
    void handleRemoveItem();
    void handleAddItem();

private:
    // updateSummary använder den fullt kvalificerade typen mordorData::BankAccount*
    void updateSummary(mordorData::BankAccount* account);

    // Medlemsvariabler
    Mordor* parentApp;
    InformationPanel* infoPane;
    mordorData::Player* player;
    DataBank* dataBank;
    SICPane* sicPane;

    // GUI-komponenter
    QLineEdit* tfDeposit;
    QLineEdit* tfWithdraw;
    QPushButton* jbDepAll;
    QPushButton* jbWdAll;
    QPushButton* jbPartyPool;
    QPushButton* jbPartyDeposit;
    QPushButton* jbPartyPoolDeposit;
    QPushButton* jbInfo;
    QPushButton* jbExit;
    QListWidget* liItemList;
    QLabel* jlAccountSummary;
    QPushButton* jbRemoveItem;
    QPushButton* jbAddItem;

    void setupUI();
};

#endif // BANKPANE_H
