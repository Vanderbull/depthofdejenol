#ifndef CREATECHARACTERDIALOG_H
#define CREATECHARACTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox> // Added for stat distribution
#include <QLabel> // Added for labels and stat point display
#include <QListWidget> // Added for the Guilds Allowed list
#include <QMap> // Added for managing stat widgets
#include <QVector>
#include <QString>
// Assuming this struct definition exists somewhere accessible
#include "src/race_data/RaceData.h" 

class CreateCharacterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateCharacterDialog(const QVector<RaceStats>& raceData, const QVector<QString>& guildData, QWidget *parent = nullptr);
    ~CreateCharacterDialog();

signals:
    /**
     * @brief Signal emitted upon successful creation and saving of a new character.
     * @param characterName The name of the character that was created.
     */
    void characterCreated(const QString &characterName);

private slots:
    // New slot for Race ComboBox change
    void updateRaceStats(int index); 
    
    // New slots for bottom buttons (replacing on_ok/on_cancel)
    void onRaceStatsClicked();
    void onGuildStatsClicked();
    void onSaveCharacterClicked(); 
    void onTutorialClicked();
    void onExitClicked(); 

private:
    // Helper Functions
    void updateAlignmentOptions(const RaceStats& race);
    void updateGuildListStyle(const RaceStats& race);

    // UI components (Names updated to match new implementation)
    QLineEdit *nameEdit = nullptr;
    QComboBox *raceBox = nullptr;
    QComboBox *sexBox = nullptr; // Added
    QComboBox *alignmentBox = nullptr; // Added
    QListWidget *guildsListWidget = nullptr; // Added
    
    // Stat Distribution Components
    QMap<QString, QSpinBox*> statSpinBoxes; 
    QMap<QString, QLabel*> statRangeLabels; 
    QMap<QString, QLabel*> statValueLabels; 
    QLabel *statPointsLeftLabel = nullptr; 

    // Data members
    QVector<RaceStats> raceData;
    QVector<QString> guildData;
    int statPoints = 5; // Tracks the 5 distribution points
};

#endif // CREATECHARACTERDIALOG_H
