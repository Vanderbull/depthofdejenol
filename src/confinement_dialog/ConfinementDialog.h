#ifndef CONFINEMENTANDHOLDINGDIALOG_H
#define CONFINEMENTANDHOLDINGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QSpinBox>
#include <QRegularExpression>

#include "gameStateManager.h"
#include <QObject>

class ConfinementAndHoldingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfinementAndHoldingDialog(QWidget *parent = nullptr);
    ~ConfinementAndHoldingDialog() = default;

private slots:
    // Action Slots
    void bindCompanion();
    void identifyCompanion();
    void identifyCompanionGNE();
    void sellCompanion();
    void realignCompanionID();
    void buyCompanion();
    void showCompanionInfo();
    void searchCompanion();
    void updateBuyFieldsFromList();
    void addGhostHoundOnExit();

private:
    // Initialization Helpers
    void setupUi();
    void applyThemeAndWindowSettings();
    void initializeGameState();
    void populateStockList();
    void connectSignalsAndSlots();

    // UI Builders
    QWidget* createLeftColumn();
    QWidget* createRightColumn();

    // UI Controls - Bind Section
    QLineEdit *bindCompLineEdit = nullptr;
    QLineEdit *bindCostLineEdit = nullptr;
    QPushButton *bindButton = nullptr;

    // UI Controls - Identify/Sell Section
    QLineEdit *identifyCompLineEdit = nullptr;
    QLineEdit *identifyValueLineEdit = nullptr;
    QLineEdit *identifyIdCostLineEdit = nullptr;
    QPushButton *identifyGneButton = nullptr;
    QPushButton *identifyInfoButton = nullptr;
    QPushButton *identifySellButton = nullptr;
    QPushButton *identifyIdButton = nullptr;

    // UI Controls - Buy Section
    QListWidget *buyCreatureListWidget = nullptr;
    QLineEdit *buyCompanionLineEdit = nullptr;
    QLineEdit *buyCostLineEdit = nullptr;
    QLineEdit *searchLineEdit = nullptr;
    QPushButton *buyButton = nullptr;
    QPushButton *buyInfoButton = nullptr;
    QPushButton *exitButton = nullptr;
    QLabel *gneLabel = nullptr;
};

#endif // CONFINEMENTANDHOLDINGDIALOG_H
