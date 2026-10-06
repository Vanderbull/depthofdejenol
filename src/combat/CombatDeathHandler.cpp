#include "CombatDeathHandler.h"
#include "src/core/SoundEffects.h"

CombatDeathHandler::CombatDeathHandler(CombatState* state, TurnEngine* engine, CombatActions* actions)
    : m_state(state), m_engine(engine), m_actions(actions) {}

bool CombatDeathHandler::isPartyWipe() const {
    if (!m_state) return false;
    return m_state->livingPlayerCount() == 0;
}

bool CombatDeathHandler::isVictory() const {
    if (!m_state) return false;
    return m_state->livingMonsterCount() == 0;
}

int CombatDeathHandler::deadPartyMemberCount() const {
    if (!m_state) return 0;
    int count = 0;
    for (int i = 0; i < m_state->participantCount(); ++i) {
        if (m_state->participant(i).isPlayer && !m_state->participant(i).isAlive) {
            count++;
        }
    }
    return count;
}

int CombatDeathHandler::livingPartyMemberCount() const {
    if (!m_state) return 0;
    return m_state->livingPlayerCount();
}

QStringList CombatDeathHandler::processDeaths() {
    QStringList messages;
    if (!m_state) return messages;

    for (int i = 0; i < m_state->participantCount(); ++i) {
        CombatParticipant& p = m_state->participant(i);
        if (p.isPlayer && !p.isAlive && !p.hasActed) {
            // This player just died (hasActed is false because they died before acting)
            messages.append(QString("%1 has fallen!").arg(p.name));
            SoundEffects::instance()->play(SoundEffects::Type::Death);
        }
    }

    return messages;
}

bool CombatDeathHandler::handleGameOver() {
    if (!m_state) return false;
    return isPartyWipe();
}

void CombatDeathHandler::reviveAllPartyMembers() {
    if (!m_state) return;

    for (int i = 0; i < m_state->participantCount(); ++i) {
        CombatParticipant& p = m_state->participant(i);
        if (p.isPlayer && !p.isAlive) {
            p.isAlive = true;
            p.hp = 1;
        }
    }
}

bool CombatDeathHandler::isPartyMemberDead(int combatIndex) const {
    if (!m_state || combatIndex < 0 || combatIndex >= m_state->participantCount()) {
        return false;
    }
    const CombatParticipant& p = m_state->participant(combatIndex);
    return p.isPlayer && !p.isAlive;
}

QStringList CombatDeathHandler::getDeadPartyMemberNames() const {
    QStringList names;
    if (!m_state) return names;

    for (int i = 0; i < m_state->participantCount(); ++i) {
        const CombatParticipant& p = m_state->participant(i);
        if (p.isPlayer && !p.isAlive) {
            names.append(p.name);
        }
    }
    return names;
}
