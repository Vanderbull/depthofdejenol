#ifndef COMBATSTATE_H
#define COMBATSTATE_H

#include <QString>
#include <QList>
#include <QMetaType>

// A single combat participant (party member or monster).
struct CombatParticipant {
    QString name;
    int hp = 10;
    int maxHp = 10;
    int att = 5;       // attack power
    int def = 5;       // defense
    int speed = 5;     // initiative
    int dex = 5;       // dexterity (to-hit, initiative)
    bool isPlayer = false;
    bool isAlive = true;
    bool hasActed = false;  // true once this participant has acted this round
    bool isDefending = false;  // true after Defend; halves incoming damage this round
    bool hasFled = false;      // true once this monster has fled the fight
    bool deathAnnounced = false;  // true once the death message has been emitted

    // Monster-specific
    int level = 1;
    int damageMod = 100;  // percentage (100 = normal)
    int swings = 1;
    int levelScale = 0;   // damage scaling with level

    // Mana (for spellcasting)
    int mana = 0;
    int maxMana = 0;

    // Status effects (bitmask from GameConstants::EntityStatus)
    uint statusFlags = 0;
    // Duration (in rounds) for each status effect
    int poisonDuration = 0;
    int blindDuration = 0;
    int fireDuration = 0;
    int confusionDuration = 0;

    // School mechanics (driven by SpellMechanics)
    int slowDuration = 0;   // rounds of cold slow remaining
    int slowAmount = 0;     // speed removed by the slow, restored on expiry
    int stunDuration = 0;   // rounds the participant loses its turn (mind)

    // Monster abilities
    bool canPoison = false;
    bool canBreathFire = false;
    bool canRegenerate = false;
    int regenerateAmount = 0;
    bool canCastSpells = false;
};

// Pure combat state — no Qt UI dependency. Holds participants, initiative order,
// round counter, and per-participant action state.
class CombatState {
public:
    CombatState();

    // Participant management
    void addParticipant(const CombatParticipant& p);
    void clear();
    int participantCount() const { return m_participants.size(); }

    // Initiative
    void rollInitiative();
    const QList<int>& initiativeOrder() const { return m_initiativeOrder; }

    // Turn cycling
    void startRound();
    bool nextTurn();            // advances to next living participant; false if round over
    int currentTurnIndex() const { return m_currentTurnIndex; }
    const CombatParticipant* currentParticipant() const;
    int currentRound() const { return m_currentRound; }

    // Queries
    bool isCombatOver() const;
    int livingPlayerCount() const;
    int livingMonsterCount() const;
    QList<int> livingPlayerIndices() const;
    QList<int> livingMonsterIndices() const;

    // Direct access for combat resolution
    CombatParticipant& participant(int index) { return m_participants[index]; }
    const CombatParticipant& participant(int index) const { return m_participants[index]; }

    // Mark a participant as having acted this round
    void markActed(int index);

    // Reset hasActed for all participants (call at round start)
    void resetActedFlags();

private:
    QList<CombatParticipant> m_participants;
    QList<int> m_initiativeOrder;  // indices into m_participants, sorted by initiative
    int m_currentRound = 0;
    int m_currentTurnIndex = -1;   // index into m_initiativeOrder
};

#endif // COMBATSTATE_H
