#include "PartyManager.h"

// The constructor must match the header's signature
PartyManager::PartyManager(QObject *parent) 
    : QObject(parent) 
{
    // Initialize with an empty party if needed
    m_party.sharedGold = 0;
}

Character& PartyManager::getMember(int index) {
    if (m_party.members.isEmpty()) {
        static Character nullChar;
        return nullChar;
    }
    if (index >= 0 && index < m_party.members.size()) {
        return m_party.members[index];
    }
    return m_party.members[0];
}

bool PartyManager::isWholePartyDead() const {
    if (m_party.members.isEmpty()) return true;
    for (const auto& member : m_party.members) {
        if (member.isAlive) return false;
    }
    return true;
}

void PartyManager::addExperienceToParty(int totalXp) {
    QList<int> livingIndices;
    for (int i = 0; i < m_party.members.size(); ++i) {
        if (m_party.members[i].isAlive) livingIndices.append(i);
    }
    if (livingIndices.isEmpty()) return;

    int xpPerMember = totalXp / livingIndices.size();
    for (int index : livingIndices) {
        addExperienceToCharacter(index, xpPerMember);
    }
}

void PartyManager::addExperienceToCharacter(int index, int amount) {
    if (index < 0 || index >= m_party.members.size()) return;
    
    Character& c = m_party.members[index];
    c.experience += amount;
    
    int nextLevel = c.level * 1000;
    if (c.experience >= nextLevel) {
        c.level++;
        emit leveledUp(index, c.level);
    }
    
    emit partyUpdated();
}

void PartyManager::updateMemberStatus(int index, bool isAlive) {
    if (index >= 0 && index < m_party.members.size()) {
        m_party.members[index].isAlive = isAlive;
        emit partyUpdated();
    }
}
