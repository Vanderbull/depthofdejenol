#include "GeneralStore.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDebug>
#include <QStringList>
#include <QLabel>
#include <QVariant>
#include <QMap>
#include <QMessageBox> // Included for the feedback dialog

GeneralStore::GeneralStore(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("General Store");
    setupUi();
    populateBuyItemsList();

    // Connect signals and slots
    connect(uncurseButton, &QPushButton::clicked, this, &GeneralStore::uncurseItem);
    connect(combineButton, &QPushButton::clicked, this, &GeneralStore::combineItems);
    connect(sellButton, &QPushButton::clicked, this, &GeneralStore::identifySellItem); // Using one slot for ID/Sell
    connect(idButton, &QPushButton::clicked, this, &GeneralStore::identifySellItem);
    connect(infoButton, &QPushButton::clicked, this, &GeneralStore::showItemInfo);
    connect(buyButton, &QPushButton::clicked, this, &GeneralStore::buyItem);
    connect(buyInfoButton, &QPushButton::clicked, this, &GeneralStore::showItemInfo);
    connect(searchItemsLineEdit, &QLineEdit::textChanged, this, &GeneralStore::searchItems);
    connect(exitButton, &QPushButton::clicked, this, &GeneralStore::exitStore);

    // MODIFIED LOGIC: Update cost by extracting 'price' from the QVariantMap
    connect(buyItemsListWidget, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *current, QListWidgetItem *previous) {
        Q_UNUSED(previous);
        if (current) {
            // Retrieve the QVariantMap stored in Qt::UserRole
            QVariantMap itemMap = current->data(Qt::UserRole).toMap();
            
            // Extract the 'price' value using the map key
            QString priceString = itemMap.value("price").toString(); 
            
            // Update the cost label and line edit
            buyItemCostLabel->setText(QString("Cost: %1").arg(priceString)); 
            buyItemLineEdit->setText(current->text());
        } else {
            buyItemCostLabel->setText("Cost: 0");
            buyItemLineEdit->clear();
        }
    });
}

// --- UI Setup ---

