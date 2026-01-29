#include "ObjectsSpellsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListView>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include <QMessageBox>

ObjectsSpellsDialog::ObjectsSpellsDialog(QWidget *parent) :
    QDialog(parent),
    m_itemsModel(new QStringListModel(this)),
    m_spellsModel(new QStringListModel(this)),
    m_companionsModel(new QStringListModel(this))
{
    // --- 1. Tab Widget Setup (Lists and Display Areas) ---
    m_tabWidget = new QTabWidget(this);
    // Items Tab
    m_itemsListView = new QListView;
    m_itemsListView->setModel(m_itemsModel);
    m_tabWidget->addTab(m_itemsListView, "Items");
    // Spells Tab
    m_spellsListView = new QListView;
    m_spellsListView->setModel(m_spellsModel);
    m_tabWidget->addTab(m_spellsListView, "Spells");
    // Companions Tab
    m_companionsListView = new QListView;
    m_companionsListView->setModel(m_companionsModel);
    m_tabWidget->addTab(m_companionsListView, "Companions");
    // Char Tab (Assuming details are displayed here, possibly read-only text)
    m_charInfoDisplay = new QTextEdit;
    m_charInfoDisplay->setReadOnly(true);
    m_tabWidget->addTab(m_charInfoDisplay, "Char");
    // Guild Tab
    m_guildInfoDisplay = new QTextEdit;
    m_guildInfoDisplay->setReadOnly(true);
    m_tabWidget->addTab(m_guildInfoDisplay, "Guild");
    // Buffers Tab
    m_buffsInfoDisplay = new QTextEdit;
    m_buffsInfoDisplay->setReadOnly(true);
    m_tabWidget->addTab(m_buffsInfoDisplay, "Buffers");
    // Misc Tab
    m_miscInfoDisplay = new QTextEdit;
    m_miscInfoDisplay->setReadOnly(true);
    m_tabWidget->addTab(m_miscInfoDisplay, "Misc");
    // --- 2. Character Details Panel (Right Side) ---
    m_charNameLabel = new QLabel("<b>Goodie Gil'N'Rhaile</b>");
    m_leadLabel = new QLabel("lead: 120");
    m_castLabel = new QLabel("cast: 120");
    m_guildLabel = new QLabel("guild: Master of Fire Magic");
    m_expLabel = new QLabel("exp: 7,000,000,000");

    QVBoxLayout *detailsLayout = new QVBoxLayout;
    detailsLayout->addWidget(m_charNameLabel);
    detailsLayout->addWidget(m_leadLabel);
    detailsLayout->addWidget(m_castLabel);
    detailsLayout->addWidget(m_guildLabel);
    detailsLayout->addWidget(m_expLabel);
    detailsLayout->addStretch(1); // Push everything to the top
    // Create a container widget for the details panel
    QWidget *detailsPanel = new QWidget;
    detailsPanel->setLayout(detailsLayout);
    detailsPanel->setFixedWidth(200); // Fixed width to separate from tabs
    // --- 3. Main Center Layout ---
    QHBoxLayout *centerLayout = new QHBoxLayout;
    centerLayout->addWidget(m_tabWidget);
    centerLayout->addWidget(detailsPanel); // Add the details panel to the right
    // --- 4. Assemble Main Dialog Layout ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(centerLayout);

    setWindowTitle("Objects & Spells");
    this->setMinimumSize(800, 600); // Set a reasonable default size
    // --- 5. Initial Data and Connections ---
    updateItems({"Item #15: Static Sash [g][49] (cursed)", "Item #14: Unicorn Horn [12]", "Item #12: Ring of Frost [g][15]", "Item #10: Rod of Entrapment [g][9]"});
    updateCompanions({"1. (Defender) 571210", "2. (Defender) 72V229"});
    updateBuffs({"\"Acidic Spray\"", "\"Paralyzing Death\"", "Static Mesh", "Mighty Blow"});
    updateChar({"HP: 100/100", "MP: 50/50", "Damage: 10-20", "Armor: 50"});
    updateGuild({"Guild Name: The Mages", "Rank: Master", "Members: 12/12"});
    updateMisc({"Current Quest: Find the Rod", "Game Version: 1.0"});
    // Connect tab change signal
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &ObjectsSpellsDialog::on_tabWidget_currentChanged);
    // Connect list view item clicks
    connect(m_itemsListView, &QListView::clicked, this, &ObjectsSpellsDialog::on_itemsListView_clicked);
    connect(m_spellsListView, &QListView::clicked, this, &ObjectsSpellsDialog::on_spellsListView_clicked);
    connect(m_companionsListView, &QListView::clicked, this, &ObjectsSpellsDialog::on_companionsListView_clicked);
}
// --- Update Methods ---
void ObjectsSpellsDialog::updateItems(const QStringList& items)
{
    m_itemsModel->setStringList(items);
}

void ObjectsSpellsDialog::updateSpells(const QStringList& spells)
{
    m_spellsModel->setStringList(spells);
}

void ObjectsSpellsDialog::updateCompanions(const QStringList& companions)
{
    m_companionsModel->setStringList(companions);
}

void ObjectsSpellsDialog::updateGuild(const QStringList& guildInfo)
{
    m_guildInfoDisplay->setText(guildInfo.join("\n"));
}

void ObjectsSpellsDialog::updateChar(const QStringList& charInfo)
{
    m_charInfoDisplay->setText(charInfo.join("\n"));
}

void ObjectsSpellsDialog::updateBuffs(const QStringList& buffs)
{
    m_buffsInfoDisplay->setText(buffs.join("\n"));
}

void ObjectsSpellsDialog::updateMisc(const QStringList& miscInfo)
{
    m_miscInfoDisplay->setText(miscInfo.join("\n"));
}
// --- Slot Implementations ---
void ObjectsSpellsDialog::on_tabWidget_currentChanged(int index)
{
    QString tabName = m_tabWidget->tabText(index);
    // You could trigger a refresh of the data for the tab here
    QMessageBox::information(this, "Tab Change", tabName + " tab selected!");
}

void ObjectsSpellsDialog::on_itemsListView_clicked(const QModelIndex& index)
{
    if (index.isValid()) {
        QString itemDescription = m_itemsModel->data(index, Qt::DisplayRole).toString();
        emit itemSelected(itemDescription);
        QMessageBox::information(this, "Item Selected", "Selected: " + itemDescription);
    }
}

void ObjectsSpellsDialog::on_spellsListView_clicked(const QModelIndex& index)
{
    if (index.isValid()) {
        QString spellDescription = m_spellsModel->data(index, Qt::DisplayRole).toString();
        emit spellSelected(spellDescription);
        QMessageBox::information(this, "Spell Selected", "Selected: " + spellDescription);
    }
}

void ObjectsSpellsDialog::on_companionsListView_clicked(const QModelIndex& index)
{
    if (index.isValid()) {
        QString companionDescription = m_companionsModel->data(index, Qt::DisplayRole).toString();
        emit companionSelected(companionDescription);
        QMessageBox::information(this, "Companion Selected", "Selected: " + companionDescription);
    }
}

ObjectsSpellsDialog::~ObjectsSpellsDialog(){}
