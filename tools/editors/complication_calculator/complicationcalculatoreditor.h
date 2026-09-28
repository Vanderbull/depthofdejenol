#ifndef COMPLICATION_CALC_EDITOR_H
#define COMPLICATION_CALC_EDITOR_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QFrame>
#include <QColor>
#include <QFont>

// Simple calculator: (Age * 2 / RaceMaxAge) * 100
class ComplicationCalcEditor : public QMainWindow
{
    Q_OBJECT
public:
    explicit ComplicationCalcEditor(QWidget* parent = nullptr);

private slots:
    void calculate();
    void addExample();
    void reset();

private:
    QSpinBox* m_ageSpin;
    QSpinBox* m_maxAgeSpin;
    QComboBox* m_raceCombo;
    QLabel* m_resultLabel;
    QLabel* m_resultBar;
    QPushButton* m_calcButton;
    QPushButton* m_exampleButton;
    QPushButton* m_resetButton;

    struct RaceInfo { const char* name; int maxAge; };
    static constexpr RaceInfo s_races[] = {
        {"Human", 100},
        {"Elf", 150},
        {"Dwarf", 120},
        {"Goblie", 50},
        {"Kobold", 40},
        {"Orc", 70},
        {"Nomad", 90},
        {"Warrior", 85},
        {"Mage", 110},
        {"Thief", 80},
        {"Paladin", 95},
        {"Ninja", 75},
        {"Villain", 85},
        {"Seeker", 100},
        {"Scavenger", 80},
        {"Sorcerer", 105},
        {"Wizard", 120},
        {"Healer", 95},
        {"Other", 90},
    };
    static constexpr int s_raceCount = sizeof(s_races) / sizeof(s_races[0]);

    void updateResult(float pct);
};

#endif