void GeneralStore::setupUi()
{
    // Initialize UI elements
    uncurseItemLineEdit = new QLineEdit();
    uncurseButton = new QPushButton("UNCURSE");

    combineItemLineEdit = new QLineEdit();
    combineItemsLineEdit = new QLineEdit();
    combineButton = new QPushButton("COMBINE");

    identifySellItemLineEdit = new QLineEdit();
    itemValueLabel = new QLabel("Value: 0");
    infoButton = new QPushButton("INFO");
    sellButton = new QPushButton("SELL");
    idButton = new QPushButton("ID");
    idCostLabel = new QLabel("ID Cost: 100");

    buyItemsListWidget = new QListWidget();
    searchItemsLineEdit = new QLineEdit();
    searchItemsLineEdit->setPlaceholderText("Search items...");
    buyItemLineEdit = new QLineEdit();
    buyItemLineEdit->setReadOnly(true); // Typically read-only, shows selected item
    buyItemCostLabel = new QLabel("Cost: 0");
    buyButton = new QPushButton("BUY");
    buyInfoButton = new QPushButton("INFO");
    exitButton = new QPushButton("EXIT");

    // Layout for "Uncurse Items"
    QGroupBox *uncurseBox = new QGroupBox("Uncurse Items");
    QHBoxLayout *uncurseLayout = new QHBoxLayout();
    uncurseLayout->addWidget(new QLabel("Item:"));
    uncurseLayout->addWidget(uncurseItemLineEdit);
    uncurseLayout->addWidget(uncurseButton);
    uncurseBox->setLayout(uncurseLayout);

    // Layout for "Combine Items"
    QGroupBox *combineBox = new QGroupBox("Combine Items");
    QGridLayout *combineGridLayout = new QGridLayout();
    combineGridLayout->addWidget(new QLabel("Item A:"), 0, 0);
    combineGridLayout->addWidget(combineItemLineEdit, 0, 1);
    combineGridLayout->addWidget(new QLabel("Item B:"), 1, 0);
    combineGridLayout->addWidget(combineItemsLineEdit, 1, 1);
    combineGridLayout->addWidget(combineButton, 0, 2, 2, 1); // Span 2 rows
    combineBox->setLayout(combineGridLayout);

    // Layout for "Identify, Realign & Sell Items"
    QGroupBox *idSellBox = new QGroupBox("Identify, Realign & Sell Items");
    QGridLayout *idSellGridLayout = new QGridLayout();
    idSellGridLayout->addWidget(new QLabel("Item Name/ID:"), 0, 0);
    idSellGridLayout->addWidget(identifySellItemLineEdit, 0, 1, 1, 3); // Span 3 columns
    idSellGridLayout->addWidget(itemValueLabel, 1, 0, 1, 2); // Span 2 columns
    idSellGridLayout->addWidget(idCostLabel, 1, 2, 1, 2); // Span 2 columns
    idSellGridLayout->addWidget(infoButton, 2, 0);
    idSellGridLayout->addWidget(idButton, 2, 1);
    idSellGridLayout->addWidget(sellButton, 2, 2);
    idSellBox->setLayout(idSellGridLayout);

    // Layout for "Buy Items"
    QGroupBox *buyBox = new QGroupBox("Buy Items");
    QVBoxLayout *buyLayout = new QVBoxLayout();
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Search:"));
    searchLayout->addWidget(searchItemsLineEdit);
    buyLayout->addLayout(searchLayout);
    buyLayout->addWidget(buyItemsListWidget);
    QHBoxLayout *buyDetailsLayout = new QHBoxLayout();
    buyDetailsLayout->addWidget(buyItemLineEdit);
    buyDetailsLayout->addWidget(buyItemCostLabel);
    buyLayout->addLayout(buyDetailsLayout);
    QHBoxLayout *buyControlsLayout = new QHBoxLayout();
    buyControlsLayout->addWidget(buyInfoButton);
    buyControlsLayout->addWidget(buyButton);
    buyLayout->addLayout(buyControlsLayout);
    buyBox->setLayout(buyLayout);

    // Main Layout (Top half for all services, Bottom for Buy/Exit)
    QHBoxLayout *topLayout = new QHBoxLayout();
    QVBoxLayout *leftColumn = new QVBoxLayout();
    leftColumn->addWidget(uncurseBox);
    leftColumn->addWidget(combineBox);
    leftColumn->addWidget(idSellBox);
    topLayout->addLayout(leftColumn);
    topLayout->addWidget(buyBox);

    // Final main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(exitButton);

    setLayout(mainLayout);
    resize(800, 600); // Set a reasonable default size
}

GeneralStore::~GeneralStore()
{
    // Destructor is empty if all members are managed by Qt's parent/child mechanism
}

// --- Item Data Population ---

