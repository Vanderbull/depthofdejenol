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

namespace Ui {
class DungeonDialog;
}

class DungeonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DungeonDialog(QWidget *parent = nullptr);
    ~DungeonDialog();

    void updateDungeonView(const QImage& dungeonImage);
    void updateCompass(const QString& direction);
    void updateLocation(const QString& location);
    void updateMinimap(int x, int y); 

signals:
    void teleporterUsed();
    void companionAttacked(int companionId);
    void companionCarried(int companionId);

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

private:
    // FIX: Moved the enum here to ensure MOC compatibility.
    enum MonsterAttitude {
        Hostile, // Also acts as the "No Encounter" state when no fight is active
        Neutral,
        Friendly
    };
    
    Ui::DungeonDialog *ui;
    QGraphicsScene *m_dungeonScene; // Scene for the first-person view

    // --- Members for Map, Timer, and Game State ---
    QGraphicsScene *m_fullMapScene;     // Scene for the full 30x30 map
    QTimer *m_spawnTimer;               // Timer to check for monster spawning
    int m_playerMapX;                   // Player's X coordinate on the map
    int m_playerMapY;                   // Player's Y coordinate on the map
    bool m_chestFound;                  // Tracks if a chest is currently present
    
    // m_currentMonsterAttitude is listed here as it appears in the constructor
    MonsterAttitude m_currentMonsterAttitude; // Attitude of the currently spawned monster

    // --- Pointers to UI Widgets (for access outside constructor) ---
    QLabel *m_locationLabel;
    QLabel *m_compassLabel;
    QGraphicsView *m_miniMapView;       // View for the full map (no scrollbars)
    QListWidget *m_messageLog;
    
    // m_chestButton is listed here as it appears in the constructor
    QPushButton *m_chestButton;         // Pointer to the button for dynamic enabling/disabling

    // --- Helper and Event Functions ---
    void logMessage(const QString& message); // Helper function for message log
    void keyPressEvent(QKeyEvent *event) override; // Override for arrow key movement
};

#endif // DUNGEONDIALOG_H
