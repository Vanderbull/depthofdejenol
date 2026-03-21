#ifndef PARTYMANAGER_H
#define PARTYMANAGER_H

#include <QObject>
#include "character.h"

class PartyManager : public QObject {
    Q_OBJECT
public:
    explicit PartyManager(QObject *parent = nullptr);

    // Getters
    Party& currentParty() { return m_party; }
    Character& getMember(int index);
    bool isWholePartyDead() const;

    // Actions
    void addExperienceToParty(int totalXp);
    void addExperienceToCharacter(int index, int amount);
    void updateMemberStatus(int index, bool isAlive);
    
    // Data conversion (for UI/Saving)
    QVariantMap getPartyAsMap() const { return m_party.toMap(); }
    void loadPartyFromMap(const QVariantMap &map) { m_party.loadFromMap(map); }

signals:
    void partyUpdated(); // Tell the UI to refresh
    void leveledUp(int memberIndex, int newLevel);

private:
    Party m_party;
};

#endif
