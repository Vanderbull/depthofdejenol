#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

#include "./src/event/EventManager.h"

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
    // Called when the EXTRAS menu item is activated.
    void onExtrasClicked();
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
    QLabel *m_pressHintLabel = nullptr;
    QLabel *m_pressHintLabelRight = nullptr;
    QWidget *m_sidebar = nullptr;
    QVBoxLayout *m_sidebarLayout = nullptr;
    QPushButton *m_sidebarContinueButton = nullptr;
    QPushButton *m_sidebarNewGameButton = nullptr;
    QPushButton *m_sidebarLoadGameButton = nullptr;
    QPushButton *m_sidebarOptionsButton = nullptr;
    QPushButton *m_sidebarExtrasButton = nullptr;
    QPushButton *m_sidebarQuitButton = nullptr;
    QWidget *m_content = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QLabel *m_continueSubtitleLabel = nullptr;
    QLabel *m_sidebarContinueSubtitleLabel = nullptr;
    QPushButton *m_extrasButton = nullptr;
    QLabel *m_logoLabel1 = nullptr;
    QLabel *m_logoLabel2 = nullptr;
    QLabel *m_menuTitleLabel = nullptr;
    QLabel *m_sidebarTitleLabel = nullptr;
    QSettings m_settings;
    QString m_subfolderName;
    QPixmap m_backgroundPixmap;

signals:

};

#endif // GAMEMENU_H
