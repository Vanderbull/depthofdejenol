#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPushButton>

class GameMenu : public QWidget {
    Q_OBJECT

public:
    explicit GameMenu(QWidget *parent = nullptr);
    ~GameMenu();

private slots:
    void startNewGame();
    void loadGame();
    void showRecords();
    void showCredits();
    void quitGame();
    void onInventoryClicked();

private:
    QLabel *titleLabel;
    QLabel *menuArtLabel;
    QPushButton *newButton;
    QPushButton *loadButton;
    QPushButton *recordsButton;
    QPushButton *creditsButton;
    QPushButton *quitButton;
};

#endif // GAMEMENU_H
