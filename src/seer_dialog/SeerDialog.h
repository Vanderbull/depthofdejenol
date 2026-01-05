// SeerDialog.h
#ifndef SEERDIALOG_H
#define SEERDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include "GameStateManager.h" // Include the GameStateManager header

class SeerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SeerDialog(QWidget *parent = nullptr);
    ~SeerDialog();

private slots:
    void on_characterButton_clicked();
    void on_monsterButton_clicked();
    void on_itemButton_clicked();
    void on_exitButton_clicked();

private:
    QLineEdit *searchLineEdit;
    QLabel *welcomeLabel;
    //QLabel *optionsLabel;
    QPushButton *characterButton;
    QPushButton *monsterButton;
    QPushButton *itemButton;
    QPushButton *exitButton;

    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
};

#endif // SEERDIALOG_H
