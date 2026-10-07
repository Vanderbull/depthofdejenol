#ifndef BESTIARYDIALOG_H
#define BESTIARYDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariantMap>

class QListWidget;
class QListWidgetItem;
class QTextEdit;
class QComboBox;
class QLabel;
class QPushButton;
class QLineEdit;

// Bestiary: the monster reference in the Library.
//
// Reads data/bestiary.json, which tools/gen_bestiary.py builds from the two
// sources already in the repo: MDATA5.csv (every number) and
// other/walkthrough.txt (category headings, abilities and the prose detail
// blocks). The picture in MDATA5's picID column selects
// resources/images/MON<picID>.jpg and is what ties the two together.
class BestiaryDialog : public QDialog {
    Q_OBJECT

public:
    explicit BestiaryDialog(QWidget *parent = nullptr);
    ~BestiaryDialog() override;

    // Data access, shared by the dialog and the self-tests. entries() is
    // empty when data/bestiary.json is missing or unreadable.
    static const QList<QVariantMap>& entries();
    static QStringList categories();
    static QVariantMap entry(const QString& name);
    // Resolved image path for an entry ("" when it has no picture).
    static QString imagePath(const QVariantMap& entry);

private slots:
    void onMonsterSelected(QListWidgetItem *item);
    void onFilterChanged(int index);
    void onSearchChanged(const QString& text);
    void onExitClicked();

private:
    void setupUi();
    void refreshList();
    void showEntry(const QString& name);

    QListWidget *m_monsterList = nullptr;
    QLabel *m_imageLabel = nullptr;
    QTextEdit *m_descriptionText = nullptr;
    QComboBox *m_filterCombo = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QLabel *m_countLabel = nullptr;
    QPushButton *m_exitBtn = nullptr;
};

#endif // BESTIARYDIALOG_H
