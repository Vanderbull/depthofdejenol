#ifndef CREATECHARACTERDIALOG_H
#define CREATECHARACTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QVector>
#include <QString>
#include "src/race_data/RaceData.h" 
#include "GameStateManager.h"

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
    void updateRaceStats(int index); 
    void onRaceStatsClicked();
    void onGuildStatsClicked();
    void onSaveCharacterClicked(); 
    void onTutorialClicked();
    void onExitClicked(); 

private:
    // Helper Functions
    void updateAlignmentOptions(const RaceStats& race);
    void updateGuildListStyle(const RaceStats& race);

    // UI components
    QLineEdit *nameEdit = nullptr;
    QComboBox *raceBox = nullptr;
    QComboBox *sexBox = nullptr;
    QComboBox *alignmentBox = nullptr;
    QListWidget *guildsListWidget = nullptr;
    
    // Stat Distribution Components
    QMap<QString, QSpinBox*> statSpinBoxes; 
    QMap<QString, QLabel*> statRangeLabels; 
    QMap<QString, QLabel*> statValueLabels; 
    QLabel *statPointsLeftLabel = nullptr; 

    // Data members
    QVector<RaceStats> raceData;
    QVector<QString> guildData;
    int statPoints = 5;
};

#endif // CREATECHARACTERDIALOG_H
