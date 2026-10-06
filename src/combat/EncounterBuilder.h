#ifndef ENCOUNTERBUILDER_H
#define ENCOUNTERBUILDER_H

#include "src/combat/CombatState.h"
#include <QString>
#include <QList>
#include <QVariantMap>

// Builds a group of monsters from MDATA5 data.
// Uses numGroups (number of groups) and hits (monsters per group) to determine
// how many monsters spawn and their stats.
class EncounterBuilder {
public:
    // Build a list of CombatParticipants for a monster encounter.
    // monsterName: the monster type (e.g. "Orc", "Rattlesnake")
    // monsterData: the full monster data list from gameStateManager
    // Returns a list of CombatParticipant objects ready for combat.
    static QList<CombatParticipant> buildEncounter(
        const QString& monsterName,
        const QList<QVariantMap>& monsterData);

    // Get the number of monsters that would spawn for this type.
    static int getGroupSize(const QString& monsterName,
                            const QList<QVariantMap>& monsterData);

    // Get a display name for a specific monster in a group (e.g. "Orc 1", "Orc 2").
    static QString getMonsterName(const QString& baseName, int index);
};

#endif // ENCOUNTERBUILDER_H
