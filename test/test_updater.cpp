#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QDebug>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QEventLoop>
#include <QHostAddress>

#include "UpdateManager.h"
#include "version.h"

static const int SERVER_PORT = 19876;

// ----------------------------------------------------------------------
// Tiny HTTP server that serves two URLs:
//   /manifest.json   — the update manifest
//   /patch.zip       — the patch archive
// ----------------------------------------------------------------------
class TestHttpServer : public QTcpServer {
public:
    explicit TestHttpServer(QObject *parent = nullptr) : QTcpServer(parent) {}

    void setManifestContent(const QByteArray &content) { m_manifest = content; }
    void setPatchContent(const QByteArray &content) { m_patch = content; }

private:
    void incomingConnection(qint64 socketDescriptor) override {
        QTcpSocket *sock = new QTcpSocket(this);
        sock->setSocketDescriptor(socketDescriptor);
        connect(sock, &QTcpSocket::readyRead, this, [this, sock]() {
            handleRequest(sock);
        });
        connect(sock, &QTcpSocket::disconnected, sock, &QTcpSocket::deleteLater);
    }

    void handleRequest(QTcpSocket *sock) {
        QByteArray request = sock->readAll();
        QString path;
        {
            QString reqStr = QString::fromUtf8(request);
            int sp = reqStr.indexOf(' ');
            int sp2 = reqStr.indexOf(' ', sp + 1);
            if (sp >= 0 && sp2 > sp)
                path = reqStr.mid(sp + 1, sp2 - sp - 1);
        }

        QByteArray responseBody;
        int statusCode = 200;
        QString mime = "application/octet-stream";

        if (path == "/manifest.json") {
            responseBody = m_manifest;
            mime = "application/json";
        } else if (path == "/patch.zip") {
            responseBody = m_patch;
            mime = "application/zip";
        } else {
            statusCode = 404;
            responseBody = "Not Found";
        }

        QString statusText = statusCode == 200 ? "OK" : "Not Found";
        QString header = QString(
            "HTTP/1.1 %1 %2\r\n"
            "Content-Type: %3\r\n"
            "Content-Length: %4\r\n"
            "Connection: close\r\n"
            "\r\n"
        ).arg(statusCode).arg(statusText).arg(mime).arg(responseBody.size());

        sock->write(header.toUtf8());
        sock->write(responseBody);
        sock->flush();
        sock->disconnectFromHost();
    }

    QByteArray m_manifest;
    QByteArray m_patch;
};

