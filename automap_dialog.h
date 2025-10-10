#ifndef AUTOMAP_DIALOG_H
#define AUTOMAP_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QString>

class AutomapDialog : public QDialog {
    Q_OBJECT

public:
    explicit AutomapDialog(QWidget *parent = nullptr);

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
    QLabel *mapDisplay; // Represents the visual map area
    QLabel *positionLabel; // Displays X, Y, Z, Facing
    
    QPushButton *positionButton;
    QPushButton *clearButton;
    QPushButton *legendButton;
    QPushButton *closeButton;
    
    // State and mock data
    bool isPositionVisible;
    int currentX;
    int currentY;
    int currentZ;
    QString currentFacing;
};

#endif // AUTOMAP_DIALOG_H
