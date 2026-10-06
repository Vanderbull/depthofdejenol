#include "MonsterAI.h"
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

    // Flee if HP is low (below 25%)
    if (monster.hp < monster.maxHp / 4) {
        // 50% chance to flee when low
        if (QRandomGenerator::global()->bounded(100) < 50) {
            return Decision::Flee;
        }
    }

    // Flee if very low (below 10%)
    if (monster.hp < monster.maxHp / 10) {
        return Decision::Flee;
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
        m_engine->markCurrentActed();
        return result;
    }
    case Decision::CastSpell: {
        // Monsters don't cast spells in this simplified system
        m_engine->markCurrentActed();
        return QString("%1 tries to cast a spell but fails!").arg(monsterName);
    }
    }

    m_engine->markCurrentActed();
    return QString("%1 does nothing.").arg(monsterName);
}
