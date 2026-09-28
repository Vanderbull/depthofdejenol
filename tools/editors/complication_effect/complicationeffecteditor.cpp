#include "complicationeffecteditor.h"
#include "../../complication_effect/ComplicationEffect.h"

ComplicationEffectEditor::ComplicationEffectEditor(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Dejenol Complication Effect Editor");
    resize(700, 620);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    setStyleSheet(
        "QMainWindow { background: #1e1e1e; }"
        "QGroupBox { background: #252526; border: 1px solid #3a3a3c; "
        "border-radius: 6px; margin-top: 10px; font-weight: bold; color: #d4d4d4; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; color: #569cd6; }"
        "QLabel { color: #d4d4d4; font-size: 13px; }"
        "QSpinBox { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; "
        "border-radius: 3px; padding: 3px; font-size: 13px; width: 60px; }"
        "QTableWidget { background: #252526; color: #d4d4d4; gridline-color: #3a3a3c; "
        "font-size: 13px; }"
        "QTableWidget::item { padding: 2px 6px; }"
        "QTableWidget::item:selected { background: #007acc; color: white; }"
        "QHeaderView::section { background: #3a3a3c; color: #d4d4d4; "
        "border: none; padding: 3px; font-weight: bold; font-size: 12px; }"
        "QPushButton { background: #0e639c; color: white; border: none; "
        "padding: 7px 16px; border-radius: 3px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background: #1177bb; }"
        "QPushButton#secondary { background: #4a4a4a; }"
        "QPushButton#secondary:hover { background: #5a5a5a; }"
        "QLineEdit { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; "
        "border-radius: 3px; padding: 3px; font-size: 13px; }"
    );

    // --- Title ---
    QLabel* title = new QLabel("Complication Effect Simulator");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #569cd6;");
    mainLayout->addWidget(title);

    QLabel* desc = new QLabel("Applies complication effects: -5 to each stat (floor at racial minimum), "
                               "max hits × 0.8333, age +15 years.");
    desc->setStyleSheet("color: #888; font-size: 11px;");
    mainLayout->addWidget(desc);

    // --- Current Stats ---
    QGroupBox* currentGroup = new QGroupBox("Current Character Stats");
    QGridLayout* grid = new QGridLayout(currentGroup);
    grid->setSpacing(8);

    const char* statLabels[] = {"Str", "Dex", "Int", "Cha", "Con", "Wis"};
    m_strSpin = new QSpinBox();   m_strSpin->setRange(0, 200); m_strSpin->setValue(60);
    m_dexSpin = new QSpinBox();   m_dexSpin->setRange(0, 200); m_dexSpin->setValue(55);
    m_intSpin = new QSpinBox();   m_intSpin->setRange(0, 200); m_intSpin->setValue(40);
    m_chaSpin = new QSpinBox();   m_chaSpin->setRange(0, 200); m_chaSpin->setValue(35);
    m_conSpin = new QSpinBox();   m_conSpin->setRange(0, 200); m_conSpin->setValue(50);
    m_wisSpin = new QSpinBox();   m_wisSpin->setRange(0, 200); m_wisSpin->setValue(45);
    m_maxHitsSpin = new QSpinBox(); m_maxHitsSpin->setRange(0, 9999); m_maxHitsSpin->setValue(500);
    m_ageSpin = new QSpinBox();   m_ageSpin->setRange(0, 300); m_ageSpin->setValue(45);

    struct { QSpinBox* spin; int row; const char* label; } statMap[] = {
        {m_strSpin, 0, "Strength"}, {m_dexSpin, 0, "Dexterity"}, {m_intSpin, 0, "Intelligence"},
        {m_chaSpin, 0, "Charisma"}, {m_conSpin, 0, "Constitution"}, {m_wisSpin, 0, "Wisdom"},
        {nullptr, -1, nullptr}, {nullptr, -1, nullptr}
    };
    statMap[6].spin = m_maxHitsSpin; statMap[6].row = 1; statMap[6].label = "Max Hits";
    statMap[7].spin = m_ageSpin;    statMap[7].row = 1; statMap[7].label = "Age";

    int col = 0;
    for (int i = 0; i < 8; ++i) {
        if (statMap[i].label == nullptr) continue;
        QLabel* lbl = new QLabel(statMap[i].label);
        lbl->setStyleSheet("color: #9cdcfe; font-weight: bold;");
        grid->addWidget(lbl, statMap[i].row, col);
        grid->addWidget(statMap[i].spin, statMap[i].row, col + 1);
        col += 2;
        if (col > 6) { col = 0; }
    }

    mainLayout->addWidget(currentGroup);

    // --- Racial Minimums ---
    QGroupBox* minGroup = new QGroupBox("Racial Minimums (stat floors)");
    QGridLayout* minGrid = new QGridLayout(minGroup);
    minGrid->setSpacing(8);

    m_minStrSpin = new QSpinBox(); m_minStrSpin->setRange(0, 100); m_minStrSpin->setValue(15);
    m_minDexSpin = new QSpinBox(); m_minDexSpin->setRange(0, 100); m_minDexSpin->setValue(15);
    m_minIntSpin = new QSpinBox(); m_minIntSpin->setRange(0, 100); m_minIntSpin->setValue(15);
    m_minChaSpin = new QSpinBox(); m_minChaSpin->setRange(0, 100); m_minChaSpin->setValue(15);
    m_minConSpin = new QSpinBox(); m_minConSpin->setRange(0, 100); m_minConSpin->setValue(15);
    m_minWisSpin = new QSpinBox(); m_minWisSpin->setRange(0, 100); m_minWisSpin->setValue(15);

    minGrid->addWidget(new QLabel("Str:"), 0, 0);
    minGrid->addWidget(m_minStrSpin, 0, 1);
    minGrid->addWidget(new QLabel("Dex:"), 0, 2);
    minGrid->addWidget(m_minDexSpin, 0, 3);
    minGrid->addWidget(new QLabel("Int:"), 0, 4);
    minGrid->addWidget(m_minIntSpin, 0, 5);
    minGrid->addWidget(new QLabel("Cha:"), 1, 0);
    minGrid->addWidget(m_minChaSpin, 1, 1);
    minGrid->addWidget(new QLabel("Con:"), 1, 2);
    minGrid->addWidget(m_minConSpin, 1, 3);
    minGrid->addWidget(new QLabel("Wis:"), 1, 4);
    minGrid->addWidget(m_minWisSpin, 1, 5);

    mainLayout->addWidget(minGroup);

    // --- Buttons ---
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_applyButton = new QPushButton(" Apply Complication Effects ");
    m_applyButton->setMinimumHeight(36);
    m_exampleButton = new QPushButton("Load Example");
    m_exampleButton->setObjectName("secondary");
    m_resetButton = new QPushButton("Reset");
    m_resetButton->setObjectName("secondary");

    btnLayout->addWidget(m_applyButton);
    btnLayout->addWidget(m_exampleButton);
    btnLayout->addStretch();
    btnLayout->addWidget(m_resetButton);
    mainLayout->addLayout(btnLayout);

    connect(m_applyButton, &QPushButton::clicked, this, &ComplicationEffectEditor::applyEffects);
    connect(m_resetButton, &QPushButton::clicked, this, &ComplicationEffectEditor::resetStats);
    connect(m_exampleButton, &QPushButton::clicked, this, &ComplicationEffectEditor::loadExamples);

    // --- Output Table ---
    QGroupBox* outGroup = new QGroupBox("Effect Results");
    m_outputTable = new QTableWidget();
    m_outputTable->setColumnCount(3);
    m_outputTable->setHorizontalHeaderLabels({"Stat", "Before", "After"});
    m_outputTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_outputTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_outputTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_outputTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_outputTable->setSelectionMode(QAbstractItemView::NoSelection);

    mainLayout->addWidget(m_outputTable, 1);

    // Initial display
    applyEffects();
}

