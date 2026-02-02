#ifndef HALLOFRECORDSDIALOG_H
#define HALLOFRECORDSDIALOG_H

#include <QDialog>

class HallOfRecordsDialog : public QDialog {
    Q_OBJECT
public:
    explicit HallOfRecordsDialog(QWidget* parent = nullptr);
    ~HallOfRecordsDialog() override;
private:
};

#endif // HALLOFRECORDSDIALOG_H
