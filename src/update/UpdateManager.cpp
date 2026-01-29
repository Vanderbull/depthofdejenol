#include "UpdateManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QJsonParseError>

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent)
{
}

UpdateManager::~UpdateManager()
{
    if (m_manifestReply) m_manifestReply->deleteLater();
    if (m_downloadReply) m_downloadReply->deleteLater();
    if (m_checksumReply) m_checksumReply->deleteLater();
    delete m_downloadFile;
}

void UpdateManager::checkForUpdates(const QUrl &manifestUrl)
{
    emit checkingStarted();

    if (m_manifestReply) {
        m_manifestReply->abort();
        m_manifestReply->deleteLater();
        m_manifestReply = nullptr;
    }
    QNetworkRequest req(manifestUrl);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("DepthOfDejenol-Updater/1.0"));
    m_manifestReply = m_net.get(req);
    connect(m_manifestReply, &QNetworkReply::finished, this, &UpdateManager::onManifestReplyFinished);
}
// New: Query GitHub Releases API for the latest release of owner/repo
void UpdateManager::checkForUpdatesFromGitHub(const QString &owner, const QString &repo)
{
    emit checkingStarted();

    if (m_manifestReply) {
        m_manifestReply->abort();
        m_manifestReply->deleteLater();
        m_manifestReply = nullptr;
    }

    const QString apiUrl = QStringLiteral("https://api.github.com/repos/%1/%2/releases/latest").arg(owner, repo);
    QNetworkRequest req{QUrl(apiUrl)}; // use braces to avoid vexing parse
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("DepthOfDejenol-Updater/1.0"));
    // Ask for JSON explicitly (not strictly necessary)
    req.setRawHeader("Accept", "application/vnd.github.v3+json");

    m_manifestReply = m_net.get(req);
    connect(m_manifestReply, &QNetworkReply::finished, this, &UpdateManager::onManifestReplyFinished);
}

void UpdateManager::onManifestReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() != QNetworkReply::NoError) {
        emit checkingFailed(reply->errorString());
        reply->deleteLater();
        m_manifestReply = nullptr;
        return;
    }
    const QByteArray body = reply->readAll();
    reply->deleteLater();
    m_manifestReply = nullptr;
    // Try to parse as JSON (works for both our manifest JSON and GitHub release JSON)
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(body, &err);
    if (doc.isNull() || !doc.isObject() || err.error != QJsonParseError::NoError) {
        emit checkingFailed(QStringLiteral("Invalid manifest/release JSON: %1").arg(err.errorString()));
        return;
    }

    QJsonObject root = doc.object();
    // Distinguish between a plain manifest and GitHub release JSON.
    // If 'tag_name' exists we assume GitHub release format.
    QString remoteVersion;
    QString notes;
    QString patchUrlStr;
    QString sha256;

    if (root.contains(QStringLiteral("tag_name"))) {
        // GitHub release JSON
        remoteVersion = root.value(QStringLiteral("tag_name")).toString();
        notes = root.value(QStringLiteral("body")).toString();

        // Find assets: prefer first asset with a downloadable URL (browser_download_url)
        QJsonArray assets = root.value(QStringLiteral("assets")).toArray();
        QUrl assetUrl;
        QString checksumAssetUrlStr;

        for (const QJsonValue &v : assets) {
            if (!v.isObject()) continue;
            QJsonObject a = v.toObject();
            QString name = a.value(QStringLiteral("name")).toString();
            QString browserUrl = a.value(QStringLiteral("browser_download_url")).toString();
            if (browserUrl.isEmpty()) continue;
            // Prefer archive/executable assets as patch
            if (name.endsWith(QStringLiteral(".zip"), Qt::CaseInsensitive) ||
                name.endsWith(QStringLiteral(".tar.gz"), Qt::CaseInsensitive) ||
                name.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive) ||
                name.endsWith(QStringLiteral(".AppImage"), Qt::CaseInsensitive) ||
                name.endsWith(QStringLiteral(".dmg"), Qt::CaseInsensitive)) {
                assetUrl = QUrl(browserUrl);
            }
            // If asset looks like a checksum, prefer to fetch it
            if (name.endsWith(QStringLiteral(".sha256"), Qt::CaseInsensitive) ||
                name.endsWith(QStringLiteral(".sha256.txt"), Qt::CaseInsensitive) ||
                name.endsWith(QStringLiteral(".sha256sum"), Qt::CaseInsensitive)) {
                checksumAssetUrlStr = browserUrl;
            }
        }
        // Fallback: if no preferred asset found, take first asset if present
        if (assetUrl.isEmpty() && !assets.isEmpty()) {
            QJsonObject first = assets.first().toObject();
            QString browserUrl = first.value(QStringLiteral("browser_download_url")).toString();
            assetUrl = QUrl(browserUrl);
        }
        if (assetUrl.isEmpty()) {
            emit checkingFailed(QStringLiteral("No downloadable release asset found in GitHub release"));
            return;
        }
        // If checksum asset exists, fetch it asynchronously, then emit updateAvailable when checksum reply finishes.
        if (!checksumAssetUrlStr.isEmpty()) {
            QNetworkRequest creq{QUrl(checksumAssetUrlStr)}; // braces to avoid vexing parse
            creq.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("DepthOfDejenol-Updater/1.0"));
            m_checksumReply = m_net.get(creq);
            // Store the parsed fields on the reply as properties so we can access them later
            m_checksumReply->setProperty("assetUrl", assetUrl.toString());
            m_checksumReply->setProperty("remoteVersion", remoteVersion);
            m_checksumReply->setProperty("notes", notes);
            connect(m_checksumReply, &QNetworkReply::finished, this, &UpdateManager::onChecksumReplyFinished);
            return;
        } else {
            // No checksum available; emit updateAvailable without sha256
            emit updateAvailable(remoteVersion, notes, assetUrl, QByteArray());
            return;
        }
    } else {
        // Treat as simple manifest format
        remoteVersion = root.value(QStringLiteral("version")).toString();
        notes = root.value(QStringLiteral("notes")).toString();
        patchUrlStr = root.value(QStringLiteral("patchUrl")).toString();
        sha256 = root.value(QStringLiteral("sha256")).toString();
        if (remoteVersion.isEmpty() || patchUrlStr.isEmpty()) {
            emit checkingFailed(QStringLiteral("Manifest missing required fields (version/patchUrl)"));
            return;
        }
        QByteArray shaBytes;
        if (!sha256.isEmpty()) shaBytes = QByteArray::fromHex(sha256.toUtf8());

        emit updateAvailable(remoteVersion, notes, QUrl(patchUrlStr), shaBytes);
        return;
    }
}

