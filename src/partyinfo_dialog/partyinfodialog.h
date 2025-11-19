#ifndef PARTYINFODIALOG_H
#define PARTYINFODIALOG_H

#include <QDialog>
#include <QPixmap>
#include <QLabel> 
class PartyInfoDialog : public QDialog {
    Q_OBJECT

public:
    explicit PartyInfoDialog(QWidget *parent = nullptr);
    ~PartyInfoDialog();

    void setPartyMembers(const QStringList& members);

private:
    void setupUi();
    QStringList partyMembers;
    QVector<QLabel*> memberLabels;
};

#endif // PARTYINFODIALOG_H
