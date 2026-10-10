#ifndef DOORANDSEARCH_H
#define DOORANDSEARCH_H

#include <QString>
#include <QSet>
#include <QMap>
#include <QPair>
#include <QRandomGenerator>

// Locked doors, keys, and secret-door discovery.
// Pure logic — the dungeon dialog owns the actual tile containers.

// A keyed door. The key is identified by name; a party either has it or not.
struct DoorState {
    QPair<int, int> position;
    QString keyName;      // empty = not locked
    bool locked = true;
    bool secret = false;  // secret doors are hidden until found
    int difficulty = 0;   // for secret doors: search DC
};

class DoorAndSearch {
public:
    // Can the party open this door? A door with no key is always openable.
    static bool canOpen(const DoorState& door, const QSet<QString>& keysHeld);

    // Try to open a door. Returns true and unlocks it on success.
    // `reason` describes the outcome for the message log.
    static bool tryOpen(DoorState& door, const QSet<QString>& keysHeld, QString& reason);

    // Search a 3x3 area around (x, y) for secret doors.
    // Rolls d20 + WIS modifier + INT modifier against each door's difficulty.
    // `found` receives the positions discovered. Returns how many were found.
    static int searchForSecretDoors(const QMap<QPair<int, int>, DoorState>& doors,
                                    int x, int y,
                                    int wisdom, int intelligence,
                                    QList<QPair<int, int>>& found,
                                    QRandomGenerator& rng);

    // Search roll: d20 + wisMod + intMod. Exposed for testing.
    static int searchRoll(int wisdom, int intelligence, QRandomGenerator& rng);

    // Difficulty for a secret door on a given floor.
    static int secretDoorDifficulty(int floorLevel);

    // Search a 3x3 area around (x, y) for traps.
    // Uses the same search roll as secret doors; DC is 8 + floorLevel.
    // `found` receives the positions of detected traps. Returns count.
    static int searchForTraps(const QMap<QPair<int, int>, QString>& traps,
                              int x, int y,
                              int wisdom, int intelligence,
                              int floorLevel,
                              QList<QPair<int, int>>& found,
                              QRandomGenerator& rng);

    // Difficulty class for detecting a trap on a given floor.
    static int trapDetectionDifficulty(int floorLevel);
};

#endif // DOORANDSEARCH_H