ComplicationEffectEditor::Stats ComplicationEffectEditor::readInputs() const
{
    return {
        m_strSpin->value(), m_dexSpin->value(), m_intSpin->value(),
        m_chaSpin->value(), m_conSpin->value(), m_wisSpin->value(),
        m_maxHitsSpin->value(), m_ageSpin->value()
    };
}

void ComplicationEffectEditor::setInputs(const Stats& s)
{
    m_strSpin->setValue(s.str);
    m_dexSpin->setValue(s.dex);
    m_intSpin->setValue(s.intl);
    m_chaSpin->setValue(s.cha);
    m_conSpin->setValue(s.con);
    m_wisSpin->setValue(s.wis);
    m_maxHitsSpin->setValue(s.maxHits);
    m_ageSpin->setValue(s.age);
}

void ComplicationEffectEditor::writeOutputs(const Stats& before, const Stats& after)
{
    // Update the spin boxes to reflect the new stats
    m_strSpin->setValue(after.str);
    m_dexSpin->setValue(after.dex);
    m_intSpin->setValue(after.intl);
    m_chaSpin->setValue(after.cha);
    m_conSpin->setValue(after.con);
    m_wisSpin->setValue(after.wis);
    m_maxHitsSpin->setValue(after.maxHits);
    m_ageSpin->setValue(after.age);
}

