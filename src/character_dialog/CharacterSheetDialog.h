#ifndef CHARACTERSHEETDIALOG_H
#define CHARACTERSHEETDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QObject>

class QLabel;
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

    // v6.1k: Additional character data - spouse, children, status effects
    QLabel *m_spouseLabel = nullptr;
    QLabel *m_childrenLabel = nullptr;
    QLabel *m_statusLabel = nullptr;

    // v6.1k: Companion skills
    QTableWidget *m_companionTable = nullptr;
    QLabel *m_companionPointsLabel = nullptr;

    // v6.1k: Inventory
    QTableWidget *m_inventoryTable = nullptr;
    QLabel *m_inventoryPointsLabel = nullptr;

    QTableWidget *m_statsTable = nullptr;
    QTableWidget *m_equipTable = nullptr;
    QTableWidget *m_guildTable = nullptr;
    QTextEdit *m_spellsText = nullptr;
    QPushButton *m_exitBtn = nullptr;
    // v3.0.0 slice 3.4: per-stat spend buttons, one per stats-table row
    QList<QPushButton*> m_spendButtons;

    // v6.1.0 slice 6.1a: skill allocation
    QTableWidget *m_skillTable = nullptr;
    QPushButton *m_learnSkillButton = nullptr;
    QLabel *m_skillPointsLabel = nullptr;

private slots:
    void onMemberChanged(int index);
    void onExitClicked();
    void onSpendStatPoint(int statIndex);
    void onLearnSkill();
};

#endif // CHARACTERSHEETDIALOG_H
