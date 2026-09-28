#include "mapeditorqt.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QSize>

/* ============================================================
   MapCanvas implementation
   ============================================================ */

MapCanvas::MapCanvas(QWidget* parent)
    : QWidget(parent)
{
    m_tileColors[MapEditor::EMPTY] = QColor(30, 30, 35);
    m_tileColors[MapEditor::WALL]  = QColor(120, 120, 120);
    m_tileColors[MapEditor::FLOOR] = QColor(70, 65, 55);
    m_tileColors[MapEditor::WATER] = QColor(40, 90, 130);
    m_wallColor = QColor(150, 150, 150);
    setMouseTracking(true);
    setMinimumSize(400, 300);
}

void MapCanvas::setEditor(MapEditor* editor)
{
    m_editor = editor;
    update();
}

void MapCanvas::setSelectedTile(MapEditor::TileType type)
{
    m_selectedTile = type;
    update();
}

void MapCanvas::setPaintWalls(bool walls)
{
    m_paintWalls = walls;
}

void MapCanvas::setZoom(int zoom)
{
    m_zoom = qMax(1, qMin(8, zoom));
    update();
}

void MapCanvas::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.fillRect(rect(), QColor(18, 18, 22));

    if (!m_editor) return;

    int w  = m_editor->getWidth();
    int h  = m_editor->getHeight();
    int ts = 20 * m_zoom;
    int totalW = w * ts;
    int totalH = h * ts;
    int ox = (width()  - totalW) / 2;
    int oy = (height() - totalH) / 2;

    if (totalW > width() || totalH > height()) {
        ox = qMax(ox, 0);
        oy = qMax(oy, 0);
    }

    const auto& baseMap  = m_editor->getBaseMap();
    const auto& wallMap  = m_editor->getWallMap();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int px = ox + x * ts;
            int py = oy + y * ts;
            if (px + ts < 0 || py + ts < 0 || px > width() || py > height()) continue;

            if (wallMap[y][x]) {
                p.fillRect(px, py, ts, ts, m_wallColor);
                p.setPen(QColor(90, 90, 90));
                p.drawRect(px + 0.5, py + 0.5, ts - 1, ts - 1);
            } else {
                p.fillRect(px, py, ts, ts, m_tileColors[baseMap[y][x]]);
                p.setPen(QColor(50, 50, 50));
                p.drawRect(px + 0.5, py + 0.5, ts - 1, ts - 1);
            }
        }
    }

    if (m_zoom >= 3) {
        p.setPen(QColor(60, 60, 60));
        for (int y = 0; y <= h; ++y) {
            int py = oy + y * ts;
            p.drawLine(ox, py, ox + totalW, py);
        }
        for (int x = 0; x <= w; ++x) {
            int px = ox + x * ts;
            p.drawLine(px, oy, px, oy + totalH);
        }
    }

    QBrush selBrush(m_tileColors[m_selectedTile]);
    p.setPen(QColor(200, 200, 200));
    p.drawRect(width() - 170, 8, 160, 22);
    p.fillRect(width() - 168, 10, 20, 18, selBrush);
    p.drawText(QRect(width() - 144, 10, 136, 20),
               Qt::AlignLeft | Qt::AlignVCenter,
               QString("Selected: %1").arg(
                   m_selectedTile == MapEditor::EMPTY  ? "Empty" :
                   m_selectedTile == MapEditor::WALL   ? "Wall"  :
                   m_selectedTile == MapEditor::FLOOR  ? "Floor" : "Water"));
}

void MapCanvas::mousePressEvent(QMouseEvent* event)
{
    if (!m_editor || event->button() != Qt::LeftButton) return;
    m_mouseDown = true;
    paintTileAt(event->pos());
}

void MapCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_editor || !m_mouseDown) return;
    paintTileAt(event->pos());
}

void MapCanvas::mouseReleaseEvent(QMouseEvent*)
{
    m_mouseDown = false;
    emit mapModified();
}

