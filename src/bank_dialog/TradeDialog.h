#ifndef TRADEDIALOG_H
#define TRADEDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QStringList>

#include "GameStateManager.h" // Include the GameStateManager header

// Forward declaration
class BankDialog; 

class TradeDialog : public QDialog
{
    Q_OBJECT

public:
    // The constructor takes the player's item model and the bank's item model
    explicit TradeDialog(QStandardItemModel *playerModel, QStandardItemModel *bankModel, QWidget *parent = nullptr);
    ~TradeDialog();

private slots:
    // Move item from player inventory to bank
    void on_depositItemButton_clicked();
    // Move item from bank vault to player inventory
    void on_withdrawItemButton_clicked();
    // Close the dialog
    void on_closeButton_clicked();

private:
    // --- Widget Declarations ---
    QLabel *playerLabel;
    QListView *playerListView;
    QPushButton *depositItemButton;
    QPushButton *withdrawItemButton;
    QListView *bankListView;
    QLabel *bankLabel;
    QPushButton *closeButton;
    // --- Data Models ---
    // Reference to the player's current inventory model
    QStandardItemModel *playerItemModel; 
    // Reference to the bank's item model
    QStandardItemModel *bankItemModel;   
    // --- Core Methods ---
    void setupUi();
    void createConnections();
    // Helper function to safely move an item between two models
    void moveSelectedItem(QListView *sourceView, QStandardItemModel *sourceModel, QStandardItemModel *destModel);
};

#endif // TRADEDIALOG_H
