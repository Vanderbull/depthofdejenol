#ifndef THECITY_H
#define THECITY_H

#include <QDialog>
#include <QToolButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QMediaPlayer>
#include <QAudioOutput>

class TheCity : public QDialog
{
    Q_OBJECT

public:
    explicit TheCity(QWidget *parent = nullptr);
    ~TheCity();

private slots:
    // Existing Action Slots
    void on_generalStoreButton_clicked();
    void on_morgueButton_clicked();
    void on_guildsButton_clicked();
    void on_dungeonButton_clicked();
    void on_confinementButton_clicked();
    void on_seerButton_clicked();
    void on_bankButton_clicked();
    void on_exitButton_clicked();

    // Multiplayer Slots
    void sendChatMessage();
    void handlePlayerJoined(QString name);
    void handlePlayerLeft(QString name);
    void handleChatReceived(QString from, QString message);

private:
    QMediaPlayer *m_cityPlayer;
    QAudioOutput *m_cityAudioOutput;
    // UI Elements - Header & Grid
    QLabel *titleLabel;
    QToolButton *generalStoreButton;
    QToolButton *morgueButton;
    QToolButton *guildsButton;
    QToolButton *dungeonButton;
    QToolButton *confinementButton;
    QToolButton *seerButton;
    QToolButton *bankButton;
    QToolButton *exitButton;

    // UI Elements - Multiplayer
    QListWidget *playerList;
    QTextEdit *chatDisplay;
    QLineEdit *chatInput;
    QPushButton *sendButton;

    // Helper Functions
    void setupUi();
    void setupStyling();
    void loadButtonIcons();
    void setupMultiplayerConnections();

    // Standalone
    bool isOfflineMode = false;
    void startOfflineMode();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // THECITY_H
