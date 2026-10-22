#ifndef CHARACTERSHEETDIALOG_H
#define CHARACTERSHEETDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QObject>
#include <QVBoxLayout>

class QLabel;
class QListWidget;
class QPushButton;
class QTableWidget;
class QTextEdit;
class QComboBox;

// Character sheet: equipped items, effective stats, guild levels, known spells.
class CharacterSheetDialog : public QDialog {
    Q_OBJECT

public:
    explicit CharacterSheetDialog(QWidget *parent = nullptr);
    ~CharacterSheetDialog() override;

private slots:
    void onMemberChanged(int index);
    void onExitClicked();
    void onSpendStatPoint(int statIndex);
    void onTameCompanion();
    void onSellCompanion();

private:
    void setupUi();
    void refreshSheet();

    QComboBox *m_memberCombo = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_raceLabel = nullptr;
    QLabel *m_classLabel = nullptr;
    QLabel *m_levelLabel = nullptr;
    QLabel *m_xpLabel = nullptr;
    QLabel *m_hpLabel = nullptr;
    QLabel *m_manaLabel = nullptr;
    QLabel *m_hungerLabel = nullptr;
    QLabel *m_goldLabel = nullptr;
    QLabel *m_statPointsLabel = nullptr;
    QLabel *m_playTimeLabel = nullptr;
    QTableWidget *m_statsTable = nullptr;
    QTableWidget *m_equipTable = nullptr;
    QTableWidget *m_guildTable = nullptr;
    QListWidget *m_spellsList = nullptr;
    QLabel *m_spellsText = nullptr;
    // v6.14.0 slice 2: the class's granted spell list, which is a different
    // thing from the guild/level-gated "Known Spells" list above it.
    QListWidget *m_classSpellsList = nullptr;
    QLabel *m_classSpellsText = nullptr;
    QPushButton *m_exitBtn = nullptr;
    // v3.0.0 slice 3.4: per-stat spend buttons, one per stats-table row
    QList<QPushButton*> m_spendButtons;
    QListWidget *m_companionsList = nullptr;
    QLabel *m_companionsText = nullptr;
    QPushButton *m_tameBtn = nullptr;
    QPushButton *m_sellCompBtn = nullptr;
    // v6.15.0 slice 0: companion group layout reference
    QVBoxLayout *m_mainLayout = nullptr;
};

#endif // CHARACTERSHEETDIALOG_H
