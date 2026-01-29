#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

class GameServer : public QTcpServer {
    Q_OBJECT
public:
    GameServer(QObject *parent = nullptr) : QTcpServer(parent) {}

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket *client = new QTcpSocket(this);
        client->setSocketDescriptor(socketDescriptor);
        clients << client;

        qDebug() << "New connection from:" << client->peerAddress().toString();
        // Handle incoming packets
        connect(client, &QTcpSocket::readyRead, this, [this, client]() {
            while (client->canReadLine()) {
                QByteArray message = client->readLine();
                QJsonDocument doc = QJsonDocument::fromJson(message);
                
                if (doc.isNull()) continue;
                QJsonObject obj = doc.object();
                QString action = obj["action"].toString();
                // LOGIC: Player joins the city
                if (action == "enter_zone") {
                    QString username = obj["username"].toString();
                    if (username.isEmpty()) username = "Unknown Hero";
                    // Store the name on the socket object for later reference
                    client->setProperty("username", username);
                    qDebug() << "Player Identified:" << username;
                    // 1. Tell the NEW player about everyone who is ALREADY here
                    for (QTcpSocket *otherClient : clients) {
                        if (otherClient != client && otherClient->property("username").isValid()) {
                            QJsonObject syncObj;
                            syncObj["type"] = "player_join";
                            syncObj["username"] = otherClient->property("username").toString();
                            client->write(QJsonDocument(syncObj).toJson(QJsonDocument::Compact) + "\n");
                        }
                    }
                    // 2. Tell EVERYONE (including the new player) to add this player to the list
                    QJsonObject joinObj;
                    joinObj["type"] = "player_join";
                    joinObj["username"] = username;
                    broadcast(joinObj);
                }
                // LOGIC: Chat relay
                else if (action == "chat") {
                    QJsonObject chatObj;
                    chatObj["type"] = "chat";
                    chatObj["sender"] = client->property("username").toString();
                    chatObj["message"] = obj["message"].toString();
                    broadcast(chatObj);
                }
            }
        });
        // Cleanup and Notify on Disconnect
        connect(client, &QTcpSocket::disconnected, this, [this, client]() {
            QString username = client->property("username").toString();
            clients.removeOne(client);
            
            if (!username.isEmpty()) {
                QJsonObject leaveObj;
                leaveObj["type"] = "player_leave";
                leaveObj["username"] = username;
                broadcast(leaveObj);
            }
            qDebug() << "Player disconnected:" << username << ". Online:" << clients.size();
            client->deleteLater();
        });
    }

private:
    QList<QTcpSocket*> clients;
    // Helper to send JSON to every connected player
    void broadcast(const QJsonObject &obj) {
        QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
        for (QTcpSocket *client : clients) {
            if (client->state() == QAbstractSocket::ConnectedState) {
                client->write(data);
            }
        }
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    GameServer server;
    quint16 port = 12345;

    if (!server.listen(QHostAddress::Any, port)) {
        qCritical() << "Unable to start the server:" << server.errorString();
        return 1;
    }

    qDebug() << "---------------------------------------";
    qDebug() << " THE CITY - MULTIPLAYER SERVER ";
    qDebug() << " Running on port:" << port;
    qDebug() << "---------------------------------------";

    return a.exec();
}

#include "main.moc"
