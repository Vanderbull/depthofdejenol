#include "BestiaryDialog.h"
#include "gameStateManager.h"
#include "character.h"
#include <QtWidgets>
#include <QMessageBox>

BestiaryDialog::BestiaryDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    loadBestiary();
    refreshList();
}

BestiaryDialog::~BestiaryDialog() = default;

void BestiaryDialog::setupUi() {
    setWindowTitle(tr("Bestiary"));
    setMinimumSize(500, 400);

    auto *mainLayout = new QVBoxLayout(this);

    // Filter
    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(tr("Filter:")));
    m_filterCombo = new QComboBox;
    m_filterCombo->addItem(tr("All"));
    m_filterCombo->addItem(tr("Floor 1-5"));
    m_filterCombo->addItem(tr("Floor 6-10"));
    m_filterCombo->addItem(tr("Floor 11-15"));
    filterLayout->addWidget(m_filterCombo);
    mainLayout->addLayout(filterLayout);

    // Monster list
    m_monsterList = new QListWidget;
    m_monsterList->setMaximumWidth(200);
    mainLayout->addWidget(m_monsterList);

    // Description
    m_descriptionText = new QTextEdit;
    m_descriptionText->setReadOnly(true);
    mainLayout->addWidget(m_descriptionText);

    // Count
    m_countLabel = new QLabel;
    mainLayout->addWidget(m_countLabel);

    // Exit
    m_exitBtn = new QPushButton(tr("Close"));
    mainLayout->addWidget(m_exitBtn, 0, Qt::AlignRight);

    connect(m_monsterList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *previous) {
                Q_UNUSED(previous);
                onMonsterSelected(current);
            });
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BestiaryDialog::onFilterChanged);
    connect(m_exitBtn, &QPushButton::clicked, this, &BestiaryDialog::onExitClicked);
}

void BestiaryDialog::loadBestiary() {
    auto *gsm = gameStateManager::instance();
    const QList<QVariantMap>& monsters = gsm->monsterData();

    for (const auto& m : monsters) {
        QString name = m.value("name").toString();
        if (name.isEmpty()) continue;

        int floor = m.value("levelFound", 0).toInt();
        int hp = m.value("hp", 0).toInt();
        int att = m.value("att", 0).toInt();
        int def = m.value("def", 0).toInt();

        QString desc = tr("<b>%1</b><br><br>"
                          "<b>Floor:</b> %2<br>"
                          "<b>HP:</b> %3<br>"
                          "<b>Attack:</b> %4<br>"
                          "<b>Defense:</b> %5")
                          .arg(name)
                          .arg(floor > 0 ? QString::number(floor) : tr("Unknown"))
                          .arg(hp)
                          .arg(att)
                          .arg(def);

        m_monsters[name] = desc;
        m_monsterFloors[name] = floor;
    }
}

void BestiaryDialog::refreshList() {
    m_monsterList->clear();

    int filterIdx = m_filterCombo->currentIndex();
    int minFloor = 0, maxFloor = 999;
    if (filterIdx == 1) { minFloor = 1; maxFloor = 5; }
    else if (filterIdx == 2) { minFloor = 6; maxFloor = 10; }
    else if (filterIdx == 3) { minFloor = 11; maxFloor = 15; }

    int count = 0;
    for (auto it = m_monsters.begin(); it != m_monsters.end(); ++it) {
        int floor = m_monsterFloors.value(it.key(), 0);
        if (floor >= minFloor && floor <= maxFloor) {
            new QListWidgetItem(it.key(), m_monsterList);
            count++;
        }
    }

    m_countLabel->setText(tr("Monsters: %1").arg(count));

    if (m_monsterList->count() > 0) {
        m_monsterList->setCurrentRow(0);
    }
}

void BestiaryDialog::onMonsterSelected(QListWidgetItem *item) {
    if (!item) return;
    QString name = item->text();
    m_descriptionText->setHtml(m_monsters.value(name, tr("No data available.")));
}

void BestiaryDialog::onFilterChanged(int index) {
    Q_UNUSED(index)
    refreshList();
}

void BestiaryDialog::onExitClicked() {
    accept();
}
