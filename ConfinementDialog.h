#ifndef CONFINEMENTANDHOLDINGDIALOG_H
#define CONFINEMENTANDHOLDINGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QSpinBox> 

class ConfinementAndHoldingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfinementAndHoldingDialog(QWidget *parent = nullptr);
    ~ConfinementAndHoldingDialog();

private slots:
    void bindCompanion();
    void identifyCompanion();
    void sellCompanion();
    void buyCompanion();
    void showCompanionInfo();
    void searchCompanion();

private:
    void setupUi();

    // Bind Companions
    QLineEdit *bindCompLineEdit;
    QLineEdit *bindCostLineEdit;
    QPushButton *bindButton;

    // Identify, Realign & Sell Companions
    QLineEdit *identifyCompLineEdit;
    QLineEdit *identifyValueLineEdit;
    QPushButton *identifyGneButton; 
    QPushButton *identifyInfoButton;
    QPushButton *identifySellButton;
    QPushButton *identifyIdButton;
    QLineEdit *identifyIdCostLineEdit;

    // Buy Companions
    QListWidget *buyCreatureListWidget;
    QLineEdit *buyCompanionLineEdit; 
    QLineEdit *buyCostLineEdit;
    QPushButton *buyButton;
    QPushButton *buyInfoButton;
    QLineEdit *searchLineEdit;
    QPushButton *exitButton;

    // Labels for "G N E" in "Buy Companions" section
    QLabel *gneLabel;
};

#endif // CONFINEMENTANDHOLDINGDIALOG_H