void ComplicationEffectEditor::updateResultDisplay(const Stats& before, const Stats& after)
{
    QString statNames[] = {"Strength", "Dexterity", "Intelligence", "Charisma",
                           "Constitution", "Wisdom", "Max Hits", "Age"};
    int beforeVals[] = {before.str, before.dex, before.intl, before.cha,
                        before.con, before.wis, before.maxHits, before.age};
    int afterVals[] = {after.str, after.dex, after.intl, after.cha,
                       after.con, after.wis, after.maxHits, after.age};

    m_outputTable->setRowCount(8);
    for (int i = 0; i < 8; ++i) {
        QTableWidgetItem* nameItem = new QTableWidgetItem(statNames[i]);
        nameItem->setForeground(QColor("#9cdcfe"));
        nameItem->setFont(QFont("Consolas", 10, QFont::Bold));

        QTableWidgetItem* beforeItem = new QTableWidgetItem(QString::number(beforeVals[i]));
        beforeItem->setForeground(QColor("#d4d4d4"));

        QTableWidgetItem* afterItem = new QTableWidgetItem(QString::number(afterVals[i]));
        QColor afterColor = (afterVals[i] < beforeVals[i]) ? QColor("#ff6b6b") : QColor("#4ec9b0");
        afterItem->setForeground(afterColor);
        afterItem->setFont(QFont("Consolas", 10, QFont::Bold));

        m_outputTable->setItem(i, 0, nameItem);
        m_outputTable->setItem(i, 1, beforeItem);
        m_outputTable->setItem(i, 2, afterItem);
    }
}

void ComplicationEffectEditor::applyEffects()
{
    Stats before = readInputs();

    // Build racial mins
    CharacterStats mins;
    mins.strength = m_minStrSpin->value();
    mins.dexterity = m_minDexSpin->value();
    mins.intelligence = m_minIntSpin->value();
    mins.charisma = m_minChaSpin->value();
    mins.constitution = m_minConSpin->value();
    mins.wisdom = m_minWisSpin->value();

    // Current stats
    CharacterStats current;
    current.strength = before.str;
    current.dexterity = before.dex;
    current.intelligence = before.intl;
    current.charisma = before.cha;
    current.constitution = before.con;
    current.wisdom = before.wis;
    current.maxHits = before.maxHits;
    current.ageInYears = before.age;

    // Apply
    ComplicationEffect effect;
    effect.applyEffects(current, mins);

    // Read back
    Stats after;
    after.str = current.strength;
    after.dex = current.dexterity;
    after.intl = current.intelligence;
    after.cha = current.charisma;
    after.con = current.constitution;
    after.wis = current.wisdom;
    after.maxHits = current.maxHits;
    after.age = current.ageInYears;

    updateResultDisplay(before, after);
    writeOutputs(before, after); // Actually set the inputs to after values
}

void ComplicationEffectEditor::resetStats()
{
    Stats reset = {60, 55, 40, 35, 50, 45, 500, 45};
    setInputs(reset);
    applyEffects();
}

void ComplicationEffectEditor::loadExamples()
{
    struct Example { Stats s; Stats mins; const char* desc; };
    static constexpr Example examples[] = {
        {{60, 55, 40, 35, 50, 45, 500, 45},
         {15, 15, 15, 15, 15, 15, 0, 0},
         "Hero (60 Str)"},
        {{18, 18, 18, 18, 18, 18, 100, 20},
         {15, 15, 15, 15, 15, 15, 0, 0},
         "Near-minimum stats (demonstrates floor)"},
        {{80, 70, 60, 50, 75, 55, 800, 60},
         {20, 20, 20, 20, 20, 20, 0, 0},
         "High-stat character"},
        {{30, 30, 30, 30, 30, 30, 200, 35},
         {10, 10, 10, 10, 10, 10, 0, 0},
         "Low-stat character"},
    };
    static int current = 0;
    auto& ex = examples[current % (sizeof(examples) / sizeof(examples[0]))];
    setInputs(ex.s);
    m_minStrSpin->setValue(ex.mins.str);
    m_minDexSpin->setValue(ex.mins.dex);
    m_minIntSpin->setValue(ex.mins.intl);
    m_minChaSpin->setValue(ex.mins.cha);
    m_minConSpin->setValue(ex.mins.con);
    m_minWisSpin->setValue(ex.mins.wis);
    current++;
    applyEffects();
}
