#ifndef DUNGEONHANDLERS_H
#define DUNGEONHANDLERS_H

#include <QPair>
#include <QString>

// Forward declaration to avoid circular includes
class DungeonDialog;

class DungeonHandlers {
public:
    static void handleWater(DungeonDialog* dialog, int x, int y);
    static void handleAntimagic(DungeonDialog* dialog, int x, int y);
    static void handleTrap(DungeonDialog* dialog, int x, int y);
    static void handleChute(DungeonDialog* dialog, int x, int y);
    static void handleExtinguisher(DungeonDialog* dialog, int x, int y);
    static void handleEncounters(DungeonDialog* dialog, int x, int y);
    static void handleTreasure(DungeonDialog* dialog, int x, int y);
};

#endif // DUNGEONHANDLERS_H
