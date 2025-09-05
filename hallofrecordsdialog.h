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
    QLabel* titleLabel;
    QLabel* recordsTitle;
    QLabel* mastersTitle;
    QPushButton* exitButton;
};

#endif // HALLOFRECORDSDIALOG_H
