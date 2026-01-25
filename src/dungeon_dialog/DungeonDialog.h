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
#include <QRandomGenerator>
#include <QHash>

#include "src/inventory_dialog/inventorydialog.h"
#include "src/partyinfo_dialog/partyinfodialog.h"
#include "../event/EventManager.h"
#include "../../GameStateManager.h"
#include "MiniMapDialog.h"
// Define MAP_SIZE here to resolve initialization errors in the class definition
const int MAP_SIZE = 30; 
struct TilePos {
    int x, y, z;
    bool operator==(const TilePos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};
// Custom hash for TilePos to be used in QSet and QMap
inline uint qHash(const TilePos& key, uint seed) 
{
    return qHash(key.x, seed) ^ qHash(key.y, seed) ^ qHash(key.z, seed);
}

namespace Ui {
    class DungeonDialog;
}

class DungeonDialog : public QDialog
{
    Q_OBJECT
    friend class DungeonHandlers; // allow the handler to see private members
public:
    explicit DungeonDialog(QWidget *parent = nullptr);
    //void enterLevel(int level);
    void enterLevel(int level, bool movingUp = false);
    ~DungeonDialog();
    void updateDungeonView(const QImage& dungeonImage);
    void updateCompass(const QString& direction);
    void updateLocation(const QString& location);
    void updateMinimap(int x, int y, int z);
signals:
    void teleporterUsed();
    void companionAttacked(int companionId);
    void companionCarried(int companionId);
    void exitedDungeonToCity();
private slots:
    void processCombatTick(); // Add this under private slots
    void on_fightButton_clicked();
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
    //void on_fightButton_clicked();
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
    void togglePartyInfo();
    
private:
    PartyInfoDialog *m_charSheet = nullptr; // Track the window here
    QPair<int, int> getCurrentPosition(); // The helper function
    QTimer *m_combatTimer = nullptr; // MUST be here
    int m_playerAttackCooldown = 0;
    int m_monsterAttackCooldown = 0;
    bool m_isFighting = false;


    void performPlayerAttack();
    void performMonsterAttack();

    QString m_activeMonsterName;
    int m_activeMonsterHP;
    bool m_isInCombat = false;
    QSet<QPair<int, int>> m_roomFloorTiles; // Tracks tiles that are part of rooms
    MinimapDialog *m_standaloneMinimap = nullptr;
    QList<QPair<int, int>> m_breadcrumbPath; // Stores the history of player positions
    const int MAX_BREADCRUMBS = 50;           // Limits the length of the trail
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
    //QString m_partyLeaderName; 
    //QString m_partyLeaderRace; 
    //QString m_partyLeaderAlignment; 
    // UI Member Widgets
    QLabel *m_locationLabel;
    QLabel *m_compassLabel;
    QGraphicsView *m_miniMapView;
    QListWidget *m_messageLog;
    // Buttons
    QPushButton *m_fightButton;
    QPushButton *m_spellButton;
    QPushButton *m_restButton;
    QPushButton *m_talkButton;
    QPushButton *m_searchButton;
    QPushButton *m_pickupButton;
    QPushButton *m_dropButton;
    QPushButton *m_openButton;
    QPushButton *m_mapButton;
    QPushButton *m_chestButton;
    QPushButton *m_exitButton;
    QPushButton *m_teleportButton;
    QPushButton *m_upButton;
    QPushButton *m_downButton;
    QPushButton *m_leftButton;
    QPushButton *m_rightButton;
    QPushButton *m_rotateLeftButton;
    QPushButton *m_rotateRightButton;
    QPushButton *m_stairsUpButton;
    QPushButton *m_stairsDownButton;
    // Experience Management Members
    QLabel *m_experienceLabel;
    void updateExperienceLabel();
    // Gold Management Members
    QLabel *m_goldLabel;
    //void refreshHealthUI();
    void updateGoldLabel();
    PartyInfoDialog *m_partyInfoDialog;
    // Party Management Member
    //QTableWidget *m_partyTable;
    // Health Management Helper
    void updatePartyMemberHealth(int row, int damage);
    // Map generation/management
    // Change all coordinate-based containers to use TilePos
    QSet<TilePos> m_visitedTiles3D; 
    QSet<TilePos> m_obstaclePositions3D;
    QSet<TilePos> m_chutePositions3D;
    QMap<TilePos, QString> m_monsterPositions3D;
    QMap<TilePos, QString> m_treasurePositions3D;
    QSet<QPair<int, int>> m_obstaclePositions;
    void generateRandomObstacles(int obstacleCount, QRandomGenerator& rng);
    void generateStairs(QRandomGenerator& rng); 
    void generateSpecialTiles(int tileCount, QRandomGenerator& rng);
    QPair<int, int> m_stairsUpPosition; 
    QPair<int, int> m_stairsDownPosition; 
    // Special Tile Position Sets (Combined list)
    QSet<QPair<int, int>> m_antimagicPositions; // implemented
    QSet<QPair<int, int>> m_extinguisherPositions; // implemented 
    QSet<QPair<int, int>> m_fogPositions; // implemented
    QSet<QPair<int, int>> m_pitPositions; // implemented
    QSet<QPair<int, int>> m_rotatorPositions; // implemented
    QSet<QPair<int, int>> m_studPositions; // implemented
    QSet<QPair<int, int>> m_chutePositions; // implemeted
    QSet<QPair<int, int>> m_teleportPositions; // implemented
    QSet<QPair<int, int>> m_waterPositions; // implemented
    QSet<QPair<int, int>> m_teleporterPositions; // implemented
    // Map data
    // In the private section of DungeonDialog class
    QSet<QPair<int, int>> m_visitedTiles; // Tracks which (x, y) coordinates have been seen
    QMap<QPair<int, int>, QString> m_monsterPositions;
    QMap<QPair<int, int>, QString> m_treasurePositions;
    QMap<QPair<int, int>, QString> m_trapPositions;
    QMap<QString, QString> m_MonsterAttitude;
    enum class StairDirection {
        Up,
        Down
    };
    void drawMinimap(); 
    void handleMovement(int actionIndex);
    void handleSurfaceExit();
    void transitionLevel(StairDirection direction);
    void tryUseStairs(bool goingUp);
    void rotate(int step);
    void movePlayer(int dx, int dy, int dz);
    // Helper functions
    void logMessage(const QString& message); 
    void spawnMonsters(const QString& monsterType, int count);
    void revealAroundPlayer(int x, int y, int z);
    void populateRandomTreasures(int level);
    void processTreasureOpening();
    void keyPressEvent(QKeyEvent *event) override;
    QGraphicsScene* m_threeDScene;
// ... other private members ...
    QGraphicsView* m_graphicsView;   // Add this line
    void update3DView();
    void drawWireframeWall(int depth, bool left, bool right, bool front);
    bool isWallAt(int x, int y);
    bool isWallAtSide(int x, int y, const QString& side);
    void renderWireframeView();
    void drawBrickPattern(const QPolygon& wallPoly, int depth);
    void  drawChute(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawMonster(int d, int xL, int xR, int yB);
    void drawTeleporter(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawSpinner(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawWater(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawAntimagic(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // DUNGEONDIALOG_H
