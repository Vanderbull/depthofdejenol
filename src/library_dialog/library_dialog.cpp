#include "library_dialog.h"
#include "gameStateManager.h"
#include "src/core/GameConstants.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QInputDialog>

LibraryDialog::LibraryDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("The Library");
    setFixedSize(600, 450);
    loadKnowledge();
    setupUI();
    refreshDisplay();
}

void LibraryDialog::setupUI() {
    auto *layout = new QVBoxLayout(this);

    // Top Controls
    auto *topLayout = new QHBoxLayout();
    categoryBox = new QComboBox(this);
    categoryBox->addItems({GameConstants::CATEGORY_MAGIC, GameConstants::CATEGORY_MONSTERS, GameConstants::CATEGORY_ITEMS});

    searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Search...");

    topLayout->addWidget(new QLabel("Category:"));
    topLayout->addWidget(categoryBox);
    topLayout->addStretch();
    topLayout->addWidget(searchField);
    layout->addLayout(topLayout);

    // Content area
    auto *contentLayout = new QHBoxLayout();
    bookList = new QListWidget(this);
    descriptionText = new QTextEdit(this);
    descriptionText->setReadOnly(true);

    contentLayout->addWidget(bookList, 1);
    contentLayout->addWidget(descriptionText, 2);
    layout->addLayout(contentLayout);

    // Bottom area
    countLabel = new QLabel(this);
    auto *closeBtn = new QPushButton("Close", this);

    layout->addWidget(countLabel, 0, Qt::AlignCenter);
    layout->addWidget(closeBtn);

    // Logic Connections
    connect(categoryBox, &QComboBox::currentTextChanged, this, &LibraryDialog::refreshDisplay);
    connect(searchField, &QLineEdit::textChanged, this, &LibraryDialog::refreshDisplay);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(bookList, &QListWidget::currentTextChanged, [this](const QString &name) {
        if (name.isEmpty()) return;
        QString cat = categoryBox->currentText();
        // Wrap the name in an H2 and show the list of stats generated in loadKnowledge
        descriptionText->setHtml(QString("<h2>%1</h2>%2")
            .arg(name, knowledgeBase[cat].value(name, "<i>No details available.</i>")));
    });
//    connect(bookList, &QListWidget::currentTextChanged, [this](const QString &name) {
//        QString cat = categoryBox->currentText();
//        descriptionText->setHtml(QString("<h2>%1</h2><p>%2</p>")
//            .arg(name, knowledgeBase[cat].value(name, "No data.")));
//    });
}

void LibraryDialog::loadKnowledge() {
    auto* gsm = gameStateManager::instance();
    knowledgeBase.clear();

    // Helper to turn all map data into a readable HTML string
    auto parse = [&](const QList<QVariantMap>& dataList, const QString& category) {
        for (const auto& entry : dataList) {
            QString name = entry["name"].toString();
            if (name.isEmpty()) continue;

            QString details = "<ul>";
            // Iterate through every key in the data map
            for (auto it = entry.begin(); it != entry.end(); ++it) {
                // Skip the name since it's already the header
                if (it.key() == "name") continue;

                QString value = it.value().toString();
                // Special formatting for Cursed items
                if (it.key() == "cursed") {
                    value = (it.value().toInt() == 1) ? "<b style='color:red;'>YES</b>" : "No";
                }

                details += QString("<li><b>%1:</b> %2</li>").arg(it.key(), value);
            }
            details += "</ul>";
            knowledgeBase[category].insert(name, details);
        }
    };

    // Process all categories with all available stats
    parse(gsm->monsterData(), GameConstants::CATEGORY_MONSTERS);
    parse(gsm->spellData(), GameConstants::CATEGORY_MAGIC);
    parse(gsm->itemData(), GameConstants::CATEGORY_ITEMS);
}
/*
void LibraryDialog::loadKnowledge() {
    auto* gsm = gameStateManager::instance();
    // Lambda to helper parse data maps into the knowledgeBase
    auto parse = [&](const QList<QVariantMap>& data, const QString& cat, const QString& fmt) {
        for (const auto& entry : data) {
            QString name = entry["name"].toString();
            if (name.isEmpty()) continue;
            // Simplified formatting using entry map directly
            QString desc = fmt;
            for (auto it = entry.begin(); it != entry.end(); ++it)
                desc.replace("%" + it.key(), it.value().toString());
            knowledgeBase[cat].insert(name, desc);
        }
    };
    parse(gsm->monsterData(), GameConstants::CATEGORY_MONSTERS, "Level: %levelFound | HP: %hits | Att: %att");
    parse(gsm->spellData(), GameConstants::CATEGORY_MAGIC, "Class: %Class | Dmg: %damage1-%damage2");
    parse(gsm->itemData(), GameConstants::CATEGORY_ITEMS, "Price: %price | Str Req: %StrReq");
}
*/
void LibraryDialog::refreshDisplay() {
    QString cat = categoryBox->currentText();
    QString filter = searchField->text().toLower();
    bookList->clear();
    for (const QString& name : knowledgeBase[cat].keys()) {
        if (name.toLower().contains(filter)) {
            bookList->addItem(name);
        }
    }

    if (bookList->count() > 0) bookList->setCurrentRow(0);
    countLabel->setText(QString("Library: %1 Spells | %2 Monsters | %3 Items")
        .arg(knowledgeBase[GameConstants::CATEGORY_MAGIC].size())
        .arg(knowledgeBase[GameConstants::CATEGORY_MONSTERS].size())
        .arg(knowledgeBase[GameConstants::CATEGORY_ITEMS].size()));
}
