#ifndef QUESTCHAIN_DRIVER_H
#define QUESTCHAIN_DRIVER_H

#include <QString>
#include <QStringList>
#include <QList>

// Drives the main quest chain from live game state. Reads progress from
// gameStateManager (DeepestFloor, DungeonLevelRegistry, party inventory),
// grants rewards for newly completed steps, and writes journal entries.
// Called from DungeonDialog::handleVictory() after each boss kill.
class QuestChainDriver {
public:
    struct StepReward {
        int stepIndex = -1;
        QString stepId;
        QString stepTitle;
        int gold = 0;
        int xp = 0;
    };

    struct AdvanceResult {
        bool chainJustCompleted = false;
        QList<StepReward> grantedRewards;
    };

    // Check live state, grant rewards for any newly completed steps, and
    static AdvanceResult advance();

    // True when the chain is complete (Prince of Devils defeated).
    static bool isChainComplete();

    // The index of the next incomplete step, or -1 when done.
    static int nextStepIndex();

    // Human-readable progress for the journal / quest board.
    static QString progressText();

private:
    static int deepestFloor();
    static QList<int> defeatedBossFloors();
    static QStringList itemsHeld();
};

#endif // QUESTCHAIN_DRIVER_H
