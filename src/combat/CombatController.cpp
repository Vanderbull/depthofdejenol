#include "CombatController.h"
#include "src/core/gameStateManager.h"
#include "src/core/BossEncounter.h"
#include "src/core/DungeonThemes.h"
#include "src/combat/EncounterBuilder.h"
#include "src/combat/VictoryReward.h"
#include "src/core/QuestChainDriver.h"
#include "src/core/Endgame.h"
#include "src/core/DeathRecovery.h"
#include "src/journal_dialog/JournalDialog.h"
#include "src/core/GameSettings.h"
#include "src/items/ItemDatabase.h"
#include "src/core/DungeonLevelState.h"
#include "src/items/ConsumableResolver.h"
#include "src/victory_dialog/VictoryDialog.h"
#include "src/quest_board/QuestBoardDialog.h"
#include "src/core/QuestChainDriver.h"
#include <QInputDialog>
#include <QTimer>

CombatController::CombatController(QObject *parent)
    : QObject(parent)
    , m_combatState(new CombatState())
    , m_turnEngine(new TurnEngine())
    , m_combatActions(new CombatActions(m_combatState, m_turnEngine))
    , m_monsterAI(new MonsterAI(m_combatState, m_turnEngine, m_combatActions))
    , m_deathHandler(new CombatDeathHandler(m_combatState, m_turnEngine, m_combatActions))
{
    m_turnEngine->setCombatState(m_combatState);
}

CombatController::~CombatController() = default;

void CombatController::setCombatWidgets(QGroupBox* group, QLabel* monsterHpLabel,
                                         QListWidget* targetList, QLabel* partyHpLabel,
                                         QPushButton* attackBtn, QPushButton* defendBtn,
                                         QPushButton* fleeBtn, QPushButton* useItemBtn,
                                         QPushButton* spellBtn, QPushButton* rowBtn,
                                         QTimer* fxTimer)
{
    m_combatGroup = group;
    m_combatMonsterHpLabel = monsterHpLabel;
    m_combatTargetList = targetList;
    m_combatPartyHpLabel = partyHpLabel;
    m_combatAttackBtn = attackBtn;
    m_combatDefendBtn = defendBtn;
    m_combatFleeBtn = fleeBtn;
    m_combatUseItemBtn = useItemBtn;
    m_combatSpellBtn = spellBtn;
    m_combatRowBtn = rowBtn;
    m_combatFxTimer = fxTimer;
}

void CombatController::setLogCallback(std::function<void(const QString&)> callback)
{
    m_logCallback = callback;
}

void CombatController::setRenderCallback(std::function<void()> callback)
{
    m_renderCallback = callback;
}

bool CombatController::startCombatAt(const QPair<int, int>& pos)
{
    gameStateManager* gsm = gameStateManager::instance();
    QMap<QPair<int, int>, QString>& monsterPositions = *const_cast<QMap<QPair<int, int>, QString>*>(
        reinterpret_cast<const QMap<QPair<int, int>, QString>*>(&monsterPositions));

    if (!gsm->getPartyMembers().isEmpty()) {
        // Access monster positions through a different approach
    }

    // This is a simplified version - the actual implementation needs access to
    // m_monsterPositions from DungeonDialog. For now, we'll use a different approach.
    Q_UNUSED(pos);
    return false;
}

void CombatController::attack(int targetIndex)
{
    if (!m_inCombat || !m_combatActions || !m_turnEngine->hasCurrentParticipant()) return;
    if (!m_combatActions->isPlayerTurn()) return;

    QString result;
    m_combatActions->attack(targetIndex, result);
    if (m_logCallback) m_logCallback(result);

    QStringList deaths = m_deathHandler->processDeaths();
    for (const QString& d : deaths) {
        if (m_logCallback) m_logCallback(d);
    }

    syncCombatToGameState();
    m_turnEngine->markCurrentActed();
    updateCombatUI();
    advanceCombat();
}

void CombatController::defend()
{
    if (!m_inCombat || !m_combatActions || !m_turnEngine->hasCurrentParticipant()) return;
    if (!m_combatActions->isPlayerTurn()) return;

    QString result;
    m_combatActions->defend(result);
    if (m_logCallback) m_logCallback(result);

    m_turnEngine->markCurrentActed();
    updateCombatUI();
    advanceCombat();
}

