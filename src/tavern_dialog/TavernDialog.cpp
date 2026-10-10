#include "TavernDialog.h"
#include "gameStateManager.h"
#include "src/core/GoldSinks.h"
#include "character.h"
#include <QtWidgets>
#include <QMessageBox>

TavernDialog::TavernDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    refreshStatus();
}

TavernDialog::~TavernDialog() = default;

void TavernDialog::setupUi() {
    setWindowTitle(tr("The Tavern"));
    setMinimumSize(400, 350);

    auto *mainLayout = new QVBoxLayout(this);

    // Gold display
    m_goldLabel = new QLabel;
    m_goldLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(m_goldLabel);

    // Party status
    m_hpLabel = new QLabel;
    m_manaLabel = new QLabel;
    m_statusLabel = new QLabel;
    mainLayout->addWidget(m_hpLabel);
    mainLayout->addWidget(m_manaLabel);
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addSpacing(10);

    // Rest section
    auto *restGroup = new QGroupBox(tr("Rest"));
    auto *restLayout = new QHBoxLayout(restGroup);
    restLayout->addWidget(new QLabel(tr("Hours:")));
    m_hoursSpin = new QSpinBox;
    m_hoursSpin->setRange(1, 24);
    m_hoursSpin->setValue(8);
    restLayout->addWidget(m_hoursSpin);
    m_restBtn = new QPushButton(tr("Rest"));
    restLayout->addWidget(m_restBtn);
    mainLayout->addWidget(restGroup);

    // Cure section
    auto *cureGroup = new QGroupBox(tr("Cure Status"));
    auto *cureLayout = new QVBoxLayout(cureGroup);
    m_curePoisonCheck = new QCheckBox(tr("Cure Poison"));
    m_cureBlindCheck = new QCheckBox(tr("Cure Blindness"));
    cureLayout->addWidget(m_curePoisonCheck);
    cureLayout->addWidget(m_cureBlindCheck);
    m_cureBtn = new QPushButton(tr("Cure"));
    cureLayout->addWidget(m_cureBtn);
    mainLayout->addWidget(cureGroup);

    // Advance time
    m_advanceBtn = new QPushButton(tr("Advance Time (1 year)"));
    mainLayout->addWidget(m_advanceBtn);

    // Log
    m_log = new QTextEdit;
    m_log->setReadOnly(true);
    m_log->setMaximumHeight(80);
    mainLayout->addWidget(m_log);

    // Exit
    m_exitBtn = new QPushButton(tr("Exit"));
    mainLayout->addWidget(m_exitBtn, 0, Qt::AlignRight);

    connect(m_restBtn, &QPushButton::clicked, this, &TavernDialog::onRestClicked);
    connect(m_cureBtn, &QPushButton::clicked, this, &TavernDialog::onCureClicked);
    connect(m_advanceBtn, &QPushButton::clicked, this, &TavernDialog::onAdvanceTimeClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &TavernDialog::onExitClicked);
}

void TavernDialog::refreshStatus() {
    auto *gsm = gameStateManager::instance();
    int partyGold = gsm->getPartyGold();
    m_goldLabel->setText(tr("Party Gold: %1").arg(partyGold));

    // Show first living member's HP/mana as representative
    const auto& members = gsm->getPartyMembers();
    int totalHp = 0, totalMaxHp = 0, totalMana = 0, totalMaxMana = 0;
    bool anyPoisoned = false, anyBlinded = false;
    for (const auto& c : members) {
        if (!c.isAlive) continue;
        totalHp += c.hp;
        totalMaxHp += c.maxHp;
        totalMana += c.mana;
        totalMaxMana += c.maxMana;
        if (c.statusFlags & GameConstants::Poisoned) anyPoisoned = true;
        if (c.statusFlags & GameConstants::Blinded) anyBlinded = true;
    }

    m_hpLabel->setText(tr("Party HP: %1 / %2").arg(totalHp).arg(totalMaxHp));
    m_manaLabel->setText(tr("Party Mana: %1 / %2").arg(totalMana).arg(totalMaxMana));

    QStringList statuses;
    if (anyPoisoned) statuses << tr("Poisoned");
    if (anyBlinded) statuses << tr("Blinded");
    m_statusLabel->setText(tr("Status: %1").arg(statuses.isEmpty() ? tr("Normal") : statuses.join(", ")));
}

