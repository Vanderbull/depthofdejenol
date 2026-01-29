#include "library_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMap>
#include <QDebug>
// Constructor
LibraryDialog::LibraryDialog(QWidget *parent) : QDialog(parent) 
{
    setWindowTitle("The Library of Knowledge");
    loadKnowledge();
    setupUI();    
    // Set default category to "Magic Books" and update the list
    onCategoryChanged(CATEGORY_MAGIC);
    // Select the first item in the default list
    if (bookList->count() > 0) {
        bookList->setCurrentRow(0);
        onItemSelected(bookList->item(0));
    }
}
// Sets up the main UI layout
void LibraryDialog::setupUI() 
{
    setFixedSize(650, 500);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // --- Header ---
    headerLabel = new QLabel("Mordor: The Library of Knowledge", this);
    headerLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFD700; padding: 10px;");
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);
    // --- Category Selection ---
    QHBoxLayout *categoryLayout = new QHBoxLayout();
    QLabel *categoryLabel = new QLabel("Knowledge Category:", this);
    categoryLabel->setStyleSheet("color: #AFAFAF; font-weight: bold;");
    categoryComboBox = new QComboBox(this);
    categoryComboBox->setStyleSheet("background-color: #3e2723; color: #FFFFFF; border: 1px solid #5D4037; padding: 5px;");
    categoryComboBox->addItem(CATEGORY_MAGIC);
    categoryComboBox->addItem(CATEGORY_MONSTERS);
    categoryComboBox->addItem(CATEGORY_ITEMS);
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(categoryComboBox);
    categoryLayout->addStretch(1); // Push elements to the left
    mainLayout->addLayout(categoryLayout);
    // --- Content Area (List and Description) ---
    QHBoxLayout *contentLayout = new QHBoxLayout();
    // 1. List (Left side)
    bookList = new QListWidget(this);
    bookList->setStyleSheet("background-color: #3e2723; color: #FFFFFF; border: 2px solid #5D4037; selection-background-color: #5D4037; font-size: 14px;");
    bookList->setMaximumWidth(250);
    contentLayout->addWidget(bookList);
    // 2. Description Text (Right side)
    descriptionText = new QTextEdit(this);
    descriptionText->setReadOnly(true);
    descriptionText->setStyleSheet("background-color: #121212; color: #AFAFAF; font-family: 'Consolas', monospace; border: 2px solid #5D4037; padding: 10px;");
    contentLayout->addWidget(descriptionText);
    mainLayout->addLayout(contentLayout);
    // --- Close Button ---
    closeButton = new QPushButton("Close Library", this);
    QString buttonStyle = "QPushButton { background-color: #3e2723; color: #FFFFFF; border: 2px solid #5D4037; border-radius: 8px; padding: 10px; }"
                          "QPushButton:hover { background-color: #5d4037; }";
    closeButton->setStyleSheet(buttonStyle);
    closeButton->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(closeButton);
    // --- Connections ---
    connect(bookList, &QListWidget::itemClicked, this, &LibraryDialog::onItemSelected);
    connect(closeButton, &QPushButton::clicked, this, &LibraryDialog::onCloseClicked);
    // Use a lambda connected to the index-based signal to reliably get the text.
    connect(categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, 
        [this](int index) {
            this->onCategoryChanged(this->categoryComboBox->itemText(index));
        });
    // Set dialog background style
    setStyleSheet("QDialog { background-color: #212121; }");
    setLayout(mainLayout);
}
/**
 * @brief Populates the main knowledge base with all categories.
 */
