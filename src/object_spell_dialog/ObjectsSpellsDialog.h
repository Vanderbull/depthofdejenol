#ifndef OBJECTSSPELLSDIALOG_H
#define OBJECTSSPELLSDIALOG_H

#include <QDialog>
#include <QStringListModel>
#include <QTabWidget>
#include <QListView>
#include <QLabel>
#include <QTextEdit>
#include "GameStateManager.h"

class ObjectsSpellsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectsSpellsDialog(QWidget *parent = nullptr);
    ~ObjectsSpellsDialog();

    void updateItems(const QStringList& items);
    void updateSpells(const QStringList& spells);
    void updateCompanions(const QStringList& companions);
    void updateGuild(const QStringList& guildInfo);
    void updateChar(const QStringList& charInfo);
    void updateBuffs(const QStringList& buffs);
    void updateMisc(const QStringList& miscInfo);

signals:
    void itemSelected(const QString& itemDescription);
    void spellSelected(const QString& spellDescription);
    void companionSelected(const QString& companionDescription);

private slots:
    void on_tabWidget_currentChanged(int index); // Slot for QTabWidget

    void on_itemsListView_clicked(const QModelIndex& index);
    void on_spellsListView_clicked(const QModelIndex& index);
    void on_companionsListView_clicked(const QModelIndex& index);

private:
    // Models for the list views
    QStringListModel *m_itemsModel;
    QStringListModel *m_spellsModel;
    QStringListModel *m_companionsModel;
    // UI Widgets that need member access for updates
    QTabWidget *m_tabWidget;
    QListView *m_itemsListView;
    QListView *m_spellsListView;
    QListView *m_companionsListView;
    QTextEdit *m_charInfoDisplay;
    QTextEdit *m_guildInfoDisplay;
    QTextEdit *m_buffsInfoDisplay;
    QTextEdit *m_miscInfoDisplay;
    // Details panel widgets
    QLabel *m_charNameLabel;
    QLabel *m_leadLabel;
    QLabel *m_castLabel;
    QLabel *m_guildLabel;
    QLabel *m_expLabel;
};

#endif // OBJECTSSPELLSDIALOG_H
