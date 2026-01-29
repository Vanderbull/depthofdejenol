#include "CompassWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>

CompassWidget::CompassWidget(QWidget *parent)
    : QWidget(parent)
{
    // Make sure the widget can receive keyboard focus
    setFocusPolicy(Qt::StrongFocus);
}

CompassWidget::~CompassWidget() {}

void CompassWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // Get the center of the widget
    qreal side = qMin(width(), height());
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);
    // Draw the compass face
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    painter.drawEllipse(-90, -90, 180, 180);
    // Draw the "N" for North
    painter.setFont(QFont("Arial", 15));
    painter.drawText(-10, -80, "N");
    // Rotate and draw the needle
    painter.rotate(-m_heading);
    painter.setPen(Qt::red);
    painter.setBrush(Qt::red);
    QPointF points[3] = {
        QPointF(0, -80),
        QPointF(-10, 20),
        QPointF(10, 20)
    };
    painter.drawConvexPolygon(points, 3);
}

void CompassWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        // North
        m_heading = 0.0;
        break;
    case Qt::Key_Right:
        // East
        m_heading = 90.0;
        break;
    case Qt::Key_Down:
        // South
        m_heading = 180.0;
        break;
    case Qt::Key_Left:
        // West
        m_heading = 270.0;
        break;
    default:
        // Pass other key events to the base class
        QWidget::keyPressEvent(event);
        return;
    }
    update(); // Schedule a repaint
}
