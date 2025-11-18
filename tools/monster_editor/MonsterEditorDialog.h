#ifndef MONSTEREDITORDIALOG_H
#define MONSTEREDITORDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>

class MonsterEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit MonsterEditorDialog(QWidget *parent = nullptr);
    ~MonsterEditorDialog();

    // Public method to get monster data (e.g., after the user clicks OK)
    QString getMonsterData() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUi();

    QLineEdit *nameInput;
    QSpinBox *levelInput;
    QSpinBox *hitPointsInput;
    QSpinBox *damageInput;
    QTextEdit *descriptionInput;
};

#endif // MONSTEREDITORDIALOG_H