bool CombatController::flee()
{
    if (!m_inCombat || !m_combatActions) return false;

    QString result;
    bool fled = m_combatActions->flee(result);
    if (m_logCallback) m_logCallback(result);

    if (fled) {
        m_inCombat = false;
        if (m_combatGroup) m_combatGroup->setVisible(false);
        if (m_logCallback) m_logCallback("You flee from combat!");
    } else {
        m_turnEngine->markCurrentActed();
        advanceCombat();
    }
    updateCombatUI();
    return fled;
}

void CombatController::useItem(int slotIndex)
{
    if (!m_inCombat || !m_combatActions || !m_turnEngine->hasCurrentParticipant()) return;
    if (!m_combatActions->isPlayerTurn()) return;

    gameStateManager* gsm = gameStateManager::instance();
    Character& c = gsm->getPartyMember(gsm->getCurrentCharacterIndex());

    QString result;
    m_combatActions->useItem(slotIndex, c, result);
    if (m_logCallback) m_logCallback(result);
    m_turnEngine->markCurrentActed();
    updateCombatUI();
    advanceCombat();
}

void CombatController::castSpell()
{
    // Spell casting is handled by the spell button in DungeonDialog
    // This is a placeholder for future combat spell integration
}

bool CombatController::togglePlayerRow(int participantIndex)
{
    if (!m_combatState) return false;
    if (participantIndex < 0 || participantIndex >= m_combatState->participantCount()) return false;

    CombatParticipant& p = m_combatState->participant(participantIndex);
    if (!p.isPlayer) return false;

    p.row = (p.row == CombatRow::Front) ? CombatRow::Back : CombatRow::Front;
    return true;
}

void CombatController::advanceCombat()
{
    if (!m_inCombat || !m_turnEngine || !m_combatState) return;

    if (m_combatState->isCombatOver()) {
        if (m_deathHandler->isVictory()) {
            handleVictory();
        } else if (m_deathHandler->isPartyWipe()) {
            handlePartyWipe();
        }
        return;
    }

    if (!m_turnEngine->nextTurn()) {
        m_turnEngine->startRound();
        QStringList statusMsgs = m_combatActions->tickStatusEffects();
        for (const QString& msg : statusMsgs) {
            if (m_logCallback) m_logCallback(msg);
        }
    }

    if (m_turnEngine->hasCurrentParticipant() && !m_combatActions->isPlayerTurn()) {
        QString aiResult = m_monsterAI->takeTurn();
        if (m_logCallback) m_logCallback(aiResult);
        if (aiResult.contains("damage") || aiResult.contains("slain")) {
            flashCombatFx(true);
        }
        QStringList deaths = m_deathHandler->processDeaths();
        for (const QString& d : deaths) {
            if (m_logCallback) m_logCallback(d);
        }
        syncCombatToGameState();
        updateCombatUI();
        advanceCombat();
    }
}

void CombatController::updateCombatUI()
{
    if (!m_combatState) return;

    QStringList monsterHp;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        const auto& p = m_combatState->participant(i);
        if (!p.isPlayer && p.isAlive) {
            monsterHp << QString("%1: %2/%3").arg(p.name).arg(p.hp).arg(p.maxHp);
        }
    }
    if (m_combatMonsterHpLabel) {
        m_combatMonsterHpLabel->setText("Monster: " + (monsterHp.isEmpty() ? "None" : monsterHp.join(", ")));
    }

    if (m_combatTargetList) {
        const int previous = selectedMonsterIndex();
        const int actingIdx = m_turnEngine ? m_turnEngine->currentParticipantIndex() : -1;
        m_combatTargetList->blockSignals(true);
        m_combatTargetList->clear();
        for (int i = 0; i < m_combatState->participantCount(); ++i) {
            const auto& p = m_combatState->participant(i);
            if (p.isPlayer || !p.isAlive) continue;

            QString label = QString("%1  %2/%3").arg(p.name).arg(p.hp).arg(p.maxHp);
            const QString status = combatStatusText(p);
            if (!status.isEmpty()) label += "  [" + status + "]";
            if (i == actingIdx) label.prepend("> ");

            auto* item = new QListWidgetItem(label, m_combatTargetList);
            item->setData(Qt::UserRole, i);
            if (i == previous) item->setSelected(true);
        }
        m_combatTargetList->blockSignals(false);

        if (selectedMonsterIndex() < 0 && m_combatTargetList->count() > 0) {
            m_combatTargetList->setCurrentRow(0);
        }
    }

    QStringList hpList;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        const auto& p = m_combatState->participant(i);
        if (p.isPlayer) {
            hpList << QString("%1 [%2]: %3/%4")
                          .arg(p.name)
                          .arg(p.row == CombatRow::Front ? "front" : "back")
                          .arg(p.hp).arg(p.maxHp);
        }
    }
    if (m_combatPartyHpLabel) {
        m_combatPartyHpLabel->setText("Party: " + hpList.join(" | "));
    }
}

