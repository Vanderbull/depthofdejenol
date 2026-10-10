#ifndef DUNGEONDIALOG_H
#define DUNGEONDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QImage>
#include <QKeyEvent>
#include <QSet>
#include <QPair>
#include <QTableWidget>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QHash>
#include <QVector>

#include "src/inventory_dialog/inventorydialog.h"
#include "src/partyinfo_dialog/partyinfodialog.h"
#include "../event/EventManager.h"
#include "../../gameStateManager.h"
#include "src/combat/CombatState.h"
#include "src/combat/TurnEngine.h"
#include "src/combat/CombatActions.h"
#include "src/combat/MonsterAI.h"
#include "src/combat/EncounterBuilder.h"
#include "src/combat/VictoryReward.h"
#include "src/combat/CombatDeathHandler.h"
#include "src/core/DoorAndSearch.h"
#include "MiniMapDialog.h"
#include <QObject>

// Forward declarations
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QPushButton;
class QListWidget;
class QTimer;

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

    void exitedDungeonToCity();
private slots:
    void on_fightButton_clicked();
    // --- NEW MOVEMENT SLOTS ---
    void moveForward();
    void moveBackward();
    void moveStepLeft();  // NEW: Sidestep left
    void moveStepRight(); // NEW: Sidestep right
    void on_rotateLeftButton_clicked();
    void on_rotateRightButton_clicked();
    void on_teleportButton_clicked();

    void on_mapButton_clicked();
    void openAutomap();
    void on_pickupButton_clicked();
    void on_dropButton_clicked();
    void on_spellButton_clicked();
    void on_takeButton_clicked();
    void on_openButton_clicked();
    void on_exitButton_clicked();
    void on_searchButton_clicked();
    void on_disarmButton_clicked();
    void on_restButton_clicked();
    void on_talkButton_clicked();
    void on_stairsDownButton_clicked();
    void on_stairsUpButton_clicked();
    void on_chestButton_clicked();
    void onEventTriggered(const GameEvent& event);
    void initiateFight();
    void moveMonsters();

    // Monster AI helpers.
    int distanceBetween(QPair<int, int> a, QPair<int, int> b);
    bool hasLineOfSight(QPair<int, int> from, QPair<int, int> to);
    QVector<QPair<int, int>> getWalkableNeighbors(QPair<int, int> pos);
    void on_winBattle_trigger();
    void togglePartyInfo();

    // --- Combat Slots ---
    void on_combatAttackButton_clicked();
    void on_combatDefendButton_clicked();
    void on_combatUseItemButton_clicked();
    void on_combatSpellButton_clicked();
    void updateCombatUI();
    void handlePartyWipe();
    void syncCombatToGameState();
    QStringList getThematicMonsters(int level) const;

public:
    // Start combat with the monster at the given position. Returns true if
    // combat was started. Extracted from on_fightButton_clicked so that
    // handleEncounters can call it when the player steps onto a monster.
    bool startCombatAt(const QPair<int, int>& pos);
    void on_combatFleeButton_clicked();
    bool fleeCombat(); // public: true if the party escaped
    void advanceCombat(); // public for tests
    void handleVictory(); // public for tests

private:
    void awardBattleLoot();
    void setupControls();
    void handleFalling(); // New method to handle falling through a pit
    QSet<QPair<int, int>> m_bodyPositions;

public:
    PartyInfoDialog *m_charSheet = nullptr; // Track the window here
    QMap<QPair<int, int>, QString> m_monsterPositions; // public for tests
    QPair<int, int> getCurrentPosition(); // The helper function
    // --- Phase 4: Torch / Light ---
    int m_torchFuel = 0;              // turns of light remaining on the current floor
    static const int DEFAULT_TORCH_FUEL = 50;

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
    bool m_chestFound;
    MonsterAttitude m_currentMonsterAttitude;
    // UI Member Widgets
    QLabel *m_locationLabel;
    QLabel *m_compassLabel;
    QListWidget *m_messageLog;
    // Buttons
    QMap<QString, QPushButton*> m_controls;
    // A helper to make button creation cleaner
    QPushButton* createButton(const QString& text, const char* slot);
    
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
    void updateGoldLabel();
    PartyInfoDialog *m_partyInfoDialog;
    // Health Management Helper
    void updatePartyMemberHealth(int row, int damage);

    // Combat System (from src/combat/)
    CombatState* m_combatState = nullptr;
    TurnEngine* m_turnEngine = nullptr;
    CombatActions* m_combatActions = nullptr;
    MonsterAI* m_monsterAI = nullptr;
    CombatDeathHandler* m_deathHandler = nullptr;
    bool m_inCombat = false;
    QString m_combatMonsterName;
    int m_combatMonsterLevel = 1;
    bool m_combatIsBoss = false;

    // Combat UI elements
    QPushButton* m_combatAttackBtn = nullptr;
    QPushButton* m_combatDefendBtn = nullptr;
    QPushButton* m_combatFleeBtn = nullptr;
    QPushButton* m_combatUseItemBtn = nullptr;
    QPushButton* m_combatSpellBtn = nullptr;
    QLabel* m_combatMonsterHpLabel = nullptr;
    QLabel* m_combatPartyHpLabel = nullptr;
    QGroupBox* m_combatGroup = nullptr;

    // --- Phase 3: Movement & UI ---
    // Integrated minimap (in the main layout, not a popup)
    QGraphicsView* m_miniMapViewIntegrated = nullptr;
    // Party status panel
    QListWidget* m_partyStatusList = nullptr;
    void updatePartyPanel();
    // Auto-backtrack
    void on_backtrackButton_clicked();
    QPushButton* m_backtrackButton = nullptr;
    // Diagonal movement
    void moveDiagonalForwardLeft();
    void moveDiagonalForwardRight();
    // --- Phase 4: Torch / Light ---
    void updateTorchState();
    void restForTorch();
    bool torchLit() const;
    int torchFuel() const;
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
    QSet<QPair<int, int>> m_waterPositions; // implemented
    QSet<QPair<int, int>> m_teleporterPositions; // implemented
    QMap<QPair<int, int>, DoorState> m_hiddenDoorPositions;
    // Map data
    // In the private section of DungeonDialog class
    QSet<QPair<int, int>> m_visitedTiles; // Tracks which (x, y) coordinates have been seen
    QMap<QPair<int, int>, QString> m_treasurePositions;
    QMap<QPair<int, int>, QString> m_trapPositions;
    QSet<QPair<int, int>> m_lockedChests;      // chests that need a key
    QMap<QPair<int, int>, QString> m_chestKeys; // chest pos -> key name
    QSet<QPair<int, int>> m_openedChests;      // chests already opened
    QSet<QPair<int, int>> m_triggeredTraps;
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
    // Main first-person wireframe view (fills the left panel).
    QGraphicsView* m_graphicsView;
    void fitViewport();
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
