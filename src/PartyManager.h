#ifndef PARTYMANAGER_H
#define PARTYMANAGER_H

#include <QObject>
#include <QList>
#include "character.h"

class PartyManager : public QObject {
    Q_OBJECT
public:
    static PartyManager& instance() {
        static PartyManager i;
        return i;
    }

    // Core Data Access
    QList<Character>& party() { return m_party; }
    Character* activeCharacter() { 
        return (m_activeIndex >= 0 && m_activeIndex < m_party.size()) ? &m_party[m_activeIndex] : nullptr; 
    }

    // Logic Methods
    void addCharacter(const Character& c) {
        m_party.append(c);
        emit partyChanged();
    }

    void removeCharacter(int index) {
        if (index >= 0 && index < m_party.size()) {
            m_party.removeAt(index);
            emit partyChanged();
        }
    }

    void setActiveIndex(int index) {
        if (index >= 0 && index < m_party.size()) {
            m_activeIndex = index;
            emit activeCharacterChanged();
        }
    }

    // High-level Actions (Encapsulates logic formerly in GameStateManager)
    void distributeExperience(int totalXp) {
        if (m_party.isEmpty()) return;
        int share = totalXp / m_party.size();
        for (auto& c : m_party) {
            if (c.isAlive) c.experience += share;
        }
        emit partyChanged();
    }

signals:
    void partyChanged();
    void activeCharacterChanged();

private:
    PartyManager() : m_activeIndex(0) {}
    QList<Character> m_party;
    int m_activeIndex;
};

#endif
