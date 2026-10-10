#include "DoorAndSearch.h"
#include <QList>
#include <cmath>

bool DoorAndSearch::canOpen(const DoorState& door, const QSet<QString>& keysHeld) {
    if (!door.locked) return true;
    if (door.keyName.isEmpty()) return true;
    return keysHeld.contains(door.keyName);
}

bool DoorAndSearch::tryOpen(DoorState& door, const QSet<QString>& keysHeld, QString& reason) {
    if (door.secret) {
        reason = "You cannot find a way through.";
        return false;
    }
    if (!door.locked) {
        reason = "The door is already open.";
        return true;
    }
    if (door.keyName.isEmpty()) {
        door.locked = false;
        reason = "The door swings open.";
        return true;
    }
    if (keysHeld.contains(door.keyName)) {
        door.locked = false;
        reason = QString("The %1 turns in the lock; the door opens.").arg(door.keyName);
        return true;
    }
    reason = QString("The door is locked. It needs the %1.").arg(door.keyName);
    return false;
}

int DoorAndSearch::searchRoll(int wisdom, int intelligence, QRandomGenerator& rng) {
    int roll = rng.bounded(1, 21);
    int wisMod = (wisdom - 10) / 2;
    int intMod = (intelligence - 10) / 2;
    return roll + wisMod + intMod;
}

int DoorAndSearch::secretDoorDifficulty(int floorLevel) {
    // Floor 1 is DC 10; each floor adds 1.
    return 10 + (floorLevel - 1);
}

int DoorAndSearch::searchForSecretDoors(const QMap<QPair<int, int>, DoorState>& doors,
                                        int x, int y,
                                        int wisdom, int intelligence,
                                        QList<QPair<int, int>>& found,
                                        QRandomGenerator& rng) {
    int count = 0;
    int roll = searchRoll(wisdom, intelligence, rng);

    for (auto it = doors.constBegin(); it != doors.constEnd(); ++it) {
        const DoorState& door = it.value();
        if (!door.secret) continue;

        // Only a 3x3 area around the searcher.
        int dx = qAbs(it.key().first - x);
        int dy = qAbs(it.key().second - y);
        if (dx > 1 || dy > 1) continue;

        if (roll >= door.difficulty) {
            found.append(it.key());
            count++;
        }
    }
    return count;
}

int DoorAndSearch::trapDetectionDifficulty(int floorLevel) {
    // Floor 1 is DC 9; each floor adds 1.
    return 8 + floorLevel;
}

int DoorAndSearch::searchForTraps(const QMap<QPair<int, int>, QString>& traps,
                                  int x, int y,
                                  int wisdom, int intelligence,
                                  int floorLevel,
                                  QList<QPair<int, int>>& found,
                                  QRandomGenerator& rng) {
    int count = 0;
    int roll = searchRoll(wisdom, intelligence, rng);
    int dc = trapDetectionDifficulty(floorLevel);

    for (auto it = traps.constBegin(); it != traps.constEnd(); ++it) {
        // Only a 3x3 area around the searcher.
        int dx = qAbs(it.key().first - x);
        int dy = qAbs(it.key().second - y);
        if (dx > 1 || dy > 1) continue;

        if (roll >= dc) {
            found.append(it.key());
            count++;
        }
    }
    return count;
}
