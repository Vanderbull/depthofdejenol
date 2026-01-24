#ifndef MORGUEDIALOG_H
#define MORGUEDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QList>

class QLabel;
class QPushButton;

// Metadata for dead characters found in the save folder
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
    
    // File manipulation helpers
    bool updateCharacterFile(const QString &fileName, bool resurrect);
    bool moveBodyToCityInFile(const QString &fileName);
    
    int calculateRescueCost(int level) const;

    QLabel *m_welcomeLabel;
    QPushButton *m_raiseBtn;
    QPushButton *m_hireBtn;
    QPushButton *m_grabBtn;
    QPushButton *m_exitBtn;
};

#endif // MORGUEDIALOG_H
