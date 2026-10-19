#ifndef DUNGEONDIALOG_H
#define DUNGEONDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <functional>
#include <QImage>
#include <QKeyEvent>
#include <QSet>
#include <QPair>
#include <QTableWidget>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QHash>
#include <QVector>
#include <QColor>

#include "src/inventory_dialog/inventorydialog.h"
#include "src/partyinfo_dialog/partyinfodialog.h"
#include "../event/EventManager.h"
#include "src/core/gameStateManager.h"
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
struct FloorTheme;

// Define MAP_SIZE here to resolve initialization errors in the class definition
const int MAP_SIZE = 50;
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

class CombatController;

class DungeonDialog : public QDialog
{
    Q_OBJECT
    friend class DungeonHandlers; // allow the handler to see private members
public:
    explicit DungeonDialog(QWidget *parent = nullptr);
    
    void enterLevel(int level, bool movingUp = false);
    ~DungeonDialog();
    void updateDungeonView(const QImage& dungeonImage);
    void updateCompass(const QString& direction);
    void updateLocation(const QString& location);
    void updateMinimap(int x, int y, int z);
signals:
    void teleporterUsed();

    void exitedDungeonToCity();
    void exitedDungeonToMainMenu();
public slots:
    void on_eatButton_clicked();
    void on_restButton_clicked();

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



    void on_pickupButton_clicked();
    void on_dropButton_clicked();
    void on_spellButton_clicked();
    void on_takeButton_clicked();
    void on_openButton_clicked();

    void on_searchButton_clicked();
    void on_disarmButton_clicked();
    void on_talkButton_clicked();
    void on_stairsDownButton_clicked();
    void on_stairsUpButton_clicked();
    void on_stairsLowerButton_clicked();
    void on_chestButton_clicked();

    void moveMonsters();

    // Monster AI helpers.
    int distanceBetween(QPair<int, int> a, QPair<int, int> b);
    bool hasLineOfSight(QPair<int, int> from, QPair<int, int> to);
    QVector<QPair<int, int>> getWalkableNeighbors(QPair<int, int> pos);

    void togglePartyInfo();

    // --- Combat Slots ---
    void on_combatDefendButton_clicked();
    void on_combatUseItemButton_clicked();
    void on_combatSpellButton_clicked();
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
    void on_combatAttackButton_clicked(); // public for tests
    void updateCombatUI(); // public for tests

private:
    void setupControls();
    void setupWindowAndState(gameStateManager* gsm);
    void setupMainLayout();
    void setupSidebarContent();
    void setupMapGeneration();
    void setupActionButtons();
    void setupSignalConnections();
    void setupCombatSystem();
    int m_initialLevel = 1;

    QSet<QPair<int, int>> m_bodyPositions;

public:
    PartyInfoDialog *m_charSheet = nullptr; // Track the window here
    QMap<QPair<int, int>, QString> m_monsterPositions; // public for tests
    CombatController* m_combatController = nullptr; // public for tests
    QPair<int, int> getCurrentPosition(); // The helper function
    // --- Phase 4: Torch / Light ---
    int m_torchFuel = 0;              // turns of light remaining on the current floor
    static const int DEFAULT_TORCH_FUEL = 50;

    bool m_isInCombat = false;
    QSet<QPair<int, int>> m_roomFloorTiles; // Tracks tiles that are part of rooms

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
    QPushButton* createButton(const QString& text, std::function<void()> slot);

    QPushButton *m_upButton;
    QPushButton *m_downButton;
    QPushButton *m_leftButton;
    QPushButton *m_rightButton;
    QPushButton *m_rotateLeftButton;
    QPushButton *m_rotateRightButton;
    QPushButton *m_stairsUpButton;
    QPushButton *m_stairsDownButton;
    QPushButton *m_stairsLowerButton = nullptr;
    QPushButton *m_fightButton = nullptr;
    QPushButton *m_spellButton = nullptr;
    QPushButton *m_restButton = nullptr;
    QPushButton *m_talkButton = nullptr;
    QPushButton *m_eatButton = nullptr;
    QPushButton *m_takeButton = nullptr;
    QPushButton *m_disarmButton = nullptr;
    QPushButton *m_searchButton = nullptr;
    QPushButton *m_pickupButton = nullptr;
    QPushButton *m_dropButton = nullptr;
    QPushButton *m_openButton = nullptr;

    QPushButton *m_chestButton = nullptr;

    QPushButton *m_teleportButton = nullptr;
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
    QPushButton* m_combatRowBtn = nullptr;   // v3.0.0 slice 3.3: front/back toggle
    QLabel* m_combatMonsterHpLabel = nullptr;
    QLabel* m_combatPartyHpLabel = nullptr;
    QGroupBox* m_combatGroup = nullptr;

    // --- v3.0.0 slice 3.1: target selection ---
    // The player chooses which monster to hit; group encounters add one row per
    // monster. Rows carry the participant index in Qt::UserRole.
    QListWidget* m_combatTargetList = nullptr;
    QTimer* m_combatFxTimer = nullptr;   // v3.0.0 slice 3.8: clears flash effect

