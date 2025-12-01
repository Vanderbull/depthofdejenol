#ifndef DUNGEONDIALOG_H
#define DUNGEONDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QImage>
// FIX: Corrected the include path to find the file in src/inventory_dialog/
#include "../inventory_dialog/inventorydialog.h"

namespace Ui {
class DungeonDialog;
}

class DungeonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DungeonDialog(QWidget *parent = nullptr);
    ~DungeonDialog();

    void updateDungeonView(const QImage& dungeonImage);
    void updateCompass(const QString& direction);
    void updateLocation(const QString& location);

signals:
    void teleporterUsed();
    void companionAttacked(int companionId);
    void companionCarried(int companionId);

private slots:
    void on_teleportButton_clicked();
    void on_attackCompanionButton_clicked();
    void on_carryCompanionButton_clicked();

    void on_mapButton_clicked();
    void on_pickupButton_clicked();
    void on_dropButton_clicked();
    void on_fightButton_clicked();
    void on_spellButton_clicked();
    void on_takeButton_clicked();
    void on_openButton_clicked();
    void on_exitButton_clicked();

private:
    Ui::DungeonDialog *ui;
    QGraphicsScene *m_dungeonScene;
};

#endif // DUNGEONDIALOG_H
