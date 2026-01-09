#ifndef CONFINEMENTANDHOLDINGDIALOG_H
#define CONFINEMENTANDHOLDINGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QSpinBox>
#include <QRegularExpression> // For Qt 6 regex parsing

#include "GameStateManager.h" // Include the GameStateManager header (assumed)

class ConfinementAndHoldingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfinementAndHoldingDialog(QWidget *parent = nullptr);
    ~ConfinementAndHoldingDialog();

private slots:
    // Bind/Acquire
    void bindCompanion();    
    // Identify/Sell/Realign
    void identifyCompanion();
    void identifyCompanionGNE(); // Calculate GNE value
    void sellCompanion();
    void realignCompanionID();   // Realign ID
    // Buy
    void buyCompanion();
    void showCompanionInfo();
    void searchCompanion();
    // Handles selection change in the creature list
    void updateBuyFieldsFromList();
    // Slot to be triggered when exiting the dungeon
    void addGhostHoundOnExit();
private:
    void setupUi();
    // Bind Companions
    QLineEdit *bindCompLineEdit;
    QLineEdit *bindCostLineEdit;
    QPushButton *bindButton;
    // Identify, Realign & Sell Companions
    QLineEdit *identifyCompLineEdit;
    QLineEdit *identifyValueLineEdit; // Value output (Read-only)
    QPushButton *identifyGneButton; 
    QPushButton *identifyInfoButton;
    QPushButton *identifySellButton;
    QPushButton *identifyIdButton;
    QLineEdit *identifyIdCostLineEdit;
    // Buy Companions
    QListWidget *buyCreatureListWidget;
    QLineEdit *buyCompanionLineEdit; 
    QLineEdit *buyCostLineEdit; // Cost output (Read-only)
    QPushButton *buyButton;
    QPushButton *buyInfoButton;
    QLineEdit *searchLineEdit;
    QPushButton *exitButton;
    // Labels for "G N E" in "Buy Companions" section
    QLabel *gneLabel;
};

#endif // CONFINEMENTANDHOLDINGDIALOG_H
