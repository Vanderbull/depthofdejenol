#ifndef PARTYINFODIALOG_H
#define PARTYINFODIALOG_H

#include <QDialog>
#include <QLabel> 
#include <QVector> 
#include <QStringList>
#include <QVariant>

class PartyInfoDialog : public QDialog {
    Q_OBJECT

public:
    explicit PartyInfoDialog(QWidget *parent = nullptr);
    ~PartyInfoDialog();

private slots:
    void onSwitchToClicked();
    void onOptionsClicked();
    void onLeaveClicked();
    void onGameStateChanged(const QString& key, const QVariant& value);

private:
    void updatePartyLabels();
    void refreshFromGameState(); 
    
    QStringList partyMembers;
    QVector<QLabel*> memberLabels;
    int activeMemberIndex; 
};

#endif
