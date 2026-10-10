#include "TempleDialog.h"
#include "gameStateManager.h"
#include "src/core/DeathRecovery.h"
#include "src/core/SoundEffects.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QMessageBox>

TempleDialog::TempleDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Temple - 'Pray you will win'");
    setMinimumSize(400, 300);
    setupUi();
    refreshStatus();
}

TempleDialog::~TempleDialog() = default;

void TempleDialog::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_goldLabel = new QLabel(this);
    layout->addWidget(m_goldLabel);

    m_hpLabel = new QLabel(this);
    layout->addWidget(m_hpLabel);

    m_manaLabel = new QLabel(this);
    layout->addWidget(m_manaLabel);

    m_partyList = new QListWidget(this);
    layout->addWidget(m_partyList);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_healBtn = new QPushButton("Heal (100g)", this);
    m_cureBtn = new QPushButton("Cure (50g)", this);
    m_resurrectBtn = new QPushButton("Resurrect", this);
    m_exitBtn = new QPushButton("Exit", this);
    btnLayout->addWidget(m_healBtn);
    btnLayout->addWidget(m_cureBtn);
    btnLayout->addWidget(m_resurrectBtn);
    btnLayout->addWidget(m_exitBtn);
    layout->addLayout(btnLayout);

    m_log = new QTextEdit(this);
    m_log->setReadOnly(true);
    m_log->setMaximumHeight(80);
    layout->addWidget(m_log);

    connect(m_healBtn, &QPushButton::clicked, this, &TempleDialog::onHealClicked);
    connect(m_cureBtn, &QPushButton::clicked, this, &TempleDialog::onCureClicked);
    connect(m_resurrectBtn, &QPushButton::clicked, this, &TempleDialog::onResurrectClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &TempleDialog::onExitClicked);
}

void TempleDialog::refreshStatus()
{
    gameStateManager *gsm = gameStateManager::instance();
    m_goldLabel->setText(QString("Party Gold: %1").arg(gsm->getPartyGold()));

    auto &members = gsm->getPartyMembers();
    m_partyList->clear();

    int totalHp = 0, totalMaxHp = 0;
    int totalMana = 0, totalMaxMana = 0;
    for (const Character &c : members) {
        QString status = c.isAlive ? "Alive" : "DEAD";
        m_partyList->addItem(QString("%1 (Lv %2) — %3 — HP: %4/%5 — MP: %6/%7")
            .arg(c.name).arg(c.level).arg(status)
            .arg(c.hp).arg(c.maxHp).arg(c.mana).arg(c.maxMana));
        if (c.isAlive) {
            totalHp += c.hp;
            totalMaxHp += c.maxHp;
            totalMana += c.mana;
            totalMaxMana += c.maxMana;
        }
    }

    m_hpLabel->setText(QString("Party HP: %1/%2").arg(totalHp).arg(totalMaxHp));
    m_manaLabel->setText(QString("Party Mana: %1/%2").arg(totalMana).arg(totalMaxMana));
}

void TempleDialog::onHealClicked()
{
    gameStateManager *gsm = gameStateManager::instance();
    int cost = 100;
    if (gsm->getPartyGold() < cost) {
        QMessageBox::warning(this, "Temple", "Not enough gold to heal.");
        return;
    }

    auto &members = gsm->getPartyMembers();
    bool healed = false;
    for (Character &c : members) {
        if (!c.isAlive) continue;
        if (c.hp < c.maxHp) {
            c.hp = c.maxHp;
            healed = true;
        }
        if (c.mana < c.maxMana) {
            c.mana = c.maxMana;
            healed = true;
        }
    }

    if (healed) {
        gsm->spendPartyGold(cost);
        SoundEffects::instance()->play(SoundEffects::Type::Rest);
        m_log->append("The temple priests chant and your wounds close. HP and Mana restored.");
    } else {
        m_log->append("Everyone is already at full health.");
    }
    refreshStatus();
}

void TempleDialog::onCureClicked()
{
    gameStateManager *gsm = gameStateManager::instance();
    int cost = 50;
    if (gsm->getPartyGold() < cost) {
        QMessageBox::warning(this, "Temple", "Not enough gold to cure.");
        return;
    }

    auto &members = gsm->getPartyMembers();
    bool cured = false;
    for (Character &c : members) {
        if (!c.isAlive) continue;
        // Clear every combat ailment the temple can treat.
        if (c.statusFlags & (StatusFlag::Poisoned | StatusFlag::Blinded | StatusFlag::OnFire)) {
            c.removeStatus(StatusFlag::Poisoned);
            c.removeStatus(StatusFlag::Blinded);
            c.removeStatus(StatusFlag::OnFire);
            cured = true;
        }
    }

    if (cured) {
        gsm->spendPartyGold(cost);
        SoundEffects::instance()->play(SoundEffects::Type::Rest);
        m_log->append("A warm light washes over you. All ailments are cured.");
    } else {
        m_log->append("There is nothing to cure.");
    }
    refreshStatus();
}

void TempleDialog::onResurrectClicked()
{
    gameStateManager *gsm = gameStateManager::instance();
    auto &members = gsm->getPartyMembers();

    Character *dead = nullptr;
    for (Character &c : members) {
        if (!c.isAlive) {
            dead = &c;
            break;
        }
    }

    if (!dead) {
        m_log->append("No one needs resurrection.");
        return;
    }

    BodyLocation loc;
    loc.valid = true;
    loc.inCity = true;

    // resurrect() deducts its fee from the gold passed by reference; apply
    // exactly that deduction to the party purse (there is no setPartyGold).
    int goldBefore = gsm->getPartyGold();
    int partyGold = goldBefore;
    QString reason;
    if (DeathRecovery::resurrect(*dead, loc, partyGold, reason)) {
        gsm->spendPartyGold(goldBefore - partyGold);
        SoundEffects::instance()->play(SoundEffects::Type::Rest);
        m_log->append(reason);
    } else {
        m_log->append(reason);
    }
    refreshStatus();
}

void TempleDialog::onExitClicked()
{
    close();
}
