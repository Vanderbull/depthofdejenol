#ifndef BANKDIALOG_H
#define BANKDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>

#include "GameStateManager.h" // Include the GameStateManager header

class BankDialog : public QDialog
{
    Q_OBJECT

public:
    // Constructor: Takes an optional parent widget
    explicit BankDialog(QWidget *parent = nullptr);
    // Destructor
    ~BankDialog();
    // Accessors/Mutators for external gold management are removed, 
    // as they now go through GameStateManager.
    // Accessor for the bank's item model (needed by the new TradeDialog)
    QStandardItemModel* getItemModel() { return itemModel; }
signals:
    // Signals to communicate actions back to the main application
    void depositGold(long long amount);
    void withdrawGold(long long amount);
    // NEW: Signals for item transactions
    void itemDeposited(const QString& itemName);
    void itemWithdrawn(const QString& itemName);
private slots:
    // Slots connected to the buttons
    void on_depositAllButton_clicked();
    void on_withdrawAllButton_clicked();
    void on_poolAndDepositButton_clicked();
    void on_partyDepositButton_clicked();
    void on_partyPoolAndDepositButton_clicked();
    void on_exitButton_clicked();
    void on_infoButton_clicked(); 
    // Slots for line edits (connecting manually)
    void updateDepositValue(const QString &text);
    void updateWithdrawValue(const QString &text);
    // Slot for QListView selection
    void handleItemSelectionChanged();
private:
    // --- Widget Declarations ---
    QLabel *statusLabel;
    // Deposit/Withdraw Gold
    QLineEdit *depositLineEdit;
    QPushButton *depositAllButton;
    QLineEdit *withdrawLineEdit;
    QPushButton *withdrawAllButton;
    // Item List
    QListView *itemListView;
    QStandardItemModel *itemModel; 
    QStandardItemModel *playerItemModel; 
    // Party Buttons
    QPushButton *poolAndDepositButton;
    QPushButton *partyDepositButton;
    QPushButton *partyPoolAndDepositButton;
    // Bottom Buttons
    QPushButton *infoButton;
    QPushButton *exitButton;
    // --- Private Data ---
    // --- Core Methods ---
    void setupUi();
    void createConnections();
    void updateAccountStatus(long long playerGold, long long bankGold, int freeSlots);     
    // Helper to read and convert gold from GameStateManager
    long long getPlayerGold();
    long long getBankedGold();
    // NEW Helper to read freeSlots from GameStateManager
    int getFreeSlots();
};

#endif // BANKDIALOG_H
