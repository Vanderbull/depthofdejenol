#ifndef MINIMAPDIALOG_H
#define MINIMAPDIALOG_H

#include <QCoreApplication>
#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QKeyEvent>

class MinimapDialog : public QDialog {
    Q_OBJECT
public:
    explicit MinimapDialog(QWidget *parent = nullptr) : QDialog(parent) 
    {
        setWindowTitle("Dungeon Automap");
        resize(450, 500);        
        QVBoxLayout *layout = new QVBoxLayout(this);
        
        // Add the Debug Toggle
        m_revealAllCheck = new QCheckBox("Reveal Entire Map (Debug)", this);
        m_revealAllCheck->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(m_revealAllCheck);
        
        m_view = new QGraphicsView(this);
        m_view->setRenderHint(QPainter::Antialiasing);
        m_view->setBackgroundRole(QPalette::Dark);
        m_view->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(m_view);
        
        // When checkbox is clicked, tell the parent to redraw the map
        connect(m_revealAllCheck, &QCheckBox::toggled, [this](){
            emit requestMapUpdate();
        });
    }

    bool isRevealAllEnabled() const 
    { 
        return m_revealAllCheck->isChecked();
    }

    void updateScene(QGraphicsScene *newScene) 
    {
        m_view->setScene(newScene);
        m_view->fitInView(newScene->sceneRect(), Qt::KeepAspectRatio);
    }

signals:
    void requestMapUpdate(); // Signal to tell DungeonDialog to redraw

private:
    QGraphicsView *m_view;
    QCheckBox *m_revealAllCheck;

protected:
    // We override event() to catch keys before they are handled by QDialog's 
    // internal focus navigation (which often consumes Arrow keys).
    bool event(QEvent *e) override 
    {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(e);
            switch (ke->key()) {
                case Qt::Key_Up:
                case Qt::Key_Down:
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_W:
                case Qt::Key_A:
                case Qt::Key_S:
                case Qt::Key_D:
                case Qt::Key_Q:
                case Qt::Key_E:
                case Qt::Key_Shift:
                case Qt::Key_Control:
                    if (parentWidget()) {
                        // Forward the key event to the parent (DungeonDialog)
                        QCoreApplication::sendEvent(parentWidget(), e);
                        return true; // Mark as handled locally
                    }
                    break;
                default:
                    break;
            }
        }
        return QDialog::event(e);
    }
};

#endif
