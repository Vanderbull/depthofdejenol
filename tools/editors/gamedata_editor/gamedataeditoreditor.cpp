#include "gamedataeditoreditor.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>
#include <QJsonValue>
GameDataEditor::GameDataEditor(QWidget* parent)
    : QMainWindow(parent)
    , m_tabWidget(new QTabWidget(this))
{
    setWindowTitle("Depth of Dejenol — Game Data Editor");
    resize(960, 640);

    setStyleSheet(
        "QMainWindow { background: #1e1e1e; }"
        "QTabWidget::pane { background: #252526; border: 1px solid #3a3a3c; "
        "border-radius: 5px; }"
        "QTabBar::tab { background: #3a3a3c; color: #d4d4d4; "
        "padding: 6px 16px; border: 1px solid #3a3a3c; border-bottom: none; "
        "border-top-left-radius: 5px; border-top-right-radius: 5px; font-size: 12px; }"
        "QTabBar::tab:selected { background: #569cd6; color: white; }"
        "QTabBar::tab:!selected:hover { background: #4a4a4a; }"
        "QGroupBox { background: #252526; border: 1px solid #3a3a3c; "
        "border-radius: 5px; margin-top: 8px; font-weight: bold; color: #d4d4d4; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; color: #569cd6; }"
        "QLabel { color: #d4d4d4; font-size: 12px; }"
        "QTableWidget { background: #252526; color: #d4d4d4; gridline-color: #3a3a3c; "
        "font-size: 12px; }"
        "QTableWidget::item { padding: 2px 4px; }"
        "QTableWidget::item:selected { background: #007acc; color: white; }"
        "QHeaderView::section { background: #3a3a3c; color: #d4d4d4; "
        "border: none; padding: 3px; font-weight: bold; font-size: 11px; }"
        "QTextBrowser { background: #1e1e1e; color: #d4d4d4; "
        "font-family: Consolas, monospace; font-size: 11px; border: 1px solid #3a3a3c; }"
        "QPushButton { background: #0e639c; color: white; border: none; "
        "padding: 6px 14px; border-radius: 3px; font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background: #1177bb; }"
        "QPushButton#secondary { background: #4a4a4a; }"
        "QPushButton#secondary:hover { background: #5a5a5a; }"
        "QLineEdit { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; "
        "border-radius: 3px; padding: 3px; font-size: 12px; }"
        "QStatusBar { background: #252526; color: #d4d4d4; }"
    );

    // --- Menu ---
    QMenuBar* menubar = menuBar();
    QMenu* fileMenu = menubar->addMenu("&File");
    fileMenu->addAction("Load MDATA1.js...", this, &GameDataEditor::loadFile);
    fileMenu->addAction("Save As...", this, &GameDataEditor::saveFile);
    fileMenu->addSeparator();
    fileMenu->addAction("Load Example (MDATA1.js)", this, &GameDataEditor::loadExample);
    fileMenu->addAction("Exit", this, &QMainWindow::close);

    QMenu* helpMenu = menubar->addMenu("&Help");
    helpMenu->addAction("About", [this]() {
        QMessageBox::information(this, "Game Data Editor",
            "Edits game metadata: Races, Guilds, Item Types, Monster Types.\n"
            "Load MDATA1.js to edit. Changes are exported as JSON.");
    });

    // --- Central: tab widget + toolbar ---
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    QToolBar* toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    mainLayout->addWidget(toolbar);

    QPushButton* loadBtn = new QPushButton("Load File");
    connect(loadBtn, &QPushButton::clicked, this, &GameDataEditor::loadFile);
    toolbar->addWidget(loadBtn);

    QPushButton* saveBtn = new QPushButton("Save As");
    connect(saveBtn, &QPushButton::clicked, this, &GameDataEditor::saveFile);
    toolbar->addWidget(saveBtn);

    QPushButton* exampleBtn = new QPushButton("Load Example");
    exampleBtn->setObjectName("secondary");
    connect(exampleBtn, &QPushButton::clicked, this, &GameDataEditor::loadExample);
    toolbar->addWidget(exampleBtn);

    toolbar->addSeparator();

    QPushButton* refreshBtn = new QPushButton("Refresh Tables");
    connect(refreshBtn, &QPushButton::clicked, this, &GameDataEditor::refreshAllTabs);
    toolbar->addWidget(refreshBtn);

    mainLayout->addWidget(m_tabWidget, 1);

    // --- Create tabs ---
    createRaceTab();
    createGuildTab();
    createItemTypeTab();
    createMonsterTypeTab();

    // --- Status bar ---
    statusBar()->addPermanentWidget(new QLabel("No file loaded"));
    statusBar()->setStyleSheet("QStatusBar { background: #252526; color: #d4d4d4; }");

    // Load example by default
    loadExample();
}

