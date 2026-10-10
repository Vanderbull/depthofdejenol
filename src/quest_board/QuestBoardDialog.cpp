#include "QuestBoardDialog.h"
#include "gameStateManager.h"
#include "character.h"
#include <QtWidgets>
#include <QMessageBox>

// --- Static state ---
static QStringList s_accepted;
static QMap<QString, int> s_kills;
static QMap<QString, bool> s_fetched;

QList<BoardQuest> QuestBoardDialog::availableQuests() {
    QList<BoardQuest> quests;

    auto make = [](const QString& id, const QString& title, const QString& desc,
                   const QString& giver, int floor, const QString& monster,
                   int kills, int gold, int xp) {
        BoardQuest q;
        q.id = id;
        q.title = title;
        q.description = desc;
        q.giver = giver;
        q.targetFloor = floor;
        q.targetMonster = monster;
        q.killCount = kills;
        q.rewardGold = gold;
        q.rewardXp = xp;
        return q;
    };

    quests.append(make("kill_rats", "Vermin in the Mines",
                       "Rats have overrun the first floor. Cull them.",
                       "Guildmaster", 1, "Giant Rat", 5, 200, 500));
    quests.append(make("kill_goblins", "Goblin Troubles",
                       "Goblins raid the supply caravans. Drive them off.",
                       "Captain of the Guard", 3, "Goblin", 8, 500, 1500));
    quests.append(make("kill_skeletons", "The Restless Dead",
                       "Skeletons stir in the crypts. Put them to rest.",
                       "Priest of Light", 7, "Skeleton", 10, 1000, 3000));
    quests.append(make("kill_orcs", "Orc Warband",
                       "An orc warband holds the deep halls. Break them.",
                       "Guildmaster", 10, "Orc Warrior", 12, 2500, 7500));
    quests.append(make("fetch_herb", "Medicinal Herbs",
                       "The temple needs moonpetal herbs from the upper floors.",
                       "Temple Acolyte", 2, QString(), 0, 300, 800));

    BoardQuest fetch = quests.last();
    fetch.isFetch = true;
    fetch.fetchItem = "Moonpetal";
    quests[quests.size() - 1] = fetch;

    return quests;
}

BoardQuest QuestBoardDialog::questById(const QString& id) {
    for (const BoardQuest& q : availableQuests()) {
        if (q.id == id) return q;
    }
    BoardQuest invalid;
    invalid.id = QString();
    return invalid;
}

bool QuestBoardDialog::acceptQuest(const QString& id) {
    BoardQuest q = questById(id);
    if (q.id.isEmpty()) return false;
    if (s_accepted.contains(id)) return false;
    s_accepted.append(id);
    return true;
}

bool QuestBoardDialog::isAccepted(const QString& id) {
    return s_accepted.contains(id);
}

QStringList QuestBoardDialog::acceptedQuests() {
    return s_accepted;
}

bool QuestBoardDialog::reportKill(const QString& monsterName, int floor) {
    bool any = false;
    for (const QString& id : s_accepted) {
        BoardQuest q = questById(id);
        if (q.id.isEmpty() || q.isFetch) continue;
        if (q.targetMonster != monsterName) continue;
        if (q.targetFloor != 0 && q.targetFloor != floor) continue;
        int current = s_kills.value(id, 0);
        if (current < q.killCount) {
            s_kills[id] = current + 1;
            any = true;
        }
    }
    return any;
}

bool QuestBoardDialog::reportFetch(const QString& itemId) {
    bool any = false;
    for (const QString& id : s_accepted) {
        BoardQuest q = questById(id);
        if (q.id.isEmpty() || !q.isFetch) continue;
        if (q.fetchItem != itemId) continue;
        if (!s_fetched.value(id, false)) {
            s_fetched[id] = true;
            any = true;
        }
    }
    return any;
}

bool QuestBoardDialog::isComplete(const QString& id) {
    BoardQuest q = questById(id);
    if (q.id.isEmpty()) return false;
    if (!s_accepted.contains(id)) return false;

    if (q.isFetch) {
        return s_fetched.value(id, false);
    }
    return s_kills.value(id, 0) >= q.killCount;
}

bool QuestBoardDialog::turnIn(const QString& id, int& goldOut, int& xpOut) {
    goldOut = 0;
    xpOut = 0;

    if (!isComplete(id)) return false;

    BoardQuest q = questById(id);
    goldOut = q.rewardGold;
    xpOut = q.rewardXp;

    auto *gsm = gameStateManager::instance();
    gsm->addPartyGold(goldOut);

    // Award XP to all living members.
    auto& members = gsm->getPartyMembers();
    for (auto& c : members) {
        if (c.isAlive) {
            c.addExperience(xpOut);
        }
    }

    s_accepted.removeAll(id);
    s_kills.remove(id);
    s_fetched.remove(id);
    return true;
}

void QuestBoardDialog::reset() {
    s_accepted.clear();
    s_kills.clear();
    s_fetched.clear();
}

// --- UI ---

QuestBoardDialog::QuestBoardDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    refreshList();
}

