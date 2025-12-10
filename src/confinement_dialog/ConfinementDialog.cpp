#include "ConfinementDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug> 
#include <QFontDatabase>
#include <QSpacerItem> // Added for layout spacing

ConfinementAndHoldingDialog::ConfinementAndHoldingDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Confinement & Holding");
    
    // --- Set the Monospace Font for retro/aligned look ---
    QFont fixedFont;
    fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fixedFont.setPointSize(10); 
    setFont(fixedFont);

    setupUi();
    resize(850, 650); 

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

    // --- Identiy Buttons Layout (No Stretch, fixed width) ---
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

    // "G N E" header alignment using a spacer/fixed width
    QHBoxLayout *gneHeaderLayout = new QHBoxLayout();
    // Use a fixed spacer to push "G N E" to the right columns for alignment
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
    
    // Example data formatted for monospace alignment (G E N columns)
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
    // In the real game, this usually calculates the GNE value of the identified companion
    QMessageBox::information(this, "Identify G N E", "Calculating G N E for: " + identifyCompLineEdit->text());
}

void ConfinementAndHoldingDialog::sellCompanion()
{
    QMessageBox::information(this, "Sell Companion", "Selling companion: " + identifyCompLineEdit->text());
    qDebug() << "Sell Companion: " << identifyCompLineEdit->text();
}

void ConfinementAndHoldingDialog::realignCompanionID()
{
    // In the real game, this reassigns a companion ID at a cost.
    QMessageBox::information(this, "Realign ID", "Attempting to realign ID for: " + identifyCompLineEdit->text() + " at cost: " + identifyIdCostLineEdit->text());
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
    // Implement the live-search/filter functionality
    QString searchText = searchLineEdit->text().trimmed();
    qDebug() << "Searching for: " << searchText;
    
    // Loop through list items and hide/show based on text
    for (int i = 0; i < buyCreatureListWidget->count(); ++i) {
        QListWidgetItem *item = buyCreatureListWidget->item(i);
        // Hide if the item text does NOT contain the search text (case-insensitive)
        bool matches = item->text().contains(searchText, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}
