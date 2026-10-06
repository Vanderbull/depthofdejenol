#ifndef BESTIARYDIALOG_H
#define BESTIARYDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>

class QListWidget;
class QListWidgetItem;
class QTextEdit;
class QComboBox;
class QLabel;
class QPushButton;

// Bestiary: auto-populates the Library as monsters are encountered.
class BestiaryDialog : public QDialog {
    Q_OBJECT

public:
    explicit BestiaryDialog(QWidget *parent = nullptr);
    ~BestiaryDialog() override;

private slots:
    void onMonsterSelected(QListWidgetItem *item);
    void onFilterChanged(int index);
    void onExitClicked();

private:
    void setupUi();
    void loadBestiary();
    void refreshList();

    QListWidget *m_monsterList = nullptr;
    QTextEdit *m_descriptionText = nullptr;
    QComboBox *m_filterCombo = nullptr;
    QLabel *m_countLabel = nullptr;
    QPushButton *m_exitBtn = nullptr;

    // Monster name -> description
    QMap<QString, QString> m_monsters;
    // Monster name -> floor first encountered
    QMap<QString, int> m_monsterFloors;
};

#endif // BESTIARYDIALOG_H
