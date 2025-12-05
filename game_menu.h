#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPushButton>

// --- FIX 1: Include full class definitions for member variables ---
#include <QSettings>     // For QSettings
#include <QMediaPlayer>  // For QMediaPlayer
#include <QAudioOutput>  // For QAudioOutput

#include "./src/event/EventManager.h" 

class GameMenu : public QWidget {
    Q_OBJECT

public:
    explicit GameMenu(QWidget *parent = nullptr);
    ~GameMenu();

protected:
    void resizeEvent(QResizeEvent *event) override;

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
    void toggleMenuState(bool characterIsLoaded); 
    void onEventTriggered(const GameEvent& event);

private:
    // ... other UI elements ...
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subTitleLabel = nullptr;
    QPushButton *m_newButton = nullptr;
    QPushButton *m_exitButton = nullptr;
    QPushButton *m_helpButton = nullptr;
    QPushButton *m_loadButton = nullptr;
    QPushButton *m_recordsButton = nullptr;
    QPushButton *m_optionsButton = nullptr;
    QPushButton *m_aboutButton = nullptr;
    QPushButton *m_characterListButton = nullptr;
    QPushButton *m_runButton = nullptr;
    
    // Member variables for state/config
    // QSettings is now fully defined
    QSettings m_settings;
    QString m_subfolderName;
    QPixmap m_backgroundPixmap; 
    
    // Audio components (pointers are fine with full definitions)
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;

signals:
    void logMessageTriggered(const QString &message);
};

#endif // GAMEMENU_H
