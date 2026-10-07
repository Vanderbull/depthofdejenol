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

private slots:
    void onRestClicked();
    void onCureClicked();
    void onAdvanceTimeClicked();
    void onExitClicked();

private:
    void setupUi();
    void refreshStatus();

    QLabel *m_goldLabel = nullptr;
    QLabel *m_hpLabel = nullptr;
    QLabel *m_manaLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QSpinBox *m_hoursSpin = nullptr;
    QCheckBox *m_curePoisonCheck = nullptr;
    QCheckBox *m_cureBlindCheck = nullptr;
    QPushButton *m_restBtn = nullptr;
    QPushButton *m_cureBtn = nullptr;
    QPushButton *m_advanceBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;
    QTextEdit *m_log = nullptr;
};

#endif // TAVERNDIALOG_H
