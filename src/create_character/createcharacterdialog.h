#ifndef CREATECHARACTERDIALOG_H
#define CREATECHARACTERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <QMap>
#include <QStringList> 
#include <QListWidget> 
#include <QFile>
// Include the new header file for RaceStats and declarations for global load functions
#include "RaceData.h" 


class CreateCharacterDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateCharacterDialog(QWidget *parent = nullptr);
    ~CreateCharacterDialog();

private slots:
    void onRaceStatsClicked();
    void onGuildStatsClicked();
    void onSaveCharacterClicked();
    void onTutorialClicked();
    void onExitClicked();
    
    void updateRaceStats(int index);

private:
    int statPoints = 5;
    QLabel *statPointsLeftLabel;
    
    // Member variables to hold race data and stat/guild widgets
    QVector<RaceStats> raceData;
    QVector<QString> guildData; 
    
    QLineEdit *nameEdit; 
    QComboBox *raceBox;
    QComboBox *sexBox; 
    QComboBox *alignmentBox; 
    QListWidget *guildsListWidget; 

    QMap<QString, QSpinBox*> statSpinBoxes;
    QMap<QString, QLabel*> statRangeLabels;
    QMap<QString, QLabel*> statValueLabels;

    // Helper functions 
    void updateAlignmentOptions(const RaceStats& race);
    void updateGuildListStyle(const RaceStats& race);
    
    // NOTE: loadRaceData() and loadGuildData() declarations REMOVED to fix linker error.
};

#endif // CREATECHARACTERDIALOG_H
