#include "MonsterAI.h"
#include "src/core/GameConstants.h"
#include "src/spell_casting/SpellBook.h"
#include <QRandomGenerator>

MonsterAI::MonsterAI(CombatState* state, TurnEngine* engine, CombatActions* actions)
    : m_state(state), m_engine(engine), m_actions(actions) {}

bool MonsterAI::isMonsterTurn() const {
    if (!m_state || !m_engine) return false;
    int idx = m_engine->currentParticipantIndex();
    if (idx < 0) return false;
    return !m_state->participant(idx).isPlayer;
}

MonsterAI::Decision MonsterAI::decide() const {
    if (!m_state || !m_engine) return Decision::Attack;
    int idx = m_engine->currentParticipantIndex();
    if (idx < 0) return Decision::Attack;

    const CombatParticipant& monster = m_state->participant(idx);

    // Bosses never run; they fight to the end.
    if (monster.level >= 5) {
        return Decision::Attack;
    }

    if (monster.hp > 0 && monster.maxHp > 0) {
        // Critically wounded (below 10%): always flee.
        if (monster.hp < monster.maxHp / 10) {
            return Decision::Flee;
        }
        // Badly wounded (below 25%): sometimes flee.
        if (monster.hp < monster.maxHp / 4) {
            if (QRandomGenerator::global()->bounded(100) < 40) {
                return Decision::Flee;
            }
        }
    }

    // Spellcasters sometimes open with magic instead of a melee swing.
    if (monster.canCastSpells) {
        if (QRandomGenerator::global()->bounded(100) < 40) {
            return Decision::CastSpell;
        }
    }

    // Otherwise attack
    return Decision::Attack;
}

int MonsterAI::chooseTarget() const {
    if (!m_state || !m_engine) return -1;
    int monsterIdx = m_engine->currentParticipantIndex();
    if (monsterIdx < 0) return -1;

    QList<int> livingPlayers = m_state->livingPlayerIndices();
    if (livingPlayers.isEmpty()) return -1;

    // Strategy: 60% front row (first living player), 25% weakest, 15% random
    int roll = QRandomGenerator::global()->bounded(100);
    if (roll < 60) {
        // Front row: first living player
        return livingPlayers.first();
    } else if (roll < 85) {
        // Weakest: lowest HP
        int weakestIdx = livingPlayers.first();
        int weakestHp = m_state->participant(weakestIdx).hp;
        for (int idx : livingPlayers) {
            if (m_state->participant(idx).hp < weakestHp) {
                weakestHp = m_state->participant(idx).hp;
                weakestIdx = idx;
            }
        }
        return weakestIdx;
    } else {
        // Random
        return livingPlayers[QRandomGenerator::global()->bounded(livingPlayers.size())];
    }
}

QString MonsterAI::takeTurn() {
    if (!m_state || !m_engine || !m_actions) return "No combat";
    if (!isMonsterTurn()) return "Not a monster's turn";

    Decision d = decide();
    int monsterIdx = m_engine->currentParticipantIndex();
    QString monsterName = m_state->participant(monsterIdx).name;
    const CombatParticipant& monster = m_state->participant(monsterIdx);

    switch (d) {
    case Decision::Flee: {
        QString result;
        m_actions->flee(result);
        m_engine->markCurrentActed();
        return result;
    }
    case Decision::Attack: {
        int targetIdx = chooseTarget();
        if (targetIdx < 0) {
            m_engine->markCurrentActed();
            return QString("%1 has no target!").arg(monsterName);
        }
        QString result;
        m_actions->attack(targetIdx, result);

        // Apply poison on hit
        if (monster.canPoison && m_state->participant(targetIdx).isAlive) {
            QString poisonResult;
            m_actions->applyStatus(targetIdx, GameConstants::Poisoned, 3, poisonResult);
            result += " " + poisonResult;
        }

        m_engine->markCurrentActed();
        return result;
    }
    case Decision::CastSpell: {
        // Monster casts a spell (fire breath, etc.)
        if (monster.canBreathFire) {
            // Fire breath hits all players and sets them on fire.
            QString result;
            int totalDamage = 0;
            for (int i = 0; i < m_state->participantCount(); ++i) {
                CombatParticipant& p = m_state->participant(i);
                if (p.isPlayer && p.isAlive) {
                    int damage = 5 + QRandomGenerator::global()->bounded(10) + monster.level;
                    p.hp -= damage;
                    totalDamage += damage;
                    if (p.hp <= 0) {
                        p.hp = 0;
                        p.isAlive = false;
                    } else {
                        // Set on fire for 2 rounds (DoT handled by tickStatusEffects).
                        QString fireResult;
                        m_actions->applyStatus(i, GameConstants::OnFire, 2, fireResult);
                    }
                }
            }
            result = QString("%1 breathes fire for %2 total damage!").arg(monsterName).arg(totalDamage);
            m_engine->markCurrentActed();
            return result;
        }
        // Generic spell casting via SpellBook
        if (SpellBook::instance().isLoaded()) {
            QList<SpellDef> knownSpells = SpellBook::instance().spellsFor(
                [&]() {
                    Character c;
                    c.name = monster.name;
                    c.level = monster.level;
                    c.strength = monster.att;
                    c.intelligence = monster.att;
                    c.wisdom = monster.att;
                    c.constitution = monster.def;
                    c.charisma = monster.def;
                    c.dexterity = monster.speed;
                    c.mana = 1000;
                    c.maxMana = 1000;
                    return c;
                }()
            );
            if (!knownSpells.isEmpty()) {
                const SpellDef& spell = knownSpells.first();
                int targetIdx = chooseTarget();
                if (targetIdx >= 0) {
                    QString result;
                    m_actions->castSpellBySchool(targetIdx, spell.name, result);
                    m_engine->markCurrentActed();
                    return result;
                }
            }
        }
        // Fallback: regular attack
        int targetIdx = chooseTarget();
        if (targetIdx < 0) {
            m_engine->markCurrentActed();
            return QString("%1 has no target!").arg(monsterName);
        }
        QString result;
        m_actions->attack(targetIdx, result);
        m_engine->markCurrentActed();
        return result;
    }
    }

    m_engine->markCurrentActed();
    return QString("%1 does nothing.").arg(monsterName);
}
