#ifndef TUTORIALDIALOG_H
#define TUTORIALDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QString>

class TutorialDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TutorialDialog(QWidget *parent = nullptr);

    void showStep(int step);

private:
    QListWidget *m_listWidget = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
    int m_currentStep = 0;
};

#endif // TUTORIALDIALOG_H
