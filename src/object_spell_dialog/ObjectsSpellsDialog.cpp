#include "ObjectsSpellsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

ObjectsSpellsDialog::ObjectsSpellsDialog(QWidget *parent) :
    QDialog(parent),
    m_itemsModel(new QStringListModel(this)),
    m_spellsModel(new QStringListModel(this)),
    m_companionsModel(new QStringListModel(this))
{
    setupUi();
    createConnections();
    
    setWindowTitle("Character Profile & Inventory");
    setMinimumSize(900, 600);
}

void ObjectsSpellsDialog::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    // --- Left Side: Tabs ---
    m_tabWidget = new QTabWidget(this);
    
    m_itemsListView = new QListView(this);
    m_itemsListView->setModel(m_itemsModel);
    m_tabWidget->addTab(m_itemsListView, "Items");

    m_spellsListView = new QListView(this);
    m_spellsListView->setModel(m_spellsModel);
    m_tabWidget->addTab(m_spellsListView, "Spells");

    m_charInfoDisplay = new QTextEdit(this);
    m_charInfoDisplay->setReadOnly(true);
    m_tabWidget->addTab(m_charInfoDisplay, "Full Stats");

    // --- Right Side: Summary Panel ---
    QGroupBox *detailsGroup = new QGroupBox("Character Summary", this);
    QFormLayout *detailsLayout = new QFormLayout();

    m_charNameLabel = new QLabel("<b>Loading...</b>");
    m_leadLabel = new QLabel("0");
    m_castLabel = new QLabel("0");
    m_guildLabel = new QLabel("None");
    m_expLabel = new QLabel("0");

    detailsLayout->addRow("Name:", m_charNameLabel);
    detailsLayout->addRow("Leadership:", m_leadLabel);
    detailsLayout->addRow("Spellpower:", m_castLabel);
    detailsLayout->addRow("Guild:", m_guildLabel);
    detailsLayout->addRow("Experience:", m_expLabel);

    detailsGroup->setLayout(detailsLayout);
    detailsGroup->setFixedWidth(250);

    // Assembly
    mainLayout->addWidget(m_tabWidget, 1); // Stretch factor 1
    mainLayout->addWidget(detailsGroup, 0); // Stretch factor 0 (fixed)
}

void ObjectsSpellsDialog::createConnections()
{
    // Use modern Lambda syntax for simple debug logs or specific logic
    connect(m_itemsListView, &QListView::clicked, this, [this](const QModelIndex &index){
        QString desc = m_itemsModel->data(index, Qt::DisplayRole).toString();
        emit itemSelected(desc);
    });

    // Handle tab changes without annoying popups
    connect(m_tabWidget, &QTabWidget::currentChanged, this, [](int index){
        // Silently log or refresh data
    });
}

void ObjectsSpellsDialog::updateCharStats(const QString& name, int lead, int cast, const QString& guild, long long exp)
{
    m_charNameLabel->setText(QString("<b>%1</b>").arg(name));
    m_leadLabel->setText(QString::number(lead));
    m_castLabel->setText(QString::number(cast));
    m_guildLabel->setText(guild);
    // Format large numbers with commas for readability
    m_expLabel->setText(QLocale(QLocale::English).toString(exp));
}

void ObjectsSpellsDialog::updateItems(const QStringList& items) {
    m_itemsModel->setStringList(items);
}
