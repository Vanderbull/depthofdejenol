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
    // --- Slice 5: multiplayer hardening ---
    void resyncRequested(const QString& source);
    void syncAcknowledged();
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

    // --- Slice 5: multiplayer hardening ---
    // Track last seen seq per source (for zone_sync) to detect gaps
    QMap<QString, int> m_lastZoneSeq;
    // Sequence number for our own zone_sync messages
    int m_zoneSyncSeq = 0;
    // Whether a system-wide sync is in progress (prevents cascade re-requests)
    bool m_syncInProgress = false;
    // Sanitize chat control characters
    QString sanitizeChatMessage(const QString& message) const;

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
