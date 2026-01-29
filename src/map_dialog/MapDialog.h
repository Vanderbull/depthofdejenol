#ifndef MAPDIALOG_H
#define MAPDIALOG_H

#include <QDialog>
#include "Map.h"
#include "MapWidget.h"

class MapDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MapDialog(QWidget *parent = nullptr);
    ~MapDialog();

private:
    Map m_gameMap;
    MapWidget *m_mapWidget;
};

#endif // MAPDIALOG_H
