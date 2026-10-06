#ifndef NPCDIALOG_H
#define NPCDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QList>

class QListWidget;
class QListWidgetItem;
class QTextEdit;
class QLabel;
class QPushButton;

// An NPC with personality and dungeon hints.
struct NPC {
    QString name;
    QString role;
    QString personality;
    QString greeting;
    QStringList hints;      // dungeon hints
    QString location;       // where in the city
};

// NPC dialog: talk to city NPCs for personality and dungeon hints.
class NPCDialog : public QDialog {
    Q_OBJECT

public:
    explicit NPCDialog(QWidget *parent = nullptr);
    ~NPCDialog() override;

    // All NPCs in the city.
    static QList<NPC> allNPCs();

    // An NPC by name, or an invalid NPC (empty name) when not found.
    static NPC npcByName(const QString& name);

    // Hints from all NPCs.
    static QStringList allHints();

    // Hints from a specific NPC.
    static QStringList hintsFrom(const QString& name);

private slots:
    void onNPCSelected(QListWidgetItem *item);
    void onTalkClicked();
    void onHintClicked();
    void onExitClicked();

private:
    void setupUi();
    void refreshList();
    void refreshDetail();

    QListWidget *m_npcList = nullptr;
    QTextEdit *m_detailText = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_roleLabel = nullptr;
    QPushButton *m_talkBtn = nullptr;
    QPushButton *m_hintBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;
};

#endif // NPCDIALOG_H
