#include "CharacterSheetDialog.h"
#include "gameStateManager.h"
#include "character.h"
#include "src/items/ItemDatabase.h"
#include "src/spell_casting/SpellBook.h"
#include <QtWidgets>
#include <QMessageBox>

CharacterSheetDialog::CharacterSheetDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    refreshSheet();
}

CharacterSheetDialog::~CharacterSheetDialog() = default;

void CharacterSheetDialog::setupUi() {
    setWindowTitle(tr("Character Sheet"));
    setMinimumSize(500, 600);

    auto *mainLayout = new QVBoxLayout(this);

    // Member selector
    auto *selectorLayout = new QHBoxLayout;
    selectorLayout->addWidget(new QLabel(tr("Member:")));
    m_memberCombo = new QComboBox;
    selectorLayout->addWidget(m_memberCombo);
    mainLayout->addLayout(selectorLayout);

    // Identity header
    m_nameLabel = new QLabel;
    m_nameLabel->setStyleSheet("font-weight: bold; font-size: 16px;");
    mainLayout->addWidget(m_nameLabel);

    m_raceLabel = new QLabel;
    m_classLabel = new QLabel;
    m_levelLabel = new QLabel;
    m_xpLabel = new QLabel;
    m_hpLabel = new QLabel;
    m_manaLabel = new QLabel;
    m_goldLabel = new QLabel;
    mainLayout->addWidget(m_raceLabel);
    mainLayout->addWidget(m_classLabel);
    mainLayout->addWidget(m_levelLabel);
    mainLayout->addWidget(m_xpLabel);
    mainLayout->addWidget(m_hpLabel);
    mainLayout->addWidget(m_manaLabel);
    mainLayout->addWidget(m_goldLabel);

    mainLayout->addSpacing(10);

    // Stats table
    auto *statsGroup = new QGroupBox(tr("Effective Stats"));
    auto *statsLayout = new QVBoxLayout(statsGroup);
    m_statsTable = new QTableWidget(6, 2);
    m_statsTable->setHorizontalHeaderLabels({tr("Stat"), tr("Value")});
    m_statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statsTable->setMaximumHeight(150);
    statsLayout->addWidget(m_statsTable);
    mainLayout->addWidget(statsGroup);

    // Equipment table
    auto *equipGroup = new QGroupBox(tr("Equipped Items"));
    auto *equipLayout = new QVBoxLayout(equipGroup);
    m_equipTable = new QTableWidget(0, 3);
    m_equipTable->setHorizontalHeaderLabels({tr("Slot"), tr("Item"), tr("Bonus")});
    m_equipTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_equipTable->setMaximumHeight(120);
    equipLayout->addWidget(m_equipTable);
    mainLayout->addWidget(equipGroup);

    // Guild table
    auto *guildGroup = new QGroupBox(tr("Guilds"));
    auto *guildLayout = new QVBoxLayout(guildGroup);
    m_guildTable = new QTableWidget(0, 2);
    m_guildTable->setHorizontalHeaderLabels({tr("Guild"), tr("Level")});
    m_guildTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_guildTable->setMaximumHeight(100);
    guildLayout->addWidget(m_guildTable);
    mainLayout->addWidget(guildGroup);

    // Spells
    auto *spellsGroup = new QGroupBox(tr("Known Spells"));
    auto *spellsLayout = new QVBoxLayout(spellsGroup);
    m_spellsText = new QTextEdit;
    m_spellsText->setReadOnly(true);
    m_spellsText->setMaximumHeight(100);
    spellsLayout->addWidget(m_spellsText);
    mainLayout->addWidget(spellsGroup);

    // Exit
    m_exitBtn = new QPushButton(tr("Close"));
    mainLayout->addWidget(m_exitBtn, 0, Qt::AlignRight);

    connect(m_memberCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CharacterSheetDialog::onMemberChanged);
    connect(m_exitBtn, &QPushButton::clicked, this, &CharacterSheetDialog::onExitClicked);
}

