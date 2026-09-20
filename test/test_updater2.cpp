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
#include <QJsonArray>
#include <QTimer>
#include <QEventLoop>
#include <QHostAddress>
#include <QProcess>
#include <QTemporaryFile>

#include "UpdateManager.h"
#include "version.h"

static const int SERVER_PORT = 19877;

// ----------------------------------------------------------------------
// Tiny HTTP server that serves a manifest and one or more patch blobs.
// ----------------------------------------------------------------------
class TestHttpServer : public QTcpServer {
public:
    explicit TestHttpServer(QObject *parent = nullptr) : QTcpServer(parent) {}

    void setManifestContent(const QByteArray &content) { m_manifest = content; }
    void setPatchContent(const QByteArray &content, const QString &path = "/patch.zip") {
        m_patches[path] = content;
    }

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

        int qpos = path.indexOf('?');
        if (qpos >= 0) path = path.left(qpos);

        if (path == "/manifest.json") {
            responseBody = m_manifest;
            mime = "application/json";
        } else if (m_patches.contains(path)) {
            responseBody = m_patches[path];
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
    QHash<QString, QByteArray> m_patches;
};

// ---------- helpers ----------

static int parseVersionInt(const QString &v) {
    QString s = v.trimmed();
    if (s.isEmpty()) return -1;
    if (s.startsWith(QLatin1Char('v')) || s.startsWith(QLatin1Char('V')))
        s.remove(0, 1);
    bool ok;
    int n = s.toInt(&ok);
    return ok ? n : -1;
}

static QByteArray buildManifest(const QString &version, const QString &notes,
                                 const QString &patchPath, const QByteArray &shaHex) {
    QJsonObject obj;
    obj["version"] = version;
    obj["notes"] = notes;
    obj["patchUrl"] = QStringLiteral("http://127.0.0.1:%1%2").arg(SERVER_PORT).arg(patchPath);
    obj["sha256"] = QString::fromLatin1(shaHex);
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

static QByteArray buildGitHubManifest(const QString &tag, const QString &notes,
                                       const QString &zipPath, const QString &shaPath) {
    QJsonObject obj;
    obj["tag_name"] = tag;
    obj["body"] = notes;
    QJsonArray assets;
    {
        QJsonObject z;
        z["name"] = zipPath;
        z["browser_download_url"] = QStringLiteral("http://127.0.0.1:%1%2").arg(SERVER_PORT).arg(zipPath);
        assets.append(z);
    }
    {
        QJsonObject s;
        s["name"] = shaPath;
        s["browser_download_url"] = QStringLiteral("http://127.0.0.1:%1%2").arg(SERVER_PORT).arg(shaPath);
        assets.append(s);
    }
    obj["assets"] = assets;
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

// Make a minimal zip using python3 (always available).
static QByteArray makeFakeZip(const QStringList &fileNames) {
    QString pythonScript =
        "import zipfile, sys, io\n"
        "out = sys.argv[1]\n"
        "names = sys.argv[2:]\n"
        "with zipfile.ZipFile(out, 'w', compresslevel=9) as zf:\n"
        "    for n in names:\n"
        "        zf.writestr(n, b'patch-content-for-' + n.encode())\n";

    QTemporaryFile tmp;
    if (!tmp.open()) return QByteArray();
    QString tmpPath = tmp.fileName();
    tmp.close();

    QProcess py;
    py.setProgram(QStringLiteral("python3"));
    QStringList args;
    args << QStringLiteral("-c") << pythonScript << tmpPath;
    for (const QString &name : fileNames)
        args << name;
    py.setArguments(args);
    py.start();
    if (!py.waitForFinished(10000)) return QByteArray();
    if (py.exitStatus() != QProcess::NormalExit || py.exitCode() != 0) {
        qWarning() << "python zip failed:" << py.readAllStandardError();
        return QByteArray();
    }

    QFile f(tmpPath);
    if (!f.open(QIODevice::ReadOnly)) return QByteArray();
    QByteArray data = f.readAll();
    QFile::remove(tmpPath);
    return data;
}

// List entries in a zip file using python3.
static QStringList listZipEntries(const QByteArray &zipData) {
    QString pythonScript =
        "import zipfile, sys, io\n"
        "data = sys.stdin.buffer.read()\n"
        "with zipfile.ZipFile(io.BytesIO(data)) as zf:\n"
        "    for n in zf.namelist():\n"
        "        print(n)\n";

    QProcess py;
    py.setProgram(QStringLiteral("python3"));
    py.setArguments({QStringLiteral("-c"), pythonScript});
    py.start();
    py.write(zipData);
    py.closeWriteChannel();
    if (!py.waitForFinished(5000)) return {};
    QString out = QString::fromUtf8(py.readAllStandardOutput());
    QStringList result = out.split('\n', Qt::SkipEmptyParts);
    std::sort(result.begin(), result.end());
    return result;
}

// Run `unzip -t` on a zip file to check integrity.
static bool testZipIntegrity(const QString &zipPath) {
    QProcess p;
    p.setProgram(QStringLiteral("unzip"));
    p.setArguments({QStringLiteral("-t"), zipPath});
    p.start();
    if (!p.waitForFinished(10000)) return false;
    QString out = QString::fromUtf8(p.readAllStandardOutput() + p.readAllStandardError());
    return p.exitCode() == 0 && out.contains("OK");
}

struct TestCase {
    QString name;
    bool expectUpdate;
    bool expectDownloadOk;
    bool expectExtractionOk;
    QString expectedEntry;
};

// ---------- main ----------

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    int failures = 0;
    int total = 0;

    QString localVer = QString::fromLatin1(GameConstants::FULL_VERSION);
    int localNum = parseVersionInt(localVer);
    if (localNum < 0) {
        qCritical() << "Cannot parse local version" << localVer;
        return 3;
    }

    // Helper macro-style lambda to run a test case and advance counters.
    auto runTest = [&](const TestCase &tc, std::function<void()> body) {
        body();
        // body sets pass via qCritical on failure; we track failures inside.
        // This helper just counts.
        Q_UNUSED(tc);
    };

    // =====================================================================
    // TEST 1: Simple manifest, remote newer -> update + download + extract
    // =====================================================================
    {
        TestCase tc{"Simple manifest, newer remote", true, true, true, "patch_file.txt"};

        QString remoteVer = QStringLiteral("v%1").arg(localNum + 1);
        QByteArray patchData = makeFakeZip({tc.expectedEntry});
        if (patchData.isEmpty()) {
            qCritical() << "FAIL" << tc.name << "could not create test zip";
            ++failures; ++total;
        } else {
            QByteArray sha = QCryptographicHash::hash(patchData, QCryptographicHash::Sha256).toHex();
            QByteArray manifest = buildManifest(remoteVer, "Test notes", "/patch.zip", sha);

            TestHttpServer server;
            server.setManifestContent(manifest);
            server.setPatchContent(patchData, "/patch.zip");
            if (!server.listen(QHostAddress::Any, SERVER_PORT)) {
                qCritical() << "Cannot listen on port" << SERVER_PORT;
                ++failures; ++total;
            } else {
                UpdateManager mgr;
                QUrl manifestUrl = QUrl::fromUserInput(
                    QStringLiteral("http://127.0.0.1:%1/manifest.json").arg(SERVER_PORT));

                bool updateFired = false;
                bool downloadOk = false;
                QString dlPath;
                bool gotDownloadFinished = false;
                QEventLoop loop;
                QTimer t; t.setSingleShot(true); t.setInterval(15000);
                QObject::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
                t.start();

                QObject::connect(&mgr, &UpdateManager::updateAvailable,
                    [&](const QString &v, const QString &n, const QUrl &u, const QByteArray &s) {
                        qDebug() << "[T1] updateAvailable" << v;
                        updateFired = true;
                        mgr.downloadPatch(u, s);
                    });
                QObject::connect(&mgr, &UpdateManager::noUpdateAvailable,
                    [&]() { qDebug() << "[T1] noUpdateAvailable (unexpected)"; loop.quit(); });
                QObject::connect(&mgr, &UpdateManager::downloadFinished,
                    [&](bool ok, const QString &p, const QString &e) {
                        qDebug() << "[T1] downloadFinished" << ok << p << e;
                        downloadOk = ok;
                        dlPath = p;
                        gotDownloadFinished = true;
                        loop.quit();
                    });

                qDebug() << "[T1] Starting...";
                mgr.checkForUpdates(manifestUrl);
                loop.exec();

                if (!gotDownloadFinished) {
                    qCritical() << "FAIL" << tc.name << "never got downloadFinished signal";
                    ++failures;
                } else {
                    bool extractOk = false;
                    QStringList entries;
                    if (downloadOk && !dlPath.isEmpty()) {
                        QFile f(dlPath);
                        if (f.open(QIODevice::ReadOnly)) {
                            QByteArray zipData = f.readAll();
                            entries = listZipEntries(zipData);
                            extractOk = entries.contains(tc.expectedEntry);
                            qDebug() << "[T1] Zip entries:" << entries;
                        }
                    }

                    bool pass = updateFired == tc.expectUpdate
                             && downloadOk == tc.expectDownloadOk
                             && extractOk == tc.expectExtractionOk;
                    if (pass) {
                        qDebug() << "PASS" << tc.name;
                    } else {
                        qCritical() << "FAIL" << tc.name
                                    << "update=" << updateFired << "(expected" << tc.expectUpdate << ")"
                                    << "downloadOk=" << downloadOk << "(expected" << tc.expectDownloadOk << ")"
                                    << "extractOk=" << extractOk << "(expected" << tc.expectExtractionOk << ")";
                        ++failures;
                    }
                }
                ++total;
                server.close();
            }
        }
    }

    // =====================================================================
    // TEST 2: Simple manifest, remote older -> no update
    // =====================================================================
    {
        TestCase tc{"Simple manifest, older remote", false, false, false, QString()};

        QString remoteVer = QStringLiteral("v%1").arg(localNum - 1);
        QByteArray patchData = makeFakeZip({"whatever.txt"});
        QByteArray sha = QCryptographicHash::hash(patchData, QCryptographicHash::Sha256).toHex();
        QByteArray manifest = buildManifest(remoteVer, "Old notes", "/patch.zip", sha);

        TestHttpServer server;
        server.setManifestContent(manifest);
        server.setPatchContent(patchData, "/patch.zip");
        server.listen(QHostAddress::Any, SERVER_PORT);

        UpdateManager mgr;
        QUrl mu = QUrl::fromUserInput(
            QStringLiteral("http://127.0.0.1:%1/manifest.json").arg(SERVER_PORT));

        bool updateFired = false;
        bool noUpdateFired = false;
        QEventLoop loop;
        QTimer t; t.setSingleShot(true); t.setInterval(15000);
        QObject::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start();

        QObject::connect(&mgr, &UpdateManager::updateAvailable,
            [&]() { updateFired = true; });
        QObject::connect(&mgr, &UpdateManager::noUpdateAvailable,
            [&]() { noUpdateFired = true; loop.quit(); });

        mgr.checkForUpdates(mu);
        loop.exec();

        if (!updateFired && noUpdateFired) {
            qDebug() << "PASS" << tc.name;
        } else {
            qCritical() << "FAIL" << tc.name << "update=" << updateFired << "noUpdate=" << noUpdateFired;
            ++failures;
        }
        ++total;
        server.close();
    }

    // =====================================================================
    // TEST 3: Malformed manifest (missing version) -> should not crash
    // =====================================================================
    {
        TestCase tc{"Malformed manifest (no version)", false, false, false, QString()};

        QJsonObject obj;
        obj["notes"] = "Missing version field";
        obj["patchUrl"] = "http://127.0.0.1:19877/patch.zip";
        obj["sha256"] = "abc";
        QJsonDocument doc(obj);
        QByteArray manifest = doc.toJson(QJsonDocument::Compact);

        TestHttpServer server;
        server.setManifestContent(manifest);
        server.setPatchContent(makeFakeZip({"x.txt"}), "/patch.zip");
        server.listen(QHostAddress::Any, SERVER_PORT);

        UpdateManager mgr;
        QUrl mu = QUrl::fromUserInput(
            QStringLiteral("http://127.0.0.1:%1/manifest.json").arg(SERVER_PORT));

        bool updateFired = false;
        bool resolved = false;
        QEventLoop loop;
        QTimer t; t.setSingleShot(true); t.setInterval(15000);
        QObject::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start();

        QObject::connect(&mgr, &UpdateManager::updateAvailable,
            [&]() { updateFired = true; loop.quit(); });
        QObject::connect(&mgr, &UpdateManager::noUpdateAvailable,
            [&]() { resolved = true; loop.quit(); });
        QObject::connect(&mgr, &UpdateManager::checkingFailed,
            [&](const QString &r) { qDebug() << "[T3] checkingFailed:" << r; resolved = true; loop.quit(); });

        mgr.checkForUpdates(mu);
        loop.exec();

        if (!updateFired && resolved) {
            qDebug() << "PASS" << tc.name;
        } else {
            qCritical() << "FAIL" << tc.name << "update=" << updateFired << "resolved=" << resolved;
            ++failures;
        }
        ++total;
        server.close();
    }

    // =====================================================================
    // TEST 4: Bad SHA256 (mismatch) -> downloadFinished should report error
    // =====================================================================
    {
        TestCase tc{"Bad SHA256", true, false, false, QString()};

        QString remoteVer = QStringLiteral("v%1").arg(localNum + 1);
        QByteArray patchData = makeFakeZip({"real.txt"});
        QByteArray wrongSha = QCryptographicHash::hash(patchData, QCryptographicHash::Sha256).toHex();
        wrongSha[0] = (wrongSha[0] == 'a') ? 'b' : 'a';
        QByteArray manifest = buildManifest(remoteVer, "Bad checksum test", "/patch.zip", wrongSha);

        TestHttpServer server;
        server.setManifestContent(manifest);
        server.setPatchContent(patchData, "/patch.zip");
        server.listen(QHostAddress::Any, SERVER_PORT);

        UpdateManager mgr;
        QUrl mu = QUrl::fromUserInput(
            QStringLiteral("http://127.0.0.1:%1/manifest.json").arg(SERVER_PORT));

        bool updateFired = false;
        bool downloadOk = false;
        QString dlErr;
        bool gotDl = false;
        QEventLoop loop;
        QTimer t; t.setSingleShot(true); t.setInterval(15000);
        QObject::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start();

        QObject::connect(&mgr, &UpdateManager::updateAvailable,
            [&](const QString &v, const QString &n, const QUrl &u, const QByteArray &s) {
                qDebug() << "[T4] updateAvailable" << v;
                updateFired = true;
                mgr.downloadPatch(u, s);
            });
        QObject::connect(&mgr, &UpdateManager::downloadFinished,
            [&](bool ok, const QString &p, const QString &e) {
                qDebug() << "[T4] downloadFinished" << ok << p << e;
                downloadOk = ok;
                dlErr = e;
                gotDl = true;
                loop.quit();
            });

        mgr.checkForUpdates(mu);
        loop.exec();

        if (!gotDl) {
            qCritical() << "FAIL" << tc.name << "never got downloadFinished";
            ++failures;
        } else if (updateFired && !downloadOk && !dlErr.isEmpty()) {
            qDebug() << "PASS" << tc.name << "(err:" << dlErr << ")";
        } else {
            qCritical() << "FAIL" << tc.name
                        << "update=" << updateFired << "downloadOk=" << downloadOk << "err=" << dlErr;
            ++failures;
        }
        ++total;
        server.close();
    }

    // =====================================================================
    // TEST 5: GitHub release path -> update + download
    // =====================================================================
    {
        TestCase tc{"GitHub release path", true, true, true, "github_patch.txt"};

        QString remoteVer = QStringLiteral("v%1").arg(localNum + 2);
        QByteArray patchData = makeFakeZip({tc.expectedEntry});
        QByteArray sha = QCryptographicHash::hash(patchData, QCryptographicHash::Sha256).toHex();
        QByteArray manifest = buildGitHubManifest(remoteVer, "GitHub notes",
                                                   "/gh_patch.zip", "/gh_sha.txt");
        QByteArray shaContent = sha + "  gh_patch.zip";

        TestHttpServer server;
        server.setManifestContent(manifest);
        server.setPatchContent(patchData, "/gh_patch.zip");
        server.setPatchContent(shaContent, "/gh_sha.txt");
        server.listen(QHostAddress::Any, SERVER_PORT);

        UpdateManager mgr;
        QUrl mu = QUrl::fromUserInput(
            QStringLiteral("http://127.0.0.1:%1/manifest.json").arg(SERVER_PORT));

        bool updateFired = false;
        bool downloadOk = false;
        QString dlPath;
        bool gotDl = false;
        QEventLoop loop;
        QTimer t; t.setSingleShot(true); t.setInterval(15000);
        QObject::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start();

        QObject::connect(&mgr, &UpdateManager::updateAvailable,
            [&](const QString &v, const QString &n, const QUrl &u, const QByteArray &s) {
                qDebug() << "[T5] updateAvailable" << v;
                updateFired = true;
                mgr.downloadPatch(u, s);
            });
        QObject::connect(&mgr, &UpdateManager::downloadFinished,
            [&](bool ok, const QString &p, const QString &e) {
                qDebug() << "[T5] downloadFinished" << ok << p << e;
                downloadOk = ok;
                dlPath = p;
                gotDl = true;
                loop.quit();
            });

        mgr.checkForUpdates(mu);
        loop.exec();

        if (!gotDl) {
            qCritical() << "FAIL" << tc.name << "never got downloadFinished";
            ++failures;
        } else {
            bool extractOk = false;
            if (downloadOk && !dlPath.isEmpty()) {
                QFile f(dlPath);
                if (f.open(QIODevice::ReadOnly)) {
                    auto entries = listZipEntries(f.readAll());
                    extractOk = entries.contains(tc.expectedEntry);
                    qDebug() << "[T5] Zip entries:" << entries;
                }
            }

            if (updateFired && downloadOk && extractOk) {
                qDebug() << "PASS" << tc.name;
            } else {
                qCritical() << "FAIL" << tc.name
                            << "update=" << updateFired
                            << "downloadOk=" << downloadOk
                            << "extractOk=" << extractOk;
                ++failures;
            }
        }
        ++total;
        server.close();
    }

    // =====================================================================
    // TEST 6: Version equal -> no update (not newer)
    // =====================================================================
    {
        TestCase tc{"Equal version", false, false, false, QString()};

        QString remoteVer = localVer;
        QByteArray patchData = makeFakeZip({"same.txt"});
        QByteArray sha = QCryptographicHash::hash(patchData, QCryptographicHash::Sha256).toHex();
        QByteArray manifest = buildManifest(remoteVer, "Same version", "/patch.zip", sha);

        TestHttpServer server;
        server.setManifestContent(manifest);
        server.setPatchContent(patchData, "/patch.zip");
        server.listen(QHostAddress::Any, SERVER_PORT);

        UpdateManager mgr;
        QUrl mu = QUrl::fromUserInput(
            QStringLiteral("http://127.0.0.1:%1/manifest.json").arg(SERVER_PORT));

        bool updateFired = false;
        bool noUpdateFired = false;
        QEventLoop loop;
        QTimer t; t.setSingleShot(true); t.setInterval(15000);
        QObject::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start();

        QObject::connect(&mgr, &UpdateManager::updateAvailable,
            [&]() { updateFired = true; });
        QObject::connect(&mgr, &UpdateManager::noUpdateAvailable,
            [&]() { noUpdateFired = true; loop.quit(); });

        mgr.checkForUpdates(mu);
        loop.exec();

        if (!updateFired && noUpdateFired) {
            qDebug() << "PASS" << tc.name;
        } else {
            qCritical() << "FAIL" << tc.name << "update=" << updateFired << "noUpdate=" << noUpdateFired;
            ++failures;
        }
        ++total;
        server.close();
    }

    // =====================================================================
    // REPORT
    // =====================================================================
    qDebug() << "\n===== SUMMARY =====";
    qDebug() << "Total:" << total << "Failures:" << failures;
    return failures > 0 ? 1 : 0;
}
