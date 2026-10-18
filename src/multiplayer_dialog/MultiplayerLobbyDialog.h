#ifndef MULTIPLAYERLOBBYDIALOG_H
#define MULTIPLAYERLOBBYDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVariantMap>

class MultiplayerLobbyDialog : public QDialog {
    Q_OBJECT

public:
    explicit MultiplayerLobbyDialog(QWidget *parent = nullptr);
    ~MultiplayerLobbyDialog();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onStartServerClicked();
    void onChatSend();
    void onMoveClicked();
    void onFightClicked();
    void onConnected();
    void onDisconnected();
    void onChatReceived(const QString &from, const QString &message);
    void onPlayerJoined(const QString &playerName);
    void onPlayerLeft(const QString &playerName);
    void onConnectionAttemptFailed(int attempt, int backoffMs);
    void onConnectionReestablished();
    void onZoneUpdateReceived(const QVariantMap &zoneData);

private:
    void setupUI();
    void setupConnections();
    void updateStatus(const QString &status);
    void addChatMessage(const QString &from, const QString &message);
    void refreshPlayerList();

    // UI elements
    QLineEdit *m_hostInput = nullptr;
    QLineEdit *m_portInput = nullptr;
    QLineEdit *m_usernameInput = nullptr;
    QPushButton *m_connectButton = nullptr;
    QPushButton *m_disconnectButton = nullptr;
    QPushButton *m_startServerButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    QListWidget *m_playerList = nullptr;
    QTextEdit *m_chatDisplay = nullptr;
    QLineEdit *m_chatInput = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_moveButton = nullptr;
    QPushButton *m_fightButton = nullptr;

    bool m_connected = false;
};

#endif // MULTIPLAYERLOBBYDIALOG_H
