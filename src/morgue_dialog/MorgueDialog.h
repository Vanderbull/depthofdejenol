#ifndef MORGUEDIALOG_H
#define MORGUEDIALOG_H

#include <QDialog>
#include <QStringList>

class QLabel;
class QPushButton;

class MorgueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MorgueDialog(QWidget *parent = nullptr);
    ~MorgueDialog() override;

private slots:
    // Unified slot to handle all morgue actions (Raise, Hire, Grab)
    void onActionClicked();

private:
    void setupUi();
    QStringList fetchDeadCharacterFiles() const;
    bool updateCharacterVitality(const QString &fileName, bool alive);

    // UI Members
    QLabel *m_welcomeLabel = nullptr;
    QPushButton *m_raiseBtn = nullptr;
    QPushButton *m_hireBtn = nullptr;
    QPushButton *m_grabBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;

    QString m_selectedCharacter; 
};

#endif // MORGUEDIALOG_H
