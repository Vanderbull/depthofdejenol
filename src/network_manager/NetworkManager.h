#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QAbstractSocket>
#include <QVariant>
#include <QMap>
#include <QString>


// Forward declarations to reduce header weight
class QTcpSocket;
class QString;
template <typename K, typename V> class QMap;
typedef QMap<QString, QVariant> QVariantMap;

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

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();

private:
    explicit NetworkManager(QObject *parent = nullptr);
    static NetworkManager* m_instance;
    QTcpSocket *m_socket;
};

#endif // NETWORKMANAGER_H
