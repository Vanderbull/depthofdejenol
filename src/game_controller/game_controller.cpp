#include "game_controller.h"
// No need for other includes if they are in the header
GameController::GameController(QWidget *parent) : QWidget(parent)
{
    // Important: Set the focus policy to ensure the widget receives key events.
    setFocusPolicy(Qt::StrongFocus);
}

void GameController::keyPressEvent(QKeyEvent *event)
{
    // Check for arrow keys
    switch (event->key()) {
    case Qt::Key_Up:
        handleArrowPress(Qt::Key_Up);
        event->accept(); // Consume the event
        break;
    case Qt::Key_Down:
        handleArrowPress(Qt::Key_Down);
        event->accept();
        break;
    case Qt::Key_Left:
        handleArrowPress(Qt::Key_Left);
        event->accept();
        break;
    case Qt::Key_Right:
        handleArrowPress(Qt::Key_Right);
        event->accept();
        break;
    default:
        // Pass other key events up to the base class
        QWidget::keyPressEvent(event);
        break;
    }
}

void GameController::handleArrowPress(Qt::Key key)
{
    // This function runs every time an arrow key is pressed
    QString keyName;
    switch (key) {
    case Qt::Key_Up:    keyName = "UP ⬆️";    break;
    case Qt::Key_Down:  keyName = "DOWN ⬇️";  break;
    case Qt::Key_Left:  keyName = "LEFT ⬅️";  break;
    case Qt::Key_Right: keyName = "RIGHT ➡️"; break;
    default: return;
    }
    
    qDebug() << "GameController detected:" << keyName;
    // Call your game logic functions here (e.g., move_player(key);)
}
