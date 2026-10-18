#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QAbstractSocket>
#include <QTimer>
#include <QByteArray>
#include <QVariantMap>

// Forward declarations to reduce header weight
class QTcpSocket;

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    static NetworkManager* instance();

    void connectToServer(const QString &host, quint16 port);
    void sendAction(const QString &action, const QVariantMap &data);
    QAbstractSocket::SocketState state() const;

signals:
    void connected();
    void disconnected();
    // Use const references to avoid unnecessary object copies
    void playerJoined(const QString &playerName);
    void playerLeft(const QString &playerName);
    void chatReceived(const QString &from, const QString &message);
    void zoneUpdateReceived(const QVariantMap &zoneData);

    // Additional signals
    void connectionAttemptFailed(int attempt, int backoffMs);
    void connectionReestablished();

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onPingTimeout();
    void attemptReconnect();

private:
    explicit NetworkManager(QObject *parent = nullptr);
    static NetworkManager* m_instance;
    QTcpSocket *m_socket;

    // New members for robustness & reconnect
    QByteArray m_recvBuffer;
    QTimer m_pingTimer;
    QTimer m_reconnectTimer;
    int m_reconnectAttempts = 0;
    const int m_protocolVersion = 1;
    QString m_host;
    quint16 m_port = 0;
};

#endif // NETWORKMANAGER_H
