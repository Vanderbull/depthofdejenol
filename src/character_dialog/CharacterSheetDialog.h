#ifndef CHARACTERSHEETDIALOG_H
#define CHARACTERSHEETDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>

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

private slots:
    void onMemberChanged(int index);
    void onExitClicked();

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
    QLabel *m_goldLabel = nullptr;
    QTableWidget *m_statsTable = nullptr;
    QTableWidget *m_equipTable = nullptr;
    QTableWidget *m_guildTable = nullptr;
    QTextEdit *m_spellsText = nullptr;
    QPushButton *m_exitBtn = nullptr;
};

#endif // CHARACTERSHEETDIALOG_H
