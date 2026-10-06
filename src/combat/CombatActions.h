#ifndef COMBATACTIONS_H
#define COMBATACTIONS_H

#include "CombatState.h"
#include "TurnEngine.h"
#include <QString>

// Combat actions a player can take. Pure logic — no Qt UI dependency.
// Each action modifies CombatState/TurnEngine and returns a result string.
class CombatActions {
public:
    CombatActions(CombatState* state, TurnEngine* engine);

    // Attack: current participant attacks target at targetIndex.
    // Returns damage dealt, or -1 on miss/failure.
    int attack(int targetIndex, QString& result);

    // Defend: current participant defends (halves damage taken this round).
    void defend(QString& result);

    // Cast spell: current participant casts spell at target.
    // spellPower determines damage/heal amount.
    int castSpell(int targetIndex, int spellPower, QString& result);

    // Cast spell with mana cost and damage range.
    // spellName: name of the spell (e.g. "Fireball")
    // manaCost: mana deducted from caster
    // damageRange: "min-max" damage range (e.g. "15-30")
    // isAoE: if true, hits all living targets of the same type as targetIndex
    // Returns total damage dealt, or -1 on failure.
    int castSpellAdvanced(int targetIndex, const QString& spellName, int manaCost,
                          const QString& damageRange, bool isAoE, QString& result);

    // Heal spell: current participant heals target.
    // healAmount: base healing amount
    int castHeal(int targetIndex, int healAmount, QString& result);

    // Check if caster has enough mana
    bool hasEnoughMana(int manaCost) const;

    // Status effects
    // Apply a status effect to a target for a duration (in rounds)
    void applyStatus(int targetIndex, uint statusFlag, int duration, QString& result);

    // Check if a participant has a status effect
    bool hasStatus(int index, uint statusFlag) const;

    // Get remaining duration of a status effect
    int getStatusDuration(int index, uint statusFlag) const;

    // Tick all status effects at round end. Returns a list of messages.
    QStringList tickStatusEffects();

    // Check if a participant is blinded (reduces to-hit)
    bool isBlinded(int index) const;

    // Check if a participant is confused (risks friendly fire)
    bool isConfused(int index) const;

    // Check if a participant is poisoned (DoT)
    bool isPoisoned(int index) const;

    // Check if a participant is on fire (DoT)
    bool isOnFire(int index) const;

    // Use item: current participant uses consumable item.
    // itemIndex is the inventory index of the item.
    bool useItem(int itemIndex, QString& result);

    // Flee: attempt to flee combat. Returns true on success.
    bool flee(QString& result);

    // Check if current participant is a player (can take actions)
    bool isPlayerTurn() const;

private:
    CombatState* m_state;
    TurnEngine* m_engine;
};

#endif // COMBATACTIONS_H