    // Index of the monster the player has selected, or -1 when none is chosen.
    // Reads the selection list, so it is the same value the Attack button uses.
    int selectedMonsterIndex() const;
    // Select the first living monster (used when nothing is selected, and after
    // the selected monster dies so keyboard-only play keeps working).
    int selectFirstLivingMonster();

    // Short human-readable status for a combat row: "poison 3", "fire 2",
    // "blind", "stunned", "slowed". Empty when the participant is clean.
    // Static and free of UI state so the self-tests can check it directly.
    static QString combatStatusText(const CombatParticipant& p);

    // v3.0.0 slice 3.3: move a party member between the front and back row.
    // Melee only reaches the front, so this is how a caster is protected.
    // Returns the member's new row, or false when the index is not a player.
    bool togglePlayerRow(int participantIndex);

    // v3.0.0 slice 3.8: flash the combat panel red on damage, green on heal.
    // `true` = damage (red), `false` = heal (green). No-op when combatFxEnabled is false.
    void flashCombatFx(bool damage);
    // Is combat visual feedback enabled? Reads game state (default on).
    static bool combatFxEnabled();
    // Current stylesheet of the combat group (for testing flash effects).
    QString combatGroupStyleSheet() const;

    // --- v5.0.0 slice 5.1: hunger is visible ---
    // One party-panel row's text: name, level, HP and hunger.
    // Static and free of UI state so the self-tests can check it directly.
    static QString partyMemberStatusText(const Character& c);
    // The colour a party-panel row is drawn in: red for dead/starving, yellow
    // for low HP or low hunger, green when healthy.
    static QColor partyMemberColor(const Character& c);
    // The message shown when a member crosses into the low-hunger band.
    // Empty when the member is not low.
    static QString hungerWarningMessage(const Character& c);
    // Announce any member that has just crossed into the low-hunger band.
    // Fires once per crossing (not every step) and returns how many were
    // announced. Called after each move.
    int announceHungerWarnings();

    // --- v5.0.0 slice 5.2: eating ---
    // Inventory index of the first food item a party member holds, or -1.
    // Static and UI-free so the self-tests can check it directly.
    static int firstFoodIndex(const Character& c);
    // The active member's first food slot (-1 when none). Public for tests.
    int foodInventoryIndex(int memberIndex) const;
    // Eat the active member's first food item. Returns true when a meal was
    // eaten. Public for tests; also bound to the Eat action and the 'F' key.
    bool eatFromInventory();

    // --- Phase 3: Movement & UI ---
    // Integrated minimap (in the main layout, not a popup)
    QGraphicsView* m_miniMapViewIntegrated = nullptr;
    // Party status panel
    QListWidget* m_partyStatusList = nullptr;
    void updatePartyPanel();
    // v5.0.0 slice 5.1: indices of members already warned about low hunger,
    // so the crossing is announced once rather than every step.
    QSet<int> m_hungerWarned;
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
    QList<QPair<int, int>> m_roomCenters;
    QVector<QVector<int>> m_zoneMap;
    QList<QPair<int, int>> m_zoneCenters;
    QSet<QPair<int, int>> m_lowerLevelTiles;
    QPair<int, int> m_lowerStairsUpPosition;
    int m_currentLevel = 1;
    bool m_onLowerLevel = false;
    void generateRandomObstacles(int roomCount, QRandomGenerator& rng);
    void bspSplit(struct BspNode* node, QRandomGenerator& rng, int depth);
    void bspPlaceRoom(struct BspNode* node, QRandomGenerator& rng);
    void bspConnect(struct BspNode* node, QRandomGenerator& rng);
    void assignZones(QRandomGenerator& rng);
    void generateMultiLevelRooms(QRandomGenerator& rng);
    void generateStairs(QRandomGenerator& rng);
    void generateSpecialTiles(int tileCount, QRandomGenerator& rng, const FloorTheme* theme = nullptr);
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
    int m_fogTimer = 0; // Turns remaining with reduced vision (fog tiles)
    int m_lastRotationStep = 0; // Last rotation applied by a rotator tile (for tests)
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
    void exitToMainMenu();
    void transitionLevel(StairDirection direction);

    void rotate(int step);
    void movePlayer(int dx, int dy, int dz);
    // Helper functions
    void logMessage(const QString& message);

    void revealAroundPlayer(int x, int y, int z);
    void populateRandomTreasures(int level);
    void processTreasureOpening();
    void keyPressEvent(QKeyEvent *event) override;
    // Main first-person wireframe view (fills the left panel).
    QGraphicsView* m_graphicsView;
    void fitViewport();
    bool isWallAt(int x, int y, int z = 0);
    bool isWallAtSide(int x, int y, const QString& side);
    void renderWireframeView();
    void drawBrickPattern(const QPolygon& wallPoly, int depth);
    void drawPit(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawFog(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawExtinguisher(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawTreasure(int d, int xL, int xR, int yB);
    void drawTrap(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawHiddenDoor(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawStud(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
    void drawBody(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB);
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
