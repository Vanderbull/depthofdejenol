#ifndef AUTOMAPDIALOG_H
#define AUTOMAPDIALOG_H

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QCheckBox>
#include <QKeyEvent>

class DungeonDialog;

// Full-size dungeon map, opened from DungeonDialog with Ctrl+Shift+M.
//
// This dialog owns no tile data. It is fed by DungeonDialog::drawMapTo(), the
// same renderer the integrated minimap uses, so the two views cannot drift
// apart. "Reveal Entire Map" lifts fog of war for this view only — it does not
// change what the minimap or the game itself remembers as visited.
class AutoMapDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AutoMapDialog(QWidget *parent = nullptr);

    // Render the dungeon into this view. `source` supplies the tile data.
    void refreshFrom(DungeonDialog *source);

    // Whether fog of war is lifted in this view.
    bool isRevealAll() const;

signals:
    void revealAllChanged(bool on);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsView *m_view = nullptr;
    QCheckBox *m_revealAllCheck = nullptr;
    // Larger than the minimap's 10px so a 50x50 floor stays readable.
    static constexpr int AUTOMAP_TILE_SIZE = 12;
};

#endif // AUTOMAPDIALOG_H
