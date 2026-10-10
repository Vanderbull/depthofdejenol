#ifndef COMBATACTIONS_H
#define COMBATACTIONS_H

#include "CombatState.h"
#include "TurnEngine.h"
#include "character.h"
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

    // Apply pre-resolved spell damage to combat participants. Use this when the
    // damage (and mana cost) were already computed elsewhere — e.g. by the
    // spell-casting dialog — so the effect still flows through the combat layer
    // (death handling, HP bookkeeping) instead of poking participant HP.
    // isAoE hits every living participant of the same type as targetIndex.
    // Returns total damage dealt, or -1 on failure.
    int applySpellDamage(int targetIndex, const QString& spellName, int damage,
                         bool isAoE, QString& result);

    // School-aware variant of applySpellDamage: looks the spell up in the
    // SpellBook and applies the mechanic its school dictates (fire AoE, cold
    // slow, lightning chain, mind stun/confuse) via SpellMechanics. Falls back
    // to a single-target hit when the spell is unknown.
    // Returns total damage dealt, or -1 on failure.
    int applySpellDamageBySchool(int targetIndex, const QString& spellName,
                                 int damage, QString& result);

    // Cast a spell the school-aware way: resolves the spell in the SpellBook,
    // checks and deducts mana, rolls damage from the spell's own range, then
    // applies the school mechanic. Returns total damage dealt, or -1 on failure.
    int castSpellBySchool(int targetIndex, const QString& spellName, QString& result);

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

    // Remove a status effect from a participant (cure/cleanse).
    void cureStatus(int targetIndex, uint statusFlag, QString& result);

    // Use item: current participant uses consumable item from `user`'s
    // inventory. itemIndex is the inventory index of the item. The effect is
    // applied to both the combat participant and the Character, and the item
    // is consumed. Returns false when the item is missing or unusable.
    bool useItem(int itemIndex, Character& user, QString& result);

    // Flee: attempt to flee combat. Returns true on success.
    bool flee(QString& result);

    // Check if current participant is a player (can take actions)
    bool isPlayerTurn() const;

private:
    CombatState* m_state;
    TurnEngine* m_engine;
};

#endif // COMBATACTIONS_H
