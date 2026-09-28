#ifndef MAP_EDITOR_QT_H
#define MAP_EDITOR_QT_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QFrame>
#include <QPainter>
#include <QMouseEvent>
#include <QList>
#include <QColor>

#include "../../map_editor/mapeditor.h"

class MapCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit MapCanvas(QWidget* parent = nullptr);

    void setEditor(MapEditor* editor);
    void setSelectedTile(MapEditor::TileType type);
    void setPaintWalls(bool walls);
    void setZoom(int zoom);

signals:
    void mapModified();

private:
    MapEditor* m_editor = nullptr;
    MapEditor::TileType m_selectedTile = MapEditor::FLOOR;
    bool m_paintWalls = false;
    int m_zoom = 1;
    QColor m_tileColors[4];
    QColor m_wallColor;
    bool m_mouseDown = false;

    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintTileAt(const QPoint& pos);
};

class MapEditorApp : public QMainWindow
{
    Q_OBJECT
public:
    explicit MapEditorApp(QWidget* parent = nullptr);

private slots:
    void newMap();
    void loadMap();
    void saveMap();
    void zoomIn();
    void zoomOut();
    void setTile(int typeIndex);

private:
    MapEditor* m_editor;
    MapCanvas* m_canvas;
    QSpinBox* m_widthSpin;
    QSpinBox* m_heightSpin;
    QLabel* m_statusLabel;
    QList<QPushButton*> m_tileButtons;
    QPushButton* m_wallToggle;

    void updateStatus(const QString& msg);
};

#endif
