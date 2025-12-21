#ifndef DUNGEONDIALOG_H
#define DUNGEONDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QImage>
#include <QTimer>
#include <QLabel>
#include <QListWidget>
#include <QKeyEvent>
#include <QPushButton>
#include <QSet>
#include <QPair>
#include <QTableWidget>
#include "src/partyinfo_dialog/partyinfodialog.h"
#include "../event/EventManager.h"
#include "../../GameStateManager.h"

// Define MAP_SIZE here to resolve initialization errors in the class definition
const int MAP_SIZE = 30; 

namespace Ui {
class DungeonDialog;
}

class DungeonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DungeonDialog(QWidget *parent = nullptr);
    void enterLevel(int level);

    ~DungeonDialog();

    void updateDungeonView(const QImage& dungeonImage);
    void updateCompass(const QString& direction);
    void updateLocation(const QString& location);
    void updateMinimap(int x, int y);

signals:
    void teleporterUsed();
    void companionAttacked(int companionId);
    void companionCarried(int companionId);
    void exitedDungeonToCity();

private slots:
    // --- NEW MOVEMENT SLOTS ---
    void moveForward();
    void moveBackward();
    void moveStepLeft();  // NEW: Sidestep left
    void moveStepRight(); // NEW: Sidestep right

    void on_rotateLeftButton_clicked();
    void on_rotateRightButton_clicked();
    void on_teleportButton_clicked();
    void on_attackCompanionButton_clicked();
    void on_carryCompanionButton_clicked();

    void on_mapButton_clicked();
    void on_pickupButton_clicked();
    void on_dropButton_clicked();
    void on_fightButton_clicked();
    void on_spellButton_clicked();
    void on_takeButton_clicked();
    void on_openButton_clicked();
    void on_exitButton_clicked();
    
    void on_searchButton_clicked(); 
    void on_restButton_clicked(); 
    void on_talkButton_clicked();
    void on_stairsDownButton_clicked();
    void on_stairsUpButton_clicked();
    
    void on_chestButton_clicked();
    void onEventTriggered(const GameEvent& event);
    void checkMonsterSpawn();
    void initiateFight();
    void on_winBattle_trigger();


private:
    enum MonsterAttitude {
        Hostile,
        Neutral,
        Friendly
    };

    // Core State Members
    QGraphicsScene *m_dungeonScene;
    QGraphicsScene *m_fullMapScene; 
    QTimer *m_spawnTimer;           
    
    bool m_chestFound;
    MonsterAttitude m_currentMonsterAttitude;
    
    // Character State Members (for GameState)
    QString m_partyLeaderName; 
    QString m_partyLeaderRace; 
    QString m_partyLeaderAlignment; 
    
    // UI Member Widgets
    QLabel *m_locationLabel;
    QLabel *m_compassLabel;
    QGraphicsView *m_miniMapView;
    QListWidget *m_messageLog;
    QPushButton *m_chestButton; 

    // Gold Management Members
    QLabel *m_goldLabel;
    void refreshHealthUI();
    void updateGoldLabel();

    PartyInfoDialog *m_partyInfoDialog;
    
    // Party Management Member
    QTableWidget *m_partyTable;

    // Health Management Helper
    void updatePartyMemberHealth(int row, int damage);

    // Map generation/management
    QSet<QPair<int, int>> m_obstaclePositions;
    void generateRandomObstacles(int obstacleCount);
    void generateStairs(); 
    void generateSpecialTiles(int tileCount); 

    QPair<int, int> m_stairsUpPosition; 
    QPair<int, int> m_stairsDownPosition; 

    // Special Tile Position Sets (Combined list)
    QSet<QPair<int, int>> m_antimagicPositions;
    QSet<QPair<int, int>> m_extinguisherPositions;
    QSet<QPair<int, int>> m_fogPositions;
    QSet<QPair<int, int>> m_pitPositions;
    QSet<QPair<int, int>> m_rotatorPositions;
    QSet<QPair<int, int>> m_studPositions;
    QSet<QPair<int, int>> m_chutePositions;
    QSet<QPair<int, int>> m_teleportPositions; 
    QSet<QPair<int, int>> m_waterPositions;    
    
    // Map data
    QMap<QPair<int, int>, QString> m_monsterPositions;
    QMap<QPair<int, int>, QString> m_treasurePositions;
    QMap<QPair<int, int>, QString> m_trapPositions;
    QMap<QString, QString> m_MonsterAttitude;
    
    void drawMinimap(); 
    void handleEncounter(int x, int y);
    void handleTreasure(int x, int y);
    void handleTrap(int x, int y);
    void handleChute(int x, int y);
    void movePlayer(int dx, int dy);
    
    // Helper functions
    void logMessage(const QString& message); 
    void keyPressEvent(QKeyEvent *event) override;
    void spawnMonsters(const QString& monsterType, int count);
    
};

#endif // DUNGEONDIALOG_H
