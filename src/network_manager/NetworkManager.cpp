#include "NetworkManager.h"
#include <QTcpSocket>    // Included here instead of the header
#include <QJsonDocument> // Included here instead of the header
#include <QJsonObject>   // Included here instead of the header
#include <QVariantMap>   // Included here instead of the header
#include <QDebug>

NetworkManager* NetworkManager::m_instance = nullptr;

NetworkManager* NetworkManager::instance() {
    if (!m_instance) m_instance = new NetworkManager();
    return m_instance;
}

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
}

void NetworkManager::connectToServer(const QString &host, quint16 port) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->connectToHost(host, port);
    }
}

void NetworkManager::sendAction(const QString &action, const QVariantMap &data) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;

    QJsonObject obj = QJsonObject::fromVariantMap(data);
    obj["action"] = action;

    QJsonDocument doc(obj);
    // Newline acting as packet delimiter
    m_socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

void NetworkManager::onReadyRead() {
    while (m_socket->canReadLine()) {
        QByteArray line = m_socket->readLine();
        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (doc.isNull()) continue;

        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        // Route incoming server packets to the correct signals
        if (type == "chat") {
            emit chatReceived(obj["sender"].toString(), obj["message"].toString());
        } else if (type == "player_join") {
            emit playerJoined(obj["username"].toString());
        } else if (type == "player_leave") {
            emit playerLeft(obj["username"].toString());
        } else if (type == "zone_sync") {
            emit zoneUpdateReceived(obj["data"].toVariant().toMap());
        }
    }
}

void NetworkManager::onConnected() {
    qDebug() << "Connected to game server.";
    emit connected();
}

void NetworkManager::onDisconnected() {
    qDebug() << "Disconnected from game server.";
    emit disconnected();
}

QAbstractSocket::SocketState NetworkManager::state() const {
    return m_socket->state();
}