void GeneralStore::populateBuyItemsList()
{
    // The QStringList array to hold all item data from MDATA3.csv.
    // Each string represents one item/row.
    QStringList buyItemData;
    buyItemData << "Hands,0,0,0,0,0,0,0,2,0,0,0,0,4095,0,0.5,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Gnarled Hands,1,-3,-3,255671,1,25,0,1,0,0,0,0,8,0,1,1,2,0,0,10,0,0,0,0,6,0,0,0,0,0,0,1,0,-1";
    buyItemData << "Initiate's Hands,2,3,0,2512764,1,50,0,2,0,0,0,0,8,3,1.2,10,2,0,0,6,0,0,0,0,6,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Magic Hands,3,6,0,12433122,4,40,0,3,0,0,0,0,8,8,1.3,10,2,0,0,6,0,0,0,0,6,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Master's Hand,4,9,0,23433232,6,25,0,3,0,0,0,0,8,15,1.4,10,2,0,0,6,0,0,0,0,6,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Hands of Death,5,12,0,45344232,9,15,16,4,0,0,0,0,8,25,1.5,10,2,0,0,6,0,0,0,0,6,0,0,0,0,0,0,1,0,-1";
    buyItemData << "Hands of Slaying,6,15,0,75899432,12,10,16,5,0,0,0,0,8,35,1.6,10,2,0,0,6,0,0,0,0,6,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Hands of Destruction,7,21,0,158766432,15,1,16,6,0,0,0,0,8,45,1.8,10,2,0,0,6,0,0,0,0,6,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Bronze Sword,8,3,0,350,0,51,0,1,0,0,0,0,191,0,1.1,1,1,3,0,6,0,0,0,0,6,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Iron Sword,9,3,0,56598,1,50,0,1,0,0,0,0,191,2,1.3,14,1,3,0,10,0,0,0,0,10,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Backlash Sword,10,-3,0,544351,1,15,0,1,0,0,0,0,191,1,0.1,1,2,3,0,10,0,0,0,0,10,0,0,0,0,0,1,0,0,-1";
    buyItemData << "Thief Sword,11,6,0,7232593,2,2,32,2,0,0,0,0,217,5,1.5,14,1,3,0,12,0,0,0,0,18,0,0,0,0,0,1,0,0,-1";
    buyItemData << "Katana,12,6,0,8322132,2,2,16,2,0,0,0,0,8,5,1.6,10,2,3,0,13,0,14,0,0,18,0,0,0,0,0,1,0,0,-1";
    buyItemData << "Crystal Sword,13,6,6,3541231,4,3,0,2,0,0,0,0,191,9,1.5,14,1,3,0,14,0,0,0,0,14,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Steel Sword,14,6,0,1366742,4,25,0,2,0,0,0,0,191,8,1.4,14,1,3,0,12,0,0,0,0,12,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Dominator,15,6,6,9845672,4,1,16,2,0,0,0,0,191,15,2,14,2,3,0,16,12,12,14,0,16,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Adamantite Sword,16,9,3,5798677,5,30,0,2,0,0,0,0,191,15,1.5,14,1,3,0,14,0,0,0,0,14,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Mithril Sword,17,12,3,18965342,7,15,16,3,0,0,0,0,191,25,1.6,14,1,3,0,18,0,0,16,0,18,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Avenger,18,15,3,32678540,7,1,16,3,0,0,0,0,63,30,2.3,14,2,3,0,18,16,16,18,0,18,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Sword of Slaying,19,15,6,45321784,11,15,16,3,0,0,0,0,191,40,1.7,14,1,3,0,18,14,14,18,0,18,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Lightning Blade,20,24,6,166745328,15,3,0,4,0,78,65,35,55,60,1.8,14,1,3,0,20,18,18,20,0,20,0,0,0,0,0,0,0,225,-1";
    buyItemData << "Eliminator,21,27,9,303566432,15,1,16,4,0,0,0,0,2,80,2.6,14,2,3,0,22,18,18,20,0,22,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Bronze Battle Axe,22,3,0,345,0,1,0,1,0,0,0,0,179,0,1.3,1,2,6,0,10,0,0,0,0,10,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Axe of the Blackguard,23,-3,0,543321,1,15,0,1,0,0,0,0,179,1,0.1,1,2,6,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,1,0,-1";
    buyItemData << "Iron Battle Axe,24,3,0,55675,2,45,0,1,0,0,0,0,179,1,1.4,14,2,6,0,12,0,0,0,0,12,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Steel Battle Axe,25,6,0,1433231,4,35,0,2,0,0,0,0,179,9,1.5,14,2,6,0,14,0,0,0,0,14,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Adamantite Battle Axe,26,9,0,5654326,5,25,0,2,0,0,0,0,179,15,1.7,14,2,6,0,14,0,0,0,0,14,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Mithril Battle Axe,27,12,0,19655340,7,20,16,3,0,0,0,0,179,25,1.8,14,2,6,0,16,0,0,0,0,16,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Axe of Power,28,15,0,45344312,11,10,16,3,0,0,0,0,179,40,1.9,14,2,6,0,18,16,16,0,0,18,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Shadow Axe,29,21,0,124322432,15,3,80,4,0,84,69,45,179,55,1.8,14,2,6,0,20,18,18,18,0,20,0,0,0,0,0,0,0,225,-1";
    buyItemData << "Barrier Battle Axe,30,21,18,278566432,15,1,16,4,0,0,0,0,179,75,2.2,14,2,6,0,22,18,18,20,0,22,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Bronze Dagger,31,3,0,250,0,65,0,1,0,0,0,0,4095,0,1,1,1,1,0,4,0,0,0,0,6,0,0,0,0,0,0,0,0,0";
    buyItemData << "Bone Dagger,32,-3,-3,322132,1,25,0,1,0,0,0,0,4095,1,0.1,1,1,1,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,1,0,0";
    buyItemData << "Iron Dagger,33,3,0,23377,1,50,0,1,0,0,0,0,4095,1,1.2,14,1,1,0,10,0,0,0,0,10,0,0,0,0,0,0,0,0,0";
    buyItemData << "Lethe Dagger,34,6,0,3265432,2,2,0,2,0,0,0,0,4095,6,1.3,14,1,1,0,13,0,0,0,0,14,0,0,0,0,0,0,0,0,0";
    buyItemData << "Steel Dagger,35,6,0,1025674,3,40,0,2,0,0,0,0,4095,6,1.3,14,1,1,0,12,0,0,0,0,15,0,0,0,0,0,0,0,0,0";
    buyItemData << "Adamantite Dagger,36,9,0,4556231,5,30,0,2,0,0,0,0,4095,12,1.4,14,1,1,0,12,0,0,0,0,14,0,0,0,0,0,0,0,0,0";
    buyItemData << "Mithril Dagger,37,12,0,14566431,6,20,32,3,0,0,0,0,4095,23,1.5,14,1,1,0,14,0,0,0,0,16,0,0,0,0,0,0,0,0,0";
    buyItemData << "Sabre Blade,38,15,3,46533420,10,15,32,4,0,0,0,0,4095,32,1.6,14,1,1,0,15,16,16,16,0,18,0,0,0,0,0,0,0,0,0";
    buyItemData << "Vampire Fang,39,21,0,87655432,13,10,32,4,0,0,0,0,4095,35,1.7,14,1,1,0,18,16,16,0,0,20,0,0,0,0,0,0,0,0,0";
    buyItemData << "Dagger of Swiftness,40,21,9,198655744,15,1,32,5,0,0,0,0,4095,60,1.8,14,1,1,0,18,16,16,0,0,20,0,0,0,0,0,0,0,0,0";
    buyItemData << "Bronze Mace,41,3,0,325,0,45,0,1,0,0,0,0,167,0,1.1,1,1,5,0,10,0,0,0,0,10,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Mace of Kordul,42,-3,0,544341,1,10,0,1,0,0,0,0,4095,1,0.1,1,1,5,0,10,0,0,10,0,10,0,0,0,0,0,0,1,0,-1";
    buyItemData << "Iron Mace,43,3,0,34612,2,45,0,1,0,0,0,0,167,2,1.3,14,1,5,0,12,0,0,10,0,12,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Steel Mace,44,6,0,1896744,4,35,0,2,0,0,0,0,167,7,1.4,14,1,5,0,16,0,0,0,0,16,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Adamantite Mace,45,9,0,4566756,5,25,0,2,0,0,0,0,167,15,1.6,14,1,5,0,16,0,0,0,0,16,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Mithril Mace,46,12,0,16877644,7,15,16,3,0,0,0,0,167,25,1.7,14,1,5,0,16,0,0,0,0,16,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Mace of Disruption,47,15,0,67455320,13,10,16,3,0,0,0,0,167,50,1.8,14,1,5,0,18,0,0,18,0,18,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Warlord's Mace,48,21,0,178677536,15,2,16,3,0,0,0,0,167,65,1.9,14,1,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Leather Armor,49,0,3,125,0,1,0,0,0,0,0,0,4095,0,1,1,0,8,0,3,0,0,0,0,5,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Padded Leather Armor,50,0,6,367455,1,40,0,0,0,0,0,0,4095,1,1,14,0,8,0,10,0,0,0,0,10,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Suede Leather Armor,51,0,9,567344,2,35,0,0,0,0,0,0,4095,5,1.4,14,0,8,0,10,0,0,0,0,10,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Studded Leather Armor,52,0,12,3455321,3,27,0,0,0,0,0,0,4095,10,1.6,14,0,8,0,12,0,0,0,0,12,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Cuirboulli Armor,53,0,15,6544341,7,20,0,0,0,0,0,0,4095,20,2.1,14,0,8,0,14,0,0,0,0,14,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Studded Cuirboulli,54,0,18,8345623,9,10,0,0,0,0,0,0,4095,30,2.3,14,0,8,0,16,0,0,12,0,16,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Thief's Armor,55,0,33,234544320,15,1,0,0,0,0,0,0,192,60,2.9,14,0,8,0,6,6,0,9,0,10,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Ninja's Armor,56,0,24,198566432,15,1,0,0,0,0,0,0,4095,60,2.9,14,0,8,0,8,0,0,8,0,10,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Magician's Armor,57,0,45,206566432,15,1,0,0,0,0,0,0,3840,60,2.9,14,0,8,0,10,10,0,10,0,10,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Bronze Chain Mail,58,0,9,18325,0,54,0,0,0,0,0,0,183,0,1.00259,1,0,9,0,10,0,0,0,0,10,0,0,0,0,0,-1,0,0,-1";
    buyItemData << "Iron Chain Mail,59,0,15,506433,1,50,0,0,0,0,0,0,183,5,1.4,14,0,9,0,12,0,0,0,0,12,0,0,0,0,0,-1,0,0,-1";
    buyItemData << "Black Chain Mail,60,0,-9,1544321,1,15,0,0,0,0,0,0,183,3,1.2,1,0,9,0,0,0,0,0,0,0,0,0,0,0,-2,1,0,-1";
    buyItemData << "Steel Chain Mail,61,0,18,3455645,3,35,0,0,0,0,0,0,183,10,1.6,14,0,9,0,14,0,0,0,0,14,0,0,0,0,0,-1,0,0,-1";
    buyItemData << "Adamantite Chain,62,0,21,8235123,5,25,0,0,0,0,0,0,183,23,2.1,14,0,9,0,15,6,0,6,0,15,0,0,0,0,0,-1,0,0,-1";
    buyItemData << "Mithril Chain Mail,63,0,24,19455320,7,15,0,0,0,0,0,0,183,35,2.5,14,0,9,0,16,6,0,6,0,16,0,0,0,0,0,-1,0,0,-1";
    buyItemData << "Spectral Chain Mail,64,0,27,78455640,11,2,0,0,0,0,0,0,183,50,2.7,14,0,9,0,9,6,0,9,0,6,0,0,0,0,0,-1,0,0,-1";
    buyItemData << "Bronze Plate Mail,65,6,9,21825,0,1,0,0,0,0,0,0,39,0,1,1,0,10,0,10,0,0,0,0,10,0,0,0,0,0,-2,0,0,-1";
    buyItemData << "Ulsik Plate Mail,66,-3,-9,2455341,1,15,0,0,0,0,0,0,39,0,1,1,0,10,0,12,9,0,12,0,9,0,0,0,0,0,-3,1,0,-1";
    buyItemData << "Iron Plate Mail,67,6,15,1567433,2,40,0,0,0,0,0,0,39,5,1.4,14,0,10,0,12,0,0,0,0,12,0,0,0,0,0,-2,0,0,-1";
    buyItemData << "Steel Plate Mail,68,9,15,8677435,4,25,0,0,0,0,0,0,39,10,1.6,14,0,10,0,14,0,0,0,0,14,0,0,0,0,0,-2,0,0,-1";
    buyItemData << "Adamantite Plate,69,12,18,12543212,6,15,0,0,0,0,0,0,39,23,2.1,14,0,10,0,16,9,0,12,0,16,0,0,0,0,0,-2,0,0,-1";
    buyItemData << "Mithril Plate Armor,70,15,21,25677432,9,5,0,0,0,0,0,0,39,35,2.5,14,0,10,0,18,9,0,12,0,18,0,0,0,0,0,-2,0,0,-1";
    buyItemData << "Spectral Plate Armor,71,18,24,48344320,13,2,0,0,0,0,0,0,39,50,2.7,14,0,10,0,18,9,0,12,0,18,0,0,0,0,0,-2,0,0,-1";
    buyItemData << "Warrior's Armor,72,21,30,256433232,15,1,0,0,0,0,0,0,3,75,3.1,14,0,10,0,20,9,0,18,0,20,0,0,0,0,0,-2,0,0,-1";
    buyItemData << "Paladin's Armor,73,6,33,287455328,15,1,0,0,0,0,0,0,39,70,3,14,0,10,0,14,6,0,9,0,14,0,0,0,0,0,-2,0,0,-1";
    buyItemData << "Copper Helm,74,0,3,75,0,30,0,0,0,0,0,0,183,0,1,1,0,13,0,2,0,0,0,0,3,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Bronze Helmet,75,0,6,12435,0,41,0,0,0,0,0,0,183,1,1,1,0,13,0,9,0,0,0,0,8,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Iron Helmet,76,0,6,89456,2,50,0,0,0,0,0,0,183,3,1.2,14,0,13,0,10,0,0,0,0,10,0,1,0,0,0,0,0,0,-1";
    buyItemData << "Steel Helmet,77,0,9,873232,3,35,0,0,0,0,0,0,183,7,1.5,14,0,13,0,12,0,0,0,0,12,0,1,0,0,0,0,0,0,-1";
    buyItemData << "Helm of Astral Projection,78,0,3,5213451,5,2,0,0,0,80,66,10,4095,10,1.6,14,0,13,0,14,16,16,0,0,14,0,0,0,0,0,0,0,75,-1";
    buyItemData << "Adamantite Helmet,79,0,12,3265428,5,15,8,0,0,0,0,0,183,15,1.9,14,0,13,0,12,0,0,0,0,12,0,1,0,0,0,0,0,0,-1";
    buyItemData << "Helm of Sight,80,0,6,10326543,6,15,8,0,0,0,0,0,4095,17,1.9,14,0,13,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,-1";
    buyItemData << "Mithril Helmet,81,3,18,27154298,7,10,8,0,0,0,0,0,183,25,2.2,14,0,13,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,-1";
    buyItemData << "Helm of Knowledge,82,6,6,89566432,12,2,8,0,0,0,0,0,4095,40,2.6,14,0,13,0,14,18,18,16,0,16,0,3,3,0,0,0,0,0,-1";
    buyItemData << "Helm of Brilliance,83,12,24,128455328,14,2,8,0,0,78,65,40,183,55,2.8,14,0,13,0,18,18,18,16,0,18,0,1,1,0,0,0,0,210,-1";
    buyItemData << "Cross of Redemption,84,-6,-3,784532,1,10,0,1,0,0,0,0,4095,2,0.1,1,1,2,0,10,16,16,0,0,15,0,0,0,0,0,0,1,0,-1";
    buyItemData << "Silver Cross,85,3,3,5516133,2,5,0,2,0,0,0,0,3840,2,1.5,14,1,2,0,12,14,14,12,0,12,0,0,0,0,0,0,0,0,-1";
    buyItemData << "Cross of Life,86,3,3,13237651,6,3,0,1,0,59,49,2,4095,0,1,14,1,2,0,0,7,17,9,0,10,0,0,0,0,0,0,0,90,-1";
    buyItemData << "Cross of Commanding,87,6,6,12386539,7,20,0,4,0,45,37,42,3840,15,1.7,14,1,2,0,14,16,16,14,0,16,0,0,0,0,0,0,0,90,-1";

    // Define the headers to be used as keys
    QStringList headers;
    headers << "name" << "ID" << "att" << "def" << "price" << "floor" << "rarity" << "abilities" << "swings" << "specialType" << "spellIndex" << "spellID" << "charges" << "guilds" << "levelScale" << "damageMod" << "alignmentFlags" << "nHands" << "type" << "resistanceFlags" << "StrReq" << "IntReq" << "WisReq" << "ConReq" << "ChaReq" << "DexReq" << "StrMod" << "IntMod" << "WisMod" << "ConMod" << "ChaMod" << "DexMod" << "cursed" << "spellLvl" << "classRestricted";


    // Loop through the data array to populate the QListWidget
    for (const QString &itemData : buyItemData) {
        QStringList values = itemData.split(",");
        
        // Safety check for data integrity
        if (values.size() != headers.size()) {
            qDebug() << "Warning: Item data does not match header count for:" << itemData;
            continue; // Skip malformed rows
        }

        QVariantMap itemMap;
        for (int i = 0; i < headers.size(); ++i) {
            // Store all values as QVariants (QString) in the map
            itemMap.insert(headers.at(i), values.at(i));
        }

        // Extract the item name
        QString name = itemMap.value("name").toString();
        
        // Create a new list widget item with the name
        QListWidgetItem *item = new QListWidgetItem(name);
        
        // Store the QVariantMap in Qt::UserRole
        item->setData(Qt::UserRole, itemMap); 
        
        buyItemsListWidget->addItem(item);
    }
}

