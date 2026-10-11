#ifndef AREADIALOG_H
#define AREADIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include "src/core/Arena.h"

class ArenaDialog : public QDialog {
    Q_OBJECT
public:
    explicit ArenaDialog(QWidget *parent = nullptr);

private slots:
    void on_startButton_clicked();
    void on_fightButton_clicked();
    void on_fleeButton_clicked();
    void on_attackButton_clicked();
    void on_defendButton_clicked();
    void on_spellButton_clicked();
    void on_useItemButton_clicked();
    void on_nextWaveButton_clicked();
    void on_exitButton_clicked();

private:
    void setupUi();
    void startWave(int wave);
    void updateUI();
    void logMessage(const QString& msg);
    void clearCombat();
    void advanceCombat();
    void handleVictory();
    void handleDefeat();
    void recordScore();

    // UI elements
    QLabel* m_titleLabel = nullptr;
    QLabel* m_waveLabel = nullptr;
    QLabel* m_scoreLabel = nullptr;
    QLabel* m_partyHpLabel = nullptr;
    QLabel* m_monsterHpLabel = nullptr;
    QListWidget* m_messageLog = nullptr;
    QListWidget* m_targetList = nullptr;
    QPushButton* m_startBtn = nullptr;
    QPushButton* m_fightBtn = nullptr;
    QPushButton* m_fleeBtn = nullptr;
    QPushButton* m_attackBtn = nullptr;
    QPushButton* m_defendBtn = nullptr;
    QPushButton* m_spellBtn = nullptr;
    QPushButton* m_useItemBtn = nullptr;
    QPushButton* m_nextWaveBtn = nullptr;
    QPushButton* m_exitBtn = nullptr;

    // Combat state
    int m_currentWave = 0;
    int m_totalScore = 0;
    int m_totalKills = 0;
    int m_totalDamageTaken = 0;
    int m_totalRounds = 0;
    bool m_inCombat = false;
    bool m_arenaComplete = false;
    QList<Arena::WaveResult> m_waveResults;
};

#endif // AREADIALOG_H
