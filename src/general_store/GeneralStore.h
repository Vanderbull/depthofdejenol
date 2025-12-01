#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QMessageBox> // Added for dialog feedback

class GeneralStore : public QDialog
{
    Q_OBJECT

public:
    explicit GeneralStore(QWidget *parent = nullptr);
    virtual ~GeneralStore(); // Made destructor explicitly virtual to resolve linker error

private slots:
    // Slots for item interaction
    void uncurseItem();
    void combineItems();
    void identifySellItem();
    void buyItem();
    void showItemInfo();
    void searchItems(const QString &searchText);
    void exitStore();

private:
    // UI Elements for "Uncurse Items"
    QLineEdit *uncurseItemLineEdit;
    QPushButton *uncurseButton;

    // UI Elements for "Combine Items"
    QLineEdit *combineItemLineEdit;
    QLineEdit *combineItemsLineEdit; // Assuming multiple items to combine
    QPushButton *combineButton;

    // UI Elements for "Identify, Realign & Sell Items"
    QLineEdit *identifySellItemLineEdit;
    QLabel *itemValueLabel;
    QPushButton *infoButton;
    QPushButton *sellButton;
    QPushButton *idButton;
    QLabel *idCostLabel; // To display ID cost

    // UI Elements for "Buy Items"
    QListWidget *buyItemsListWidget;
    QLineEdit *searchItemsLineEdit;
    QLineEdit *buyItemLineEdit; // For selected item details or quantity
    QLabel *buyItemCostLabel;
    QPushButton *buyButton;
    QPushButton *buyInfoButton; // "INFO" button next to BUY
    QPushButton *exitButton;

    void setupUi();
    void populateBuyItemsList(); // Function to add example items
    void showFeedbackDialog(const QString &title, const QString &message, QMessageBox::Icon icon = QMessageBox::Information);
};
