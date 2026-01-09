#ifndef INVENTORYDIALOG_H
#define INVENTORYDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>

#include "GameStateManager.h" // Include the GameStateManager header

class InventoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit InventoryDialog(QWidget *parent = nullptr);
    ~InventoryDialog();
private slots:
    void onEquipButtonClicked(); // The new slot for the equip button
    void onDropButtonClicked(); // The new slot for the drop button
    void onInfoButtonClicked();
private:
    void setupUi();
    void initializeItemData(); // Function to populate item data
    void loadInventoryData();
    QTabWidget *tabWidget;
    QListWidget *inventoryList;
    QListWidget *equippedList; // Add a member for the equipped list
    QListWidget *spellsList; // Add a member for the spells list
    QPushButton *equipButton;
    QPushButton *useButton;
    QPushButton *dropButton;
    QPushButton *infoButton;
    // A map to store item descriptions and stats
    QMap<QString, QString> itemInfoMap;
};

#endif // INVENTORYDIALOG_H
