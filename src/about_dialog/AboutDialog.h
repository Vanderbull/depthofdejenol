#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

// Forward declarations
class GameStateManager;

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    void setupUi();
    QString getGameVersionInfo() const;
};

#endif // ABOUTDIALOG_H