void GameDataEditor::setupUi()
{
    // Already done in constructor
}

void GameDataEditor::createRaceTab()
{
    TabData tab;
    tab.name = "Races (9)";

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    tab.table = new QTableWidget();
    tab.preview = new QTextBrowser();

    // Race table: name, then 7 stat mins, 7 stat maxes, 12 resistances,
    // alignment, size, bonusPoints, maxAge, expFactor
    tab.table->setColumnCount(30);
    tab.table->setHorizontalHeaderLabels({
        "Name", "StrMin","IntMin","WisMin","ConMin","ChaMin","DexMin","U7Min",
        "StrMax","IntMax","WisMax","ConMax","ChaMax","DexMax","U7Max",
        "ResFire","ResCold","ResElectric","ResMind","ResPoison","ResDisease",
        "ResMagic","ResPhysical","ResWeapon","ResSpell","ResSpecial",
        "Alignment","Size","BonusPts","MaxAge","ExpFactor"
    });
    tab.table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    tab.table->setSelectionBehavior(QAbstractItemView::SelectRows);
    tab.table->setAlternatingRowColors(true);
    tab.table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tab.table->setSortingEnabled(true);

    splitter->addWidget(tab.table);
    splitter->addWidget(tab.preview);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    tab.table->setContextMenuPolicy(Qt::CustomContextMenu);

    m_tabWidget->addTab(splitter, tab.name);
    m_tabs << tab;
}

void GameDataEditor::createGuildTab()
{
    TabData tab;
    tab.name = "Guilds (9)";

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    tab.table = new QTableWidget();
    tab.preview = new QTextBrowser();

    // Simplified guild columns for editable view
    tab.table->setColumnCount(14);
    tab.table->setHorizontalHeaderLabels({
        "Name", "AvgHits", "MaxLevel", "MH", "ExpFactor",
        "ReqStr","ReqInt","ReqWis","ReqCon","ReqCha","ReqDex","ReqU7",
        "Alignment", "RaceMask"
    });
    tab.table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    tab.table->setSelectionBehavior(QAbstractItemView::SelectRows);
    tab.table->setAlternatingRowColors(true);
    tab.table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tab.table->setSortingEnabled(true);

    splitter->addWidget(tab.table);
    splitter->addWidget(tab.preview);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    m_tabWidget->addTab(splitter, tab.name);
    m_tabs << tab;
}

void GameDataEditor::createItemTypeTab()
{
    TabData tab;
    tab.name = "Item Types (37)";

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    tab.table = new QTableWidget();
    tab.preview = new QTextBrowser();

    tab.table->setColumnCount(2);
    tab.table->setHorizontalHeaderLabels({"Name", "IsEquipable"});
    tab.table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    tab.table->setSelectionBehavior(QAbstractItemView::SelectRows);
    tab.table->setAlternatingRowColors(true);
    tab.table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tab.table->setSortingEnabled(true);

    splitter->addWidget(tab.table);
    splitter->addWidget(tab.preview);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    m_tabWidget->addTab(splitter, tab.name);
    m_tabs << tab;
}

void GameDataEditor::createMonsterTypeTab()
{
    TabData tab;
    tab.name = "Monster Types (2)";

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    tab.table = new QTableWidget();
    tab.preview = new QTextBrowser();

    tab.table->setColumnCount(2);
    tab.table->setHorizontalHeaderLabels({"Name", "NotUsed"});
    tab.table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    tab.table->setSelectionBehavior(QAbstractItemView::SelectRows);
    tab.table->setAlternatingRowColors(true);
    tab.table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tab.table->setSortingEnabled(true);

    splitter->addWidget(tab.table);
    splitter->addWidget(tab.preview);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    m_tabWidget->addTab(splitter, tab.name);
    m_tabs << tab;
}

