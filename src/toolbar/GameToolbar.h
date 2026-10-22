#ifndef GAMETOOLBAR_H
#define GAMETOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QLabel>

class GameToolbar : public QToolBar
{
    Q_OBJECT

public:
    explicit GameToolbar(QWidget *parent = nullptr);
    ~GameToolbar() override;

    // Toolbar modes
    enum CommandMode { CommandPage, InfoPage };
    Q_ENUM(CommandMode)

    // Toggle between command page and info page
    void setMode(CommandMode mode);
    CommandMode mode() const { return m_mode; }

    // F11 toggles the mode
    void toggleMode();

signals:
    // Emitted when the mode changes
    void modeChanged(CommandMode newMode);

private:
    CommandMode m_mode = CommandPage;

    // Common command actions (no logic duplication - connects to existing slots)
    QAction *m_fightAction = nullptr;
    QAction *m_restAction = nullptr;
    QAction *m_spellAction = nullptr;
    QAction *m_teleportAction = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_escapeAction = nullptr;

    // Common info actions
    QAction *m_sheetAction = nullptr;
    QAction *m_guildsAction = nullptr;
    QAction *m_bestiaryAction = nullptr;
    QAction *m_journalAction = nullptr;
    QAction *m_debugAction = nullptr;

    QLabel *m_modeLabel = nullptr;
};

#endif // GAMETOOLBAR_H