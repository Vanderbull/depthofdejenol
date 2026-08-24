#pragma once

#include <QPoint>
#include <QList>
#include <QHash>

// Simple representation of an entity on the grid
struct EntityInfo {
    QString id;
    QPoint position;
    bool isEnemy;
    int health;
};

// Represents the snapshot of the board passed to AI for decision-making
struct GridState {
    int width = 0;
    int height = 0;
    
    // All known entities (enemies, players, companions) currently on the grid
    QList<EntityInfo> entities; 
    
    // Optional: Obstacles, walls, or blocked tiles
    QList<QPoint> blockedTiles;

    // Helper query methods for the AI to use
    bool isTileBlocked(const QPoint &pos) const {
        return blockedTiles.contains(pos);
    }
};
