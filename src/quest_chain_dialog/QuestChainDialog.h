#ifndef QUESTCHAINDIALOG_H
#define QUESTCHAINDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QObject>

class QListWidget;
class QListWidgetItem;
class QTextEdit;
class QLabel;
class QPushButton;

// QuestChainDialog: displays the main quest chain to the player.
// Shows the current objective, progress, and rewards for each step.
class QuestChainDialog : public QDialog {
    Q_OBJECT

public:
    explicit QuestChainDialog(QWidget *parent = nullptr);
    ~QuestChainDialog() override;

private slots:
    void onStepSelected(QListWidgetItem *item);
    void onExitClicked();

private:
    void setupUi();
    void refreshList();
    void updateDetail();

    QListWidget *m_stepList = nullptr;
    QTextEdit *m_detailText = nullptr;
    QLabel *m_progressLabel = nullptr;
    QPushButton *m_exitBtn = nullptr;
};

#endif // QUESTCHAINDIALOG_H
