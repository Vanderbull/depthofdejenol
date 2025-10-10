#ifndef AUTOMAP_DIALOG_H
#define AUTOMAP_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QWidget> 

/**
 * @brief Custom QWidget class responsible for drawing the map and the player's position.
 */
class MapViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapViewWidget(QWidget *parent = nullptr);
    void setPlayerPosition(int x, int y, int z, const QString& facing, bool visible);
    
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int playerX = 0;
    int playerY = 0;
    int playerZ = 0;
    QString playerFacing = "NORTH";
    bool playerVisible = true;
};

class AutomapDialog : public QDialog {
    Q_OBJECT

public:
    explicit AutomapDialog(QWidget *parent = nullptr);
    
    /**
     * @brief Public interface to move the player's position on the map.
     * * This function is called by the main game loop to update the map view 
     * whenever the player moves or changes facing direction.
     * * @param x The new X coordinate.
     * @param y The new Y coordinate.
     * @param facing The new direction the player is facing (e.g., "NORTH", "EAST").
     */
    void updatePlayerPosition(int x, int y, const QString &facing); // Added public function

private slots:
    void onCloseClicked();
    void onClearMapClicked();
    void onTogglePositionClicked();
    void onShowLegendClicked();

private:
    void setupUI();
    void updatePositionLabel();

    // UI elements
    QLabel *headerLabel;
    MapViewWidget *mapDisplay; 
    QLabel *positionLabel; // Displays X, Y, Z, Facing
    
    QPushButton *positionButton;
    QPushButton *clearButton;
    QPushButton *legendButton;
    QPushButton *closeButton;
    
    // State and mock data
    bool isPositionVisible;
    int currentX;
    int currentY;
    int currentZ; // Note: For movement, Z (dungeon level) often remains static.
    QString currentFacing;
};

#endif // AUTOMAP_DIALOG_H
