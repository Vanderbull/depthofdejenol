#ifndef TAVERNDIALOG_H
#define TAVERNDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QObject>

class QLabel;
class QPushButton;
class QSpinBox;
class QCheckBox;
class QTextEdit;

// Tavern/Inn: rest to restore HP/mana, cure status effects, advance time.
class TavernDialog : public QDialog {
    Q_OBJECT

public:
    explicit TavernDialog(QWidget *parent = nullptr);
    ~TavernDialog() override;

    // GoldSinks-backed pricing, shared with the self-tests. restCost is
    // hours * rate * living members; cureCost sums the selected cures.
    static int restCost(int hours, int livingMembers);
    static int cureCost(bool poison, bool blindness);

public:
    void onRestClicked();
    void onCureClicked();
    void onAdvanceTimeClicked();
    void onExitClicked();

    QSpinBox *m_hoursSpin = nullptr;
    QTextEdit *m_log = nullptr;

private:
    void setupUi();
    void refreshStatus();

    QLabel *m_goldLabel = nullptr;
    QLabel *m_hpLabel = nullptr;
    QLabel *m_manaLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QCheckBox *m_curePoisonCheck = nullptr;
    QCheckBox *m_cureBlindCheck = nullptr;
    QPushButton *m_restBtn = nullptr;
    QPushButton *m_cureBtn = nullptr;
    QPushButton *m_advanceBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;
};

#endif // TAVERNDIALOG_H