void LibraryDialog::loadKnowledge() 
{
    // --- 1. Magic Books (Based on "The Teachings of Magic") ---
    QMap<QString, QString> magicMap;
    magicMap["Fire Magic"] = "Teaches spells focusing on destructive heat and flame. Essential for destroying creatures vulnerable to fire.";
    magicMap["Cold Magic"] = "Focuses on chilling and freezing enemies. Used to slow opponents and deal devastating cold damage.";
    magicMap["Electrical Magic"] = "Harnesses the power of lightning and shock. Effective against metal-clad or water-based enemies.";
    magicMap["Mind Magic"] = "Involves psychic attacks and illusions. Used to confuse, frighten, and influence the minds of creatures.";
    magicMap["Damage Magic"] = "General offensive spells that deal raw physical or magical damage to a single target.";
    magicMap["Element Magic"] = "Spells that call upon the basic elements (Earth, Air, Water, Fire) to affect the environment or enemies.";
    magicMap["Kill Magic"] = "Potentially devastating spells designed to instantly slay weak or powerful foes, often resisted by high-level creatures.";
    magicMap["Charm Magic"] = "Spells used to pacify or temporarily control creatures, turning them into allies or stopping them from attacking.";
    magicMap["Bind Magic"] = "Focuses on rooting or binding targets, preventing them from moving or attacking for a duration.";
    magicMap["Heal Magic"] = "Essential spells for restoring health and curing status ailments like poison and disease.";
    magicMap["Movement Magic"] = "Allows for teleportation or rapid travel within the depths, bypassing dangerous areas.";
    // FIX 2: Corrected typo 'magiMap' to 'magicMap'
    magicMap["Banish Magic"] = "Used to forcibly remove certain types of enemies from the area or send them back to their origin.";
    magicMap["Dispell Magic"] = "Spells designed to counter and nullify hostile magical effects and enchantments on the party or the environment.";
    magicMap["Resistant Magic"] = "Teaches defensive spells that provide temporary resistance or immunity to various damage types.";
    magicMap["Visual Magic"] = "Spells for improving sight in darkness, revealing hidden objects, or creating light sources.";
    magicMap["Protection Magic"] = "Focuses on enhancing armor class and providing a magical barrier against incoming physical and magical attacks.";
    knowledgeBase[CATEGORY_MAGIC] = magicMap;
    // --- 2. Creatures (Monsters) ---
    QMap<QString, QString> monsterMap;
    monsterMap["Goblin Shaman"] = "Weak but clever. Uses minor elemental spells. Avoid low-level encounters if injured.";
    monsterMap["Stone Golem"] = "Immune to physical damage from non-magical weapons. Highly resistant to elemental magic, weak to critical hits.";
    monsterMap["Giant Spider"] = "Fast and deadly. Its bite injects a powerful poison that causes rapid health loss. Requires a high-level cure.";
    monsterMap["Dark Imp"] = "A flying demon that relies on hit-and-run tactics. Highly resistant to Mind and Charm magic.";
    knowledgeBase[CATEGORY_MONSTERS] = monsterMap;
    // --- 3. Items ---
    QMap<QString, QString> itemMap;
    itemMap["Small Healing Potion"] = "Restores $20-40$ hit points. Common. Always carry several.";
    itemMap["Elixir of Fortitude"] = "Grants temporary immunity to poison and disease for $10$ turns. Extremely valuable in the lower depths.";
    itemMap["Mithril Plate Armor"] = "Lightweight, granting superior defense without movement penalty. Requires high Strength and Armor Skill.";
    itemMap["Amulet of Recall"] = "Teleports the entire party back to the City of Mordor instantly. Use only in emergencies; it shatters upon use.";
    knowledgeBase[CATEGORY_ITEMS] = itemMap;
}
/**
 * @brief Updates the QListWidget based on the selected category.
 */
void LibraryDialog::updateList(const QString &category) 
{
    bookList->clear();
    descriptionText->clear();    
    if (knowledgeBase.contains(category)) {
        const QMap<QString, QString>& currentMap = knowledgeBase.value(category);
        for (const QString& item : currentMap.keys()) {
            new QListWidgetItem(item, bookList);
        }
        // Select the first item if the new list is not empty
        if (bookList->count() > 0) {
            bookList->setCurrentRow(0);
            onItemSelected(bookList->item(0));
        }
    } else {
        descriptionText->setHtml("<h2>Error</h2><p>Knowledge Base for this category is corrupt or missing.</p>");
    }
}
// Slot called when a magic book is selected from the list
void LibraryDialog::onItemSelected(QListWidgetItem *item) 
{
    if (!item) return;
    QString itemName = item->text();
    QString currentCategory = categoryComboBox->currentText();
    QString description = "No description found.";
    if (knowledgeBase.contains(currentCategory)) {
        description = knowledgeBase.value(currentCategory).value(itemName, "The details of this item are lost to time.");
    }
    // Format the text for the QTextEdit
    QString formattedText = QString("<h2>%1</h2><p style='color:#E0E0E0;'>%2</p>")
                                .arg(itemName)
                                .arg(description);
    descriptionText->setHtml(formattedText);
}
// Slot called when the category combo box value changes
void LibraryDialog::onCategoryChanged(const QString &categoryName) 
{
    updateList(categoryName);
}
// Slot called when the 'Close' button is clicked
void LibraryDialog::onCloseClicked() 
{
    this->accept();
}
