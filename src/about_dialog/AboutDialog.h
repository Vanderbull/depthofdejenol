#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <memory>
// Forward declarations
class GameStateManager;

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

private:
    void setupUi();
    static QString getGameVersionInfo();
};

#endif // ABOUTDIALOG_H