// --- Feedback Helper ---
void GeneralStore::showFeedbackDialog(const QString &title, const QString &message, QMessageBox::Icon icon)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.exec();
}

// --- Slot Implementations (Updated to use QMessageBox) ---

void GeneralStore::uncurseItem()
{
    QString itemName = uncurseItemLineEdit->text().trimmed();
    
    if (itemName.isEmpty()) {
        showFeedbackDialog("Uncurse Error", "Please enter the name of the item you wish to uncurse.", QMessageBox::Warning);
        return;
    }

    // Placeholder Logic:
    // 1. Check if the player has the item named 'itemName'.
    // 2. Check if the item is actually cursed (cursed flag == 1).
    // 3. Deduct the uncursing cost.
    // 4. Update the item's 'cursed' flag to 0.

    // Simulate success/failure based on a simple check
    if (itemName.toLower() == "gnarled hands") {
        showFeedbackDialog("Uncurse Success", 
                           QString("Successfully removed the curse from '%1'. It is now safe to use.").arg(itemName),
                           QMessageBox::Information);
    } else {
        showFeedbackDialog("Uncurse Status", 
                           QString("Failed to uncurse '%1'. Either you don't own it, it's not cursed, or the process was too expensive.").arg(itemName),
                           QMessageBox::Critical);
    }
}

