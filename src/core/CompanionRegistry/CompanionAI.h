#pragma once

#include <QObject>
#include <QPoint>

// Forward declarations to keep dependencies clean and decoupled
class Companion;
struct GridState; 

enum class TacticalStance {
    Vanguard,   // Seeks out and intercepts front-tile threats
    Bastion,    // Holds ground, guards adjacent grid coordinates
    Skirmisher, // Hits flanks, retreats when low on health
    Preserver   // Consumes resources conservatively, heals/supports
};

class CompanionAI : public QObject {
    Q_OBJECT
public:
    explicit CompanionAI(Companion *owner, QObject *parent = nullptr);
    
    // Core decision-making hook called by your game loop or registry
    void evaluateTurn(const GridState &boardState);
    
    void setStance(TacticalStance stance);
    TacticalStance stance() const;

signals:
    // Emitted when the AI decides on a grid movement or action
    void requestedMove(const QString &companionId, const QPoint &targetTile);
    void requestedAction(const QString &companionId, const QString &actionType);

private:
    Companion *m_owner;
    TacticalStance m_stance;
    
    QPoint calculateBestMove(const GridState &boardState);
};