int CombatController::selectedMonsterIndex() const
{
    if (!m_combatTargetList || !m_combatState) return -1;
    const QListWidgetItem* item = m_combatTargetList->currentItem();
    if (!item) return -1;
    const int idx = item->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= m_combatState->participantCount()) return -1;
    const auto& p = m_combatState->participant(idx);
    if (p.isPlayer || !p.isAlive) return -1;
    return idx;
}

int CombatController::selectFirstLivingMonster()
{
    if (!m_combatState || !m_combatTargetList) return -1;

    if (m_combatTargetList->count() == 0) {
        updateCombatUI();
        const int autoPicked = selectedMonsterIndex();
        if (autoPicked >= 0) return autoPicked;
    }

    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        const auto& p = m_combatState->participant(i);
        if (p.isPlayer || !p.isAlive) continue;
        for (int row = 0; row < m_combatTargetList->count(); ++row) {
            QListWidgetItem* item = m_combatTargetList->item(row);
            if (item && item->data(Qt::UserRole).toInt() == i) {
                m_combatTargetList->setCurrentRow(row);
                return i;
            }
        }
    }
    return -1;
}

QString CombatController::combatStatusText(const CombatParticipant& p)
{
    QStringList parts;
    if (p.poisonDuration > 0) parts << QString("<font color='green'>poison %1</font>").arg(p.poisonDuration);
    if (p.fireDuration > 0)   parts << QString("<font color='red'>fire %1</font>").arg(p.fireDuration);
    if (p.blindDuration > 0)  parts << QString("<font color='yellow'>blind %1</font>").arg(p.blindDuration);
    if (p.confusionDuration > 0) parts << QString("<font color='magenta'>confused %1</font>").arg(p.confusionDuration);
    if (p.stunDuration > 0)   parts << QString("<font color='cyan'>stunned %1</font>").arg(p.stunDuration);
    if (p.slowDuration > 0)   parts << QString("<font color='gray'>slowed %1</font>").arg(p.slowDuration);
    if (p.isDefending)        parts << "<font color='blue'>defending</font>";
    return parts.join(", ");
}

bool CombatController::isPlayerTurn() const
{
    return m_combatActions && m_combatActions->isPlayerTurn();
}

