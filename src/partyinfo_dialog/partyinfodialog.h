#ifndef PARTYINFODIALOG_H
#define PARTYINFODIALOG_H

#include <QDialog>
#include <QPixmap>

class PartyInfoDialog : public QDialog {
    Q_OBJECT

public:
    explicit PartyInfoDialog(QWidget *parent = nullptr);
    ~PartyInfoDialog();

private:
    void setupUi();
};

#endif // PARTYINFODIALOG_H
