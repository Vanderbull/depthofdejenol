#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include "GameStateManager.h" // Include the GameStateManager header

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    void setupUi();
    QString getGameVersionInfo() const; // New private method to fetch game version/gold
};

#endif // ABOUTDIALOG_H
