#include "complicationcalculatoreditor.h"
#include "../../complication_calculator/ComplicationCalculator.h"
#include <QApplication>

// Helper: wrap the free function from ComplicationCalculator
float calcChance(int age, int maxAge)
{
    return calculateComplicationChance(age, maxAge);
}

ComplicationCalcEditor::ComplicationCalcEditor(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Dejenol Complication Calculator");
    resize(520, 520);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // --- Styling ---
    setStyleSheet(
        "QMainWindow { background: #1e1e1e; }"
        "QGroupBox { background: #252526; border: 1px solid #3a3a3c; "
        "border-radius: 6px; margin-top: 12px; font-weight: bold; color: #d4d4d4; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; color: #569cd6; }"
        "QLabel { color: #d4d4d4; font-size: 14px; }"
        "QSpinBox { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; "
        "border-radius: 4px; padding: 4px; font-size: 14px; }"
        "QComboBox { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; "
        "border-radius: 4px; padding: 4px; font-size: 14px; }"
        "QPushButton { background: #0e639c; color: white; border: none; "
        "padding: 8px 18px; border-radius: 4px; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background: #1177bb; }"
        "QPushButton:pressed { background: #0a5588; }"
        "QPushButton#secondary { background: #4a4a4a; }"
        "QPushButton#secondary:hover { background: #5a5a5a; }"
    );

    // --- Title ---
    QLabel* title = new QLabel("Complication Risk Calculator");
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #569cd6; "
                         "margin-bottom: 4px;");
    mainLayout->addWidget(title);

    QLabel* subtitle = new QLabel("Calculates the percentage chance of age-related complications.\n"
                                   "Formula: (Age \u00d7 2 / Race Max Age) \u00d7 100");
    subtitle->setStyleSheet("color: #888; font-size: 12px;");
    mainLayout->addWidget(subtitle);

    // --- Input Group ---
    QGroupBox* inputGroup = new QGroupBox("Input Parameters");
    QGridLayout* grid = new QGridLayout(inputGroup);
    grid->setSpacing(12);

    QLabel* ageLabel = new QLabel("Character Age (years):");
    m_ageSpin = new QSpinBox();
    m_ageSpin->setRange(1, 200);
    m_ageSpin->setValue(50);
    m_ageSpin->setSuffix(" yr");

    QLabel* maxAgeLabel = new QLabel("Race Max Age:");
    m_maxAgeSpin = new QSpinBox();
    m_maxAgeSpin->setRange(10, 300);
    m_maxAgeSpin->setValue(90);
    m_maxAgeSpin->setSuffix(" yr");

    QLabel* raceLabel = new QLabel("Race / Class:");
    m_raceCombo = new QComboBox();
    for (int i = 0; i < s_raceCount; ++i)
        m_raceCombo->addItem(s_races[i].name);
    m_raceCombo->setCurrentIndex(6); // Nomad = 90

    // Auto-update max age when race changes
    connect(m_raceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                if (idx >= 0 && idx < s_raceCount) {
                    m_maxAgeSpin->setValue(s_races[idx].maxAge);
                }
            });

    grid->addWidget(ageLabel, 0, 0);
    grid->addWidget(m_ageSpin, 0, 1);
    grid->addWidget(raceLabel, 1, 0);
    grid->addWidget(m_raceCombo, 1, 1);
    grid->addWidget(maxAgeLabel, 2, 0);
    grid->addWidget(m_maxAgeSpin, 2, 1);

    mainLayout->addWidget(inputGroup);

    // --- Calculate Button ---
    m_calcButton = new QPushButton(" Calculate Risk ");
    m_calcButton->setMinimumHeight(40);
    mainLayout->addWidget(m_calcButton);

    connect(m_calcButton, &QPushButton::clicked, this, &ComplicationCalcEditor::calculate);
    connect(m_ageSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ComplicationCalcEditor::calculate);
    connect(m_maxAgeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ComplicationCalcEditor::calculate);
    connect(m_raceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComplicationCalcEditor::calculate);

    // --- Result Display ---
    QGroupBox* resultGroup = new QGroupBox("Result");
    QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);

    m_resultLabel = new QLabel("---");
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #d4d4d4;");

    m_resultBar = new QLabel();
    m_resultBar->setAlignment(Qt::AlignCenter);
    m_resultBar->setMinimumHeight(24);
    m_resultBar->setStyleSheet("background: #2d2d2d; border-radius: 4px; "
                               "color: white; font-size: 12px; padding: 4px;");

    resultLayout->addWidget(m_resultLabel);
    resultLayout->addWidget(m_resultBar);
    mainLayout->addWidget(resultGroup);

    // --- Quick Examples ---
    QGroupBox* exampleGroup = new QGroupBox("Quick Examples");
    QHBoxLayout* exampleLayout = new QHBoxLayout(exampleGroup);

    m_exampleButton = new QPushButton("Load Examples");
    m_exampleButton->setObjectName("secondary");
    m_resetButton = new QPushButton("Reset");
    m_resetButton->setObjectName("secondary");

    exampleLayout->addWidget(m_exampleButton);
    exampleLayout->addStretch();
    exampleLayout->addWidget(m_resetButton);

    mainLayout->addWidget(exampleGroup);

    connect(m_exampleButton, &QPushButton::clicked, this, &ComplicationCalcEditor::addExample);
    connect(m_resetButton, &QPushButton::clicked, this, &ComplicationCalcEditor::reset);

    // Initial calculation
    calculate();
}

