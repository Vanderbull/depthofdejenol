#include "ConfinementDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug> 
#include <QFontDatabase>
#include <QSpacerItem> 
#include <QRegularExpressionMatch> // Needed for matching results in Qt 6

// NOTE: GameStateManager.h is included, but the class is not defined here.
// For the buyCompanion() implementation below, we assume GameStateManager 
// is a globally accessible class (e.g., a Singleton) with methods for handling gold.
// e.g., GameStateManager::instance().removeCharacterGold(cost);

ConfinementAndHoldingDialog::ConfinementAndHoldingDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Confinement & Holding");
    
    // Set the Monospace Font for retro/aligned look
    QFont fixedFont;
    fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fixedFont.setPointSize(10); 
    setFont(fixedFont);

    setupUi();
    resize(850, 650); 


    // 1. Declare and initialize the gsm variable
    GameStateManager* gsm = GameStateManager::instance();
    // Check if a Ghost Hound is waiting to be added
// 1. Move pending dungeon exits into persistent stock
    if (gsm->getGameValue("GhostHoundPending").toBool()) {
        gsm->incrementStock("Ghost hounf");
        gsm->setGameValue("GhostHoundPending", false);
    }

// 2. Define the full display strings for each key
/*
    QMap<QString, QString> itemData;
    itemData["Kobold"]          = "Kobold 49216         0  9  0";
    itemData["Orc"]             = "Orc 169993           0  0  3";
    itemData["Clean-Up"]        = "Clean-Up 272431      0  2  0";
    itemData["Black Bear"]      = "Black Bear 75000     0  2  0";
    itemData["Giant Owl"]       = "Giant Owl 70421      0  2  0";
    itemData["Giant Spider"]    = "Giant Spider 109163  0  2  0";
    itemData["Giant Centipede"] = "Giant Centipede 107871 0  2  0";
    itemData["Zombie"]          = "Zombie 223007        0  2  0";
    itemData["Footpad"]         = "Footpad 37471        0  2  0";
    itemData["Gredlan Rogue"]   = "Gredlan Rogue 57586  0  2  0";
    itemData["Ghost hounf"]     = "Ghost hounf 75000    0  2  0";
*/
    // 2. Clear the widget and repopulate from the persistent Stock Map
    buyCreatureListWidget->clear();
    
    QMap<QString, int> currentStock = gsm->getConfinementStock();

    for (auto it = currentStock.begin(); it != currentStock.end(); ++it) {
        QString creatureName = it.key();
        int count = it.value();
        buyCreatureListWidget->addItem(creatureName);
        
        // Add the formatted string to the list 'count' times
        //if (itemData.contains(creatureName)) {
        //    for (int i = 0; i < count; ++i) {
        //        buyCreatureListWidget->addItem(itemData[creatureName]);
        //    }
        //}
    }
    //QMapIterator<QString, int> i(stock);
