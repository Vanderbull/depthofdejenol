#ifndef MONSTERAI_H
#define MONSTERAI_H

#include "CombatState.h"
#include "TurnEngine.h"
#include "CombatActions.h"
#include <QString>
#include <QList>

// Monster AI — decides what a monster does on its turn. Pure logic, no Qt UI dependency.
// Uses att, def, hits, numGroups from monster data.
class MonsterAI {
public:
    MonsterAI(CombatState* state, TurnEngine* engine, CombatActions* actions);

    // Decide and execute the current monster's action.
    // Returns a result string describing what happened.
    QString takeTurn();

    // Check if current participant is a monster
    bool isMonsterTurn() const;

    // AI decision enum
    enum class Decision { Attack, CastSpell, Flee };

    // Decide what to do (without executing)
    Decision decide() const;

    // Choose a target for the current monster
    int chooseTarget() const;

private:
    CombatState* m_state;
    TurnEngine* m_engine;
    CombatActions* m_actions;
};

#endif // MONSTERAI_H