void GeneralStore::combineItems()
{
    QString itemA = combineItemLineEdit->text().trimmed();
    QString itemB = combineItemsLineEdit->text().trimmed();
    
    if (itemA.isEmpty() || itemB.isEmpty()) {
        showFeedbackDialog("Combine Error", "Please enter two item names to attempt a combination.", QMessageBox::Warning);
        return;
    }

    // Placeholder Logic:
    // 1. Check if the player has both 'itemA' and 'itemB'.
    // 2. Check a combination recipe database.
    
    // Simulate combination based on specific inputs
    if ((itemA.toLower().contains("steel sword") && itemB.toLower().contains("crystal")) || 
        (itemB.toLower().contains("steel sword") && itemA.toLower().contains("crystal"))) {
        showFeedbackDialog("Combine Success", 
                           QString("The combination of '%1' and '%2' created a magnificent Crystal Steel Blade!").arg(itemA).arg(itemB),
                           QMessageBox::Information);
    } else {
        showFeedbackDialog("Combine Failure", 
                           QString("The items '%1' and '%2' did not combine. No known recipe exists or they are incompatible.").arg(itemA).arg(itemB),
                           QMessageBox::Warning);
    }
}

void GeneralStore::identifySellItem()
{
    QString itemName = identifySellItemLineEdit->text().trimmed();
    if (itemName.isEmpty()) return;

    GameStateManager* gsm = GameStateManager::instance();
    int activeCharIndex = gsm->getGameValue("ActiveCharacterIndex").toInt();
    
    // 1. Verify item is in Character Inventory
    QVariantList party = gsm->getGameValue("Party").toList();
    QVariantMap character = party[activeCharIndex].toMap();
    QStringList inventory = character["Inventory"].toStringList();

    int invIndex = -1;
    for (int i = 0; i < inventory.size(); ++i) {
        if (inventory[i].compare(itemName, Qt::CaseInsensitive) == 0) {
            invIndex = i;
            break;
        }
    }

    if (invIndex == -1) {
        showFeedbackDialog("Error", "Item not found in your inventory.", QMessageBox::Warning);
        return;
    }

    // 2. Find Item Value in Database (m_itemData)
    qulonglong itemValue = 0;
    const QList<QVariantMap>& db = gsm->itemData();
    for (const QVariantMap& item : db) {
        if (item["name"].toString().compare(itemName, Qt::CaseInsensitive) == 0) {
            // Based on populateBuyItemsList, price is the 5th column
            itemValue = item["price"].toULongLong(); 
            break;
        }
    }

    // 3. Process Transaction
    inventory.removeAt(invIndex);
    gsm->setCharacterInventory(activeCharIndex, inventory); // Remove item
    gsm->updateCharacterGold(activeCharIndex, itemValue, true); // Add gold

    showFeedbackDialog("Sold", QString("Sold %1 for %2 gold.").arg(itemName).arg(itemValue));
    identifySellItemLineEdit->clear();
}

