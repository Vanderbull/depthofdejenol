#ifndef CHARACTERDIALOG_H
#define CHARACTERDIALOG_H

#include <QDialog>
#include <QString>

class CharacterDialog : public QDialog {
    Q_OBJECT

public:
    // Constructor takes the character name for the window title
    explicit CharacterDialog(const QString &charName, QWidget *parent = nullptr);
    ~CharacterDialog() override = default;

private:
    void setupUi();
};

#endif // CHARACTERDIALOG_H
