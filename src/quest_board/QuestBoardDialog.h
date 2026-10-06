#ifndef QUESTBOARD_H
#define QUESTBOARD_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QList>

class QListWidget;
class QListWidgetItem;
class QTextEdit;
class QLabel;
class QPushButton;

// A quest posted on the board.
struct BoardQuest {
    QString id;
    QString title;
    QString description;
    QString giver;
    int targetFloor = 0;
    QString targetMonster;
    int killCount = 0;
    int rewardGold = 0;
    int rewardXp = 0;
    bool isFetch = false;
    QString fetchItem;
};

// Quest board: fetch and kill quests posted in the city.
class QuestBoardDialog : public QDialog {
    Q_OBJECT

public:
    explicit QuestBoardDialog(QWidget *parent = nullptr);
    ~QuestBoardDialog() override;

    // All posted quests.
    static QList<BoardQuest> availableQuests();

    // A quest by id, or an invalid quest (empty id) when not found.
    static BoardQuest questById(const QString& id);

    // Accept a quest: returns false when the quest is unknown or already accepted.
    static bool acceptQuest(const QString& id);

    // Is a quest accepted?
    static bool isAccepted(const QString& id);

    // All accepted quest ids.
    static QStringList acceptedQuests();

    // Progress a kill quest: returns true when the kill count is met.
    static bool reportKill(const QString& monsterName, int floor);

    // Progress a fetch quest: returns true when the item is held.
    static bool reportFetch(const QString& itemId);

    // Is a quest complete?
    static bool isComplete(const QString& id);

    // Turn in a completed quest for its reward. Returns false when not complete.
    static bool turnIn(const QString& id, int& goldOut, int& xpOut);

    // Clear all accepted quests (for testing).
    static void reset();

private slots:
    void onQuestSelected(QListWidgetItem *item);
    void onAcceptClicked();
    void onExitClicked();

private:
    void setupUi();
    void refreshList();
    void refreshDetail();

    QListWidget *m_questList = nullptr;
    QTextEdit *m_detailText = nullptr;
    QLabel *m_countLabel = nullptr;
    QPushButton *m_acceptBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;
};

#endif // QUESTBOARD_H
