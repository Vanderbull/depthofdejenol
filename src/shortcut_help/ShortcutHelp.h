#ifndef SHORTCUTHELP_H
#define SHORTCUTHELP_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QList>
#include <QObject>

// A single keyboard shortcut entry.
struct ShortcutEntry {
    QString key;        // e.g. "W", "Ctrl+S"
    QString action;     // e.g. "Move forward"
    QString context;    // e.g. "Dungeon", "Global"
};

// Keyboard shortcut help overlay.
class ShortcutHelp : public QDialog {
    Q_OBJECT

public:
    explicit ShortcutHelp(QWidget *parent = nullptr);
    ~ShortcutHelp() override;

    // All shortcuts, grouped by context.
    static QList<ShortcutEntry> allShortcuts();

    // Shortcuts for a specific context ("Dungeon", "Global", etc.).
    static QList<ShortcutEntry> shortcutsFor(const QString& context);

    // All available contexts.
    static QStringList contexts();

    // A shortcut by key + context, or an invalid entry when not found.
    static ShortcutEntry find(const QString& key, const QString& context);

    // Display text for a shortcut entry.
    static QString format(const ShortcutEntry& e);
};

#endif // SHORTCUTHELP_H
