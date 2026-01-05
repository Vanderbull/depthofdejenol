#include "DungeonHandlers.h"
#include "DungeonDialog.h"
#include "../../GameStateManager.h"

void DungeonHandlers::handleWater(DungeonDialog* dialog, int x, int y) {
    QPair<int, int> pos = {x, y};
    
    // Check if the current position contains water using the dialog's member
    if (dialog->m_waterPositions.contains(pos)) {
        GameStateManager* gsm = GameStateManager::instance();
        
        if (gsm->isCharacterOnFire()) {
            gsm->setCharacterOnFire(false);
            dialog->logMessage("The cool water extinguishes the flames! You are safe but soaking wet.");
        } else {
            dialog->logMessage("You splash into a shallow pool. You get soaking wet!");
        }
        
        // Example of extending logic without bloating the Dialog class
        // gsm->setGameValue("IsSoaked", true);
    }
}
