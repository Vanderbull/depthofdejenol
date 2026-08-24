#include "CompanionAI.h"
#include "Companion.h" // Your standalone companion data class
// #include "GridState.h" // Include your game grid definitions here

CompanionAI::CompanionAI(Companion *owner, QObject *parent)
    : QObject(parent), m_owner(owner), m_stance(TacticalStance::Bastion) {
}

void CompanionAI::setStance(TacticalStance stance) {
    m_stance = stance;
}

TacticalStance CompanionAI::stance() const {
    m_stance;
}

void CompanionAI::evaluateTurn(const GridState &boardState) {
    if (!m_owner || m_owner->currentStatus != Companion::Status::Active) {
        return; // Inactive or wayfaring companions don't take combat turns
    }

    // 1. Calculate the next move based on current stance
    QPoint targetTile = calculateBestMove(boardState);

    // 2. Emit signal so the game engine/board manager handles the physical movement
    emit requestedMove(m_owner->id, targetTile);
}

QPoint CompanionAI::calculateBestMove(const GridState &boardState) {
    QPoint chosenTile(0, 0);

    switch (m_stance) {
        case TacticalStance::Vanguard:
            // Logic: Find nearest enemy tile and close distance
            break;
        case TacticalStance::Bastion:
            // Logic: Hold adjacent grid coordinates relative to defense points
            break;
        case TacticalStance::Skirmisher:
            // Logic: Flank or retreat if health is low
            break;
        case TacticalStance::Preserver:
            // Logic: Stay back, prioritize support/healing range
            break;
    }

    return chosenTile;
}
