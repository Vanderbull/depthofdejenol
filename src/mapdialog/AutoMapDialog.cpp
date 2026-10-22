#include "AutoMapDialog.h"
#include "src/core/GameSettings.h"

AutoMapDialog::AutoMapDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Dungeon AutoMap");
    resize(600, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Toggle reveal-all checkbox
    m_revealAllCheck = new QCheckBox("Reveal Entire Map", this);
    m_revealAllCheck->setChecked(GameSettings::instance().revealAutomap());
    mainLayout->addWidget(m_revealAllCheck);

    // Map view
    m_view = new QGraphicsView(this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setBackgroundRole(QPalette::Dark);
    mainLayout->addWidget(m_view);

    // Connect checkbox to parent's map update
    connect(m_revealAllCheck, &QCheckBox::toggled, [this](bool on) {
        GameSettings::instance().setRevealAutomap(on);
        emit requestMapUpdate();
    });
}

void AutoMapDialog::setTiles(const QSet<QPair<int, int>> &tiles)
{
    m_tiles = tiles;
    m_view->update();
}

void AutoMapDialog::updateScene(QGraphicsScene *newScene)
{
    m_view->setScene(newScene);
    m_view->fitInView(newScene->sceneRect(), Qt::KeepAspectRatio);
}

void AutoMapDialog::drawMap(QGraphicsScene *scene, int levelWidth, int levelHeight)
{
    // Clear existing items (keep the grid)
    QList<QGraphicsItem *> items = scene->items();
    for (QGraphicsItem *item : items) {
        if (item->data(0) != "grid") {
            delete item;
        }
    }

    // Draw revealed tiles
    for (const QPair<int, int> &tile : m_tiles) {
        int x = tile.first;
        int y = tile.second;

        // Skip negative coordinates (outside map bounds)
        if (x < 0 || y < 0) continue;

        // Draw a simple rectangle for the revealed tile
        QRectF rect(x * 20, y * 20, 20, 20);
        QGraphicsRectItem *rectItem = new QGraphicsRectItem(rect);
        rectItem->setBrush(QBrush(Qt::lightGray));
        rectItem->setPen(QPen(Qt::black, 1));
        rectItem->setData(0, "grid");
        scene->addItem(rectItem);
    }
}

void AutoMapDialog::keyPressEvent(QKeyEvent *event)
{
    if (parent()) {
        QCoreApplication::sendEvent(parent(), event);
    } else {
        QDialog::keyPressEvent(event);
    }
}

