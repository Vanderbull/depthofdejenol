#ifndef MORGUEDIALOG_H
#define MORGUEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class MorgueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MorgueDialog(QWidget *parent = nullptr);
    ~MorgueDialog();

private slots:
    void onRaiseCharacterClicked();
    void onHireRescuersClicked();
    void onGrabBodyClicked();
    void onExitClicked();

private:
    QLabel *welcomeLabel;
    QPushButton *raiseCharacterButton;
    QPushButton *hireRescuersButton;
    QPushButton *grabBodyButton;
    QPushButton *exitButton;
};

#endif // MORGUEDIALOG_H
