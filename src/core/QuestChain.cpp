#include "QuestChain.h"

QList<QuestStep> QuestChain::steps() {
    QList<QuestStep> steps;

    auto make = [](int index, const QString& id, const QString& title,
                   const QString& desc) {
        QuestStep s;
        s.index = index;
        s.id = id;
        s.title = title;
        s.description = desc;
        return s;
    };

    QuestStep s0 = make(0, "first_descent", "Into the Dark",
                        "Descend into the mines and learn what lurks below.");
    s0.requiresDepth = 1;
    s0.rewardGold = 100;
    s0.rewardXp = 100;
    steps.append(s0);

    QuestStep s1 = make(1, "grotto_warden", "The Grotto Warden",
                        "Something guards the way past the fifth floor. End it.");
    s1.bossFloor = 5;
    s1.rewardGold = 1000;
    s1.rewardXp = 5000;
    steps.append(s1);

    QuestStep s2 = make(2, "deeper", "Beneath the Crypts",
                        "Push past the crypts and into the black marble halls.");
    s2.requiresDepth = 10;
    s2.rewardGold = 2500;
    s2.rewardXp = 10000;
    steps.append(s2);

    QuestStep s3 = make(3, "bone_tyrant", "The Bone Tyrant",
                        "The throne of bone has a king. Unseat it.");
    s3.bossFloor = 10;
    s3.rewardGold = 5000;
    s3.rewardXp = 25000;
    steps.append(s3);

    QuestStep s4 = make(4, "devils_threshold", "The Devil's Threshold",
                        "Reach the last gate before the Prince of Devils.");
    s4.requiresDepth = 15;
    s4.rewardGold = 10000;
    s4.rewardXp = 50000;
    steps.append(s4);

    QuestStep s5 = make(5, "prince_of_devils", "The Prince of Devils",
                        "Destroy the Prince and close the gate forever.");
    s5.bossFloor = 15;
    s5.rewardGold = 50000;
    s5.rewardXp = 250000;
    steps.append(s5);

    return steps;
}

int QuestChain::stepCount() {
    return steps().size();
}

QuestStep QuestChain::step(int index) {
    QList<QuestStep> all = steps();
    if (index < 0 || index >= all.size()) {
        QuestStep invalid;
        invalid.index = -1;
        return invalid;
    }
    return all[index];
}

QString QuestChain::objectiveText(const QuestStep& s) {
    if (s.index < 0) return "No active quest.";
    if (s.bossFloor > 0) {
        return QString("Defeat the guardian of floor %1.").arg(s.bossFloor);
    }
    if (s.requiresDepth > 0) {
        return QString("Reach dungeon level %1.").arg(s.requiresDepth);
    }
    if (!s.requiresItem.isEmpty()) {
        return QString("Recover the %1.").arg(s.requiresItem);
    }
    return s.description;
}

bool QuestChain::isStepComplete(const QuestStep& s,
                                int deepestFloorReached,
                                const QList<int>& defeatedBossFloors,
                                const QStringList& itemsHeld) {
    if (s.index < 0) return false;

    if (s.bossFloor > 0) {
        return defeatedBossFloors.contains(s.bossFloor);
    }
    if (s.requiresDepth > 0) {
        return deepestFloorReached >= s.requiresDepth;
    }
    if (!s.requiresItem.isEmpty()) {
        return itemsHeld.contains(s.requiresItem);
    }
    return false;
}

int QuestChain::nextStepIndex(int deepestFloorReached,
                              const QList<int>& defeatedBossFloors,
                              const QStringList& itemsHeld) {
    QList<QuestStep> all = steps();
    for (const QuestStep& s : all) {
        if (!isStepComplete(s, deepestFloorReached, defeatedBossFloors, itemsHeld)) {
            return s.index;
        }
    }
    return -1;
}

bool QuestChain::isChainComplete(int deepestFloorReached,
                                 const QList<int>& defeatedBossFloors,
                                 const QStringList& itemsHeld) {
    return nextStepIndex(deepestFloorReached, defeatedBossFloors, itemsHeld) == -1;
}
