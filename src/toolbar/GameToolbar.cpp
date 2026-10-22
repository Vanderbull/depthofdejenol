#include "GameToolbar.h"
#include <QVBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QApplication>

GameToolbar::GameToolbar(QWidget *parent)
    : QToolBar(parent)
{
    setObjectName("GameToolbar");
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // Mode label - shows current page
    m_modeLabel = new QLabel(tr("Commands"));
    m_modeLabel->setStyleSheet("font-weight: bold; font-size: 11px;");
    addWidget(m_modeLabel);

    // Create command actions (connect to existing menu button slots)
    m_fightAction = addAction(QIcon(":/images/fight.png"), tr("Fight"));
    m_restAction = addAction(QIcon(":/images/rest.png"), tr("Rest"));
    m_spellAction = addAction(QIcon(":/images/spell.png"), tr("Spell"));
    m_teleportAction = addAction(QIcon(":/images/teleport.png"), tr("Teleport"));
    m_openAction = addAction(QIcon(":/images/open.png"), tr("Open"));
    m_escapeAction = addAction(QIcon(":/images/exit.png"), tr("Exit"));

    // Create info actions
    m_sheetAction = addAction(QIcon(":/images/sheet.png"), tr("Character Sheet"));
    m_guildsAction = addAction(QIcon(":/images/guilds.png"), tr("Guilds"));
    m_bestiaryAction = addAction(QIcon(":/images/bestiary.png"), tr("Bestiary"));
    m_journalAction = addAction(QIcon(":/images/journal.png"), tr("Journal"));
    m_debugAction = addAction(QIcon(":/images/debug.png"), tr("Debug"));

    // Connect actions
    connect(m_fightAction, &QAction::triggered, this, [this]() {
        this->setMode(CommandPage);
        emit modeChanged(CommandPage);
    });
    connect(m_sheetAction, &QAction::triggered, this, [this]() {
        this->setMode(InfoPage);
        emit modeChanged(InfoPage);
    });
}

GameToolbar::~GameToolbar() = default;

void GameToolbar::setMode(CommandMode mode) {
    m_mode = mode;
    m_modeLabel->setText(mode == CommandPage ? tr("Commands") : tr("Info"));
    setStyleSheet(mode == CommandPage
        ? "QToolBar { background: #1a1a2e; }"
        : "QToolBar { background: #16213e; }");
    emit modeChanged(mode);
}

void GameToolbar::toggleMode() {
    setMode(m_mode == CommandPage ? InfoPage : CommandPage);
}