void GameDataEditor::updateRaceTable(const QJsonArray& races)
{
    TabData& tab = m_tabs[0];
    tab.table->setRowCount(races.size());

    for (int i = 0; i < races.size(); ++i) {
        QJsonObject r = races[i].toObject();
        tab.table->setItem(i, 0, new QTableWidgetItem(r["name"].toString()));

        QJsonValue minVal = r["minStats"];
        QJsonValue maxVal = r["maxStats"];
        QJsonValue resVal = r["resistances"];

        QJsonObject minStats = minVal.isObject() ? minVal.toObject() : QJsonObject();
        QJsonObject maxStats = maxVal.isObject() ? maxVal.toObject() : QJsonObject();
        QJsonObject res      = resVal.isObject() ? resVal.toObject() : QJsonObject();

        QString statKeys[] = {"Str","Int","Wis","Con","Cha","Dex","U7"};
        QString resKeys[] = {"Fire","Cold","Electric","Mind","Poison","Disease",
                             "Magic","Physical","Weapon","Spell","Special","U12"};

        for (int s = 0; s < 7; ++s)
            tab.table->setItem(i, 1 + s, new QTableWidgetItem(QString::number(minStats[statKeys[s]].toInt())));
        for (int s = 0; s < 7; ++s)
            tab.table->setItem(i, 8 + s, new QTableWidgetItem(QString::number(maxStats[statKeys[s]].toInt())));
        for (int r2 = 0; r2 < 12; ++r2)
            tab.table->setItem(i, 15 + r2, new QTableWidgetItem(QString::number(res[resKeys[r2]].toInt())));

        tab.table->setItem(i, 27, new QTableWidgetItem(QString::number(r["alignment"].toInt())));
        tab.table->setItem(i, 28, new QTableWidgetItem(QString::number(r["size"].toInt())));
        tab.table->setItem(i, 29, new QTableWidgetItem(QString::number(r["bonusPoints"].toInt())));
        tab.table->setItem(i, 30, new QTableWidgetItem(QString::number(r["maxAge"].toInt())));
        tab.table->setItem(i, 31, new QTableWidgetItem(QString::number(r["expFactor"].toDouble(), 'f', 2)));
    }
    tab.table->update();
}

void GameDataEditor::updateGuildTable(const QJsonArray& guilds)
{
    TabData& tab = m_tabs[1];
    tab.table->setRowCount(guilds.size());

    QString statKeys[] = {"Str","Int","Wis","Con","Cha","Dex","U7"};

    for (int i = 0; i < guilds.size(); ++i) {
        QJsonObject g = guilds[i].toObject();
        tab.table->setItem(i, 0, new QTableWidgetItem(g["name"].toString()));
        QJsonValue gAvg = g["averageHits"];
        QJsonValue gMax = g["MaxLevel"];
        QJsonValue gMH  = g["MH"];
        QJsonValue gExp = g["expFactor"];
        QJsonValue gReq = g["reqStats"];

        tab.table->setItem(i, 1, new QTableWidgetItem(QString::number(gAvg.toInt())));
        tab.table->setItem(i, 2, new QTableWidgetItem(QString::number(gMax.toInt())));
        tab.table->setItem(i, 3, new QTableWidgetItem(QString::number(gMH.toInt())));
        tab.table->setItem(i, 4, new QTableWidgetItem(QString::number(gExp.toDouble(), 'f', 2)));

        QJsonObject req = gReq.isObject() ? gReq.toObject() : QJsonObject();
        for (int s = 0; s < 7; ++s)
            tab.table->setItem(i, 5 + s, new QTableWidgetItem(QString::number(req[statKeys[s]].toInt())));

        tab.table->setItem(i, 12, new QTableWidgetItem(QString::number(g["alignment"].toInt())));
        tab.table->setItem(i, 13, new QTableWidgetItem(QString::number(g["RaceMask"].toVariant().toLongLong())));
    }
    tab.table->update();
}