void GeneralStore::buyItem()
{
    if (buyItemsListWidget->currentItem()) {
        QString selectedItemName = buyItemsListWidget->currentItem()->text();
        QVariantMap itemMap = buyItemsListWidget->currentItem()->data(Qt::UserRole).toMap();
        QString priceString = itemMap.value("price").toString();
        
        // Placeholder Logic: Assume player has enough money.
        
        showFeedbackDialog("Purchase Confirmation", 
                           QString("You have purchased the '%1' for %2 gold. It has been added to your inventory!").arg(selectedItemName).arg(priceString),
                           QMessageBox::Information);

    } else {
        showFeedbackDialog("Purchase Error", "Please select an item from the list to buy.", QMessageBox::Warning);
    }
}

void GeneralStore::showItemInfo()
{
    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    QString infoMessage;
    QString itemName;
    
    if (senderButton == infoButton) {
        itemName = identifySellItemLineEdit->text().trimmed();
        if (itemName.isEmpty()) {
            showFeedbackDialog("Info Request Error", "Please enter an item name/ID to view its details.", QMessageBox::Warning);
            return;
        }
        infoMessage = QString("Displaying full known details for item '%1'.\n\n[Placeholder: All item stats, effects, and requirements would be listed here.]").arg(itemName);
        
    } else if (senderButton == buyInfoButton) {
        if (buyItemsListWidget->currentItem()) {
            itemName = buyItemsListWidget->currentItem()->text();
            QVariantMap itemMap = buyItemsListWidget->currentItem()->data(Qt::UserRole).toMap();
            
            // Generate a simple, formatted info string from the map
            infoMessage = QString("--- %1 Details ---\n").arg(itemName);
            for (auto it = itemMap.constBegin(); it != itemMap.constEnd(); ++it) {
                // Skip the name since it's in the title
                if (it.key() == "name") continue; 
                infoMessage += QString("%1: %2\n").arg(it.key()).arg(it.value().toString());
            }
        } else {
            showFeedbackDialog("Info Request Error", "Please select an item from the list to view its details.", QMessageBox::Warning);
            return;
        }
    }
    
    if (!infoMessage.isEmpty()) {
        showFeedbackDialog(QString("%1 Information").arg(itemName), infoMessage, QMessageBox::NoIcon);
    }
}

void GeneralStore::searchItems(const QString &searchText)
{
    // The filtering logic remains the same as it doesn't need a dialog on every key press.
    QString lowerSearchText = searchText.toLower();

    for (int i = 0; i < buyItemsListWidget->count(); ++i) {
        QListWidgetItem *item = buyItemsListWidget->item(i);
        bool matchFound = false;

        // Check against the visible name first
        if (item->text().contains(searchText, Qt::CaseInsensitive)) {
            matchFound = true;
        } else {
            // Retrieve the QVariantMap
            QVariantMap itemMap = item->data(Qt::UserRole).toMap();
            
            // Iterate over all values in the map for a match
            for (const QVariant &value : itemMap.values()) {
                if (value.toString().toLower().contains(lowerSearchText)) {
                    matchFound = true;
                    break;
                }
            }
        }
        
        item->setHidden(!matchFound);
    }
}

void GeneralStore::exitStore()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Exit Store", "Are you sure you want to exit the General Store?",
                                  QMessageBox::Yes|QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Placeholder Logic: Persist state if necessary
        showFeedbackDialog("Goodbye", "Thank you for visiting the General Store!", QMessageBox::Information);
        this->close(); 
    }
}
