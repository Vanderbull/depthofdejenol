#include "library_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMap>
#include <QDebug>
#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QRegularExpression>
LibraryDialog::LibraryDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("The Library of Knowledge");
    loadKnowledge();
    setupUI();
    QString category = GameConstants::CATEGORY_MAGIC;
    qDebug() << "Selected Category:" << category;
    onCategoryChanged(GameConstants::CATEGORY_MAGIC);
    if (bookList->count() > 0) {
        bookList->setCurrentRow(0);
        onItemSelected(bookList->item(0));
    }
}
void LibraryDialog::setupUI()
{
    setFixedSize(650, 500);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    headerLabel = new QLabel("Mordor: The Library of Knowledge", this);
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);
    QHBoxLayout *topControlsLayout = new QHBoxLayout();
    QHBoxLayout *categoryLayout = new QHBoxLayout();
    QLabel *categoryLabel = new QLabel("Knowledge Category:", this);
    categoryComboBox = new QComboBox(this);
    categoryComboBox->addItem(GameConstants::CATEGORY_MAGIC);
    categoryComboBox->addItem(GameConstants::CATEGORY_MONSTERS);
    categoryComboBox->addItem(GameConstants::CATEGORY_ITEMS);
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(categoryComboBox);
    topControlsLayout->addLayout(categoryLayout);
    searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Search for Item, Spell, or Monster...");
    searchField->setMaximumWidth(250);
    topControlsLayout->addStretch(1);
    topControlsLayout->addWidget(searchField);
    mainLayout->addLayout(topControlsLayout);
    QHBoxLayout *contentLayout = new QHBoxLayout();
    bookList = new QListWidget(this);
    bookList->setMaximumWidth(250);
    contentLayout->addWidget(bookList);
    descriptionText = new QTextEdit(this);
    descriptionText->setReadOnly(true);
    contentLayout->addWidget(descriptionText);
    mainLayout->addLayout(contentLayout);
    QVBoxLayout *bottomLayout = new QVBoxLayout();
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addItemButton = new QPushButton("Add New Entry", this);
    addItemButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(addItemButton);
    closeButton = new QPushButton("Close Library", this);
    closeButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(closeButton);
    bottomLayout->addLayout(buttonLayout);
    countLabel = new QLabel(this);
    countLabel->setAlignment(Qt::AlignCenter);
    bottomLayout->addWidget(countLabel);
    mainLayout->addLayout(bottomLayout);
    updateCountLabel();
    connect(bookList, &QListWidget::currentItemChanged, this,
        [this](QListWidgetItem *current, QListWidgetItem *previous) {
            Q_UNUSED(previous);
            this->onItemSelected(current);
        });
    connect(closeButton, &QPushButton::clicked, this, &LibraryDialog::onCloseClicked);
    connect(addItemButton, &QPushButton::clicked, this, &LibraryDialog::onAddItemClicked);
    connect(categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this](int index) {
            this->searchField->clear();
            this->onCategoryChanged(this->categoryComboBox->itemText(index));
        });
    connect(searchField, &QLineEdit::textChanged, this, &LibraryDialog::onSearchTextChanged);
    setLayout(mainLayout);
}

