#ifndef CREATECHARACTERDIALOG_H
#define CREATECHARACTERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CreateCharacterDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateCharacterDialog(QWidget *parent = nullptr);
    ~CreateCharacterDialog();

private slots:
    void onRaceStatsClicked();
    void onGuildStatsClicked();
    void onSaveCharacterClicked();
    void onTutorialClicked();
    void onExitClicked();

private:
    int statPoints = 5;
    QLabel *statPointsLeftLabel;
};

#endif // CREATECHARACTERDIALOG_H
