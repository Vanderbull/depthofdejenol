#ifndef PARTYINFODIALOG_H
#define PARTYINFODIALOG_H

#include <QDialog>
#include <QLabel> 
#include <QVector> 
#include <QStringList>

class PartyInfoDialog : public QDialog {
    Q_OBJECT

public:
    explicit PartyInfoDialog(QWidget *parent = nullptr);
    ~PartyInfoDialog();

    void setPartyMembers(const QStringList& members);

private slots:
    void onSwitchToClicked();
    void onOptionsClicked();
    void onLeaveClicked();

private:
    void setupUi();
    void updatePartyLabels(); // New helper to refresh UI
    
    QStringList partyMembers;
    QVector<QLabel*> memberLabels;
    int activeMemberIndex; // Tracks the currently active member
};

#endif // PARTYINFODIALOG_H