/*    
    while (i.hasNext()) {
        i.next();
        QString name = i.key();
        int count = i.value();
        
        // Add one row for every instance in stock
        for (int j = 0; j < count; ++j) {
            // Format string based on creature name (you can expand this with a switch or data lookup)
            if (name == "Ghost hounf") {
                buyCreatureListWidget->addItem("Ghost hounf 75000      0  2  0");
            } else if (name == "Skeleton") {
                buyCreatureListWidget->addItem("Skeleton    1500       0  1  0");
            }
        }
    }
*/
    // --- Connect signals and slots ---
    
    // Left Column Connections
    connect(bindButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::bindCompanion);
    
    connect(identifyInfoButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::identifyCompanion); 
    connect(identifyGneButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::identifyCompanionGNE); 
    connect(identifySellButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::sellCompanion);
    connect(identifyIdButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::realignCompanionID); 

    // Right Column Connections
    connect(buyButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::buyCompanion);
    connect(buyInfoButton, &QPushButton::clicked, this, &ConfinementAndHoldingDialog::showCompanionInfo);
    
    // Live Search Connect
    connect(searchLineEdit, &QLineEdit::textChanged, this, &ConfinementAndHoldingDialog::searchCompanion); 
    
    // CONNECTED: Handle list selection to populate buy fields
    connect(buyCreatureListWidget, &QListWidget::itemSelectionChanged, this, &ConfinementAndHoldingDialog::updateBuyFieldsFromList);
    
    // Exit
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
    mainLayout->setSpacing(20); 

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
    identifyValueLineEdit->setReadOnly(true); 
    identifyLayout->addWidget(identifyValueLineEdit);

    // --- Identiy Buttons Layout ---
    QHBoxLayout *identifyButtonsLayout = new QHBoxLayout();
    identifyGneButton = new QPushButton("GNE"); 
    identifyInfoButton = new QPushButton("Info");
    identifySellButton = new QPushButton("Sell");
    identifyIdButton = new QPushButton("ID");
    
    int buttonWidth = 60;
    identifyGneButton->setFixedWidth(buttonWidth);
    identifyInfoButton->setFixedWidth(buttonWidth);
    identifySellButton->setFixedWidth(buttonWidth);
    identifyIdButton->setFixedWidth(buttonWidth);

    identifyButtonsLayout->addStretch(); 
    identifyButtonsLayout->addWidget(identifyGneButton);
    identifyButtonsLayout->addWidget(identifyInfoButton);
    identifyButtonsLayout->addWidget(identifySellButton);
    identifyButtonsLayout->addWidget(identifyIdButton);
    identifyButtonsLayout->addStretch(); 
    
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

    // "G N E" header alignment 
    QHBoxLayout *gneHeaderLayout = new QHBoxLayout();
    QSpacerItem *spacer = new QSpacerItem(300, 1, QSizePolicy::Fixed, QSizePolicy::Minimum); 
    gneHeaderLayout->addItem(spacer); 
    
    gneLabel = new QLabel("G N E");
    QFont boldFont = gneLabel->font();
    boldFont.setBold(true);
    gneLabel->setFont(boldFont);
    gneHeaderLayout->addWidget(gneLabel);
    
    buyLayout->addLayout(gneHeaderLayout);

    // --- List Widget Setup ---
    buyCreatureListWidget = new QListWidget();
    buyCreatureListWidget->setFont(font()); 
    
    // Example data formatted for monospace alignment (Name ID G E N columns)
/*
    buyCreatureListWidget->addItem("Kobold 49216         0  9  0");
    buyCreatureListWidget->addItem("Orc 169993           0  0  3");
    buyCreatureListWidget->addItem("Clean-Up 272431      0  2  0");
    buyCreatureListWidget->addItem("Black Bear 75000     0  2  0");
    buyCreatureListWidget->addItem("Giant Owl 70421      0  2  0");
    buyCreatureListWidget->addItem("Giant Spider 109163  0  2  0");
    buyCreatureListWidget->addItem("Giant Centipede 107871 0  2  0");
    buyCreatureListWidget->addItem("Zombie 223007        0  2  0");
    buyCreatureListWidget->addItem("Footpad 37471        0  2  0");
    buyCreatureListWidget->addItem("Gredlan Rogue 57586  0  2  0");
*/
    buyLayout->addWidget(buyCreatureListWidget);

    buyLayout->addWidget(new QLabel("Companion"));
    buyCompanionLineEdit = new QLineEdit();
    buyLayout->addWidget(buyCompanionLineEdit);
    buyLayout->addWidget(new QLabel("Cost"));
    buyCostLineEdit = new QLineEdit();
    buyCostLineEdit->setReadOnly(true); 
    buyLayout->addWidget(buyCostLineEdit);

    // --- Buy Buttons Layout ---
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

// --- Slot Implementations ---

void ConfinementAndHoldingDialog::bindCompanion()
{
    QMessageBox::information(this, "Bind Companion", "Binding companion: " + bindCompLineEdit->text());
    qDebug() << "Bind Companion: " << bindCompLineEdit->text() << ", Cost: " << bindCostLineEdit->text();
}

void ConfinementAndHoldingDialog::identifyCompanion()
{
    QMessageBox::information(this, "Identify Companion", "Identifying companion: " + identifyCompLineEdit->text());
    qDebug() << "Identify Companion: " << identifyCompLineEdit->text();
    // Placeholder: populate identifyValueLineEdit with the identified value
    identifyValueLineEdit->setText("Identified Value: 100g 50e 20n"); 
}

