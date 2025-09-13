#ifndef INVENTORYDIALOG_H
#define INVENTORYDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>

class InventoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit InventoryDialog(QWidget *parent = nullptr);
    ~InventoryDialog();

private slots:
    void onEquipButtonClicked(); // The new slot for the equip button

private:
    void setupUi();

    QTabWidget *tabWidget;
    QListWidget *inventoryList;
    QListWidget *equippedList; // Add a member for the equipped list
    QListWidget *spellsList; // Add a member for the spells list
    QPushButton *equipButton;
    QPushButton *useButton;
    QPushButton *dropButton;
    QPushButton *infoButton;
};

#endif // INVENTORYDIALOG_H
