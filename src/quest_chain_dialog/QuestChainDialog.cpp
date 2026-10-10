#include "QuestChainDialog.h"
#include "src/core/QuestChain.h"
#include "gameStateManager.h"
#include "src/core/DungeonLevelState.h"
#include <QtWidgets>
#include <QMessageBox>

QuestChainDialog::QuestChainDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    refreshList();
    updateDetail();
}

QuestChainDialog::~QuestChainDialog() = default;

void QuestChainDialog::setupUi() {
    setWindowTitle(tr("Main Quest"));
    setMinimumSize(600, 450);

    auto *mainLayout = new QVBoxLayout(this);

    // Title
    auto *titleLabel = new QLabel(tr("<h2>Main Quest</h2>"));
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Progress
    m_progressLabel = new QLabel(tr(""));
    m_progressLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_progressLabel);

    // Split layout: step list on left, detail on right
    auto *splitLayout = new QHBoxLayout;

    // Step list
    m_stepList = new QListWidget;
    m_stepList->setMaximumWidth(250);
    splitLayout->addWidget(m_stepList);

    // Detail text
    m_detailText = new QTextEdit;
    m_detailText->setReadOnly(true);
    splitLayout->addWidget(m_detailText);

    mainLayout->addLayout(splitLayout);

    // Buttons
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    m_exitBtn = new QPushButton(tr("Close"));
    btnLayout->addWidget(m_exitBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_stepList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *previous) {
                Q_UNUSED(previous);
                onStepSelected(current);
            });
    connect(m_exitBtn, &QPushButton::clicked, this, &QuestChainDialog::onExitClicked);
}

void QuestChainDialog::refreshList() {
    m_stepList->clear();

    // Get current progress from game state
    gameStateManager* gsm = gameStateManager::instance();
    int deepestFloor = gsm->getGameValue("DeepestFloor").toInt();
    if (deepestFloor < 1) {
        // Track deepest floor from current dungeon level
        deepestFloor = gsm->getGameValue("DungeonLevel").toInt();
    }

    // Get defeated boss floors from registry
    QList<int> defeatedBosses;
    for (int i = 1; i <= 15; ++i) {
        if (DungeonLevelRegistry::instance().hasLevel(i)) {
            if (DungeonLevelRegistry::instance().levelForEdit(i).bossDefeated) {
                defeatedBosses.append(i);
            }
        }
    }

    // Get items held by party
    QStringList itemsHeld;
    auto& members = gsm->getPartyMembers();
    for (const auto& member : members) {
        for (const auto& item : member.inventory) {
            if (!itemsHeld.contains(item.name)) {
                itemsHeld.append(item.name);
            }
        }
    }

    QList<QuestStep> steps = QuestChain::steps();
    for (const QuestStep& step : steps) {
        QString status = QuestChain::isStepComplete(step, deepestFloor, defeatedBosses, itemsHeld)
                             ? "[Done]"
                             : "[Active]";
        QString text = QString("%1 %2").arg(status).arg(step.title);
        m_stepList->addItem(text);
    }
}

void QuestChainDialog::updateDetail() {
    // Get current progress
    gameStateManager* gsm = gameStateManager::instance();
    int deepestFloor = gsm->getGameValue("DeepestFloor").toInt();
    if (deepestFloor < 1) {
        deepestFloor = gsm->getGameValue("DungeonLevel").toInt();
    }

    QList<int> defeatedBosses;
    for (int i = 1; i <= 15; ++i) {
        if (DungeonLevelRegistry::instance().hasLevel(i)) {
            if (DungeonLevelRegistry::instance().levelForEdit(i).bossDefeated) {
                defeatedBosses.append(i);
            }
        }
    }

    QStringList itemsHeld;
    auto& members = gsm->getPartyMembers();
    for (const auto& member : members) {
        for (const auto& item : member.inventory) {
            if (!itemsHeld.contains(item.name)) {
                itemsHeld.append(item.name);
            }
        }
    }

    int nextIdx = QuestChain::nextStepIndex(deepestFloor, defeatedBosses, itemsHeld);
    int totalSteps = QuestChain::stepCount();
    int completedSteps = totalSteps - (nextIdx >= 0 ? 1 : 0);

    if (QuestChain::isChainComplete(deepestFloor, defeatedBosses, itemsHeld)) {
        m_progressLabel->setText(tr("<b><font color='gold'>Quest Complete!</font></b>"));
        m_detailText->setHtml(tr("<h3>The Prince of Devils is defeated!</h3>"
                                  "<p>You have completed the main quest. The gate is closed forever.</p>"));
    } else if (nextIdx >= 0) {
        QuestStep step = QuestChain::step(nextIdx);
        m_progressLabel->setText(tr("Step %1 of %2").arg(nextIdx + 1).arg(totalSteps));

        QString html = tr("<h3>%1</h3>").arg(step.title);
        html += tr("<p><b>Objective:</b> %1</p>").arg(QuestChain::objectiveText(step));
        html += tr("<p>%1</p>").arg(step.description);
        html += tr("<hr>");
        html += tr("<p><b>Rewards:</b></p>");
        html += tr("<ul>");
        if (step.rewardGold > 0) {
            html += tr("<li>%1 gold</li>").arg(step.rewardGold);
        }
        if (step.rewardXp > 0) {
            html += tr("<li>%1 XP</li>").arg(step.rewardXp);
        }
        html += tr("</ul>");

        m_detailText->setHtml(html);
    } else {
        m_progressLabel->setText(tr(""));
        m_detailText->setHtml(tr("<p>No active quest.</p>"));
    }
}

void QuestChainDialog::onStepSelected(QListWidgetItem *item) {
    Q_UNUSED(item);
    // Detail is already updated in updateDetail()
}

void QuestChainDialog::onExitClicked() {
    accept();
}
