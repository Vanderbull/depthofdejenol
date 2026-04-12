#ifndef COMPASSWIDGET_H
#define COMPASSWIDGET_H

#include <QWidget>
#include "GameStateManager.h"

class CompassWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CompassWidget(QWidget *parent = nullptr);
    ~CompassWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    qreal m_heading = 0.0;
};

#endif // COMPASSWIDGET_H
