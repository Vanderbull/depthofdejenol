#include "NetworkManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>
#include <cmath>
#include <algorithm>

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

    // Ping timer: keepalive
    m_pingTimer.setInterval(10 * 1000); // 10s by default
    connect(&m_pingTimer, &QTimer::timeout, this, &NetworkManager::onPingTimeout);

    // Reconnect timer (single-shot used with backoff)
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &NetworkManager::attemptReconnect);
}

void NetworkManager::connectToServer(const QString &host, quint16 port) {
    m_host = host;
    m_port = port;
    m_reconnectAttempts = 0;

    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->connectToHost(host, port);
    }
}

void NetworkManager::sendAction(const QString &action, const QVariantMap &data) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;

    // Envelope the action with protocol metadata
    QJsonObject obj = QJsonObject::fromVariantMap(data);
    obj["action"] = action;
    obj["protocol"] = m_protocolVersion;
    // Client-side timestamp; server should provide authoritative seq
    obj["ts"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QJsonDocument doc(obj);
    QByteArray payload = doc.toJson(QJsonDocument::Compact) + "\n"; // newline framing (kept for compatibility)
    m_socket->write(payload);
}

void NetworkManager::onReadyRead() {
    // Append all available data to buffer
    m_recvBuffer.append(m_socket->readAll());

    // Safety: prevent unbounded buffer growth
    const int MAX_BUFFER_SIZE = 1 << 20; // 1 MiB
    if (m_recvBuffer.size() > MAX_BUFFER_SIZE) {
        qWarning() << "NetworkManager: receive buffer exceeded maximum size, clearing buffer for safety.";
        m_recvBuffer.clear();
        return;
    }

    // Process newline-delimited messages robustly
    while (true) {
        int idx = m_recvBuffer.indexOf('\n');
        if (idx == -1) break;

        QByteArray line = m_recvBuffer.left(idx);
        m_recvBuffer.remove(0, idx + 1);

        // Ignore empty lines
        if (line.trimmed().isEmpty()) continue;

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);
        if (doc.isNull() || parseError.error != QJsonParseError::NoError) {
            qWarning() << "NetworkManager: failed to parse JSON message:" << parseError.errorString();
            continue;
        }
        if (!doc.isObject()) {
            qWarning() << "NetworkManager: unexpected JSON type (object required)";
            continue;
        }

        QJsonObject obj = doc.object();

        // Basic validation of envelope
        int protocol = obj.value("protocol").toInt(-1);
        if (protocol != m_protocolVersion) {
            qWarning() << "NetworkManager: protocol mismatch. Expected" << m_protocolVersion << "got" << protocol;
            // Optionally emit a signal or request protocol negotiation
            continue;
        }

        QString type = obj.value("type").toString();
        if (type == "chat") {
            QString sender = obj.value("sender").toString();
            QString message = obj.value("message").toString();
            // Basic sanitization/truncation
            if (message.size() > 1024) message = message.left(1024);
            emit chatReceived(sender, message);
        } else if (type == "player_join") {
            emit playerJoined(obj.value("username").toString());
        } else if (type == "player_leave") {
            emit playerLeft(obj.value("username").toString());
        } else if (type == "zone_sync") {
            QVariantMap zoneData = obj.value("data").toVariant().toMap();
            emit zoneUpdateReceived(zoneData);
        } else if (type == "pong") {
            // keepalive response
            // could update last-seen timestamp
        } else {
            qDebug() << "NetworkManager: unhandled message type:" << type;
        }
    }
}

void NetworkManager::onConnected() {
    qDebug() << "Connected to game server.";
    m_reconnectAttempts = 0;
    m_pingTimer.start();
    emit connected();
    emit connectionReestablished();
}

void NetworkManager::onDisconnected() {
    qDebug() << "Disconnected from game server.";
    m_pingTimer.stop();
    emit disconnected();

    // schedule reconnect with exponential backoff
    m_reconnectAttempts++;
    int cappedAttempts = std::min(6, m_reconnectAttempts);
    int backoffMs = std::min(30000, int(1000 * std::pow(2.0, double(cappedAttempts)))); // up to 30s
    emit connectionAttemptFailed(m_reconnectAttempts, backoffMs);
    m_reconnectTimer.start(backoffMs);
}

void NetworkManager::onPingTimeout() {
    // send a ping; server should reply with a 'pong' message
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject ping;
        ping["type"] = "ping";
        ping["protocol"] = m_protocolVersion;
        ping["ts"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        QJsonDocument doc(ping);
        m_socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
    }
}

void NetworkManager::attemptReconnect() {
    if (m_host.isEmpty() || m_port == 0) return;
    qDebug() << "NetworkManager: attempting reconnect, attempt" << m_reconnectAttempts;
    if (m_socket->state() == QAbstractSocket::ConnectedState) return;
    m_socket->abort(); // reset socket
    m_socket->connectToHost(m_host, m_port);
}

QAbstractSocket::SocketState NetworkManager::state() const {
    return m_socket->state();
}