void MapCanvas::paintTileAt(const QPoint& pos)
{
    if (!m_editor) return;
    int w = m_editor->getWidth();
    int h = m_editor->getHeight();
    int ts = 20 * m_zoom;
    int totalW = w * ts;
    int totalH = h * ts;
    int ox = (width() - totalW) / 2;
    int oy = (height() - totalH) / 2;

    int gx = (pos.x() - ox) / ts;
    int gy = (pos.y() - oy) / ts;

    if (gx >= 0 && gx < w && gy >= 0 && gy < h) {
        if (m_paintWalls) {
            bool cur = m_editor->getWallTile(gx, gy);
            m_editor->setWallTile(gx, gy, !cur);
        } else {
            m_editor->setBaseTile(gx, gy, m_selectedTile);
        }
        update();
    }
}

/* ============================================================
   MapEditorApp implementation
   ============================================================ */

MapEditorApp::MapEditorApp(QWidget* parent)
    : QMainWindow(parent)
    , m_editor(new MapEditor(30, 30))
{
    setWindowTitle("Depth of Dejenol — Map Editor");
    resize(900, 620);

    setStyleSheet(
        "QMainWindow { background: #1e1e1e; }"
        "QGroupBox { background: #252526; border: 1px solid #3a3a3c; "
        "border-radius: 5px; margin-top: 8px; font-weight: bold; color: #d4d4d4; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; color: #569cd6; }"
        "QLabel { color: #d4d4d4; font-size: 12px; }"
        "QSpinBox { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; "
        "border-radius: 3px; padding: 3px; font-size: 12px; }"
        "QPushButton { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; "
        "border-radius: 3px; padding: 6px 12px; font-size: 12px; }"
        "QPushButton:hover { background: #4a4a4a; }"
        "QPushButton#activeBtn { background: #569cd6; color: white; border: 2px solid #7bc0e6; }"
        "QMenuBar { background: #252526; }"
        "QMenuBar::item { background: transparent; color: #d4d4d4; padding: 4px 8px; }"
        "QMenuBar::item:selected { background: #0e639c; }"
        "QMenu { background: #252526; color: #d4d4d4; border: 1px solid #3a3a3c; }"
        "QMenu::item { padding: 4px 16px; color: #d4d4d4; }"
        "QMenu::item:selected { background: #0e639c; color: white; }"
        "QStatusBar { background: #252526; color: #d4d4d4; }"
    );

    /* --- Menu --- */
    QMenuBar* menubar = menuBar();
    QMenu* fileMenu = menubar->addMenu("&File");
    fileMenu->addAction("New Map", this, &MapEditorApp::newMap);
    fileMenu->addAction("Load...", this, &MapEditorApp::loadMap);
    fileMenu->addAction("Save...", this, &MapEditorApp::saveMap);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QMainWindow::close);

    QMenu* helpMenu = menubar->addMenu("&Help");
    helpMenu->addAction("About", [this]() {
        QMessageBox::information(this, "Map Editor",
            "Depth of Dejenol Map Editor\n\n"
            "Click tiles to paint. Toggle Walls mode to place/remove walls.\n"
            "Save maps as JSON for use in the game.");
    });

    /* --- Toolbar --- */
    QToolBar* tb = addToolBar("Tools");
    tb->setMovable(false);
    tb->setIconSize(QSize(24, 24));

    QAction* newA = tb->addAction("New");
    connect(newA, &QAction::triggered, this, &MapEditorApp::newMap);
    tb->addSeparator();

    QString tileNames[] = {"Empty", "Wall", "Floor", "Water"};
    for (int i = 0; i < 4; ++i) {
        QPushButton* btn = new QPushButton(tileNames[i]);
        btn->setCheckable(true);
        btn->setMinimumHeight(32);
        connect(btn, &QPushButton::toggled, this,
                [this, i, btn](bool on) {
                    if (!on) return;
                    for (auto* b : m_tileButtons) {
                        if (b != btn) { b->setChecked(false); b->setStyleSheet(""); }
                    }
                    btn->setStyleSheet(
                        "QPushButton#activeBtn { background: #569cd6; color: white; "
                        "border: 2px solid #7bc0e6; border-radius: 3px; "
                        "padding: 6px 12px; font-size: 12px; font-weight: bold; }");
                    m_canvas->setSelectedTile(static_cast<MapEditor::TileType>(i));
                });
        m_tileButtons.append(btn);
        tb->addWidget(btn);
    }
    m_tileButtons.at(2)->setChecked(true);
    m_tileButtons.at(2)->setStyleSheet(
        "QPushButton#activeBtn { background: #569cd6; color: white; "
        "border: 2px solid #7bc0e6; border-radius: 3px; "
        "padding: 6px 12px; font-size: 12px; font-weight: bold; }");
    m_canvas->setSelectedTile(MapEditor::FLOOR);

    tb->addSeparator();

    m_wallToggle = new QPushButton("Toggle Walls: OFF");
    m_wallToggle->setCheckable(true);
    m_wallToggle->setMinimumHeight(32);
    connect(m_wallToggle, &QPushButton::toggled, this, [this](bool on) {
        m_canvas->setPaintWalls(on);
        m_wallToggle->setText(on ? "Toggle Walls: ON" : "Toggle Walls: OFF");
        m_wallToggle->setStyleSheet(on
            ? "QPushButton { background: #c586c0; color: white; border: 1px solid #d5a0d0; "
              "border-radius: 3px; padding: 6px 12px; font-size: 12px; font-weight: bold; }"
            : "QPushButton { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; "
              "border-radius: 3px; padding: 6px 12px; font-size: 12px; }");
    });
    tb->addWidget(m_wallToggle);
    tb->addSeparator();

    QPushButton* zoomInBtn  = new QPushButton("+ Zoom");
    zoomInBtn->setMinimumHeight(32);
    connect(zoomInBtn, &QPushButton::clicked, this, &MapEditorApp::zoomIn);
    tb->addWidget(zoomInBtn);

    QPushButton* zoomOutBtn = new QPushButton("- Zoom");
    zoomOutBtn->setMinimumHeight(32);
    connect(zoomOutBtn, &QPushButton::clicked, this, &MapEditorApp::zoomOut);
    tb->addWidget(zoomOutBtn);
    tb->addSeparator();

    QPushButton* loadBtn = new QPushButton("Load");
    loadBtn->setMinimumHeight(32);
    connect(loadBtn, &QPushButton::clicked, this, &MapEditorApp::loadMap);
    tb->addWidget(loadBtn);

    QPushButton* saveBtn = new QPushButton("Save");
    saveBtn->setMinimumHeight(32);
    connect(saveBtn, &QPushButton::clicked, this, &MapEditorApp::saveMap);
    tb->addWidget(saveBtn);

    /* --- Central area --- */
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout* cl = new QHBoxLayout(central);

    m_canvas = new MapCanvas(this);
    m_canvas->setEditor(m_editor);
    cl->addWidget(m_canvas, 1);

    QWidget* panel = new QWidget();
    panel->setFixedWidth(200);
    QVBoxLayout* pl = new QVBoxLayout(panel);
    pl->setSpacing(8);

    QGroupBox* sizeGB = new QGroupBox("Map Size");
    QGridLayout* sl = new QGridLayout(sizeGB);
    sl->addWidget(new QLabel("Width:"), 0, 0);
    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(5, 200);
    m_widthSpin->setValue(30);
    sl->addWidget(m_widthSpin, 0, 1);
    sl->addWidget(new QLabel("Height:"), 1, 0);
    QSpinBox* heightSpin = new QSpinBox();
    heightSpin->setRange(5, 200);
    heightSpin->setValue(30);
    sl->addWidget(heightSpin, 1, 1);
    connect(m_widthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &MapEditorApp::newMap);
    connect(heightSpin,  QOverload<int>::of(&QSpinBox::valueChanged), this, &MapEditorApp::newMap);
    pl->addWidget(sizeGB);

    QGroupBox* legendGB = new QGroupBox("Legend");
    QVBoxLayout* ll = new QVBoxLayout(legendGB);
    struct { const char* name; QColor c; } leg[] = {
        {"Empty",  QColor(30, 30, 35)},
        {"Wall",   QColor(150, 150, 150)},
        {"Floor",  QColor(70, 65, 55)},
        {"Water",  QColor(40, 90, 130)},
    };
    for (auto& l : leg) {
        QHBoxLayout* row = new QHBoxLayout();
        QLabel* sw = new QLabel("");
        sw->setFixedSize(22, 16);
        sw->setStyleSheet(QString("background: %1; border: 1px solid #555;").arg(l.c.name()));
        QLabel* txt = new QLabel(l.name);
        txt->setStyleSheet("color: #d4d4d4; font-size: 12px;");
        row->addWidget(sw);
        row->addWidget(txt);
        row->addStretch();
        ll->addLayout(row);
    }
    pl->addWidget(legendGB);

    QGroupBox* helpGB = new QGroupBox("Controls");
    QVBoxLayout* hl = new QVBoxLayout(helpGB);
    hl->addWidget(new QLabel("Left-click: Paint tile"));
    hl->addWidget(new QLabel("Toggle Walls: Add/remove walls"));
    hl->addWidget(new QLabel("File > Save: Export JSON"));
    hl->addStretch();
    pl->addWidget(helpGB);

    cl->addWidget(panel);

    m_statusLabel = new QLabel("Ready");
    statusBar()->addPermanentWidget(m_statusLabel);
    updateStatus("Ready — 30×30 map");
}

