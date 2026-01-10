#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    static NetworkManager* instance();
    
    void connectToServer(const QString &host, quint16 port);
    void sendAction(const QString &action, const QVariantMap &data);

signals:
    void connected();
    void disconnected();
    void playerJoined(QString playerName);
    void playerLeft(QString playerName);
    void chatReceived(QString from, QString message);
    void zoneUpdateReceived(QVariantMap zoneData);

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
