#ifndef MARLITH_DIALOG_H
#define MARLITH_DIALOG_H

#include <QDialog>
#include <QPushButton>

// Forward declaration of a UI class
namespace Ui {
class MarlithDialog;
}

// The MarlithDialog class inherits from QDialog
class MarlithDialog : public QDialog
{
    // Q_OBJECT macro is required for all classes that
    // use Qt's meta-object system (e.g., signals and slots)
    Q_OBJECT

public:
    // Constructor and destructor
    explicit MarlithDialog(QWidget *parent = nullptr);
    ~MarlithDialog();

private:
    // Pointer to the UI object
    Ui::MarlithDialog *ui;

    // Pointers to the QPushButton widgets
    QPushButton *storeButton;
    QPushButton *confinementButton;
    QPushButton *guildButton;
    QPushButton *bankButton;
    QPushButton *morgueButton;
    QPushButton *theSeerButton;
};

#endif // MARLITH_DIALOG_H
