#ifndef BANKDIALOG_H
#define BANKDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>

#include "gameStateManager.h"
#include <QObject>

class BankDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BankDialog(QWidget *parent = nullptr);
    ~BankDialog();

    QStandardItemModel* getItemModel() { return itemModel; }

signals:
    void depositGold(long long amount);
    void withdrawGold(long long amount);
    void itemDeposited(const QString& itemName);
    void itemWithdrawn(const QString& itemName);

private slots:
    void on_depositAllButton_clicked();
    void on_withdrawAllButton_clicked();
    void on_poolAndDepositButton_clicked();
    void on_partyDepositButton_clicked();
    void on_partyPoolAndDepositButton_clicked();
    void on_exitButton_clicked();
    void on_infoButton_clicked();
    void updateDepositValue(const QString &text);
    void updateWithdrawValue(const QString &text);
    void handleItemSelectionChanged();
    // NEW: item transfer
    void on_depositItemButton_clicked();
    void on_withdrawItemButton_clicked();

private:
    QLabel *statusLabel;
    // Gold
    QLineEdit *depositLineEdit;
    QPushButton *depositAllButton;
    QLineEdit *withdrawLineEdit;
    QPushButton *withdrawAllButton;
    // Bank vault items
    QListView *itemListView;
    QStandardItemModel *itemModel;
    // Player inventory items
    QListView *playerListView;
    QStandardItemModel *playerItemModel;
    // Transfer buttons
    QPushButton *depositItemButton;
    QPushButton *withdrawItemButton;
    // Party buttons
    QPushButton *poolAndDepositButton;
    QPushButton *partyDepositButton;
    QPushButton *partyPoolAndDepositButton;
    // Bottom buttons
    QPushButton *infoButton;
    QPushButton *exitButton;

    void setupUi();
    void createConnections();
    void updateAccountStatus(long long playerGold, long long bankGold, int freeSlots);
    long long getPlayerGold();
    long long getBankedGold();
    int getFreeSlots();
    void populatePlayerInventory();
    void populateBankInventory();
};

#endif // BANKDIALOG_H