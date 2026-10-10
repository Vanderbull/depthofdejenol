#ifndef TEMPLEDIALOG_H
#define TEMPLEDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QObject>

class QLabel;
class QPushButton;
class QTextEdit;
class QListWidget;

// Temple: heal HP, cure status effects, resurrect the dead.
class TempleDialog : public QDialog {
    Q_OBJECT

public:
    explicit TempleDialog(QWidget *parent = nullptr);
    ~TempleDialog() override;

private slots:
    void onHealClicked();
    void onCureClicked();
    void onResurrectClicked();
    void onExitClicked();

private:
    void setupUi();
    void refreshStatus();

    QLabel *m_goldLabel = nullptr;
    QLabel *m_hpLabel = nullptr;
    QLabel *m_manaLabel = nullptr;
    QListWidget *m_partyList = nullptr;
    QPushButton *m_healBtn = nullptr;
    QPushButton *m_cureBtn = nullptr;
    QPushButton *m_resurrectBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;
    QTextEdit *m_log = nullptr;
};

#endif // TEMPLEDIALOG_H
