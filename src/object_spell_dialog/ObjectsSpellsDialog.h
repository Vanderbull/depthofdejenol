#ifndef OBJECTSSPELLSDIALOG_H
#define OBJECTSSPELLSDIALOG_H

#include <QDialog>
#include <QStringListModel>

// Forward declarations speed up compilation
class QTabWidget;
class QListView;
class QTextEdit;
class QLabel;

class ObjectsSpellsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectsSpellsDialog(QWidget *parent = nullptr);
    ~ObjectsSpellsDialog() = default; // Use default if no custom cleanup is needed

    // Update methods (Consider passing a custom Struct/Object instead of QStringList later)
    void updateItems(const QStringList& items);
    void updateSpells(const QStringList& spells);
    void updateCharStats(const QString& name, int lead, int cast, const QString& guild, long long exp);

signals:
    void itemSelected(const QString& itemDescription);
    void spellSelected(const QString& spellDescription);

private:
    void setupUi();
    void createConnections();

    // Models
    QStringListModel *m_itemsModel;
    QStringListModel *m_spellsModel;
    QStringListModel *m_companionsModel;

    // UI Elements
    QTabWidget *m_tabWidget;
    QListView *m_itemsListView;
    QListView *m_spellsListView;
    QListView *m_companionsListView;
    
    QTextEdit *m_charInfoDisplay;
    QTextEdit *m_guildInfoDisplay;

    // Right side detail labels
    QLabel *m_charNameLabel;
    QLabel *m_leadLabel;
    QLabel *m_castLabel;
    QLabel *m_guildLabel;
    QLabel *m_expLabel;
};

#endif
