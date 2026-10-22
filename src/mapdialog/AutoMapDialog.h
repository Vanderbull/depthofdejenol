#ifndef AUTOMAPDIALOG_H
#define AUTOMAPDIALOG_H

#include <QDialog>
#include <QSet>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QKeyEvent>

class GameSettings;

class AutoMapDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AutoMapDialog(QWidget *parent = nullptr);

    void setTiles(const QSet<QPair<int, int>> &tiles);
    void updateScene(QGraphicsScene *newScene);
    void drawMap(QGraphicsScene *scene, int levelWidth, int levelHeight);

signals:
    void requestMapUpdate();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsView *m_view = nullptr;
    QCheckBox *m_revealAllCheck = nullptr;
    QSet<QPair<int, int>> m_tiles;
};

#endif // AUTOMAPDIALOG_H
