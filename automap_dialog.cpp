#include "automap_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMessageBox>

// Constructor
AutomapDialog::AutomapDialog(QWidget *parent) 
    : QDialog(parent), 
      isPositionVisible(true), 
      currentX(52), 
      currentY(89), 
      currentZ(3), 
      currentFacing("NORTH") // Mock starting position data
{
    setWindowTitle("Automap");
    setupUI();
    
    // Initial UI setup call
    updatePositionLabel();
}

/**
 * @brief Sets up the main UI layout for the Automap dialog.
 */
void AutomapDialog::setupUI() {
    // Set a fixed size to mimic the game's old-school interface
    setFixedSize(550, 600); 
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- Header ---
    headerLabel = new QLabel("Mordor Automap", this);
    headerLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #FFD700; padding: 10px; border-bottom: 2px solid #5D4037;");
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);

    // --- Position/Status Label ---
    positionLabel = new QLabel(this);
    positionLabel->setStyleSheet("color: #7FFF00; font-size: 14px; padding: 8px; background-color: #121212; border: 1px solid #5D4037;");
    positionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(positionLabel);

    // --- Map Display Area (Placeholder) ---
    mapDisplay = new QLabel(this);
    mapDisplay->setMinimumSize(400, 400); // Fixed size for the map
    mapDisplay->setAlignment(Qt::AlignCenter);
    
    // Use a placeholder text and style for the map area
    mapDisplay->setText("Map Data (X: 52, Y: 89, Z: 3)\n[Placeholder for Grid Drawing]");
    mapDisplay->setStyleSheet("background-color: #000000; color: #7FFF00; font-family: 'Consolas', monospace; border: 3px solid #5D4037;");
    
    mainLayout->addWidget(mapDisplay);

    // --- Button Layout ---
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QString buttonStyle = "QPushButton { background-color: #3e2723; color: #FFFFFF; border: 2px solid #5D4037; border-radius: 8px; padding: 10px 15px; }"
                          "QPushButton:hover { background-color: #5d4037; }";

    positionButton = new QPushButton("Hide Party Position", this);
    positionButton->setStyleSheet(buttonStyle);
    positionButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(positionButton);

    clearButton = new QPushButton("Clear Map", this);
    clearButton->setStyleSheet(buttonStyle);
    clearButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(clearButton);
    
    legendButton = new QPushButton("Map Legend", this);
    legendButton->setStyleSheet(buttonStyle);
    legendButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(legendButton);

    closeButton = new QPushButton("Close", this);
    closeButton->setStyleSheet(buttonStyle);
    closeButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(closeButton); 
    
    mainLayout->addLayout(buttonLayout);

    // Set dialog background style
    setStyleSheet("QDialog { background-color: #212121; }");

    // --- Connections ---
    connect(closeButton, &QPushButton::clicked, this, &AutomapDialog::onCloseClicked);
    connect(clearButton, &QPushButton::clicked, this, &AutomapDialog::onClearMapClicked);
    connect(positionButton, &QPushButton::clicked, this, &AutomapDialog::onTogglePositionClicked);
    connect(legendButton, &QPushButton::clicked, this, &AutomapDialog::onShowLegendClicked);
    
    setLayout(mainLayout);
}

/**
 * @brief Updates the status label with current coordinates and facing direction.
 */
void AutomapDialog::updatePositionLabel() {
    QString visibility = isPositionVisible ? "VISIBLE" : "HIDDEN";
    
    QString text = QString(
        "Party Position | X: <b>%1</b> Y: <b>%2</b> Z: <b>%3</b> | Facing: <b>%4</b> | Marker: <b>%5</b>"
    ).arg(currentX).arg(currentY).arg(currentZ).arg(currentFacing).arg(visibility);
        
    positionLabel->setText(text);
    
    // Update button text based on visibility state
    if (isPositionVisible) {
        positionButton->setText("Hide Party Position");
    } else {
        positionButton->setText("Show Party Position");
    }
}

// --- SLOTS Implementation ---

/**
 * @brief Slot called when the 'Close' button is clicked.
 */
void AutomapDialog::onCloseClicked() {
    this->accept(); // Closes the modal dialog
}

/**
 * @brief Slot called when the 'Clear Map' button is clicked.
 */
void AutomapDialog::onClearMapClicked() {
    // NOTE: In a real implementation, this would reset the internal map data structure.
    QMessageBox::information(this, tr("Clear Map"), 
        tr("The map data has been cleared. You will need to re-explore the current level."));
}

/**
 * @brief Slot called when the 'Toggle Party Position' button is clicked.
 */
void AutomapDialog::onTogglePositionClicked() {
    isPositionVisible = !isPositionVisible;
    
    // If the party position is hidden, clear the map marker placeholder
    if (!isPositionVisible) {
        mapDisplay->setText("Map Data\n[Placeholder - Position Hidden]");
    } else {
        mapDisplay->setText(QString("Map Data (X: %1, Y: %2, Z: %3)\n[Placeholder for Grid Drawing]").arg(currentX).arg(currentY).arg(currentZ));
    }
    
    updatePositionLabel();
}

/**
 * @brief Slot called when the 'Map Legend' button is clicked.
 */
void AutomapDialog::onShowLegendClicked() {
    QString legend = 
        "<b>Map Legend:</b>\n\n"
        "- 'X': Your Party's Current Location\n"
        "- '#': Solid Wall\n"
        "- ' ': Open Passage\n"
        "- 'D': Door (Locked or Unlocked)\n"
        "- 'T': Trap Location (Observed)\n"
        "\nConsult the Adventurer's Guide for more details on map icons.";

    QMessageBox::information(this, tr("Map Legend"), legend);
}
