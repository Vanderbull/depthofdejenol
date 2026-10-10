#include "DungeonHandlers.h"
#include "DungeonDialog.h"
#include "../../gameStateManager.h"
#include "src/core/SoundEffects.h"
#include "src/core/DoorAndSearch.h"

void DungeonHandlers::handlePit(DungeonDialog* dialog, int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (dialog->m_pitPositions.contains(pos)) {
        int damage = QRandomGenerator::global()->bounded(2, 13);
        dialog->updatePartyMemberHealth(0, damage);
        dialog->logMessage(QString("<font color='red'>You fall into a pit and take %1 damage!</font>").arg(damage));

        // 25% chance to fall to the next level
        if (QRandomGenerator::global()->bounded(100) < 25) {
            dialog->logMessage("<font color='orange'>The floor crumbles away! You tumble to the level below...</font>");
            int nextLevel = gameStateManager::instance()->getGameValue("DungeonLevel").toInt() + 1;
            dialog->enterLevel(nextLevel);
        }
    }
}

void DungeonHandlers::handleWater(DungeonDialog* dialog, int x, int y) 
{
    QPair<int, int> pos = {x, y};    
    // Check if the current position contains water using the dialog's member
    if (dialog->m_waterPositions.contains(pos)) {
        gameStateManager* gsm = gameStateManager::instance();
        if (gsm->isCharacterOnFire()) {
            gsm->setCharacterOnFire(false);
            dialog->logMessage("The cool water extinguishes the flames! You are safe but soaking wet.");
        } else {
            dialog->logMessage("You splash into a shallow pool. You get soaking wet!");
        }
    }
}

void DungeonHandlers::handleAntimagic(DungeonDialog* dialog, int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (dialog->m_antimagicPositions.contains(pos)) {
        dialog->logMessage("You enter an Antimagic Field! Your spells feel suppressed.");
    }
}

void DungeonHandlers::handleTrap(DungeonDialog* dialog, int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (dialog->m_trapPositions.contains(pos) && !dialog->m_triggeredTraps.contains(pos)) {
        QString trapType = dialog->m_trapPositions.value(pos);
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        if (members.isEmpty()) return;

        // Each trap type has a distinct effect — not just "1-10 damage".
        if (trapType == "Spike") {
            int damage = QRandomGenerator::global()->bounded(1, 11);
            dialog->updatePartyMemberHealth(0, damage);
            dialog->logMessage(QString("<font color='red'>You step on a **Spike** trap and take %1 damage!</font>").arg(damage));
        } else if (trapType == "Poison Needler") {
            int damage = QRandomGenerator::global()->bounded(1, 6);
            dialog->updatePartyMemberHealth(0, damage);
            if (members[0].isAlive) {
                members[0].addStatus(StatusFlag::Poisoned);
                dialog->logMessage(QString("<font color='red'>A **Poison Needler** hits you for %1 damage and poisons you!</font>").arg(damage));
            }
        } else if (trapType == "Dart") {
            int damage = QRandomGenerator::global()->bounded(3, 9);
            dialog->updatePartyMemberHealth(0, damage);
            dialog->logMessage(QString("<font color='red'>A **Dart** strikes you for %1 damage!</font>").arg(damage));
        } else if (trapType == "Pit Cover") {
            int damage = QRandomGenerator::global()->bounded(5, 16);
            dialog->updatePartyMemberHealth(0, damage);
            dialog->logMessage(QString("<font color='red'>The floor gives way! You fall into a pit and take %1 damage!</font>").arg(damage));
            if (QRandomGenerator::global()->bounded(100) < 25) {
                dialog->logMessage("<font color='orange'>You tumble to the level below...</font>");
                int nextLevel = gsm->getGameValue("DungeonLevel").toInt() + 1;
                dialog->enterLevel(nextLevel);
            }
        } else if (trapType == "Snare") {
            if (members[0].isAlive) {
                members[0].addStatus(StatusFlag::Snared);
                dialog->logMessage("<font color='orange'>A **Snare** snaps shut around your leg! You are immobilized!</font>");
            }
        } else {
            // Unknown trap type — fall back to generic damage.
            int damage = QRandomGenerator::global()->bounded(1, 11);
            dialog->updatePartyMemberHealth(0, damage);
            dialog->logMessage(QString("<font color='red'>You step on a **%1** trap and take %2 damage!</font>").arg(trapType).arg(damage));
        }

        SoundEffects::instance()->play(SoundEffects::Type::Trap);
        dialog->m_triggeredTraps.insert(pos);
        dialog->drawMinimap();
    }
};

void DungeonHandlers::handleChute(DungeonDialog* dialog, int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (dialog->m_chutePositions.contains(pos)) {
        dialog->logMessage("AAAHHH! You fall through a hidden chute!");
        // 1. Deal Fall Damage
        int fallDamage = QRandomGenerator::global()->bounded(5, 15);
        dialog->updatePartyMemberHealth(0, fallDamage); // Damage main character
        // 2. Determine New Level
        gameStateManager* gsm = gameStateManager::instance();
        int nextLevel = gsm->getGameValue("DungeonLevel").toInt() + 1;
        // 3. Trigger Level Transition
        dialog->enterLevel(nextLevel);
    }
};

void DungeonHandlers::handleExtinguisher(DungeonDialog* dialog, int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (dialog->m_extinguisherPositions.contains(pos)) {
        gameStateManager* gsm = gameStateManager::instance();        
        if (gsm->isCharacterOnFire()) {
            gsm->setCharacterOnFire(false);
            dialog->logMessage("Magic waters spray from the ceiling! The flames are extinguished.");
        } else {
            dialog->logMessage("A refreshing mist falls upon you.");
        }
    }
};

void DungeonHandlers::handleEncounters(DungeonDialog* dialog, int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (dialog->m_monsterPositions.contains(pos)) {
        QString monster = dialog->m_monsterPositions.value(pos);
        QString attitude = dialog->m_MonsterAttitude.value(monster, "Hostile");
        dialog->logMessage(QString("You encounter a **%1**! It looks **%2**.").arg(monster).arg(attitude));
    }
}

void DungeonHandlers::handleTreasure(DungeonDialog* dialog, int x, int y)
{
    QPair<int, int> pos = {x, y};
    if (dialog->m_treasurePositions.contains(pos)) {
        dialog->logMessage("There is a treasure chest here! Use the Open button to see what's inside.");
        SoundEffects::instance()->play(SoundEffects::Type::Gold);
    }
}
