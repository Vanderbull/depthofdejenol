#ifndef JOURNALDIALOG_H
#define JOURNALDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QObject>

class QListWidget;
class QListWidgetItem;
class QTextEdit;
class QPushButton;
class QComboBox;

// A journal entry: timestamped text.
struct JournalEntry {
    QDateTime timestamp;
    QString category;
    QString text;
};

// Journal: tracks quest progress, notable events, and player notes.
class JournalDialog : public QDialog {
    Q_OBJECT

public:
    explicit JournalDialog(QWidget *parent = nullptr);
    ~JournalDialog() override;

    // Add a journal entry programmatically.
    static void addEntry(const QString& category, const QString& text);

    // Read back everything on disk, and wipe it. Used by the self-tests to
    // verify that the game's own event paths actually write entries.
    static QList<JournalEntry> allEntries();
    static void clearAll();

private slots:
    void onEntrySelected(QListWidgetItem *item);
    void onFilterChanged(int index);
    void onAddNoteClicked();
    void onClearClicked();
    void onExitClicked();

private:
    void setupUi();
    void loadEntries();
    void refreshList();

    QListWidget *m_entryList = nullptr;
    QTextEdit *m_entryText = nullptr;
    QComboBox *m_filterCombo = nullptr;
    QPushButton *m_addNoteBtn = nullptr;
    QPushButton *m_clearBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;

    QList<JournalEntry> m_entries;
};

#endif // JOURNALDIALOG_H
