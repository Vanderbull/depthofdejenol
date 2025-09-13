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

private:
    void setupUi();

    QTabWidget *tabWidget;
    QListWidget *inventoryList;
    QPushButton *equipButton;
    QPushButton *useButton;
    QPushButton *dropButton;
    QPushButton *infoButton;
};

#endif // INVENTORYDIALOG_H
