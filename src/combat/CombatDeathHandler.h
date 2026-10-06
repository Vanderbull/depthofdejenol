#ifndef COMBATDEATHHANDLER_H
#define COMBATDEATHHANDLER_H

#include "CombatState.h"
#include "TurnEngine.h"
#include "CombatActions.h"
#include <QString>
#include <QStringList>

// Handles death in combat: individual character death, party wipe, and game over.
// Bridges combat state to party/game state.
class CombatDeathHandler {
public:
    CombatDeathHandler(CombatState* state, TurnEngine* engine, CombatActions* actions);

    // Check if combat is over due to party wipe (all players dead).
    bool isPartyWipe() const;

    // Check if combat is over due to victory (all monsters dead).
    bool isVictory() const;

    // Get the number of dead party members.
    int deadPartyMemberCount() const;

    // Get the number of living party members.
    int livingPartyMemberCount() const;

    // Process deaths: check for new deaths and return messages.
    // Call this after each combat action.
    QStringList processDeaths();

    // Handle game over: returns true if the game is over (party wipe).
    bool handleGameOver();

    // Revive all party members with 1 HP (for game over recovery).
    void reviveAllPartyMembers();

    // Check if a specific party member is dead.
    bool isPartyMemberDead(int combatIndex) const;

    // Get the names of dead party members.
    QStringList getDeadPartyMemberNames() const;

private:
    CombatState* m_state;
    TurnEngine* m_engine;
    CombatActions* m_actions;
};

#endif // COMBATDEATHHANDLER_H
