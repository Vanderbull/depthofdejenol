#include "inventorydialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

InventoryDialog::InventoryDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Inventory");
    setFixedSize(600, 400); // Set a fixed size for the dialog
    setupUi();
}

InventoryDialog::~InventoryDialog() {}

void InventoryDialog::setupUi() {
    // Main layout for the dialog
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // 1. Left side: The tab widget
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    // Create the tabs
    QWidget *tab1 = new QWidget();
    QWidget *tab2 = new QWidget();
    QWidget *tab3 = new QWidget();

    // Add the tabs to the tab widget
    tabWidget->addTab(tab1, "Inventory");
    tabWidget->addTab(tab2, "Equipped");
    tabWidget->addTab(tab3, "Spells");

    // Layout for the main Inventory tab
    QHBoxLayout *inventoryLayout = new QHBoxLayout(tab1);

    // Create a list widget for the inventory
    inventoryList = new QListWidget();
    inventoryLayout->addWidget(inventoryList);

    // 2. Right side: Buttons layout
    QVBoxLayout *buttonsLayout = new QVBoxLayout();
    buttonsLayout->setSpacing(5); // Add some spacing between buttons

    // Create and add the four buttons
    equipButton = new QPushButton("Equip");
    useButton = new QPushButton("Use");
    dropButton = new QPushButton("Drop");
    infoButton = new QPushButton("Info");

    buttonsLayout->addWidget(equipButton);
    buttonsLayout->addWidget(useButton);
    buttonsLayout->addWidget(dropButton);
    buttonsLayout->addWidget(infoButton);

    // Add the vertical buttons layout to the main horizontal layout
    mainLayout->addLayout(buttonsLayout);
}