void ConfinementAndHoldingDialog::identifyCompanionGNE()
{
    QMessageBox::information(this, "Identify G N E", "Calculating G N E for: " + identifyCompLineEdit->text());
}

void ConfinementAndHoldingDialog::sellCompanion()
{
    QMessageBox::information(this, "Sell Companion", "Selling companion: " + identifyCompLineEdit->text());
    qDebug() << "Sell Companion: " << identifyCompLineEdit->text();
}

void ConfinementAndHoldingDialog::realignCompanionID()
{
    QMessageBox::information(this, "Realign ID", "Attempting to realign ID for: " + identifyCompLineEdit->text() + " at cost: " + identifyIdCostLineEdit->text());
}

void ConfinementAndHoldingDialog::buyCompanion()
{
    // 1. Declare the variable at the start of the function
    QString selectedCreature = buyCompanionLineEdit->text().trimmed();

    if (selectedCreature.isEmpty()) {
        return;
    }

    // ... (Your gold check and purchase logic here) ...

    // 2. Use 'selectedCreature' in your success message and stock update
    GameStateManager::instance()->decrementStock(selectedCreature);

    qDebug() << "Successfully bought companion: " + selectedCreature;

    // 3. UI Cleanup
    delete buyCreatureListWidget->currentItem();
    buyCompanionLineEdit->clear();
    buyCostLineEdit->clear();
}

void ConfinementAndHoldingDialog::showCompanionInfo()
{
    QMessageBox::information(this, "Companion Info", "Showing info for companion: " + buyCompanionLineEdit->text());
    qDebug() << "Show Companion Info: " << buyCompanionLineEdit->text();
}

void ConfinementAndHoldingDialog::searchCompanion()
{
    QString searchText = searchLineEdit->text().trimmed();
    qDebug() << "Searching for: " << searchText;
    
    for (int i = 0; i < buyCreatureListWidget->count(); ++i) {
        QListWidgetItem *item = buyCreatureListWidget->item(i);
        bool matches = item->text().contains(searchText, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

void ConfinementAndHoldingDialog::updateBuyFieldsFromList()
{
    QList<QListWidgetItem*> selectedItems = buyCreatureListWidget->selectedItems();
    
    if (selectedItems.isEmpty()) {
        buyCompanionLineEdit->clear();
        buyCostLineEdit->clear();
        return;
    }
    
    QListWidgetItem *selectedItem = selectedItems.first();
    QString fullText = selectedItem->text().trimmed();

    // Use QRegularExpression (Qt 6 Standard) to robustly capture the name and ID/Cost.
    // Pattern: ^(.+?)\s+(\d+)\s+
    // Captures the Name (1) and the ID/Cost (2) up to the first space before G N E.
    QRegularExpression rx("^(.+?)\\s+(\\d+)\\s+");
    QRegularExpressionMatch match = rx.match(fullText);
    
    QString companionName;
    QString companionCost;

    if (match.hasMatch()) {
        companionName = match.captured(1).trimmed(); // Capture 1: The Name
        companionCost = match.captured(2);           // Capture 2: The ID/Cost number
    } else {
         // Fallback/Error handling if regex fails (e.g. unexpected list format)
         QStringList parts = fullText.split(' ', Qt::SkipEmptyParts);
         if (parts.size() >= 2) {
             companionName = parts.first();
             companionCost = parts.at(1);
         } else {
             companionName = fullText;
             companionCost = "";
         }
    }

    // Update the QLineEdits
    buyCompanionLineEdit->setText(companionName);
    buyCostLineEdit->setText(companionCost);
    
    qDebug() << "Selected: " << companionName << ", Cost: " << companionCost;
}

void ConfinementAndHoldingDialog::addGhostHoundOnExit()
{
    // Add the specific string requested to the buy list
    buyCreatureListWidget->addItem("Ghost hounf 75000     0  2  0");
    
    // Optional: Log for debugging
    qDebug() << "Character exited dungeon: Ghost Hound added to Confinement stock.";
}
