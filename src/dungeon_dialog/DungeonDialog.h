#ifndef DUNGEONDIALOG_H
#define DUNGEONDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QImage>
#include <QTimer>      // For the monster spawning logic
#include <QLabel>      // For location and compass updates
#include <QListWidget> // For the message log
#include <QKeyEvent>   // For arrow key movement
#include <QPushButton> // For dynamic control of the chest button

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
    void updateMinimap(int x, int y); // Function to update the player's position on the map

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
    
    // --- New Slots for Game Logic ---
    void checkMonsterSpawn(); // Slot connected to the spawn timer timeout
    void initiateFight();     // Slot to start the fight sequence
    void on_winBattle_trigger(); // Slot called upon winning a fight to check for treasure
    void on_chestButton_clicked(); // Slot for the 'Chest' button

private:
    Ui::DungeonDialog *ui;
    QGraphicsScene *m_dungeonScene; // Scene for the first-person view

    // --- Members for Map, Timer, and Game State ---
    QGraphicsScene *m_fullMapScene; // Scene for the full 30x30 map
    QTimer *m_spawnTimer;           // Timer to check for monster spawning
    int m_playerMapX = 0;           // Player's X coordinate on the map
    int m_playerMapY = 0;           // Player's Y coordinate on the map
    bool m_chestFound = false;      // Tracks if a chest is currently present

    // --- Members to allow external updating and dynamic control ---
    QLabel *m_locationLabel;
    QLabel *m_compassLabel;
    QGraphicsView *m_miniMapView;   // View for the full map
    QListWidget *m_messageLog;
    QPushButton *m_chestButton;     // Pointer to the button for dynamic enabling/disabling

    // --- Helper and Event Functions ---
    void logMessage(const QString& message); // Helper function for message log
    void keyPressEvent(QKeyEvent *event) override; // Override for arrow key movement
};

#endif // DUNGEONDIALOG_H
