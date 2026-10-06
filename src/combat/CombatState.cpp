#include "CombatState.h"
#include <QRandomGenerator>
#include <algorithm>

CombatState::CombatState() {}

void CombatState::addParticipant(const CombatParticipant& p) {
    m_participants.append(p);
}

void CombatState::clear() {
    m_participants.clear();
    m_initiativeOrder.clear();
    m_currentRound = 0;
    m_currentTurnIndex = -1;
}

void CombatState::rollInitiative() {
    m_initiativeOrder.clear();
    QVector<QPair<int, int>> initiatives;  // (initiative, index)
    for (int i = 0; i < m_participants.size(); ++i) {
        int roll = QRandomGenerator::global()->bounded(1, 21);
        int initiative = roll + m_participants[i].speed + m_participants[i].dex / 2;
        initiatives.append({initiative, i});
    }
    // Sort descending by initiative (highest first)
    std::sort(initiatives.begin(), initiatives.end(),
              [](const QPair<int, int>& a, const QPair<int, int>& b) {
                  return a.first > b.first;
              });
    for (const auto& pair : initiatives) {
        m_initiativeOrder.append(pair.second);
    }
}

void CombatState::startRound() {
    m_currentRound++;
    m_currentTurnIndex = -1;
    resetActedFlags();
}

bool CombatState::nextTurn() {
    // Find next living participant who hasn't acted
    for (int i = 0; i < m_initiativeOrder.size(); ++i) {
        m_currentTurnIndex++;
        if (m_currentTurnIndex >= m_initiativeOrder.size()) {
            // Round over
            return false;
        }
        int participantIdx = m_initiativeOrder[m_currentTurnIndex];
        if (m_participants[participantIdx].isAlive && !m_participants[participantIdx].hasActed) {
            return true;
        }
    }
    return false;
}

const CombatParticipant* CombatState::currentParticipant() const {
    if (m_currentTurnIndex < 0 || m_currentTurnIndex >= m_initiativeOrder.size()) {
        return nullptr;
    }
    int idx = m_initiativeOrder[m_currentTurnIndex];
    if (idx < 0 || idx >= m_participants.size()) {
        return nullptr;
    }
    return &m_participants[idx];
}

bool CombatState::isCombatOver() const {
    return livingPlayerCount() == 0 || livingMonsterCount() == 0;
}

int CombatState::livingPlayerCount() const {
    int count = 0;
    for (const auto& p : m_participants) {
        if (p.isPlayer && p.isAlive) count++;
    }
    return count;
}

int CombatState::livingMonsterCount() const {
    int count = 0;
    for (const auto& p : m_participants) {
        if (!p.isPlayer && p.isAlive) count++;
    }
    return count;
}

QList<int> CombatState::livingPlayerIndices() const {
    QList<int> result;
    for (int i = 0; i < m_participants.size(); ++i) {
        if (m_participants[i].isPlayer && m_participants[i].isAlive) {
            result.append(i);
        }
    }
    return result;
}

QList<int> CombatState::livingMonsterIndices() const {
    QList<int> result;
    for (int i = 0; i < m_participants.size(); ++i) {
        if (!m_participants[i].isPlayer && m_participants[i].isAlive) {
            result.append(i);
        }
    }
    return result;
}

void CombatState::markActed(int index) {
    if (index >= 0 && index < m_participants.size()) {
        m_participants[index].hasActed = true;
    }
}

void CombatState::resetActedFlags() {
    for (auto& p : m_participants) {
        p.hasActed = false;
    }
}