void UpdateManager::onChecksumReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    QUrl assetUrl = QUrl(reply->property("assetUrl").toString());
    QString remoteVersion = reply->property("remoteVersion").toString();
    QString notes = reply->property("notes").toString();

    if (reply->error() != QNetworkReply::NoError) {
        // Could not fetch checksum; still offer update but without verification
        emit updateAvailable(remoteVersion, notes, assetUrl, QByteArray());
        reply->deleteLater();
        m_checksumReply = nullptr;
        return;
    }
    QByteArray checksumBody = reply->readAll();
    reply->deleteLater();
    m_checksumReply = nullptr;
    // Try to extract first hex-looking token from checksum body
    // Common formats: "<hex>  filename" or just "<hex>"
    QByteArray hex;
    const QList<QByteArray> parts = checksumBody.split('\n');
    for (const QByteArray &line : parts) {
        QByteArray trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;
        // take the first token (space/tab separated)
        QList<QByteArray> tokens = trimmed.split(' ');
        if (!tokens.isEmpty()) {
            QByteArray candidate = tokens.first().trimmed();
            // keep only hex chars
            QByteArray filtered;
            for (char c : candidate) {
                if (QByteArray("0123456789abcdefABCDEF").contains(c)) filtered.append(c);
            }
            if (filtered.size() >= 64) { // SHA256 hex length
                hex = filtered.left(64).toLower();
                break;
            }
        }
    }
    QByteArray shaBytes;
    if (!hex.isEmpty()) {
        shaBytes = QByteArray::fromHex(hex);
    }
    emit updateAvailable(remoteVersion, notes, assetUrl, shaBytes);
}

void UpdateManager::downloadPatch(const QUrl &patchUrl, const QByteArray &expectedSha256Hex)
{
    m_expectedSha256 = expectedSha256Hex;

    if (!m_tempDir.isValid()) {
        // QTemporaryDir should allocate, but check
        qWarning() << "Temporary dir invalid";
    }

    QString tmpPath = m_tempDir.path();
    QFileInfo fi(patchUrl.path());
    QString fileName = fi.fileName();
    if (fileName.isEmpty()) fileName = QStringLiteral("patch.zip");

    QString localPath = tmpPath + QDir::separator() + fileName;

    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
    delete m_downloadFile;
    m_downloadFile = new QFile(localPath, this);
    if (!m_downloadFile->open(QIODevice::WriteOnly)) {
        emit downloadFinished(false, QString(), QStringLiteral("Could not open temporary file for writing"));
        delete m_downloadFile;
        m_downloadFile = nullptr;
        return;
    }
    QNetworkRequest req(patchUrl);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("DepthOfDejenol-Updater/1.0"));
    m_downloadReply = m_net.get(req);
    connect(m_downloadReply, &QNetworkReply::readyRead, this, &UpdateManager::onDownloadReadyRead);
    connect(m_downloadReply, &QNetworkReply::downloadProgress, this, &UpdateManager::onDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::finished, this, &UpdateManager::onDownloadFinished);
}

void UpdateManager::onDownloadReadyRead()
{
    if (!m_downloadReply || !m_downloadFile) return;
    m_downloadFile->write(m_downloadReply->readAll());
}

void UpdateManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void UpdateManager::onDownloadFinished()
{
    if (!m_downloadReply || !m_downloadFile) return;

    if (m_downloadReply->error() != QNetworkReply::NoError) {
        QString err = m_downloadReply->errorString();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
        m_downloadFile->close();
        QString path = m_downloadFile->fileName();
        delete m_downloadFile;
        m_downloadFile = nullptr;
        emit downloadFinished(false, path, err);
        return;
    }

    m_downloadFile->flush();
    m_downloadFile->close();

    QString path = m_downloadFile->fileName();
    // verify sha256 if provided
    if (!m_expectedSha256.isEmpty()) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) {
            emit downloadFinished(false, path, QStringLiteral("Failed to open downloaded file for verification"));
            delete m_downloadFile;
            m_downloadFile = nullptr;
            m_downloadReply->deleteLater();
            m_downloadReply = nullptr;
            return;
        }
        QCryptographicHash hash(QCryptographicHash::Sha256);
        while (!f.atEnd()) {
            hash.addData(f.read(64 * 1024));
        }
        QByteArray computed = hash.result();
        f.close();

        if (computed != m_expectedSha256) {
            emit downloadFinished(false, path, QStringLiteral("SHA256 mismatch - file may be corrupted"));
            delete m_downloadFile;
            m_downloadFile = nullptr;
            m_downloadReply->deleteLater();
            m_downloadReply = nullptr;
            return;
        }
    }
    // success
    m_downloadReply->deleteLater();
    m_downloadReply = nullptr;

    emit downloadFinished(true, path, QString());
    // leave the file in temp for caller to apply/execute
}