void GameDataEditor::updateItemTypeTable(const QJsonArray& items)
{
    TabData& tab = m_tabs[2];
    tab.table->setRowCount(items.size());

    for (int i = 0; i < items.size(); ++i) {
        QJsonObject it = items[i].toObject();
        tab.table->setItem(i, 0, new QTableWidgetItem(it["name"].toString()));
        tab.table->setItem(i, 1, new QTableWidgetItem(QString::number(it["isEquipable"].toInt())));
    }
    tab.table->update();
}

void GameDataEditor::updateMonsterTypeTable(const QJsonArray& monsters)
{
    TabData& tab = m_tabs[3];
    tab.table->setRowCount(monsters.size());

    for (int i = 0; i < monsters.size(); ++i) {
        QJsonObject m = monsters[i].toObject();
        tab.table->setItem(i, 0, new QTableWidgetItem(m["name"].toString()));
        tab.table->setItem(i, 1, new QTableWidgetItem(QString::number(m["notUsed"].toInt())));
    }
    tab.table->update();
}

void GameDataEditor::updateAllPreviews()
{
    // Update previews for each tab
    for (int t = 0; t < m_tabs.size(); ++t) {
        QJsonArray array;
        switch (t) {
            case 0: array = m_gameData["Races"].toArray(); break;
            case 1: array = m_gameData["Guilds"].toArray(); break;
            case 2: array = m_gameData["ItemTypes"].toArray(); break;
            case 3: array = m_gameData["MonsterTypes"].toArray(); break;
            default: continue;
        }
        QJsonDocument doc(array);
        m_tabs[t].preview->setPlainText(doc.toJson(QJsonDocument::Indented));
    }
}

void GameDataEditor::refreshAllTabs()
{
    if (m_gameData.isEmpty()) {
        QMessageBox::information(this, "Info", "No data loaded. Load a file first.");
        return;
    }

    updateRaceTable(m_gameData["Races"].toArray());
    updateGuildTable(m_gameData["Guilds"].toArray());
    updateItemTypeTable(m_gameData["ItemTypes"].toArray());
    updateMonsterTypeTable(m_gameData["MonsterTypes"].toArray());
    updateAllPreviews();

    statusBar()->showMessage(tr("Tables refreshed from %1").arg(m_currentFile), 3000);
}

void GameDataEditor::loadFile()
{
    QString fn = QFileDialog::getOpenFileName(this, "Load Game Data JSON/JS",
                                               "", "JSON / JS Files (*.json *.js)");
    if (fn.isEmpty()) return;
    m_gameData = readFromFile(fn);
    if (m_gameData.isEmpty()) return;
    m_currentFile = fn;
    refreshAllTabs();
    statusBar()->showMessage(tr("Loaded: %1").arg(fn), 3000);
}

