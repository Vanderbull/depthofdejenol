// SeerDialog.cpp
#include "SeerDialog.h"
#include <QMessageBox>
#include <QRandomGenerator> // Include for random number generation
#include <QFile>
#include <QTextStream>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QTextStream>

void loadStyleSheet(QWidget* widget, const QString& sheetName) {
    QFile file(sheetName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&file);
        widget->setStyleSheet(ts.readAll());
        file.close();
    }
}

SeerDialog::SeerDialog(QWidget *parent)
    : QDialog(parent)
{
    // 1. Get the directory of THIS .cpp file
    // __FILE__ is a built-in macro that gives the path to SeerDialog.cpp
    QFileInfo fileInfo(__FILE__);
    QString dir = fileInfo.absolutePath();

    // 2. Build the path to the .qss file in the same folder
    QString qssPath = dir + "/SeerDialog.qss";
    
// 3. Load and apply the style
    QFile styleFile(qssPath);
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&styleFile);
        this->setStyleSheet(ts.readAll());
        styleFile.close();
    } else {
        qDebug() << "Style file not found at:" << qssPath;
    }
    setWindowTitle("The Seer");
    //setFixedSize(350, 200); // Adjust size as needed
    resize(700,400);
    setMinimumSize(400,300);
    // --- Widgets ---
    welcomeLabel = new QLabel("Welcome to the City Seer!", this);
    welcomeLabel->setObjectName("welcomeLabel"); // Assign ID for CSS targeting
    
    welcomeLabel->setAlignment(Qt::AlignCenter);


    characterButton = new QPushButton("Character", this);
    monsterButton = new QPushButton("Monster", this);
    itemButton = new QPushButton("Item", this);
    exitButton = new QPushButton("Exit", this);

    // Define and initialize the search field
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("Enter item name to search for...");
    searchLineEdit->setObjectName("searchField");

    // --- Layouts ---
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(characterButton);
    buttonLayout->addWidget(monsterButton);
    buttonLayout->addWidget(itemButton);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addWidget(searchLineEdit);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(); // Pushes buttons to the top
    mainLayout->addWidget(exitButton, 0, Qt::AlignRight); // Align exit button to the bottom right

    // --- Connections ---
    connect(characterButton, &QPushButton::clicked, this, &SeerDialog::on_characterButton_clicked);
    connect(monsterButton, &QPushButton::clicked, this, &SeerDialog::on_monsterButton_clicked);
    connect(itemButton, &QPushButton::clicked, this, &SeerDialog::on_itemButton_clicked);
    connect(exitButton, &QPushButton::clicked, this, &SeerDialog::on_exitButton_clicked);

    // Set layout for the dialog
    setLayout(mainLayout);
}

SeerDialog::~SeerDialog()
{
}

void SeerDialog::on_characterButton_clicked()
{
    // High chance to find a character/clue
    int randomChance = QRandomGenerator::global()->bounded(100);
    int successThreshold = 60; // 60% chance of success

    if (randomChance < successThreshold) {
        // Success: Found a character clue
        QString characterName = "Elara, The Sun's Blade";
        QString message = QString("Success! You sense the spirit of a forgotten hero.\nSeek out **%1**.").arg(characterName);

        QMessageBox::information(this, "Seer Option - Character Insight", message);
    } else {
        // Failure: No character insight this time
        QMessageBox::information(this, "Seer Option - Search Failed", "The fates are silent. No notable character can be found at this time.");
    }
}

void SeerDialog::on_monsterButton_clicked()
{
    // Implementation from the previous step (40% chance)
    int randomChance = QRandomGenerator::global()->bounded(100);
    int successThreshold = 40; // 40% chance of success

    if (randomChance < successThreshold) {
        // Success: The monster is found.
        QString monsterName = "Gargoyle";
        QString location = "Old Watchtower (x=15, y=42)";
        QString message = QString("Success! You found a **%1**.\nIt's located at the **%2**.").arg(monsterName, location);

        QMessageBox::information(this, "Seer Option - Monster Found!", message);
    } else {
        // Failure: The monster is not found this time.
        QMessageBox::information(this, "Seer Option - Search Failed", "The city's shadows are thick. You couldn't locate a monster this time.");
    }
}

