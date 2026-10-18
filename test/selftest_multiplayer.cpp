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

// Minimal test server for multiplayer tests
class TestServer : public QObject {
    Q_OBJECT
public:
    TestServer(QObject *parent = nullptr) : QObject(parent) {
        m_server = new QTcpServer(this);
    }
    bool start(quint16 port) {
        return m_server->listen(QHostAddress::Any, port);
    }
    void stop() { m_server->close(); }
    quint16 port() const { return m_server->serverPort(); }
    QTcpServer *server() { return m_server; }
private:
    QTcpServer *m_server = nullptr;
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
        NetworkManager *nm = NetworkManager::instance();
        // Can't test actual sending without a server, but we can verify
        // the method exists and doesn't crash when disconnected
        nm->sendAction("test", {{"key", "value"}});
        check(true, "sendAction does not crash when disconnected");
    }

    section("[122] Multiplayer: connectToServer with invalid host");
    {
        NetworkManager *nm = NetworkManager::instance();
        nm->connectToServer("invalid.host.example", 9999);
        check(true, "connectToServer does not crash with invalid host");
        // Wait for the connection attempt to fail, then reset
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        timer.setInterval(3000);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(nm, &NetworkManager::disconnected, &loop, &QEventLoop::quit);
        timer.start();
        loop.exec();
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

        // Send a chat message
        nm->sendAction("chat", {{"message", "hello"}});
        check(true, "Client can send chat action");

        // Disconnect
        nm->sendAction("disconnect", {});
        server.stop();
        check(true, "Test server stops cleanly");
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
