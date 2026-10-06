#ifndef INVENTORYDIALOG_H
#define INVENTORYDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "gameStateManager.h"

class InventoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit InventoryDialog(QWidget *parent = nullptr);
    ~InventoryDialog();
private slots:
    void onEquipButtonClicked();
    void onUnequipButtonClicked();
    void onDropButtonClicked();
    void onInfoButtonClicked();
    void onUseButtonClicked();
private:
    void setupUi();
    void initializeItemData();
    void loadInventoryData();
    void updateEffectiveStatsPanel();
    QString itemTooltip(const QString& itemName) const;
    QTabWidget *tabWidget;
    QListWidget *inventoryList;
    QListWidget *equippedList;
    QListWidget *spellsList;
    QPushButton *equipButton;
    QPushButton *unequipButton;
    QPushButton *useButton;
    QPushButton *dropButton;
    QPushButton *infoButton;
    QLabel *effectiveStatsLabel;
    // A map to store item descriptions and stats
    QMap<QString, QString> itemInfoMap;
};

#endif // INVENTORYDIALOG_H
