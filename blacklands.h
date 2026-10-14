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

private slots:
    void startNewGame();
    void loadGame();
    void saveGame();
    void showRecords();
//    void showCredits();
    void quitGame();
//    void onInventoryClicked();
//    void onMarlithClicked();
    void onOptionsClicked();
  //  void onEditMonsterClicked();
  //  void onEditSpellbookClicked();
  //  void onShowStatisticsClicked();
    void onContinueClicked();
    void enterCity();
    void onEventTriggered(const GameEvent& event);
    void onCharacterCreated(const QString &characterName);
    void onCreateCharacterClicked();
    void onAssemblePartyClicked();

private:
    void setupUI();
    void setupConnections();
    void loadStyleSheet();
    void loadBackgroundImage();
    void refreshContinueState();
    QString partyPreviewText() const;

    QLabel *m_titleLabel = nullptr;
    QLabel *m_versionLabel = nullptr;
    QLabel *m_partyPreviewLabel = nullptr;
    QPushButton *m_continueButton = nullptr;
    QPushButton *m_newButton = nullptr;
    QPushButton *m_createCharacterButton = nullptr;
    QPushButton *m_assemblePartyButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_loadButton = nullptr;
    QPushButton *m_recordsButton = nullptr;
    QPushButton *m_optionsButton = nullptr;
    QPushButton *m_quitButton = nullptr;
    QSettings m_settings;
    QString m_subfolderName;
    QPixmap m_backgroundPixmap;

signals:
    void logMessageTriggered(const QString &message);
};

#endif // GAMEMENU_H
