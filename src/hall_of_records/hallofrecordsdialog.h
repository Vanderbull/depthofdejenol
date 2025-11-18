#ifndef HALLOFRECORDSDIALOG_H
#define HALLOFRECORDSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class HallOfRecordsDialog : public QDialog {
    Q_OBJECT

public:
    explicit HallOfRecordsDialog(QWidget* parent = nullptr);
    ~HallOfRecordsDialog();

private:
    // Removed unused private members: titleLabel, recordsTitle, mastersTitle, exitButton
    // All UI elements are now managed within the constructor's scope.
};

#endif // HALLOFRECORDSDIALOG_H