void CombatController::handleVictory()
{
    gameStateManager* gsm = gameStateManager::instance();
    int level = gsm->getGameValue("DungeonLevel").toInt();

    const QList<QVariantMap>& monsters = gsm->getMonsterData();
    int xp = VictoryReward::calculateXp(m_combatMonsterName, monsters);
    int gold = VictoryReward::calculateGold(m_combatMonsterName, monsters, gsm->getNgPlusLevel());
    QStringList loot = VictoryReward::calculateLoot(m_combatMonsterName, monsters, level);

    if (m_combatIsBoss) {
        xp = BossEncounter::bossXp(level);
    }

    gsm->addExperienceToParty(xp);
    gsm->addPartyGold(gold);

    for (int i = 0; i < gsm->getParty().members.size(); ++i) {
        Character& member = gsm->getPartyMember(i);
        if (!member.isAlive) continue;
        for (auto it = member.guildLevels.constBegin(); it != member.guildLevels.constEnd(); ++it) {
            member.addGuildExperience(it.key(), xp);
        }
    }

    QuestBoardDialog::reportKill(m_combatMonsterName, level);

    if (m_logCallback) {
        m_logCallback(QString("<font color='gold'>🏆 Victory! Gained %1 XP and %2 gold.</font>").arg(xp).arg(gold));
    }

    JournalDialog::addEntry(QStringLiteral("Combat"),
        QStringLiteral("Defeated %1 on floor %2 (+%3 XP, +%4 gold)")
            .arg(m_combatMonsterName).arg(level).arg(xp).arg(gold));
    if (m_combatIsBoss) {
        JournalDialog::addEntry(QStringLiteral("Combat"),
            QStringLiteral("Slew the boss %1 on floor %2!")
                .arg(m_combatMonsterName).arg(level));
    }

    for (const QString& itemName : loot) {
        HeldItem lootItem;
        lootItem.name = itemName;
        lootItem.identified = false;
        if (const ItemDef* def = ItemDatabase::instance().byName(itemName)) {
            lootItem.M4E97 = static_cast<int16_t>(def->id);
        }
        for (int i = 0; i < gsm->getParty().members.size(); ++i) {
            if (gsm->getPartyMember(i).isAlive) {
                gsm->addItemToCharacter(i, lootItem);
                if (m_logCallback) {
                    m_logCallback(QString("<font color='gold'>💰 Loot: %1 (unidentified)</font>").arg(itemName));
                }
                break;
            }
        }
    }

    if (m_combatIsBoss) {
        DungeonLevelRegistry::instance().levelForEdit(level).bossDefeated = true;
    }

    QuestChainDriver::AdvanceResult questResult = QuestChainDriver::advance();
    for (const auto& reward : questResult.grantedRewards) {
        if (m_logCallback) {
            m_logCallback(QString("<font color='gold'>📜 Quest reward: +%1 gold, +%2 XP (%3)</font>")
                           .arg(reward.gold).arg(reward.xp).arg(reward.stepTitle));
        }
    }

    m_inCombat = false;
    if (m_combatGroup) m_combatGroup->setVisible(false);

    if (questResult.chainJustCompleted) {
        QString title = Endgame::victoryTitle();
        QStringList paragraphs = Endgame::victoryParagraphs();
        VictoryDialog* dlg = new VictoryDialog(title, paragraphs, true, nullptr);
        dlg->setAttribute(Qt::WA_DeleteOnClose);

        GameRecord rec;
        rec.heroName = gsm->getPartyMembers().isEmpty() ? "Unknown" : gsm->getPartyMembers()[0].name;
        rec.highestLevel = gsm->getPartyMembers().isEmpty() ? 1 : gsm->getPartyMembers()[0].level;
        rec.mostGold = gsm->getPartyGold();
        rec.deepestFloor = level;
        rec.completionTimeSeconds = gsm->playTimeSeconds();
        rec.won = true;
        gsm->addGameRecord(rec);
        dlg->exec();
    }

    if (m_renderCallback) m_renderCallback();
}

void CombatController::handlePartyWipe()
{
    gameStateManager* gsm = gameStateManager::instance();
    if (m_logCallback) m_logCallback("<font color='red'>💀 Your party has been defeated!</font>");
    gsm->setGameValue("isAlive", 0);

    JournalDialog::addEntry(QStringLiteral("Combat"),
        QStringLiteral("The party was wiped out on floor %1.")
            .arg(gsm->getGameValue("DungeonLevel").toInt()));

    for (int i = 0; i < gsm->getParty().members.size(); ++i) {
        gsm->saveCharacterToFile(i);
    }

    m_inCombat = false;
    if (m_combatGroup) m_combatGroup->setVisible(false);
}

void CombatController::syncCombatToGameState()
{
    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();
    int memberIdx = 0;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        const auto& p = m_combatState->participant(i);
        if (p.isPlayer && memberIdx < members.size()) {
            bool wasAlive = members[memberIdx].isAlive;
            members[memberIdx].hp = p.hp;
            members[memberIdx].isAlive = p.isAlive;
            if (!p.isAlive && wasAlive) {
                int level = gsm->getGameValue("DungeonLevel").toInt();
                int x = gsm->getGameValue("DungeonX").toInt();
                int y = gsm->getGameValue("DungeonY").toInt();
                DeathRecovery::killCharacter(members[memberIdx], level, x, y);
                JournalDialog::addEntry(QStringLiteral("Combat"),
                    QStringLiteral("%1 fell in battle on floor %2 (%3,%4).")
                        .arg(members[memberIdx].name).arg(level).arg(x).arg(y));
            }
            memberIdx++;
        }
    }
}

void CombatController::flashCombatFx(bool damage)
{
    if (!combatFxEnabled()) return;
    if (!m_combatGroup) return;

    const QString color = damage ? "rgba(255,0,0,0.3)" : "rgba(0,255,0,0.3)";
    m_combatGroup->setStyleSheet(
        QString("QGroupBox { background-color: %1; }").arg(color));
    if (m_combatFxTimer) {
        m_combatFxTimer->start(GameSettings::instance().combatSpeedMs());
    }
}

bool CombatController::combatFxEnabled()
{
    const QVariant v = gameStateManager::instance()->getGameValue("CombatFxEnabled");
    if (!v.isValid() || v.isNull()) return true;
    return v.toBool();
}
