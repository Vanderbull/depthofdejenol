#include "TurnEngine.h"

TurnEngine::TurnEngine() {}

void TurnEngine::setCombatState(CombatState* state) {
    m_state = state;
}

void TurnEngine::startRound() {
    if (!m_state) return;
    m_state->rollInitiative();
    m_state->startRound();
}

int TurnEngine::currentRound() const {
    if (!m_state) return 0;
    return m_state->currentRound();
}

bool TurnEngine::isRoundOver() const {
    if (!m_state) return true;
    // Round is over when no living participant can act
    for (int idx : m_state->livingPlayerIndices()) {
        if (!m_state->participant(idx).hasActed) return false;
    }
    for (int idx : m_state->livingMonsterIndices()) {
        if (!m_state->participant(idx).hasActed) return false;
    }
    return true;
}

bool TurnEngine::nextTurn() {
    if (!m_state) return false;
    return m_state->nextTurn();
}

int TurnEngine::currentParticipantIndex() const {
    if (!m_state) return -1;
    const CombatParticipant* p = m_state->currentParticipant();
    if (!p) return -1;
    // Find the index of this participant in the state
    for (int i = 0; i < m_state->participantCount(); ++i) {
        if (&m_state->participant(i) == p) return i;
    }
    return -1;
}

bool TurnEngine::hasCurrentParticipant() const {
    return currentParticipantIndex() >= 0;
}

QString TurnEngine::currentParticipantName() const {
    if (!m_state) return QString();
    const CombatParticipant* p = m_state->currentParticipant();
    if (!p) return QString();
    return p->name;
}

void TurnEngine::markCurrentActed() {
    if (!m_state) return;
    int idx = currentParticipantIndex();
    if (idx >= 0) {
        m_state->markActed(idx);
    }
}

bool TurnEngine::hasCurrentActed() const {
    if (!m_state) return true;
    int idx = currentParticipantIndex();
    if (idx < 0) return true;
    return m_state->participant(idx).hasActed;
}

bool TurnEngine::isCombatOver() const {
    if (!m_state) return true;
    return m_state->isCombatOver();
}

QString TurnEngine::combatStatus() const {
    if (!m_state) return QString("No combat");
    int players = m_state->livingPlayerCount();
    int monsters = m_state->livingMonsterCount();
    return QString("Round %1 — %2 players vs %3 monsters")
        .arg(currentRound()).arg(players).arg(monsters);
}

QList<int> TurnEngine::livingPlayers() const {
    if (!m_state) return {};
    return m_state->livingPlayerIndices();
}

QList<int> TurnEngine::livingMonsters() const {
    if (!m_state) return {};
    return m_state->livingMonsterIndices();
}