void LibraryDialog::loadKnowledge()
{
    knowledgeBase.clear();
    gameStateManager* gsm = gameStateManager::instance();

    for (const QVariantMap& monster : gsm->monsterData()) {
        QString name = monster.value("name").toString();
        if (name.isEmpty()) continue;

        QString description = QString(
            "<b>ID:</b> %1 | <b>Level Found:</b> %2<br>"
            "<b>Stats:</b> Att:%3, Def:%4, Hits:%5<br>"
            "<b>Resistances:</b> Fire:%6, Cold:%7, Elec:%8, Mag:%9<br>"
            "<b>Alignment:</b> %10 | <b>Gold Factor:</b> %11")
            .arg(monster.value("id").toString())
            .arg(monster.value("levelFound").toString())
            .arg(monster.value("att").toString())
            .arg(monster.value("def").toString())
            .arg(monster.value("hits").toString())
            .arg(monster.value("ResFire").toString())
            .arg(monster.value("ResCold").toString())
            .arg(monster.value("ResElectric").toString())
            .arg(monster.value("ResMagic").toString())
            .arg(monster.value("alignment").toString())
            .arg(monster.value("goldFactor").toString());

        knowledgeBase[GameConstants::CATEGORY_MONSTERS].insert(name, description);
    }
    for (const QVariantMap& spell : gsm->spellData()) {
        QString name = spell.value("name").toString();
        if (name.isEmpty()) continue;

        QString description = QString(
            "<b>Level:</b> %1 | <b>Class:</b> %2<br>"
            "<b>Damage:</b> %3-%4<br>"
            "<b>Requirements:</b> Int:%5, Wis:%6")
            .arg(spell.value("Level").toString())
            .arg(spell.value("Class").toString())
            .arg(spell.value("damage1").toString())
            .arg(spell.value("damage2").toString())
            .arg(spell.value("ReqInt").toString())
            .arg(spell.value("ReqWis").toString());

        knowledgeBase[GameConstants::CATEGORY_MAGIC].insert(name, description);
    }
    for (const QVariantMap& item : gsm->itemData()) {
        QString name = item.value("name").toString();
        if (name.isEmpty()) continue;

        QString cursed = item.value("cursed").toInt() == 1
            ? "<span style='color:red;'>CURSED</span>" : "Normal";

        QString description = QString(
            "<b>Price:</b> %1 gold | <b>Status:</b> %2<br>"
            "<b>Stats:</b> Att:%3, Def:%4, Swings:%5<br>"
            "<b>Requirements:</b> Str:%6, Dex:%7")
            .arg(item.value("price").toString())
            .arg(cursed)
            .arg(item.value("att").toString())
            .arg(item.value("def").toString())
            .arg(item.value("swings").toString())
            .arg(item.value("StrReq").toString())
            .arg(item.value("DexReq").toString());

        knowledgeBase[GameConstants::CATEGORY_ITEMS].insert(name, description);
    }
}
void LibraryDialog::updateList(const QString &category)
{
    bookList->clear();
    descriptionText->clear();
    if (knowledgeBase.contains(category)) {
        const QMap<QString, QString>& currentMap = knowledgeBase.value(category);
        for (const QString& item : currentMap.keys()) {
            new QListWidgetItem(item, bookList);
        }
        onSearchTextChanged(searchField->text());
        QListWidgetItem *firstVisible = nullptr;
        for(int i = 0; i < bookList->count(); ++i) {
            if (!bookList->item(i)->isHidden()) {
                firstVisible = bookList->item(i);
                break;
            }
        }
        if (firstVisible) {
            bookList->setCurrentItem(firstVisible);
            onItemSelected(firstVisible);
        } else {
            descriptionText->clear();
        }
    } else {
        descriptionText->setHtml("<h2>Error</h2><p>Knowledge Base for this category is corrupt or missing.</p>");
    }
}
void LibraryDialog::updateCountLabel()
{
    int spellCount = knowledgeBase.value(GameConstants::CATEGORY_MAGIC).count();
    int monsterCount = knowledgeBase.value(GameConstants::CATEGORY_MONSTERS).count();
    int itemCount = knowledgeBase.value(GameConstants::CATEGORY_ITEMS).count();
    QString text = QString("Library Contents: <b>%1</b> Spells | <b>%2</b> Creatures | <b>%3</b> Items")
        .arg(spellCount)
        .arg(monsterCount)
        .arg(itemCount);
    countLabel->setText(text);
}
void LibraryDialog::onItemSelected(QListWidgetItem *item)
{
    if (!item) return;
    QString itemName = item->text();
    QString currentCategory = categoryComboBox->currentText();
    QString description = "No description found.";
    if (knowledgeBase.contains(currentCategory)) {
        description = knowledgeBase.value(currentCategory).value(itemName, "The details of this item are lost to time.");
    }
    QString formattedText = QString("<h2>%1</h2><p>%2</p>")
                                .arg(itemName)
                                .arg(description);
    descriptionText->setHtml(formattedText);
}
void LibraryDialog::onCategoryChanged(const QString &categoryName)
{
    updateList(categoryName);
}
void LibraryDialog::onSearchTextChanged(const QString &searchText)
{
    QString filter = searchText.toLower();
    for (int i = 0; i < bookList->count(); ++i) {
        QListWidgetItem *item = bookList->item(i);
        if (item->text().toLower().contains(filter)) {
            item->setHidden(false);
        } else {
            item->setHidden(true);
        }
    }
    QListWidgetItem *currentItem = bookList->currentItem();
    if (currentItem && currentItem->isHidden()) {
        QListWidgetItem *firstVisible = nullptr;
        for(int i = 0; i < bookList->count(); ++i) {
            if (!bookList->item(i)->isHidden()) {
                firstVisible = bookList->item(i);
                break;
            }
        }
        if (firstVisible) {
            bookList->setCurrentItem(firstVisible);
            onItemSelected(firstVisible);
        } else {
            descriptionText->clear();
        }
    }
}
void LibraryDialog::onAddItemClicked()
{
    bool ok;
    QStringList categories = {GameConstants::CATEGORY_MAGIC, GameConstants
        ::CATEGORY_MONSTERS, GameConstants::CATEGORY_ITEMS};
    QString selectedCategory = QInputDialog::getItem(this,
        tr("Add New Entry"),
        tr("Select Category:"),
        categories,
        0,
        false,
        &ok);
    if (!ok) return;
    QString itemName = QInputDialog::getText(this,
        tr("Add New Entry"),
        tr("Enter Name for the %1:").arg(selectedCategory),
        QLineEdit::Normal,
        QString(),
        &ok);
    if (!ok || itemName.isEmpty()) return;
    if (knowledgeBase.value(selectedCategory).contains(itemName)) {
        QMessageBox::warning(this, tr("Duplicate Entry"),
            tr("The entry '%1' already exists in the '%2' category.").arg(itemName).arg(selectedCategory));
        return;
    }
    QString itemDescription = QInputDialog::getMultiLineText(this,
        tr("Add New Entry"),
        tr("Enter Description/Details for '%1':").arg(itemName),
        QString(),
        &ok);
    if (!ok || itemDescription.isEmpty()) return;
    knowledgeBase[selectedCategory].insert(itemName, itemDescription);
    updateCountLabel();
    categoryComboBox->setCurrentText(selectedCategory);
    updateList(selectedCategory);
    QList<QListWidgetItem*> foundItems = bookList->findItems(itemName, Qt::MatchExactly);
    if (!foundItems.isEmpty()) {
        bookList->setCurrentItem(foundItems.first());
        onItemSelected(foundItems.first());
    }
    QMessageBox::information(this, tr("Success"),
        tr("'%1' has been added to the Library.").arg(itemName));
}
void LibraryDialog::onCloseClicked()
{
    this->accept();
}