void MapEditorApp::newMap()
{
    int w = m_widthSpin->value();
    QWidget* central = this->centralWidget();
    auto allSpins = central->findChildren<QSpinBox*>();
    if (allSpins.size() < 2) return;
    QSpinBox* hSpin = allSpins.at(1);
    int h = hSpin->value();
    delete m_editor;
    m_editor = new MapEditor(w, h);
    m_canvas->setEditor(m_editor);
    updateStatus(QString("New %1×%2 map").arg(w).arg(h));
}

void MapEditorApp::loadMap()
{
    QString fn = QFileDialog::getOpenFileName(this, "Load Map", "", "Map JSON (*.json)");
    if (fn.isEmpty()) return;
    if (m_editor->loadMap(fn.toStdString())) {
        m_canvas->setEditor(m_editor);
        updateStatus(tr("Loaded: %1").arg(fn));
    } else {
        QMessageBox::warning(this, "Error", "Failed to load map.");
    }
}

void MapEditorApp::saveMap()
{
    QString fn = QFileDialog::getSaveFileName(this, "Save Map", "", "Map JSON (*.json)");
    if (fn.isEmpty()) return;
    if (m_editor->saveMap(fn.toStdString())) {
        updateStatus(tr("Saved: %1").arg(fn));
    } else {
        QMessageBox::warning(this, "Error", "Failed to save map.");
    }
}

void MapEditorApp::zoomIn()
{
    int z = m_canvas->property("zoom").toInt() + 1;
    m_canvas->setZoom(z);
    m_canvas->setProperty("zoom", z);
    updateStatus(QString("Zoom: %1×").arg(z));
}

void MapEditorApp::zoomOut()
{
    int z = m_canvas->property("zoom").toInt();
    if (z > 1) {
        --z;
        m_canvas->setZoom(z);
        m_canvas->setProperty("zoom", z);
        updateStatus(QString("Zoom: %1×").arg(z));
    }
}

void MapEditorApp::setTile(int typeIndex)
{
    for (auto* b : m_tileButtons)
        b->setChecked(b == m_tileButtons.at(typeIndex));
    m_canvas->setSelectedTile(static_cast<MapEditor::TileType>(typeIndex));
}

void MapEditorApp::updateStatus(const QString& msg)
{
    m_statusLabel->setText(msg);
}

#include "moc_mapeditorqt.cpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MapEditorApp w;
    w.show();
    return app.exec();
}
