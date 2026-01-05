#ifndef DUNGEONHANDLERS_H
#define DUNGEONHANDLERS_H

#include <QPair>
#include <QString>

// Forward declaration to avoid circular includes
class DungeonDialog;

class DungeonHandlers {
public:
    /**
     * @brief Processes water-related interactions at the given coordinates.
     */
    static void handleWater(DungeonDialog* dialog, int x, int y);

    // You can add other handlers here later (e.g., handleTrap, handleAntimagic)
};

#endif // DUNGEONHANDLERS_H