void CharacterSheetDialog::refreshSheet() {
    auto *gsm = gameStateManager::instance();
    const auto& members = gsm->getPartyMembers();

    // Populate member combo. Filling it emits currentIndexChanged, which is
    // wired to onMemberChanged -> refreshSheet; without blocking that signal
    // the refill re-enters this function and recurses until the stack blows.
    const int previous = m_memberCombo->currentIndex();
    {
        const QSignalBlocker blocker(m_memberCombo);
        m_memberCombo->clear();
        for (int i = 0; i < members.size(); ++i) {
            m_memberCombo->addItem(tr("%1 (Lv %2)").arg(members[i].name).arg(members[i].level));
        }
        if (previous >= 0 && previous < m_memberCombo->count()) {
            m_memberCombo->setCurrentIndex(previous);
        }
    }

    if (members.isEmpty()) {
        m_nameLabel->setText(tr("No party members"));
        return;
    }

    int idx = m_memberCombo->currentIndex();
    if (idx < 0 || idx >= members.size()) idx = 0;
    const Character& c = members[idx];

    // Identity
    m_nameLabel->setText(c.name);
    m_raceLabel->setText(tr("Race: %1").arg(c.race));
    m_classLabel->setText(tr("Class: %1").arg(c.guildLevels.isEmpty() ? tr("None") : c.guildLevels.keys().first()));
    m_levelLabel->setText(tr("Level: %1").arg(c.level));
    m_xpLabel->setText(tr("Experience: %1").arg(c.experience));
    m_hpLabel->setText(tr("HP: %1 / %2").arg(c.hp).arg(c.maxHp));
    m_manaLabel->setText(tr("Mana: %1 / %2").arg(c.mana).arg(c.maxMana));
    m_goldLabel->setText(tr("Gold: %1").arg(c.gold));

    // Effective stats
    QStringList statNames = {tr("Strength"), tr("Intelligence"), tr("Wisdom"),
                             tr("Constitution"), tr("Charisma"), tr("Dexterity")};
    m_statsTable->setRowCount(statNames.size());
    for (int i = 0; i < statNames.size(); ++i) {
        m_statsTable->setItem(i, 0, new QTableWidgetItem(statNames[i]));
        m_statsTable->setItem(i, 1, new QTableWidgetItem(QString::number(c.effectiveStat(statNames[i]))));
    }

    // Equipped items — look up slot from ItemDatabase via item ID
    m_equipTable->setRowCount(0);
    for (int slot = 0; slot < ItemSlot::Count; ++slot) {
        bool found = false;
        for (const auto& item : c.equipped) {
            if (item.M5467 != 1) continue;
            const ItemDef* def = ItemDatabase::instance().byId(item.M4E97);
            if (!def || def->slot() != slot) continue;
            int row = m_equipTable->rowCount();
            m_equipTable->insertRow(row);
            m_equipTable->setItem(row, 0, new QTableWidgetItem(ItemSlot::name(static_cast<ItemSlot::Slot>(slot))));
            m_equipTable->setItem(row, 1, new QTableWidgetItem(item.name));
            QString bonus;
            if (def->att > 0) bonus += tr("+%1 att ").arg(def->att);
            if (def->def > 0) bonus += tr("+%1 def").arg(def->def);
            m_equipTable->setItem(row, 2, new QTableWidgetItem(bonus.isEmpty() ? tr("-") : bonus));
            found = true;
            break;
        }
        if (!found) {
            int row = m_equipTable->rowCount();
            m_equipTable->insertRow(row);
            m_equipTable->setItem(row, 0, new QTableWidgetItem(ItemSlot::name(static_cast<ItemSlot::Slot>(slot))));
            m_equipTable->setItem(row, 1, new QTableWidgetItem(tr("(empty)")));
            m_equipTable->setItem(row, 2, new QTableWidgetItem(tr("-")));
        }
    }

    // Guilds
    m_guildTable->setRowCount(0);
    for (auto it = c.guildLevels.begin(); it != c.guildLevels.end(); ++it) {
        int row = m_guildTable->rowCount();
        m_guildTable->insertRow(row);
        m_guildTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        m_guildTable->setItem(row, 1, new QTableWidgetItem(QString::number(it.value())));
    }

    // Spells
    SpellBook& sb = SpellBook::instance();
    if (sb.isLoaded()) {
        QList<SpellDef> spells = sb.spellsFor(c);
        if (spells.isEmpty()) {
            m_spellsText->setPlainText(tr("No spells known."));
        } else {
            QStringList lines;
            for (const auto& s : spells) {
                lines << tr("%1 (Mana: %2)").arg(s.name).arg(s.mana);
            }
            m_spellsText->setPlainText(lines.join("\n"));
        }
    } else {
        m_spellsText->setPlainText(tr("Spell book not loaded."));
    }
}

void CharacterSheetDialog::onMemberChanged(int index) {
    Q_UNUSED(index)
    refreshSheet();
}

void CharacterSheetDialog::onExitClicked() {
    accept();
}
