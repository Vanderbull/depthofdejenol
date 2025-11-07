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
    void onMarlithClicked();
    void onOptionsClicked();
    void onAboutClicked();
    void onEditMonsterClicked();
    void onEditSpellbookClicked();
    void onCharacterListClicked();
    void onHelpClicked();
    void onShowStatisticsClicked();
    void onRunClicked();
    void toggleMenuState();
//    void onUnloadClicked();
//    void onExchangeClicked();
//    void onChangePasswordClicked();
//    void onDeleteClicked();

private:
    QLabel *titleLabel;
    QLabel *menuArtLabel;
    QPushButton *newButton;
    QPushButton *exitButton;
    QPushButton *helpButton;

    QPushButton *loadButton;
    QPushButton *recordsButton;
    QPushButton *creditsButton;
    QPushButton *quitButton;
    QPushButton *marlithButton;
    QPushButton *optionsButton;
    QPushButton *aboutButton;
    QPushButton *spellbookButton;
    QPushButton *characterListButton;
    QPushButton *statisticsButton;
    QPushButton *DungeonDialogButton;
    QPushButton *runButton;
    QPushButton *UnloadCharacterButton;
    QPushButton *ExchangeItemsButton;
    QPushButton *ChangePasswordButton;
    QPushButton *DeleteCharacterButton;

signals:
    /**
     * @brief Signal emitted when a message needs to be logged.
     * @param message The string to be logged by the MessagesWindow.
     */
    void logMessageTriggered(const QString &message);
};

#endif // GAMEMENU_H
