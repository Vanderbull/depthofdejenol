#ifndef MINIMAPDIALOG_H
#define MINIMAPDIALOG_H

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QKeyEvent>

class MinimapDialog : public QDialog {
    Q_OBJECT
public:
    explicit MinimapDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Dungeon Automap");
        resize(450, 500);
        
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Add the Debug Toggle
        m_revealAllCheck = new QCheckBox("Reveal Entire Map (Debug)", this);
        layout->addWidget(m_revealAllCheck);

        m_view = new QGraphicsView(this);
        m_view->setRenderHint(QPainter::Antialiasing);
        m_view->setBackgroundRole(QPalette::Dark);
        layout->addWidget(m_view);

        // When checkbox is clicked, tell the parent to redraw the map
        connect(m_revealAllCheck, &QCheckBox::toggled, [this](){
            emit requestMapUpdate();
        });
    }

    bool isRevealAllEnabled() const { return m_revealAllCheck->isChecked(); }

    void updateScene(QGraphicsScene *newScene) {
        m_view->setScene(newScene);
        m_view->fitInView(newScene->sceneRect(), Qt::KeepAspectRatio);
    }

signals:
    void requestMapUpdate(); // Signal to tell DungeonDialog to redraw

private:
    QGraphicsView *m_view;
    QCheckBox *m_revealAllCheck;

protected:
    void keyPressEvent(QKeyEvent *event) override {
            if (parent()) {
                // Correct way to forward events to another object
                QCoreApplication::sendEvent(parent(), event);
            } else {
                QDialog::keyPressEvent(event);
            }
        }
};

#endif
