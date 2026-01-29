#include "automap_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QMessageBox>
#include <QPainter> // Added for drawing functionality
#include <QPen>     // Added for drawing lines
#include <QBrush>   // Added for drawing filled shapes
// --- MapViewWidget Implementation ---
MapViewWidget::MapViewWidget(QWidget *parent) : QWidget(parent) {
    // Styling for the map area
    setStyleSheet("background-color: #000000; border: 3px solid #5D4037;");
    setMinimumSize(400, 400); 
}
/**
 * @brief Sets the player's map data and triggers a repaint.
 */
void MapViewWidget::setPlayerPosition(int x, int y, int z, const QString& facing, bool visible) {
    playerX = x;
    playerY = y;
    playerZ = z;
    playerFacing = facing;
    playerVisible = visible;
    update(); // Schedules a repaint event
}
/**
 * @brief Handles drawing the map content and player marker.
 */
void MapViewWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int width = this->width();
    int height = this->height();
    // 1. Draw Map Placeholder (Boundary)
    QPen borderPen(QColor("#7FFF00")); // Neon green for lines
    borderPen.setWidth(1);
    painter.setPen(borderPen);
    // Draw a placeholder grid or border to represent the dungeon floor
    painter.drawRect(0, 0, width - 1, height - 1);
    // Add text overlay if the position is hidden
    if (!playerVisible) {
        painter.setPen(QColor("#5D4037")); // Darker color for hidden state
        painter.setFont(QFont("Consolas", 14));
        painter.drawText(rect(), Qt::AlignCenter, "Map Data (Position Hidden)");
        return; // Stop drawing the dot if hidden
    }
    // 2. Draw the Player Marker (The Dot)
    
    // Since we are using mock X, Y coordinates (e.g., 52, 89), we need to map 
    // these dungeon coordinates (which could be 1-100) onto the widget's pixel space.
    
    // Assume dungeon coordinates range from 0 to 100 for simplicity of mock data.
    const int MAP_MAX_COORD = 100;

    // Calculate normalized position (0.0 to 1.0)
    qreal normalizedX = (qreal)playerX / MAP_MAX_COORD;
    qreal normalizedY = (qreal)playerY / MAP_MAX_COORD; 
    // Calculate pixel position on the widget
    // NOTE: In dungeon crawlers, Y often increases DOWN, so 1.0 - normalizedY might be needed.
    // We'll use Y increasing downward for screen coordinates:
    int pixelX = qRound(normalizedX * width);
    int pixelY = qRound(height - (normalizedY * height)); // Invert Y for typical map drawing
    // Ensure the dot stays within bounds
    pixelX = qBound(5, pixelX, width - 5);
    pixelY = qBound(5, pixelY, height - 5);
    // Set the brush and pen for the player dot
    QBrush dotBrush(QColor("#FF4500")); // Bright red/orange for visibility
    painter.setBrush(dotBrush);
    painter.setPen(QPen(QColor("#FFFFFF"), 2)); // White border for contrast
    // Draw a filled circle (the player dot)
    const int DOT_SIZE = 8;
    painter.drawEllipse(pixelX - DOT_SIZE / 2, pixelY - DOT_SIZE / 2, DOT_SIZE, DOT_SIZE);
    // Optional: Draw facing indicator (simple line)
    QPointF start(pixelX, pixelY);
    QPointF end = start;
    int indicatorLength = 10;

    if (playerFacing == "NORTH") end.setY(end.y() - indicatorLength);
    else if (playerFacing == "SOUTH") end.setY(end.y() + indicatorLength);
    else if (playerFacing == "EAST") end.setX(end.x() + indicatorLength);
    else if (playerFacing == "WEST") end.setX(end.x() - indicatorLength);

    QPen arrowPen(QColor("#FFFFFF"));
    arrowPen.setWidth(2);
    painter.setPen(arrowPen);
    painter.drawLine(start, end);
}
// --- AutomapDialog Implementation ---

// Constructor
AutomapDialog::AutomapDialog(QWidget *parent) 
    : QDialog(parent), 
      isPositionVisible(true), 
      currentX(52), // Mock X position
      currentY(89), // Mock Y position
      currentZ(3), 
      currentFacing("NORTH")
{
    setWindowTitle("Automap");
    setupUI();
    
    // Pass initial data to the map widget
    mapDisplay->setPlayerPosition(currentX, currentY, currentZ, currentFacing, isPositionVisible);
    updatePositionLabel();
}
/**
 * @brief Public interface to move the player's position on the map and update the view.
 * * @param x The new X coordinate.
 * @param y The new Y coordinate.
 * @param facing The new direction the player is facing (NORTH, SOUTH, EAST, WEST).
 */
void AutomapDialog::updatePlayerPosition(int x, int y, const QString &facing) {
    // 1. Update internal state
    currentX = x;
    currentY = y;
    // currentZ remains the same unless a level change is detected, 
    // which would require an additional parameter or function.
    currentFacing = facing.toUpper(); // Ensure direction is capitalized

    // 2. Update the UI elements
    updatePositionLabel();
    mapDisplay->setPlayerPosition(currentX, currentY, currentZ, currentFacing, isPositionVisible);
    
    qDebug() << "Map position updated to X:" << currentX << "Y:" << currentY << "Facing:" << currentFacing;
}
/**
 * @brief Sets up the main UI layout for the Automap dialog.
 */
void AutomapDialog::setupUI() {
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

    // --- Map Display Area (MapViewWidget) ---
    mapDisplay = new MapViewWidget(this); // Now the custom widget
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
void AutomapDialog::onCloseClicked() {
    this->accept(); 
}

void AutomapDialog::onClearMapClicked() {
    QMessageBox::information(this, tr("Clear Map"), 
        tr("The map data has been cleared. You will need to re-explore the current level."));
}

void AutomapDialog::onTogglePositionClicked() {
    isPositionVisible = !isPositionVisible;
    
    // Update the MapViewWidget visibility and redraw
    mapDisplay->setPlayerPosition(currentX, currentY, currentZ, currentFacing, isPositionVisible);
    
    updatePositionLabel();
}

void AutomapDialog::onShowLegendClicked() {
    QString legend = 
        "<b>Map Legend:</b>\n\n"
        "- <font color='#FF4500'>●</font>: Your Party's Current Location\n"
        "- '#': Solid Wall\n"
        "- ' ': Open Passage\n"
        "- 'D': Door (Locked or Unlocked)\n"
        "- 'T': Trap Location (Observed)\n"
        "\nConsult the Adventurer's Guide for more details on map icons.";

    QMessageBox::information(this, tr("Map Legend"), legend);
}
