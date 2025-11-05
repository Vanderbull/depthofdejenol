#ifndef GUILDSDIALOG_H
#define GUILDSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

class GuildsDialog : public QDialog
{
    Q_OBJECT

public:
    GuildsDialog(QWidget *parent = nullptr);
    ~GuildsDialog();

private slots:
    // Slots for Left Side Buttons
    void on_makeLevelButton_clicked();
    void on_reAcquaintButton_clicked();
    void on_visitLibraryButton_clicked();

    // Slots for Right Side Buttons
    void on_expInfoButton_clicked();
    void on_readGuildLogButton_clicked();
    void on_visitButton_clicked();
    void on_exitButton_clicked();

private:
    // Widgets for the left side
    QLabel *welcomeLabel;
    QLabel *guildMasterLabel;
    QLineEdit *guildMasterLineEdit;
    QLabel *statsRequiredLabel;
    QLabel *statsLabel;
    QLabel *tooLowLabel;

    QPushButton *makeLevelButton;
    QPushButton *reAcquaintButton;
    QPushButton *visitLibraryButton;

    // Widgets for the right side
    QLabel *guildsLabel;
    QListWidget *guildsListWidget;

    QPushButton *expInfoButton;
    QPushButton *readGuildLogButton;
    QPushButton *visitButton;
    QPushButton *exitButton;

    void setupConnections(); // Utility function to connect signals and slots
};

#endif // GUILDSDIALOG_H