QuestBoardDialog::~QuestBoardDialog() = default;

void QuestBoardDialog::setupUi() {
    setWindowTitle(tr("Quest Board"));
    setMinimumSize(500, 400);

    auto *mainLayout = new QVBoxLayout(this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(tr("Posted Quests:")));
    mainLayout->addLayout(filterLayout);

    m_questList = new QListWidget;
    m_questList->setMaximumWidth(220);
    mainLayout->addWidget(m_questList);

    m_detailText = new QTextEdit;
    m_detailText->setReadOnly(true);
    mainLayout->addWidget(m_detailText);

    m_countLabel = new QLabel;
    mainLayout->addWidget(m_countLabel);

    auto *btnLayout = new QHBoxLayout;
    m_acceptBtn = new QPushButton(tr("Accept Quest"));
    m_turnInBtn = new QPushButton(tr("Turn In"));
    m_turnInBtn->setEnabled(false);
    m_exitBtn = new QPushButton(tr("Close"));
    btnLayout->addWidget(m_acceptBtn);
    btnLayout->addWidget(m_turnInBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_exitBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_questList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *previous) {
                Q_UNUSED(previous);
                onQuestSelected(current);
            });
    connect(m_acceptBtn, &QPushButton::clicked, this, &QuestBoardDialog::onAcceptClicked);
    connect(m_turnInBtn, &QPushButton::clicked, this, &QuestBoardDialog::onTurnInClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &QuestBoardDialog::onExitClicked);
}

void QuestBoardDialog::refreshList() {
    m_questList->clear();

    for (const BoardQuest& q : availableQuests()) {
        QString label = q.title;
        if (s_accepted.contains(q.id)) {
            label += isComplete(q.id) ? tr(" [Done]") : tr(" [Active]");
        }
        new QListWidgetItem(label, m_questList);
    }

    m_countLabel->setText(tr("Accepted: %1").arg(s_accepted.size()));

    if (m_questList->count() > 0) {
        m_questList->setCurrentRow(0);
    }
}

void QuestBoardDialog::refreshDetail() {
    QListWidgetItem *item = m_questList->currentItem();
    if (!item) return;

    int row = m_questList->row(item);
    QList<BoardQuest> quests = availableQuests();
    if (row < 0 || row >= quests.size()) return;

    const BoardQuest& q = quests[row];

    QString status;
    if (isComplete(q.id)) {
        status = tr("<b>Status:</b> Complete — return for reward<br>");
    } else if (s_accepted.contains(q.id)) {
        if (q.isFetch) {
            status = tr("<b>Status:</b> Accepted — find the %1<br>").arg(q.fetchItem);
        } else {
            int kills = s_kills.value(q.id, 0);
            status = tr("<b>Status:</b> Accepted — %1 / %2 killed<br>")
                .arg(kills).arg(q.killCount);
        }
    } else {
        status = tr("<b>Status:</b> Not accepted<br>");
    }

    m_detailText->setHtml(tr("<b>%1</b><br>"
                              "<i>Posted by: %2</i><br><br>"
                              "%3<br>"
                              "<b>Reward:</b> %4 gold, %5 XP<br>"
                              "%6")
        .arg(q.title)
        .arg(q.giver)
        .arg(q.description)
        .arg(q.rewardGold)
        .arg(q.rewardXp)
        .arg(status));

    // Turn In is only meaningful for a completed, accepted quest.
    m_turnInBtn->setEnabled(isComplete(q.id));
    m_acceptBtn->setEnabled(!s_accepted.contains(q.id));
}

void QuestBoardDialog::onQuestSelected(QListWidgetItem *item) {
    Q_UNUSED(item)
    refreshDetail();
}

void QuestBoardDialog::onAcceptClicked() {
    QListWidgetItem *item = m_questList->currentItem();
    if (!item) return;

    int row = m_questList->row(item);
    QList<BoardQuest> quests = availableQuests();
    if (row < 0 || row >= quests.size()) return;

    const BoardQuest& q = quests[row];

    if (s_accepted.contains(q.id)) {
        QMessageBox::information(this, tr("Quest Board"),
                                 tr("You have already accepted this quest."));
        return;
    }

    acceptQuest(q.id);
    refreshList();
    refreshDetail();
}

void QuestBoardDialog::onTurnInClicked() {
    QListWidgetItem *item = m_questList->currentItem();
    if (!item) return;

    int row = m_questList->row(item);
    QList<BoardQuest> quests = availableQuests();
    if (row < 0 || row >= quests.size()) return;

    const BoardQuest& q = quests[row];

    if (!isComplete(q.id)) {
        QMessageBox::information(this, tr("Quest Board"),
                                 tr("This quest is not complete yet."));
        return;
    }

    int gold = 0, xp = 0;
    if (turnIn(q.id, gold, xp)) {
        QMessageBox::information(this, tr("Quest Board"),
                                 tr("Quest turned in!\nReward: %1 gold, %2 XP.")
                                     .arg(gold).arg(xp));
        refreshList();
        refreshDetail();
    }
}

void QuestBoardDialog::onExitClicked() {
    accept();
}