void TavernDialog::onRestClicked() {
    auto *gsm = gameStateManager::instance();
    int hours = m_hoursSpin->value();

    // Cost: GoldSinks rate per hour per living member
    const auto& members = gsm->getPartyMembers();
    int livingCount = 0;
    for (const auto& c : members) if (c.isAlive) livingCount++;

    int cost = restCost(hours, livingCount);
    if (cost > gsm->getPartyGold()) {
        QMessageBox::warning(this, tr("Not Enough Gold"),
                             tr("Resting %1 hours costs %2 gold.").arg(hours).arg(cost));
        return;
    }

    if (!gsm->spendPartyGold(cost)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to spend gold."));
        return;
    }

    // Restore HP and mana for all living members
    auto& mutableMembers = gsm->getPartyMembers();
    for (auto& c : mutableMembers) {
        if (!c.isAlive) continue;
        c.hp = c.maxHp;
        c.mana = c.maxMana;
    }

    m_log->append(tr("Rested %1 hours. Restored HP and mana for %2 members. (-%3 gold)")
                      .arg(hours).arg(livingCount).arg(cost));
    refreshStatus();
}

void TavernDialog::onCureClicked() {
    auto *gsm = gameStateManager::instance();
    bool curePoison = m_curePoisonCheck->isChecked();
    bool cureBlind = m_cureBlindCheck->isChecked();

    if (!curePoison && !cureBlind) {
        QMessageBox::information(this, tr("Cure"), tr("Select at least one status to cure."));
        return;
    }

    int cost = cureCost(curePoison, cureBlind);

    if (cost > gsm->getPartyGold()) {
        QMessageBox::warning(this, tr("Not Enough Gold"),
                             tr("Curing costs %1 gold.").arg(cost));
        return;
    }

    if (!gsm->spendPartyGold(cost)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to spend gold."));
        return;
    }

    // Apply cures to all affected members
    auto& members = gsm->getPartyMembers();
    for (int i = 0; i < members.size(); ++i) {
        if (!members[i].isAlive) continue;
        if (curePoison && (members[i].statusFlags & GameConstants::Poisoned)) {
            members[i].removeStatus(GameConstants::Poisoned);
            gsm->setCharacterStatus(i, GameConstants::Poisoned, false);
        }
        if (cureBlind && (members[i].statusFlags & GameConstants::Blinded)) {
            members[i].removeStatus(GameConstants::Blinded);
            gsm->setCharacterStatus(i, GameConstants::Blinded, false);
        }
    }

    QStringList cured;
    if (curePoison) cured << tr("Poison");
    if (cureBlind) cured << tr("Blindness");

    m_log->append(tr("Cured %1. (-%2 gold)").arg(cured.join(", ")).arg(cost));
    refreshStatus();
}

void TavernDialog::onAdvanceTimeClicked() {
    auto *gsm = gameStateManager::instance();
    gsm->incrementPartyAge(1);
    m_log->append(tr("Time advances by 1 year."));
    refreshStatus();
}

void TavernDialog::onExitClicked() {
    accept();
}

int TavernDialog::restCost(int hours, int livingMembers) {
    if (hours <= 0 || livingMembers <= 0) return 0;
    return hours * GoldSinks::restCostPerHour() * livingMembers;
}

int TavernDialog::cureCost(bool poison, bool blindness) {
    int cost = 0;
    if (poison) cost += GoldSinks::curePoisonCost();
    if (blindness) cost += GoldSinks::cureBlindnessCost();
    return cost;
}
