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
    long long currentGold;
    int freeSlots;

    // --- Core Methods ---
    void setupUi(); // Function to create all widgets and set up the layout
    void createConnections(); // Function to connect all signals and slots
    void updateAccountStatus(long long gold, int freeSlots);
};

#endif // BANKDIALOG_H
