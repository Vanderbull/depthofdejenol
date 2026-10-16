#include "CharacterSheetDialog.h"
#include "gameStateManager.h"
#include "character.h"
#include "src/items/ItemDatabase.h"
#include "src/spell_casting/SpellBook.h"
#include "src/spells/SpellIcon.h"
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
    m_hungerLabel = new QLabel;
    m_goldLabel = new QLabel;
    m_playTimeLabel = new QLabel;
    mainLayout->addWidget(m_raceLabel);
    mainLayout->addWidget(m_classLabel);
    mainLayout->addWidget(m_levelLabel);
    mainLayout->addWidget(m_xpLabel);
    mainLayout->addWidget(m_hpLabel);
    mainLayout->addWidget(m_manaLabel);
    mainLayout->addWidget(m_hungerLabel);
    mainLayout->addWidget(m_goldLabel);
    mainLayout->addWidget(m_playTimeLabel);
    m_statPointsLabel = new QLabel;
    m_statPointsLabel->setStyleSheet("font-weight: bold; color: #2c5f2d;");
    mainLayout->addWidget(m_statPointsLabel);

    mainLayout->addSpacing(10);

    // Stats table — 3 columns: name, value, spend button
    auto *statsGroup = new QGroupBox(tr("Effective Stats"));
    auto *statsLayout = new QVBoxLayout(statsGroup);
    m_statsTable = new QTableWidget(6, 3);
    m_statsTable->setHorizontalHeaderLabels({tr("Stat"), tr("Value"), tr("Spend")});
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
    m_spellsList = new QListWidget;
    m_spellsList->setMaximumHeight(120);
    m_spellsText = new QLabel(tr("Spell book not loaded."));
    m_spellsText->setWordWrap(true);
    spellsLayout->addWidget(m_spellsList);
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
    m_hungerLabel->setText(tr("Hunger: %1 / %2 (%3%)")
                           .arg(c.hunger).arg(c.maxHunger).arg(c.hungerPercent()));
    // Warn in colour when the member is running low, so the sheet agrees with
    // the dungeon party panel.
    m_hungerLabel->setStyleSheet(c.isHungerLow() ? "color: #b8860b; font-weight: bold;"
                                                 : QString());
    m_goldLabel->setText(tr("Gold: %1").arg(c.gold));

    // Play time — show elapsed time for this run (HH:MM:SS).
    const qint64 secs = gsm->playTimeSeconds();
    const qint64 h = secs / 3600;
    const qint64 m = (secs % 3600) / 60;
    const qint64 s = secs % 60;
    m_playTimeLabel->setText(tr("Play Time: %1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0')));

    // Stat point pool — the player must see what they have to spend.
    m_statPointsLabel->setText(tr("Stat Points: %1").arg(c.statPoints));

    // Effective stats — with per-stat spend buttons (v3.0.0 slice 3.4)
    QStringList statNames = {tr("Strength"), tr("Intelligence"), tr("Wisdom"),
                             tr("Constitution"), tr("Charisma"), tr("Dexterity")};
    m_statsTable->setRowCount(statNames.size());
    m_spendButtons.clear();
    for (int i = 0; i < statNames.size(); ++i) {
        m_statsTable->setItem(i, 0, new QTableWidgetItem(statNames[i]));
        m_statsTable->setItem(i, 1, new QTableWidgetItem(QString::number(c.effectiveStat(statNames[i]))));

        // Each row gets a spend button. Disabled when the pool is empty or the
        // stat is at its race cap — the player can see exactly what is left.
        const int cap = c.raceCapForStat(statNames[i]);
        const bool canSpend = c.statPoints > 0 && (cap <= 0 || c.baseStat(statNames[i]) < cap);
        auto* btn = new QPushButton(canSpend ? tr("+") : tr("Max"));
        btn->setEnabled(canSpend);
        btn->setMaximumWidth(40);
        btn->setProperty("statIndex", i);
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            onSpendStatPoint(btn->property("statIndex").toInt());
        });
        m_spendButtons.append(btn);
        m_statsTable->setCellWidget(i, 2, btn);
    }
    m_statsTable->setColumnWidth(2, 45);

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
            m_spellsText->setText(tr("No spells known."));
        } else {
            QStringList lines;
            m_spellsList->clear();
            for (const auto& s : spells) {
                QListWidgetItem *item = new QListWidgetItem;
                item->setText(QString("%1 (Mana: %2)").arg(s.name).arg(s.mana));
                item->setIcon(SpellIcon::iconForSpell(
                    QString("resources/images/spells/%1.png")
                        .arg(s.name.toLower().replace(" ", "_"))));
                m_spellsList->addItem(item);
            }
        }
    } else {
        m_spellsText->setText(tr("Spell book not loaded."));
    }
}

void CharacterSheetDialog::onMemberChanged(int index) {
    Q_UNUSED(index)
    refreshSheet();
}

void CharacterSheetDialog::onSpendStatPoint(int statIndex) {
    auto *gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();
    if (members.isEmpty()) return;

    int idx = m_memberCombo->currentIndex();
    if (idx < 0 || idx >= members.size()) idx = 0;

    const QStringList statNames = {tr("Strength"), tr("Intelligence"), tr("Wisdom"),
                                   tr("Constitution"), tr("Charisma"), tr("Dexterity")};
    if (statIndex < 0 || statIndex >= statNames.size()) return;

    const QString& statName = statNames[statIndex];
    Character& c = members[idx];
    const int cap = c.raceCapForStat(statName);

    if (c.spendStatPoint(statName, cap)) {
        QMessageBox::information(this, tr("Stat Increased"),
            tr("%1 increased to %2 (%3 points left)")
                .arg(statName)
                .arg(c.effectiveStat(statName))
                .arg(c.statPoints));
    } else {
        QString reason;
        if (c.statPoints <= 0) reason = tr("No stat points left — level up to earn more.");
        else if (cap > 0 && c.baseStat(statName) >= cap)
            reason = tr("%1 has reached its race cap of %2.").arg(statName).arg(cap);
        else reason = tr("Cannot increase %1.").arg(statName);
        QMessageBox::warning(this, tr("Cannot Increase"), reason);
    }
    refreshSheet();
}

void CharacterSheetDialog::onExitClicked() {
    accept();
}