void GameDataEditor::saveFile()
{
    if (m_gameData.isEmpty()) {
        QMessageBox::information(this, "Info", "No data to save. Load a file first.");
        return;
    }
    QString fn = QFileDialog::getSaveFileName(this, "Save Game Data",
                                              m_currentFile.isEmpty() ? "" : m_currentFile,
                                              "JSON Files (*.json)");
    if (fn.isEmpty()) return;

    // Collect edited data from tables
    // Races
    QJsonArray racesArr;
    TabData& raceTab = m_tabs[0];
    for (int i = 0; i < raceTab.table->rowCount(); ++i) {
        QJsonObject r;
        r["name"] = raceTab.table->item(i, 0)->text();

        QJsonObject minStats, maxStats, res;
        QString statKeys[] = {"Str","Int","Wis","Con","Cha","Dex","U7"};
        QString resKeys[] = {"Fire","Cold","Electric","Mind","Poison","Disease",
                             "Magic","Physical","Weapon","Spell","Special","U12"};

        for (int s = 0; s < 7; ++s)
            minStats[statKeys[s]] = raceTab.table->item(i, 1 + s)->text().toInt();
        for (int s = 0; s < 7; ++s)
            maxStats[statKeys[s]] = raceTab.table->item(i, 8 + s)->text().toInt();
        for (int r2 = 0; r2 < 12; ++r2)
            res[resKeys[r2]] = raceTab.table->item(i, 15 + r2)->text().toInt();

        r["minStats"] = minStats;
        r["maxStats"] = maxStats;
        r["resistances"] = res;
        r["alignment"] = raceTab.table->item(i, 27)->text().toInt();
        r["size"] = raceTab.table->item(i, 28)->text().toInt();
        r["bonusPoints"] = raceTab.table->item(i, 29)->text().toInt();
        r["maxAge"] = raceTab.table->item(i, 30)->text().toInt();
        r["expFactor"] = raceTab.table->item(i, 31)->text().toFloat();
        racesArr.append(r);
    }
    m_gameData["Races"] = racesArr;

    // Guilds
    QJsonArray guildsArr;
    TabData& guildTab = m_tabs[1];
    QString statKeys[] = {"Str","Int","Wis","Con","Cha","Dex","U7"};
    for (int i = 0; i < guildTab.table->rowCount(); ++i) {
        QJsonObject g;
        g["name"] = guildTab.table->item(i, 0)->text();
        g["averageHits"] = guildTab.table->item(i, 1)->text().toInt();
        g["MaxLevel"] = guildTab.table->item(i, 2)->text().toInt();
        g["MH"] = guildTab.table->item(i, 3)->text().toInt();
        g["expFactor"] = guildTab.table->item(i, 4)->text().toDouble();

        QJsonObject req;
        for (int s = 0; s < 7; ++s)
            req[statKeys[s]] = guildTab.table->item(i, 5 + s)->text().toInt();
        g["reqStats"] = req;
        g["alignment"] = guildTab.table->item(i, 12)->text().toInt();
        g["RaceMask"] = guildTab.table->item(i, 13)->text().toLongLong();
        guildsArr.append(g);
    }
    m_gameData["Guilds"] = guildsArr;

    // Item Types
    QJsonArray itemsArr;
    TabData& itemTab = m_tabs[2];
    for (int i = 0; i < itemTab.table->rowCount(); ++i) {
        QJsonObject it;
        it["name"] = itemTab.table->item(i, 0)->text();
        it["isEquipable"] = itemTab.table->item(i, 1)->text().toInt();
        itemsArr.append(it);
    }
    m_gameData["ItemTypes"] = itemsArr;

    // Monster Types
    QJsonArray monArr;
    TabData& monTab = m_tabs[3];
    for (int i = 0; i < monTab.table->rowCount(); ++i) {
        QJsonObject m;
        m["name"] = monTab.table->item(i, 0)->text();
        m["notUsed"] = monTab.table->item(i, 1)->text().toInt();
        monArr.append(m);
    }
    m_gameData["MonsterTypes"] = monArr;

    // Write
    QFile file(fn);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument doc(m_gameData);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        statusBar()->showMessage(tr("Saved: %1").arg(fn), 3000);
    } else {
        QMessageBox::warning(this, "Error", "Could not save file.");
    }
}

QJsonObject GameDataEditor::readFromFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot open file: " + path);
        return QJsonObject();
    }

    QByteArray data = file.readAll();
    file.close();

    // Handle JS format: "const gameData = {...};"
    QString text = QString::fromUtf8(data);
    if (text.startsWith("const") || text.startsWith("var") || text.startsWith("let")) {
        int start = text.indexOf('{');
        int end = text.lastIndexOf('}');
        if (start != -1 && end != -1) {
            text = text.mid(start, end - start + 1);
        }
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, "Parse Error",
                              QString("JSON parse error: %1 at offset %2")
                              .arg(err.errorString()).arg(err.offset));
        return QJsonObject();
    }

    return doc.object();
}

void GameDataEditor::loadExample()
{
    QString examplePath = "tools/gamedataconverter/data/MDATA1.js";
    QFile f(examplePath);
    if (f.exists()) {
        m_gameData = readFromFile(examplePath);
        m_currentFile = examplePath;
        refreshAllTabs();
        statusBar()->showMessage(tr("Loaded example: %1").arg(examplePath), 3000);
    } else {
        QMessageBox::warning(this, "Warning",
            "Example file not found at: " + examplePath +
            "\nPlace MDATA1.js there or load your own file.");
    }
}


