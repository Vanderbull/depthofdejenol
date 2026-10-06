#ifndef TURNENGINE_H
#define TURNENGINE_H

#include "CombatState.h"
#include <QObject>
#include <QString>

// Turn-based combat engine. Wraps CombatState and provides round/turn cycling.
// Pure logic — no Qt UI dependency. The UI polls or uses callbacks.
class TurnEngine {
public:
    TurnEngine();

    // Setup
    void setCombatState(CombatState* state);
    CombatState* combatState() const { return m_state; }

    // Round management
    void startRound();                          // roll initiative, reset acted flags
    int currentRound() const;
    bool isRoundOver() const;

    // Turn management
    bool nextTurn();                            // advance to next living participant
    int currentParticipantIndex() const;        // index into CombatState participants, -1 if none
    bool hasCurrentParticipant() const;
    QString currentParticipantName() const;

    // Action tracking
    void markCurrentActed();
    bool hasCurrentActed() const;

    // Combat status
    bool isCombatOver() const;
    QString combatStatus() const;               // human-readable summary

    // Living participants
    QList<int> livingPlayers() const;
    QList<int> livingMonsters() const;

private:
    CombatState* m_state = nullptr;
};

#endif // TURNENGINE_H
