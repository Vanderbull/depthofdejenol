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
void DungeonHandlers::handleAntimagic(DungeonDialog* dialog, int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_antimagicPositions.contains(pos)) {
        logMessage("You enter an Antimagic Field! Your spells feel suppressed.");
    }
}
void DungeonHandlers::handleTrap(DungeonDialog* dialog, int x, int y){
    QPair<int, int> pos = {x, y};
    if (m_trapPositions.contains(pos)) {
        QString trapType = m_trapPositions.value(pos);

        int damage = QRandomGenerator::global()->bounded(1, 10);
        updatePartyMemberHealth(0, damage);

        logMessage(QString("You step on a **%1** trap and take %2 damage!").arg(trapType).arg(damage));

        m_trapPositions.remove(pos);
        drawMinimap();
    }
};
void DungeonHandlers::handleChute(DungeonDialog* dialog, int x, int y){
    QPair<int, int> pos = {x, y};
    if (m_chutePositions.contains(pos)) {
        logMessage("AAAHHH! You fall through a hidden chute!");

        // 1. Deal Fall Damage
        int fallDamage = QRandomGenerator::global()->bounded(5, 15);
        updatePartyMemberHealth(0, fallDamage); // Damage main character

        // 2. Determine New Level
        GameStateManager* gsm = GameStateManager::instance();
        int nextLevel = gsm->getGameValue("DungeonLevel").toInt() + 1;

        // 3. Trigger Level Transition
        enterLevel(nextLevel);
    }
};
void DungeonHandlers::handleExtinguisher(DungeonDialog* dialog, int x, int y){
    QPair<int, int> pos = {x, y};
    if (m_extinguisherPositions.contains(pos)) {
        GameStateManager* gsm = GameStateManager::instance();
        
        if (gsm->isCharacterOnFire()) {
            gsm->setCharacterOnFire(false);
            logMessage("Magic waters spray from the ceiling! The flames are extinguished.");
        } else {
            logMessage("A refreshing mist falls upon you.");
        }
    }
};
void DungeonHandlers::handleEncounters(DungeonDialog* dialog, int x, int y){
    QPair<int, int> pos = {x, y};
    if (m_monsterPositions.contains(pos)) {
        QString monster = m_monsterPositions.value(pos);
        QString attitude = m_MonsterAttitude.value(monster, "Hostile");
        logMessage(QString("You encounter a **%1**! It looks **%2**.").arg(monster).arg(attitude));
    }
}
void DungeonHandlers::handleTreasure(DungeonDialog* dialog, int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (m_treasurePositions.contains(pos)) {
        logMessage("There is a treasure chest here! Use the Open button to see what's inside.");
    }
}
