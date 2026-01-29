#ifndef INVENTORYDIALOG_H
#define INVENTORYDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>
#include "GameStateManager.h"

class InventoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit InventoryDialog(QWidget *parent = nullptr);
    ~InventoryDialog();
private slots:
    void onEquipButtonClicked();
    void onDropButtonClicked();
    void onInfoButtonClicked();
private:
    void setupUi();
    void initializeItemData();
    void loadInventoryData();
    QTabWidget *tabWidget;
    QListWidget *inventoryList;
    QListWidget *equippedList;
    QListWidget *spellsList;
    QPushButton *equipButton;
    QPushButton *useButton;
    QPushButton *dropButton;
    QPushButton *infoButton;
    // A map to store item descriptions and stats
    QMap<QString, QString> itemInfoMap;
};

#endif // INVENTORYDIALOG_H
