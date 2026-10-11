#include "QuestChainDriver.h"
#include "QuestChain.h"
#include "gameStateManager.h"
#include "src/journal_dialog/JournalDialog.h"
#include "DungeonLevelState.h"

int QuestChainDriver::deepestFloor() {
    gameStateManager* gsm = gameStateManager::instance();
    int d = gsm->getGameValue("DeepestFloor").toInt();
    if (d < 1) d = gsm->getGameValue("DungeonLevel").toInt();
    return d;
}

QList<int> QuestChainDriver::defeatedBossFloors() {
    QList<int> floors;
    for (int i = 1; i <= 15; ++i) {
        if (DungeonLevelRegistry::instance().hasLevel(i)) {
            if (DungeonLevelRegistry::instance().levelForEdit(i).bossDefeated) {
                floors.append(i);
            }
        }
    }
    return floors;
}

QStringList QuestChainDriver::itemsHeld() {
    QStringList items;
    auto& members = gameStateManager::instance()->getPartyMembers();
    for (const auto& member : members) {
        for (const auto& item : member.inventory) {
            if (!items.contains(item.name)) {
                items.append(item.name);
            }
        }
    }
    return items;
}

bool QuestChainDriver::isChainComplete() {
    return QuestChain::isChainComplete(deepestFloor(), defeatedBossFloors(), itemsHeld());
}

int QuestChainDriver::nextStepIndex() {
    return QuestChain::nextStepIndex(deepestFloor(), defeatedBossFloors(), itemsHeld());
}

QString QuestChainDriver::progressText() {
    int next = nextStepIndex();
    int total = QuestChain::stepCount();
    if (isChainComplete()) {
        return QString("Main quest complete — the Prince of Devils is dead.");
    }
    if (next < 0) return QString("Main quest: no active step.");
    QuestStep step = QuestChain::step(next);
    return QString("Step %1 of %2: %3").arg(next + 1).arg(total).arg(step.title);
}

QuestChainDriver::AdvanceResult QuestChainDriver::advance() {
    AdvanceResult result;

    gameStateManager* gsm = gameStateManager::instance();
    const int floor = deepestFloor();
    const QList<int> bosses = defeatedBossFloors();
    const QStringList items = itemsHeld();

    // Check every step. If it was not complete before but is now, grant reward.
    // We track "before" by checking each step individually.
    QList<QuestStep> all = QuestChain::steps();
    for (const QuestStep& step : all) {
        bool wasComplete = QuestChain::isStepComplete(step, floor, bosses, items);
        if (wasComplete) {
            // This step is complete. We need to know if it was *just* completed.
            // Since we can't diff against a previous snapshot, we check whether
            // the step's index is <= the current next step index. If a step is
            // complete and its index is beyond the previous nextStepIndex, it was
            // newly completed.
            //
            // A simpler approach: a step is "newly complete" if it is complete AND
            // the *previous* step (index - 1) is also complete (or index == 0).
            // This handles the chain correctly because steps unlock in order.
            bool previousComplete = true;
            if (step.index > 0) {
                QuestStep prev = QuestChain::step(step.index - 1);
                previousComplete = QuestChain::isStepComplete(prev, floor, bosses, items);
            }

            if (previousComplete) {
                // This step was just unlocked. Grant reward if not already given.
                // We track "already given" by checking if the step's reward was
                // already applied. Since we don't have a "rewardsGiven" set,
                // we use a game-state key to track it.
                QString key = QString("QuestRewardGiven_%1").arg(step.id);
                if (!gsm->getGameValue(key).toBool()) {
                    if (step.rewardGold > 0) gsm->addPartyGold(step.rewardGold);
                    if (step.rewardXp > 0) gsm->addExperienceToParty(step.rewardXp);
                    gsm->setGameValue(key, true);

                    StepReward r;
                    r.stepIndex = step.index;
                    r.stepId = step.id;
                    r.stepTitle = step.title;
                    r.gold = step.rewardGold;
                    r.xp = step.rewardXp;
                    result.grantedRewards.append(r);

                    JournalDialog::addEntry(QStringLiteral("Quest"),
                        QStringLiteral("Quest reward for '%1': +%2 gold, +%3 XP")
                            .arg(step.title).arg(step.rewardGold).arg(step.rewardXp));
                }
            }
        }
    }

    if (isChainComplete()) {
        result.chainJustCompleted = true;
    }

    return result;
}