// ----------------------------------------------------------------------
// Driver: creates UpdateManager, points it at the local test server,
// runs check + download, and reports whether the signals match
// expectations.
// ----------------------------------------------------------------------

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // ---- Build the test manifest -------------------------------------------------
    QString localVersion = QString::fromLatin1(GameConstants::FULL_VERSION);
    qDebug() << "Local version from version.h:" << localVersion;

    // Two scenarios:
    //  - testNewer = true  -> remote version higher, expect updateAvailable + download
    //  - testNewer = false -> remote version lower/equal, expect noUpdateAvailable
    bool testNewer = true;

    QString remoteVersion = testNewer ? QStringLiteral("v999") : QStringLiteral("v10");
    QString notes = "Test patch notes for the updater harness.";
    QString patchUrl = QStringLiteral("http://127.0.0.1:%1/patch.zip").arg(SERVER_PORT);

    QByteArray patchBody = QByteArrayLiteral("FAKE_PATCH_CONTENTS_FOR_TESTING");
    QByteArray patchSha256 = QCryptographicHash::hash(patchBody, QCryptographicHash::Sha256).toHex();

    QJsonObject manifestObj;
    manifestObj["version"] = remoteVersion;
    manifestObj["notes"] = notes;
    manifestObj["patchUrl"] = patchUrl;
    manifestObj["sha256"] = QString::fromLatin1(patchSha256);
    QJsonDocument manifestDoc(manifestObj);
    QByteArray manifestJson = manifestDoc.toJson(QJsonDocument::Compact);

    // ---- Spin up the HTTP server -------------------------------------------------
    TestHttpServer server;
    server.setManifestContent(manifestJson);
    server.setPatchContent(patchBody);

    if (!server.listen(QHostAddress::Any, SERVER_PORT)) {
        qCritical() << "Failed to start test HTTP server on port" << SERVER_PORT;
        return 2;
    }
    qDebug() << "Test HTTP server listening on port" << SERVER_PORT;

    // ---- Create UpdateManager ----------------------------------------------------
    UpdateManager updater;
    QUrl manifestUrl = QUrl::fromUserInput(QStringLiteral("http://127.0.0.1:%1/manifest.json").arg(SERVER_PORT));

    // ---- Track outcomes -----------------------------------------------------------
    bool updateAvailableFired = false;
    bool noUpdateAvailableFired = false;
    bool downloadFinishedOk = false;
    QString downloadFinishedPath;
    QString downloadFinishedError;

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(15000);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start();

    QObject::connect(&updater, &UpdateManager::checkingStarted,
        []() { qDebug() << "[TEST] checkingStarted"; });
    QObject::connect(&updater, &UpdateManager::checkingFailed,
        [](const QString &reason) { qDebug() << "[TEST] checkingFailed:" << reason; });
    QObject::connect(&updater, &UpdateManager::updateAvailable,
        [&](const QString &newVersion, const QString &notesOut,
            const QUrl &patchUrlOut, const QByteArray &shaOut) {
            qDebug() << "[TEST] updateAvailable:" << newVersion << notesOut << patchUrlOut;
            updateAvailableFired = true;
            updater.downloadPatch(patchUrlOut, shaOut);
        });
    QObject::connect(&updater, &UpdateManager::noUpdateAvailable,
        [&]() {
            qDebug() << "[TEST] noUpdateAvailable";
            noUpdateAvailableFired = true;
            loop.quit();
        });
    QObject::connect(&updater, &UpdateManager::downloadProgress,
        [](qint64 recv, qint64 total) {
            qDebug() << "[TEST] downloadProgress:" << recv << total;
        });
    QObject::connect(&updater, &UpdateManager::downloadFinished,
        [&](bool ok, const QString &path, const QString &err) {
            qDebug() << "[TEST] downloadFinished: ok=" << ok << "path=" << path << "err=" << err;
            downloadFinishedOk = ok;
            downloadFinishedPath = path;
            downloadFinishedError = err;
            loop.quit();
        });

    // ---- Kick off the check ------------------------------------------------------
    qDebug() << "[TEST] Starting update check...";
    updater.checkForUpdates(manifestUrl);

    loop.exec();

    // ---- Report ------------------------------------------------------------------
    qDebug() << "\n===== TEST RESULTS =====";
    qDebug() << "Local version :" << localVersion;
    qDebug() << "Remote version:" << remoteVersion;
    qDebug() << "TestNewer     :" << testNewer;
    qDebug() << "updateAvailableFired    :" << updateAvailableFired;
    qDebug() << "noUpdateAvailableFired  :" << noUpdateAvailableFired;
    qDebug() << "downloadFinishedOk      :" << downloadFinishedOk;
    qDebug() << "downloadFinishedPath    :" << downloadFinishedPath;
    qDebug() << "downloadFinishedError   :" << downloadFinishedError;

    bool passed = false;
    if (testNewer) {
        passed = updateAvailableFired && downloadFinishedOk && !downloadFinishedPath.isEmpty();
        if (!passed)
            qCritical() << "FAIL: expected updateAvailable + successful download";
    } else {
        passed = noUpdateAvailableFired && !updateAvailableFired;
        if (!passed)
            qCritical() << "FAIL: expected noUpdateAvailable, got updateAvailable=" << updateAvailableFired;
    }

    server.close();
    return passed ? 0 : 1;
}
