#ifndef COMPLICATION_EFFECT_EDITOR_H
#define COMPLICATION_EFFECT_EDITOR_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QSplitter>
#include <QTextBrowser>

// Editor for the ComplicationEffect system: stat reduction, max hits reduction, age increase
class ComplicationEffectEditor : public QMainWindow
{
    Q_OBJECT
public:
    explicit ComplicationEffectEditor(QWidget* parent = nullptr);

private slots:
    void applyEffects();
    void resetStats();
    void loadExamples();

private:
    // Input stats
    QSpinBox* m_strSpin;
    QSpinBox* m_dexSpin;
    QSpinBox* m_intSpin;
    QSpinBox* m_chaSpin;
    QSpinBox* m_conSpin;
    QSpinBox* m_wisSpin;
    QSpinBox* m_maxHitsSpin;
    QSpinBox* m_ageSpin;

    // Min stats (racial floor)
    QSpinBox* m_minStrSpin;
    QSpinBox* m_minDexSpin;
    QSpinBox* m_minIntSpin;
    QSpinBox* m_minChaSpin;
    QSpinBox* m_minConSpin;
    QSpinBox* m_minWisSpin;

    // Output display
    QTableWidget* m_outputTable;

    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    QPushButton* m_exampleButton;

    struct Stats { int str, dex, intl, cha, con, wis, maxHits, age; };

    Stats readInputs() const;
    void writeOutputs(const Stats& before, const Stats& after);
    void setInputs(const Stats& s);
    void updateResultDisplay(const Stats& before, const Stats& after);
};

#endif
