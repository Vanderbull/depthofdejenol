#include "AutoMapDialog.h"
#include "src/dungeon_dialog/DungeonDialog.h"
#include "src/core/GameSettings.h"
#include <QVBoxLayout>

AutoMapDialog::AutoMapDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Dungeon AutoMap"));
    resize(760, 700);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Reveal-all: lifts fog of war in this view only. The choice is remembered in
    // GameSettings so reopening the map restores it.
    m_revealAllCheck = new QCheckBox(tr("Reveal Entire Map"), this);
    m_revealAllCheck->setChecked(GameSettings::instance().revealAutomap());
    mainLayout->addWidget(m_revealAllCheck);

    m_view = new QGraphicsView(this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setBackgroundRole(QPalette::Dark);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setMinimumSize(400, 400);
    mainLayout->addWidget(m_view);

    connect(m_revealAllCheck, &QCheckBox::toggled, this, [this](bool on) {
        GameSettings::instance().setRevealAutomap(on);
        emit revealAllChanged(on);
    });
}

void AutoMapDialog::refreshFrom(DungeonDialog *source)
{
    if (!source) return;

    // setScene() does not delete the scene it replaces, and the old ones are
    // parented to this dialog, so drop them or every refresh leaks a floor.
    delete m_view->scene();

    auto *scene = new QGraphicsScene(this);
    source->drawMapTo(scene, AUTOMAP_TILE_SIZE, m_revealAllCheck->isChecked());
    m_view->setScene(scene);
    m_view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

bool AutoMapDialog::isRevealAll() const
{
    return m_revealAllCheck->isChecked();
}

void AutoMapDialog::keyPressEvent(QKeyEvent *event)
{
    // Esc closes through the QDialog default; nothing else is intercepted here.
    QDialog::keyPressEvent(event);
}
