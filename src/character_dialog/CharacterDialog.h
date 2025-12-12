#ifndef CHARACTERDIALOG_H
#define CHARACTERDIALOG_H

#include <QDialog>
#include <QString>
#include "GameStateManager.h"

class CharacterDialog : public QDialog {
    Q_OBJECT

public:
    // CORRECTED: Constructor signature now takes only the parent widget
    explicit CharacterDialog(QWidget *parent = nullptr);
    ~CharacterDialog() override = default;

private:
    void setupUi();
};

#endif // CHARACTERDIALOG_H
