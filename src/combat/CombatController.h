#ifndef COMBATCONTROLLER_H
#define COMBATCONTROLLER_H

#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QListWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

#include "src/combat/CombatState.h"
#include "src/combat/TurnEngine.h"
#include "src/combat/CombatActions.h"
#include "src/combat/MonsterAI.h"
#include "src/combat/CombatDeathHandler.h"

class CombatController : public QObject
{
    Q_OBJECT

public:
    explicit CombatController(QObject *parent = nullptr);
    ~CombatController() override;

    // Set UI widget pointers (called by DungeonDialog after creation)
    void setCombatWidgets(QGroupBox* group, QLabel* monsterHpLabel,
                          QListWidget* targetList, QLabel* partyHpLabel,
                          QPushButton* attackBtn, QPushButton* defendBtn,
                          QPushButton* fleeBtn, QPushButton* useItemBtn,
                          QPushButton* spellBtn, QPushButton* rowBtn,
                          QTimer* fxTimer);



    // Initialize combat at a given map position. Returns true if combat started.
    bool startCombatAt(const QPair<int, int>& pos);

    // Combat actions
    void attack(int targetIndex);
    void defend();
    bool flee();
    void useItem(int slotIndex);
    void castSpell();
    bool togglePlayerRow(int participantIndex);

    // Turn advancement
    void advanceCombat();

    // UI updates
    void updateCombatUI();
    int selectedMonsterIndex() const;
    int selectFirstLivingMonster();

    // Combat state queries
    bool isInCombat() const { return m_inCombat; }
    void setInCombat(bool inCombat) { m_inCombat = inCombat; }
    bool isPlayerTurn() const;

    // Combat FX
    void flashCombatFx(bool damage);
    static bool combatFxEnabled();
    static QString combatStatusText(const CombatParticipant& p);

    // Accessors for DungeonDialog integration
    TurnEngine* turnEngine() const { return m_turnEngine; }
    CombatState* combatState() const { return m_combatState; }
    CombatActions* combatActions() const { return m_combatActions; }
    MonsterAI* monsterAI() const { return m_monsterAI; }
    CombatDeathHandler* deathHandler() const { return m_deathHandler; }

    // Victory/defeat handling
    void handleVictory();
    void handlePartyWipe();

    // Sync combat HP back to gameStateManager
    void syncCombatToGameState();

    // Set the log message callback
    void setLogCallback(std::function<void(const QString&)> callback);

    // Set the render callback (called after combat ends)
    void setRenderCallback(std::function<void()> callback);

signals:
    void combatStarted();
    void combatEnded();
    void combatLog(const QString& message);

private:

    CombatState* m_combatState = nullptr;
    TurnEngine* m_turnEngine = nullptr;
    CombatActions* m_combatActions = nullptr;
    MonsterAI* m_monsterAI = nullptr;
    CombatDeathHandler* m_deathHandler = nullptr;

    // UI widgets (owned by DungeonDialog, not by this controller)
    QGroupBox* m_combatGroup = nullptr;
    QLabel* m_combatMonsterHpLabel = nullptr;
    QListWidget* m_combatTargetList = nullptr;
    QLabel* m_combatPartyHpLabel = nullptr;
    QPushButton* m_combatAttackBtn = nullptr;
    QPushButton* m_combatDefendBtn = nullptr;
    QPushButton* m_combatFleeBtn = nullptr;
    QPushButton* m_combatUseItemBtn = nullptr;
    QPushButton* m_combatSpellBtn = nullptr;
    QPushButton* m_combatRowBtn = nullptr;
    QTimer* m_combatFxTimer = nullptr;

    // Callbacks
    std::function<void(const QString&)> m_logCallback;
    std::function<void()> m_renderCallback;

    // Combat context
    bool m_inCombat = false;
    QString m_combatMonsterName;
    int m_combatMonsterLevel = 1;
    bool m_combatIsBoss = false;
};

#endif // COMBATCONTROLLER_H