void ComplicationCalcEditor::calculate()
{
    int age = m_ageSpin->value();
    int maxAge = m_maxAgeSpin->value();
    float pct = calcChance(age, maxAge);
    updateResult(pct);
}

void ComplicationCalcEditor::updateResult(float pct)
{
    QString color;
    if (pct < 25)      color = "#4ec9b0"; // teal - low risk
    else if (pct < 50) color = "#569cd6"; // blue - moderate
    else if (pct < 75) color = "#c586c0"; // purple - high
    else               color = "#ff6b6b"; // red - severe

    m_resultLabel->setText(QString("%1%").arg(pct, 0, 'f', 1));
    m_resultLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(color));

    // Progress bar style
    int barWidth = 400;
    int filled = static_cast<int>((pct / 100.0) * barWidth);
    QString barHtml = QString(
        "<table width='%1' cellpadding='0' cellspacing='0' style='margin: 0 auto;'>"
        "<tr>"
        "<td style='background: %2; width: %3px; text-align: left; padding-left: 8px; "
        "color: white; font-weight: bold;'>%.0f%%</td>"
        "<td style='background: #3a3a3c; width: %4px;'></td>"
        "</tr></table>")
        .arg(barWidth).arg(color).arg(filled)
        .arg(pct).arg(barWidth - filled);
    m_resultBar->setStyleSheet("background: #2d2d2d; border-radius: 4px;");
    m_resultBar->setText("");
    m_resultBar->setOpenExternalLinks(false);
    // Use a simple text bar instead of HTML for simplicity
    m_resultBar->setStyleSheet(QString(
        "QLabel { background: #2d2d2d; border-radius: 4px; padding: 4px; "
        "color: %1; font-weight: bold; font-size: 13px; text-align: center; }")
        .arg(color));
    m_resultBar->setText(QString("Risk: %1%  %2")
                             .arg(pct, 0, 'f', 1)
                             .arg(pct < 25 ? "Low" : pct < 50 ? "Moderate" :
                                  pct < 75 ? "High" : "Severe"));
}

void ComplicationCalcEditor::addExample()
{
    struct Example { int age; int maxAge; const char* desc; };
    static constexpr Example examples[] = {
        {25, 100, "Young Human"},
        {50, 100, "Middle-aged Human"},
        {75, 100, "Elderly Human"},
        {45, 50, "Old Goblie"},
        {30, 150, "Young Elf"},
        {100, 150, "Elderly Elf"},
        {60, 70, "Orc 60"},
        {20, 40, "Kobold 20"},
    };
    static int current = 0;
    int idx = current % (sizeof(examples) / sizeof(examples[0]));
    auto& ex = examples[idx];
    m_ageSpin->setValue(ex.age);
    m_maxAgeSpin->setValue(ex.maxAge);
    // Find race by maxAge
    for (int i = 0; i < s_raceCount; ++i) {
        if (s_races[i].maxAge == ex.maxAge) {
            m_raceCombo->setCurrentIndex(i);
            break;
        }
    }
    current++;
}

void ComplicationCalcEditor::reset()
{
    m_ageSpin->setValue(50);
    m_maxAgeSpin->setValue(90);
    m_raceCombo->setCurrentIndex(6); // Nomad
}
