#include "ConfinementDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug> 

ConfinementAndHoldingDialog::ConfinementAndHoldingDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Confinement & Holding");
    setupUi();
    setFixedSize(QSize(600, 400)); // Adjust size as needed

    // Connect signals and slots
    connect(bindButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::bindCompanion);
    connect(identifyInfoButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::identifyCompanion); 
    connect(identifySellButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::sellCompanion);
    connect(buyButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::buyCompanion);
    connect(buyInfoButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::showCompanionInfo);
    connect(exitButton, &QPushButton::clicked, this, &QDialog::accept); 
}

ConfinementAndHoldingDialog::~ConfinementAndHoldingDialog()
{
    // Destructor
}

void ConfinementAndHoldingDialog::setupUi()
{
    // Main Layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // --- Left Column Layout (Bind, Identify/Sell) ---
    QVBoxLayout *leftColLayout = new QVBoxLayout();

    // Group Box: Bind Companions
    QGroupBox *bindGroup = new QGroupBox("Bind Companions");
    QVBoxLayout *bindLayout = new QVBoxLayout(bindGroup);
    bindLayout->addWidget(new QLabel("Comp."));
    bindCompLineEdit = new QLineEdit();
    bindLayout->addWidget(bindCompLineEdit);
    bindLayout->addWidget(new QLabel("Cost"));
    bindCostLineEdit = new QLineEdit();
    bindLayout->addWidget(bindCostLineEdit);
    bindButton = new QPushButton("Bind");
    bindLayout->addWidget(bindButton);
    leftColLayout->addWidget(bindGroup);

    // Group Box: Identify, Realign & Sell Companions
    QGroupBox *identifyGroup = new QGroupBox("Identify, Realign & Sell Companions");
    QVBoxLayout *identifyLayout = new QVBoxLayout(identifyGroup);
    identifyLayout->addWidget(new QLabel("Comp."));
    identifyCompLineEdit = new QLineEdit();
    identifyLayout->addWidget(identifyCompLineEdit);
    identifyLayout->addWidget(new QLabel("Value"));
    identifyValueLineEdit = new QLineEdit();
    identifyLayout->addWidget(identifyValueLineEdit);

    QHBoxLayout *identifyButtonsLayout = new QHBoxLayout();
    identifyGneButton = new QPushButton("GNE"); 
    identifyInfoButton = new QPushButton("Info");
    identifySellButton = new QPushButton("Sell");
    identifyIdButton = new QPushButton("ID");
    identifyButtonsLayout->addWidget(identifyGneButton);
    identifyButtonsLayout->addWidget(identifyInfoButton);
    identifyButtonsLayout->addWidget(identifySellButton);
    identifyButtonsLayout->addWidget(identifyIdButton);
    identifyLayout->addLayout(identifyButtonsLayout);

    identifyLayout->addWidget(new QLabel("ID Cost"));
    identifyIdCostLineEdit = new QLineEdit();
    identifyLayout->addWidget(identifyIdCostLineEdit);
    leftColLayout->addWidget(identifyGroup);

    leftColLayout->addStretch(); 

    // --- Right Column Layout (Buy Companions) ---
    QVBoxLayout *rightColLayout = new QVBoxLayout();
    QGroupBox *buyGroup = new QGroupBox("Buy Companions");
    QVBoxLayout *buyLayout = new QVBoxLayout(buyGroup);

    // "G N E" header
    QHBoxLayout *gneHeaderLayout = new QHBoxLayout();
    gneHeaderLayout->addStretch(); 
    gneLabel = new QLabel("G N E");
    QFont font = gneLabel->font();
    font.setBold(true);
    gneLabel->setFont(font);
    gneHeaderLayout->addWidget(gneLabel);
    buyLayout->addLayout(gneHeaderLayout);


    buyCreatureListWidget = new QListWidget();
    // Populate with example data from your image
    buyCreatureListWidget->addItem("Kobold       0 9 0");
    buyCreatureListWidget->addItem("Orc          0 0 3");
    buyCreatureListWidget->addItem("Twisted Dwarf  0 2 0");
    buyCreatureListWidget->addItem("Slave        9 0 3");
    buyCreatureListWidget->addItem("Outcast Goblin 0 0 7");
    buyCreatureListWidget->addItem("Disciple     1 6 0");
    buyCreatureListWidget->addItem("Skulldragl   0 9 0");
    buyCreatureListWidget->addItem("Morloch      0 6 0");
    buyCreatureListWidget->addItem("Slaver       1 0 0");
    buyLayout->addWidget(buyCreatureListWidget);

    buyLayout->addWidget(new QLabel("Companion"));
    buyCompanionLineEdit = new QLineEdit();
    buyLayout->addWidget(buyCompanionLineEdit);
    buyLayout->addWidget(new QLabel("Cost"));
    buyCostLineEdit = new QLineEdit();
    buyLayout->addWidget(buyCostLineEdit);

    QHBoxLayout *buyButtonsLayout = new QHBoxLayout();
    buyButton = new QPushButton("BUY");
    buyInfoButton = new QPushButton("INFO");
    buyButtonsLayout->addStretch(); 
    buyButtonsLayout->addWidget(buyButton);
    buyButtonsLayout->addWidget(buyInfoButton);
    buyLayout->addLayout(buyButtonsLayout);

    buyLayout->addWidget(new QLabel("Search for What?"));
    searchLineEdit = new QLineEdit();
    buyLayout->addWidget(searchLineEdit);

    rightColLayout->addWidget(buyGroup);
    rightColLayout->addStretch(); 

    // --- Add Exit Button to the bottom right ---
    exitButton = new QPushButton("Exit");
    QHBoxLayout *exitButtonLayout = new QHBoxLayout();
    exitButtonLayout->addStretch();
    exitButtonLayout->addWidget(exitButton);
    rightColLayout->addLayout(exitButtonLayout); 

    // Add columns to main layout
    mainLayout->addLayout(leftColLayout);
    mainLayout->addLayout(rightColLayout);
}

