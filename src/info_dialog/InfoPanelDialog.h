#ifndef INFOPANELDIALOG_H
#define INFOPANELDIALOG_H

#include <QDialog>
#include <QString>

class QLabel;
class QTextEdit;
class QPushButton;

// Info panel dispatcher: F1 = Help, F2 = Area Look, F4 = Resistance,
// F8 = Buffer Information. Reuses existing panels for F3/F6/F7/F10.
class InfoPanelDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InfoPanelDialog(QWidget *parent = nullptr);
    ~InfoPanelDialog() override;

private slots:
    void onExitClicked();
    void showHelp();       // F1
    void showAreaLook();   // F2
    void showResistance(); // F4
    void showBuffer();     // F8

private:
    void setupUi();
    QLabel *m_titleLabel = nullptr;
    QTextEdit *m_body = nullptr;
    QPushButton *m_exitBtn = nullptr;
};

#endif // INFOPANELDIALOG_H
