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
#include <QTableWidget> // Include QTableWidget for the member type
#include "src/partyinfo_dialog/partyinfodialog.h"
#include "../event/EventManager.h"

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

    // --- Slots for Game Logic ---
    void checkMonsterSpawn();
    void initiateFight();
    void on_winBattle_trigger();
    void on_chestButton_clicked();
    void onEventTriggered(const GameEvent& event);

private:
    enum MonsterAttitude {
        Hostile,
        Neutral,
        Friendly
    };

    Ui::DungeonDialog *ui;
    QGraphicsScene *m_dungeonScene;
    QGraphicsScene *m_fullMapScene;
    QTimer *m_spawnTimer;
    int m_playerMapX;
    int m_playerMapY;
    int m_currentLevel; 
    bool m_chestFound;
    MonsterAttitude m_currentMonsterAttitude;
    
    // --- UI Member Widgets ---
    QLabel *m_locationLabel;
    QLabel *m_compassLabel;
    QGraphicsView *m_miniMapView;
    QListWidget *m_messageLog;
    QPushButton *m_chestButton;
    
    // --- Gold Management Members ---
    QLabel *m_goldLabel;        
    quint64 m_currentGold;      
    void updateGoldLabel();     

    PartyInfoDialog *m_partyInfoDialog;
    
    // --- Party Management Member ---
    QTableWidget *m_partyTable; 

    // --- Health Management Helper ---
    void updatePartyMemberHealth(int row, int damage);

    // --- Obstacle/Map management ---
    QSet<QPair<int, int>> m_obstaclePositions;
    void generateRandomObstacles(int obstacleCount);
    void generateStairs(); 
    void generateAntimagicTiles(int tileCount); // NEW: Antimagic tile generation

    QPair<int, int> m_stairsUpPosition; 
    QPair<int, int> m_stairsDownPosition; 
    
    QSet<QPair<int, int>> m_antimagicPositions; // NEW: Antimagic positions

    void logMessage(const QString& message);
    void keyPressEvent(QKeyEvent *event) override;
    void spawnMonsters(const QString& monsterType, int count);
};

#endif // DUNGEONDIALOG_H