// --- Slot Implementations (Placeholder) ---

void ConfinementAndHoldingDialog::bindCompanion()
{
    QMessageBox::information(this, "Bind Companion", "Binding companion: " + bindCompLineEdit->text());
    qDebug() << "Bind Companion: " << bindCompLineEdit->text() << ", Cost: " << bindCostLineEdit->text();
}

void ConfinementAndHoldingDialog::identifyCompanion()
{
    QMessageBox::information(this, "Identify Companion", "Identifying companion: " + identifyCompLineEdit->text());
    qDebug() << "Identify Companion: " << identifyCompLineEdit->text();
}

void ConfinementAndHoldingDialog::sellCompanion()
{
    QMessageBox::information(this, "Sell Companion", "Selling companion: " + identifyCompLineEdit->text());
    qDebug() << "Sell Companion: " << identifyCompLineEdit->text();
}

void ConfinementAndHoldingDialog::buyCompanion()
{
    QMessageBox::information(this, "Buy Companion", "Buying companion: " + buyCompanionLineEdit->text());
    qDebug() << "Buy Companion: " << buyCompanionLineEdit->text();
}

void ConfinementAndHoldingDialog::showCompanionInfo()
{
    QMessageBox::information(this, "Companion Info", "Showing info for companion: " + buyCompanionLineEdit->text());
    qDebug() << "Show Companion Info: " << buyCompanionLineEdit->text();
}

void ConfinementAndHoldingDialog::searchCompanion()
{
    // This would typically filter the buyCreatureListWidget
    qDebug() << "Searching for: " << searchLineEdit->text();
    // Example: loop through list items and hide/show based on text
    for (int i = 0; i < buyCreatureListWidget->count(); ++i) {
        QListWidgetItem *item = buyCreatureListWidget->item(i);
        item->setHidden(!item->text().contains(searchLineEdit->text(), Qt::CaseInsensitive));
    }
}
