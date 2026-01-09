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
};

#endif // HALLOFRECORDSDIALOG_H
