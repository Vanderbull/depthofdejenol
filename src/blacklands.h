#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <QWidget>
#include <QDialog>
#include <QSettings>

#include "./src/event/EventManager.h"

class QLabel;
class QPushButton;
class QListWidget;

class GameMenu : public QWidget {
    Q_OBJECT

public:
    explicit GameMenu(QWidget *parent = nullptr);
    ~GameMenu();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void startNewGame();
    void loadGame();
    void saveGame();
    void showRecords();
    void quitGame();
    void onOptionsClicked();
    void onContinueClicked();
    void enterCity();
    void enterDungeon();
    void onEventTriggered(const GameEvent& event);
    void onCharacterCreated(const QString &characterName);
    void onCreateCharacterClicked();
    void onAssemblePartyClicked();
    void onMultiplayerClicked();
    void onTutorialClicked();
    // Re-applies the app font's family to the widgets that carry a font of
    // their own, so the menu follows the font chosen in Options.
    void syncFontsToAppFont();

private:
    void setupUI();
    void setupConnections();
    void loadStyleSheet();
    void loadBackgroundImage();
    void refreshContinueState();
    QString partyPreviewText() const;


    QLabel *m_partyPreviewLabel = nullptr;
    QPushButton *m_continueButton = nullptr;
    QPushButton *m_newButton = nullptr;
    QPushButton *m_createCharacterButton = nullptr;
    QPushButton *m_assemblePartyButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_loadButton = nullptr;
    QPushButton *m_recordsButton = nullptr;
    QPushButton *m_optionsButton = nullptr;
    QPushButton *m_multiplayerButton = nullptr;
    QPushButton *m_tutorialButton = nullptr;
    QPushButton *m_quitButton = nullptr;
    QSettings m_settings;
    QString m_subfolderName;
    QPixmap m_backgroundPixmap;

signals:

};

#endif // GAMEMENU_H
