#include "selftest_harness.h"
#include "src/network_manager/NetworkManager.h"
#include "src/journal_dialog/JournalDialog.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>
#include <QHostAddress>
#include <QElapsedTimer>
#include <QCoreApplication>

// Minimal test server for multiplayer tests. Captures what the client writes so
// the tests can assert on the wire bytes rather than on "it did not crash".
class TestServer : public QObject {
    Q_OBJECT
public:
    TestServer(QObject *parent = nullptr) : QObject(parent) {
        m_server = new QTcpServer(this);
        connect(m_server, &QTcpServer::newConnection, this, &TestServer::acceptClient);
    }
    bool start(quint16 port) {
        return m_server->listen(QHostAddress::Any, port);
    }
    void stop() {
        if (m_socket) {
            m_socket->disconnectFromHost();
            m_socket->deleteLater();
            m_socket = nullptr;
        }
        m_server->close();
    }
    quint16 port() const { return m_server->serverPort(); }
    QTcpServer *server() { return m_server; }

    // Everything the client has sent so far.
    QByteArray received() const { return m_received; }
    int connectionCount() const { return m_connections; }

    // Process events until the client has sent something or the timeout passes.
    bool waitForData(int timeoutMs) {
        QElapsedTimer t;
        t.start();
        while (m_received.isEmpty() && t.elapsed() < timeoutMs) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        }
        return !m_received.isEmpty();
    }

private slots:
    void acceptClient() {
        while (QTcpSocket *s = m_server->nextPendingConnection()) {
            m_connections++;
            m_socket = s;
            connect(s, &QTcpSocket::readyRead, this, &TestServer::readData);
            connect(s, &QTcpSocket::disconnected, this, &QTcpSocket::deleteLater);
        }
    }
    void readData() {
        if (!m_socket) return;
        m_received += m_socket->readAll();
    }

private:
    QTcpServer *m_server = nullptr;
    QTcpSocket *m_socket = nullptr;
    QByteArray m_received;
    int m_connections = 0;
};

void runMultiplayerTests() {
    section("[120] Multiplayer: NetworkManager singleton");
    {
        NetworkManager *nm1 = NetworkManager::instance();
        NetworkManager *nm2 = NetworkManager::instance();
        check(nm1 == nm2, "NetworkManager is a singleton");
        check(nm1 != nullptr, "NetworkManager instance is not null");
    }

    section("[121] Multiplayer: sendAction produces valid JSON envelope");
    {
        TestServer server;
        const bool started = server.start(0); // random port
        check(started, "Test TCP server starts on random port");
        if (!started) return;

        NetworkManager *nm = NetworkManager::instance();
        nm->connectToServer("127.0.0.1", server.port());

        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        timer.setInterval(5000);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(nm, &NetworkManager::connected, &loop, &QEventLoop::quit);
        timer.start();
        loop.exec();

        check(nm->state() == QAbstractSocket::ConnectedState,
              "client connects for the envelope test");
        check(server.connectionCount() >= 1, "the server accepted the connection",
              QString::number(server.connectionCount()));

        nm->sendAction("test", {{"key", "value"}});
        check(server.waitForData(3000), "sendAction writes bytes to the socket");

        const QByteArray raw = server.received();
        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        check(doc.isObject(), "the payload on the wire is a JSON object",
              QString::fromUtf8(raw.left(80)));
        if (doc.isObject()) {
            const QJsonObject o = doc.object();
            check(o.value("action").toString() == "test",
                  "the envelope carries the action name");
            check(o.value("key").toString() == "value",
                  "the envelope carries the payload",
                  o.value("key").toString());
        }

        nm->sendAction("disconnect", {});
        server.stop();
    }

    section("[122] Multiplayer: connectToServer with invalid host");
    {
        NetworkManager *nm = NetworkManager::instance();
        nm->connectToServer("invalid.host.example", 9999);

        // Wait for the attempt to fail, then reset.
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        timer.setInterval(3000);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(nm, &NetworkManager::disconnected, &loop, &QEventLoop::quit);
        timer.start();
        loop.exec();

        // A refused/failed attempt must leave the manager disconnected, never
        // stuck half-open in ConnectingState.
        check(nm->state() == QAbstractSocket::UnconnectedState,
              "an invalid host leaves the manager disconnected",
              QString::number(nm->state()));

        nm->sendAction("disconnect", {}); // ensure clean state
    }

    section("[123] Multiplayer: state returns valid socket state");
    {
        NetworkManager *nm = NetworkManager::instance();
        QAbstractSocket::SocketState state = nm->state();
        check(state == QAbstractSocket::UnconnectedState || state == QAbstractSocket::ConnectingState,
              "state() returns a valid socket state",
              QString::number(state));
    }

    section("[124] Multiplayer: local TCP server + client connection");
    {
        TestServer server;
        bool started = server.start(0); // random port
        check(started, "Test TCP server starts on random port");
        if (!started) return;

        NetworkManager *nm = NetworkManager::instance();
        nm->connectToServer("127.0.0.1", server.port());

        // Wait for connection with timeout
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        timer.setInterval(5000);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(nm, &NetworkManager::connected, &loop, &QEventLoop::quit);
        timer.start();
        loop.exec();

        check(nm->state() == QAbstractSocket::ConnectedState,
              "Client connects to local TCP server");

        // Send a chat message and confirm it reaches the server on the wire.
        nm->sendAction("chat", {{"message", "hello"}});
        check(server.waitForData(3000), "the chat action reaches the server");
        {
            const QJsonDocument doc = QJsonDocument::fromJson(server.received());
            check(doc.isObject() && doc.object().value("action").toString() == "chat",
                  "the chat envelope arrives intact",
                  QString::fromUtf8(server.received().left(80)));
        }

        // Disconnect
        nm->sendAction("disconnect", {});
        server.stop();
        check(!server.server()->isListening(), "Test server stops cleanly");
    }

    section("[125] Multiplayer: journal system works");
    {
        JournalDialog::clearAll();
        JournalDialog::addEntry("Test", "Test entry 1");
        JournalDialog::addEntry("Test", "Test entry 2");
        JournalDialog::addEntry("Combat", "Combat entry");

        QList<JournalEntry> entries = JournalDialog::allEntries();
        check(entries.size() == 3, "Journal stores 3 entries",
              QString::number(entries.size()));

        bool foundTest = false;
        bool foundCombat = false;
        for (const JournalEntry& e : entries) {
            if (e.category == "Test" && e.text == "Test entry 1") foundTest = true;
            if (e.category == "Combat" && e.text == "Combat entry") foundCombat = true;
        }
        check(foundTest, "Journal finds Test entry");
        check(foundCombat, "Journal finds Combat entry");

        JournalDialog::clearAll();
        entries = JournalDialog::allEntries();
        check(entries.size() == 0, "Journal clearAll removes all entries");
    }
}

#include "selftest_multiplayer.moc"
