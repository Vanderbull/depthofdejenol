#ifndef SPELLBOOKEDITORDIALOG_H
#define SPELLBOOKEDITORDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>

class SpellbookEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit SpellbookEditorDialog(QWidget *parent = nullptr);
    ~SpellbookEditorDialog();

    // Public method to retrieve the finalized spell data
    QString getSpellData() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUi();

    QLineEdit *nameInput;
    QSpinBox *levelInput;
    QSpinBox *manaCostInput;
    QSpinBox *powerInput; // Used for Damage or Healing value
    QComboBox *targetTypeInput;
    QTextEdit *descriptionInput;
};

#endif // SPELLBOOKEDITORDIALOG_H
