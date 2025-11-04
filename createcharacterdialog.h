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
#include <QListWidget> // Required for QListWidget member variable

// Structure to hold min/max/start values for a stat (e.g., Str, Int)
struct RaceStat {
    int start; // The default value when the race is selected
    int min;
    int max;
};

// Simplified Enum based on the table values ('X' and '-')
enum AlignmentStatus {
    AS_Allowed,   // Represents 'X'
    AS_NotAllowed // Represents '-'
};

// Structure to hold all stats for a single race
struct RaceStats {
    QString raceName;
    int maxAge;
    int experience;
    RaceStat strength;
    RaceStat intelligence;
    RaceStat wisdom;
    RaceStat constitution;
    RaceStat charisma;
    RaceStat dexterity;
    AlignmentStatus good; // G
    AlignmentStatus neutral; // N
    AlignmentStatus evil; // E
    
    // Guild Eligibility Map: GuildName -> Allowed/NotAllowed
    QMap<QString, AlignmentStatus> guildEligibility; 
};


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
    QComboBox *raceBox;
    QComboBox *alignmentBox; 
    QListWidget *guildsListWidget; 

    QMap<QString, QSpinBox*> statSpinBoxes;
    QMap<QString, QLabel*> statRangeLabels;
    QMap<QString, QLabel*> statValueLabels;

    // Helper functions
    QVector<RaceStats> loadRaceData();
    QVector<QString> loadGuildData();
    void updateAlignmentOptions(const RaceStats& race);
    void updateGuildListStyle(const RaceStats& race);
};

#endif // CREATECHARACTERDIALOG_H
