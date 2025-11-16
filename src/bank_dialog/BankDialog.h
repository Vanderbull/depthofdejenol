#ifndef BANKDIALOG_H
#define BANKDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel> // Needed to hold the item data for QListView

class BankDialog : public QDialog
{
    Q_OBJECT

public:
    // Constructor: Takes an optional parent widget
    explicit BankDialog(QWidget *parent = nullptr);
    // Destructor
    ~BankDialog();

    // Accessors/Mutators for external gold management (if needed)
    long long getPlayerGold() const { return currentGold; }
    long long getBankedGold() const { return bankedGold; }
    void setPlayerGold(long long gold) { currentGold = gold; }
    void setBankedGold(long long gold) { bankedGold = gold; }


signals:
    // Signals to communicate actions back to the main application
    void depositGold(long long amount);
    void withdrawGold(long long amount);
    // You might want to add signals for item transactions as well...

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
    // These will now apply the changes to the gold variables.
    void updateDepositValue(const QString &text);
    void updateWithdrawValue(const QString &text);

    // Slot for QListView selection
    void handleItemSelectionChanged();

private:
    // --- Widget Declarations ---
    // Top section
    QLabel *statusLabel;
    
    // Deposit/Withdraw Gold
    QLineEdit *depositLineEdit;
    QPushButton *depositAllButton;
    QLineEdit *withdrawLineEdit;
    QPushButton *withdrawAllButton;

    // Item List
    QListView *itemListView;
    QStandardItemModel *itemModel; // Model to manage data in the QListView

    // Party Buttons
    QPushButton *poolAndDepositButton;
    QPushButton *partyDepositButton;
    QPushButton *partyPoolAndDepositButton;

    // Bottom Buttons
    QPushButton *infoButton;
    QPushButton *exitButton;

    // --- Private Data ---
    // Player's gold (now initialized to 0 in the .cpp file)
    long long currentGold; 
    // Gold stored in the bank (NEW member)
    long long bankedGold; 
    int freeSlots;

    // --- Core Methods ---
    void setupUi(); // Function to create all widgets and set up the layout
    void createConnections(); // Function to connect all signals and slots
    // Updated to show both player gold (currentGold) and banked gold (bankedGold)
    void updateAccountStatus(long long playerGold, long long bankGold, int freeSlots); 
};

#endif // BANKDIALOG_H
