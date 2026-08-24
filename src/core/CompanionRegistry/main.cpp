#include <QCoreApplication>
#include <QSharedPointer>
#include <QDebug>

#include "companions/Companion.h"
#include "companions/CompanionRegistry.h"
#include "companions/CompanionInventoryModel.h"
#include "companions/CompanionAI.h"
#include "core/GridState.h" // Assuming GridState.h is in a core or include folder

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qDebug() << "=== Initializing Standalone Companion Ecosystem ===";

    // 1. Initialize the Registry (Standalone manager for all world companions)
    CompanionRegistry registry;

    // 2. Create a Companion independently (e.g., encountered in the wild or recruited via camp)
    // Parameters: Unique ID, Display Name
    QSharedPointer<Companion> veteran = QSharedPointer<Companion>::create("comp_01", "Brom the Shield-Bearer");
    
    // Set initial stats
    veteran->setMorale(85);
    veteran->setLoyalty(70);
    veteran->setCurrentStatus(Companion::Status::Active);

    // Register them into the system
    registry.registerCompanion(veteran);

    // 3. Populate their Isolated Inventory (Decoupled from player inventory bloat)
    CompanionInventoryModel *satchel = veteran->inventoryModel();
    satchel->addItem(Item{"item_iron_sword", "Rusted Iron Sword", 4});
    satchel->addItem(Item{"item_health_potion", "Minor Healing Potion", 1});

    qDebug() << "Companion created:" << veteran->name() 
             << "| Morale:" << veteran->morale() 
             << "| Satchel Weight:" << satchel->totalWeight() << "/" << satchel->maxCapacity() << "kg";

    // 4. Attach Autonomous Tactical AI
    CompanionAI aiController(veteran.data());
    aiController.setStance(TacticalStance::Vanguard); // Set behavioral profile

    // Connect the AI's decision signal to our game board dispatcher
    QObject::connect(&aiController, &CompanionAI::requestedMove, [](const QString &id, const QPoint &target) {
        qDebug() << "[AI Dispatcher] Companion" << id << "evaluated grid and requested movement to tile:" << target;
    });

    // 5. Simulate a World/Combat Turn Update
    qDebug() << "\n--- Simulating Grid Turn ---";
    
    // Build a mock snapshot of the current dungeon grid
    GridState currentBoard;
    currentBoard.width = 10;
    currentBoard.height = 10;
    
    // Add an enemy entity spotted on the board
    currentBoard.entities.append(EntityInfo{
        .id = "goblin_scout",
        .position = QPoint(4, 5),
        .isEnemy = true,
        .health = 15
    });

    // Trigger the companion's turn evaluation independently
    aiController.evaluateTurn(currentBoard);

    qDebug() << "=== Simulation Complete ===";
    
    return 0; // Exiting immediately for this console example
}
