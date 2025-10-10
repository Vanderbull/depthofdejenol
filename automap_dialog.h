#ifndef AUTOMAP_DIALOG_H
#define AUTOMAP_DIALOG_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent> // Needed for keyPressEvent declaration

// Forward declarations for efficiency
class QPaintEvent; 

// --- MapViewWidget Class Declaration ---
class MapViewWidget : public QWidget
{
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

// --- AutomapDialog Class Declaration ---
class AutomapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AutomapDialog(QWidget *parent = nullptr);

    // Public method to be called by keyPressEvent
    void updatePlayerPosition(int x, int y, const QString &facing);

    // Getters for current position (optional if keyPressEvent uses currentX/Y directly)
    int getCurrentX() const { return currentX; }
    int getCurrentY() const { return currentY; }
    QString getCurrentFacing() const { return currentFacing; }

protected:
    // FIX: DECLARE the reimplemented key press event handler
    void keyPressEvent(QKeyEvent *event) override; 

private slots:
    void onCloseClicked();
    void onClearMapClicked();
    void onTogglePositionClicked();
    void onShowLegendClicked();

private:
    void setupUI();
    void updatePositionLabel();

    // UI Components
    MapViewWidget *mapDisplay = nullptr;
    QLabel *headerLabel = nullptr;
    QLabel *positionLabel = nullptr;
    QPushButton *positionButton = nullptr;
    QPushButton *clearButton = nullptr;
    QPushButton *legendButton = nullptr;
    QPushButton *closeButton = nullptr;

    // Internal State
    bool isPositionVisible;
    int currentX;
    int currentY;
    int currentZ;
    QString currentFacing;
};

#endif // AUTOMAP_DIALOG_H