void SeerDialog::on_itemButton_clicked()
{
    GameStateManager* gsm = GameStateManager::instance();
    QString searchTerm = searchLineEdit->text().trimmed();

    if (searchTerm.isEmpty()) {
        QMessageBox::warning(this, "Input Required", "Please enter the name of the item you seek.");
        return;
    }

    // 1. Find the item first to determine its Floor and Rarity
    const QList<QVariantMap>& items = gsm->itemData();
    QVariantMap foundItem;
    bool itemExists = false;

    for (const QVariantMap& item : items) {
        if (item.value("name").toString().contains(searchTerm, Qt::CaseInsensitive)) {
            foundItem = item;
            itemExists = true;
            break; 
        }
    }

    if (!itemExists) {
        QMessageBox::information(this, "Seer Vision", "The name you spoke echoes in a void. No such item exists.");
        return;
    }

    // 2. Extract Data for Cost Calculation
    int playerDepth = gsm->getGameValue("DungeonLevel").toInt();
    if (playerDepth < 1) playerDepth = 1;

    int itemFloor = foundItem.value("floor").toInt();
    int itemRarity = foundItem.value("rarity").toInt(); // Column 7 in CSV
    if (itemRarity < 1) itemRarity = 1;

    // 3. Advanced Cost Formula
    // Base Fee: 500
    // Depth Multiplier: +150 per level of player progress
    // Rarity Tax: + (Rarity * 10) gold
    // Location Premium: If the item is deeper than player progress, double the rarity tax.
    
    qulonglong baseCost = 500 + (static_cast<qulonglong>(playerDepth) * 150);
    qulonglong rarityTax = static_cast<qulonglong>(itemRarity) * 25;
    
    if (itemFloor > playerDepth) {
        rarityTax *= 2; // Harder to see things in unexplored depths
    }

    qulonglong totalCost = baseCost + rarityTax;

    // 4. Gold Validation
    qulonglong currentGold = gsm->getGameValue("CurrentCharacterGold").value<qulonglong>();
    if (currentGold < totalCost) {
        QMessageBox::warning(this, "Insufficient Gold", 
            QString("The Seer demands %1 gold for an item of this rarity (%2) at Floor %3.\nYou only have %4.")
            .arg(totalCost).arg(itemRarity).arg(itemFloor).arg(currentGold));
        return;
    }

    // 5. Deduct Gold and Roll for Success (incorporating Level interference)
    gsm->setGameValue("CurrentCharacterGold", QVariant::fromValue(currentGold - totalCost));

    int baseSuccess = 40; 
    int interference = itemFloor * 2; 
    int finalSuccessThreshold = qMax(5, baseSuccess - interference);
    int roll = QRandomGenerator::global()->bounded(100);

    // 6. Final Results
    if (roll < finalSuccessThreshold) {
        int pX = gsm->getGameValue("DungeonX").toInt();
        int pY = gsm->getGameValue("DungeonY").toInt();
        QString locationDesc = QString("Floor %1, approx [%2, %3]")
                                .arg(itemFloor)
                                .arg(pX + QRandomGenerator::global()->bounded(-4, 4))
                                .arg(pY + QRandomGenerator::global()->bounded(-4, 4));

        QMessageBox::information(this, "Vision Success", 
            QString("**Item:** %1\n**Rarity:** %2\n**Location:** %3\n\nCost: %4 gold")
            .arg(foundItem.value("name").toString()).arg(itemRarity).arg(locationDesc).arg(totalCost));
    } else {
        QMessageBox::warning(this, "Vision Failed", 
            QString("The %1 gold is spent, but the rarity of this object obscured the vision.").arg(totalCost));
    }
}

void SeerDialog::on_exitButton_clicked()
{
    QMessageBox::information(this, "Exit", "Exiting the Seer!");
    accept(); // Closes the dialog with QDialog::Accepted result
}
