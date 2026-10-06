#ifndef QUESTCHAIN_H
#define QUESTCHAIN_H

#include <QString>
#include <QStringList>
#include <QList>

// One step of the main quest. Steps unlock in order; each has a goal the
// game checks against world state.
struct QuestStep {
    int index = 0;
    QString id;
    QString title;
    QString description;
    // What completes this step.
    bool requiresBoss(int floor) const { return bossFloor == floor; }
    int bossFloor = 0;          // defeat the boss on this floor
    int requiresDepth = 0;      // reach this depth
    QString requiresItem;       // carry this item
    int rewardGold = 0;
    int rewardXp = 0;
};

// The main quest: a chain of steps from the first descent to the Prince.
class QuestChain {
public:
    static QList<QuestStep> steps();

    static int stepCount();

    // The step at an index, or an invalid step (index -1) when out of range.
    static QuestStep step(int index);

    // Human-readable goal for a step.
    static QString objectiveText(const QuestStep& s);

    // Has a step's goal been met?
    //  - boss steps need the boss on that floor dead
    //  - depth steps need the party to have reached that floor
    //  - item steps need the item in the party's possession
    static bool isStepComplete(const QuestStep& s,
                               int deepestFloorReached,
                               const QList<int>& defeatedBossFloors,
                               const QStringList& itemsHeld);

    // Index of the next incomplete step, or -1 when the chain is done.
    static int nextStepIndex(int deepestFloorReached,
                             const QList<int>& defeatedBossFloors,
                             const QStringList& itemsHeld);

    // Is the whole chain complete (the Prince is dead)?
    static bool isChainComplete(int deepestFloorReached,
                                const QList<int>& defeatedBossFloors,
                                const QStringList& itemsHeld);
};

#endif // QUESTCHAIN_H
