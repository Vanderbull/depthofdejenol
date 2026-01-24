#ifndef MORGUEDIALOG_H
#define MORGUEDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QList>

class QLabel;
class QPushButton;

// Structure to track character state from files
struct DeadCharacterInfo {
    QString fileName;
    bool inCity;
    int dungeonLevel;
};

class MorgueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MorgueDialog(QWidget *parent = nullptr);
    ~MorgueDialog() override;

private slots:
    void onActionClicked();

private:
    void setupUi();
    QList<DeadCharacterInfo> fetchDeadCharacterData() const;
    bool resurrectCharacter(const QString &fileName);
    bool moveBodyToCity(const QString &fileName);
    int calculateRescueCost(int level) const;

    QLabel *m_welcomeLabel = nullptr;
    QPushButton *m_raiseBtn = nullptr;
    QPushButton *m_hireBtn = nullptr;
    QPushButton *m_grabBtn = nullptr;
    QPushButton *m_exitBtn = nullptr;

    QString m_selectedCharacter; 
};

#endif // MORGUEDIALOG_H
