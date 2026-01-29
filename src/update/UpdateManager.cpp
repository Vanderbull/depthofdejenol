#include "UpdateManager.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>

// The private implementation class definition
class UpdateManagerPrivate {
public:
    QNetworkAccessManager net;
    QNetworkReply *manifestReply = nullptr;
    QNetworkReply *downloadReply = nullptr;
    QNetworkReply *checksumReply = nullptr;
    QFile *downloadFile = nullptr;
    QTemporaryDir tempDir;
    QByteArray expectedSha256;
};

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent), d(new UpdateManagerPrivate)
{
}

UpdateManager::~UpdateManager()
{
    if (d->manifestReply) d->manifestReply->deleteLater();
    if (d->downloadReply) d->downloadReply->deleteLater();
    if (d->checksumReply) d->checksumReply->deleteLater();
    delete d->downloadFile;
    delete d; // Delete the Pimpl data
}

void UpdateManager::checkForUpdates(const QUrl &manifestUrl)
{
    emit checkingStarted();

    if (d->manifestReply) {
        d->manifestReply->abort();
        d->manifestReply->deleteLater();
    }
    QNetworkRequest req(manifestUrl);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("DepthOfDejenol-Updater/1.0"));
    d->manifestReply = d->net.get(req);
    connect(d->manifestReply, &QNetworkReply::finished, this, &UpdateManager::onManifestReplyFinished);
}

void UpdateManager::checkForUpdatesFromGitHub(const QString &owner, const QString &repo)
{
    emit checkingStarted();

    if (d->manifestReply) {
        d->manifestReply->abort();
        d->manifestReply->deleteLater();
    }

    const QString apiUrl = QStringLiteral("https://api.github.com/repos/%1/%2/releases/latest").arg(owner, repo);
    QNetworkRequest req{QUrl(apiUrl)};
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("DepthOfDejenol-Updater/1.0"));
    req.setRawHeader("Accept", "application/vnd.github.v3+json");

    d->manifestReply = d->net.get(req);
    connect(d->manifestReply, &QNetworkReply::finished, this, &UpdateManager::onManifestReplyFinished);
}

void UpdateManager::onManifestReplyFinished()
{
    if (!d->manifestReply) return;
    QNetworkReply *reply = d->manifestReply;

    if (reply->error() != QNetworkReply::NoError) {
        emit checkingFailed(reply->errorString());
        reply->deleteLater();
        d->manifestReply = nullptr;
        return;
    }

    const QByteArray body = reply->readAll();
    reply->deleteLater();
    d->manifestReply = nullptr;

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (doc.isNull() || !doc.isObject()) {
        emit checkingFailed(QStringLiteral("Invalid JSON response"));
        return;
    }

    QJsonObject root = doc.object();
    if (root.contains(QStringLiteral("tag_name"))) {
        // GitHub Logic
        QString remoteVersion = root.value(QStringLiteral("tag_name")).toString();
        QString notes = root.value(QStringLiteral("body")).toString();
        QJsonArray assets = root.value(QStringLiteral("assets")).toArray();
        QUrl assetUrl;
        QString checksumUrl;

        for (const QJsonValue &v : assets) {
            QJsonObject a = v.toObject();
            QString name = a.value(QStringLiteral("name")).toString();
            QString browserUrl = a.value(QStringLiteral("browser_download_url")).toString();
            
            if (name.endsWith(QStringLiteral(".zip"), Qt::CaseInsensitive)) {
                assetUrl = QUrl(browserUrl);
            } else if (name.contains(QStringLiteral("sha256"))) {
                checksumUrl = browserUrl;
            }
        }

        if (!checksumUrl.isEmpty()) {
            QNetworkRequest creq{QUrl(checksumUrl)};
            creq.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("DepthOfDejenol-Updater/1.0"));
            d->checksumReply = d->net.get(creq);
            d->checksumReply->setProperty("assetUrl", assetUrl.toString());
            d->checksumReply->setProperty("remoteVersion", remoteVersion);
            d->checksumReply->setProperty("notes", notes);
            connect(d->checksumReply, &QNetworkReply::finished, this, &UpdateManager::onChecksumReplyFinished);
        } else {
            emit updateAvailable(remoteVersion, notes, assetUrl, QByteArray());
        }
    } else {
        // Simple Manifest Logic
        QString remoteVersion = root.value(QStringLiteral("version")).toString();
        QString notes = root.value(QStringLiteral("notes")).toString();
        QString patchUrlStr = root.value(QStringLiteral("patchUrl")).toString();
        QByteArray shaBytes = QByteArray::fromHex(root.value(QStringLiteral("sha256")).toString().toUtf8());
        emit updateAvailable(remoteVersion, notes, QUrl(patchUrlStr), shaBytes);
    }
}

void UpdateManager::onChecksumReplyFinished()
{
    if (!d->checksumReply) return;
    QNetworkReply *reply = d->checksumReply;
    
    QUrl assetUrl = QUrl(reply->property("assetUrl").toString());
    QString remoteVersion = reply->property("remoteVersion").toString();
    QString notes = reply->property("notes").toString();

    QByteArray checksumBody = reply->readAll();
    reply->deleteLater();
    d->checksumReply = nullptr;

    QByteArray hex = checksumBody.split(' ').first().trimmed();
    emit updateAvailable(remoteVersion, notes, assetUrl, QByteArray::fromHex(hex));
}

void UpdateManager::downloadPatch(const QUrl &patchUrl, const QByteArray &expectedSha256Hex)
{
    d->expectedSha256 = expectedSha256Hex;
    QString localPath = d->tempDir.path() + QDir::separator() + "patch.zip";

    if (d->downloadReply) {
        d->downloadReply->abort();
        d->downloadReply->deleteLater();
    }

    delete d->downloadFile;
    d->downloadFile = new QFile(localPath, this);
    if (!d->downloadFile->open(QIODevice::WriteOnly)) {
        emit downloadFinished(false, QString(), QStringLiteral("IO Error"));
        return;
    }

    QNetworkRequest req(patchUrl);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("DepthOfDejenol-Updater/1.0"));
    d->downloadReply = d->net.get(req);
    connect(d->downloadReply, &QNetworkReply::readyRead, this, &UpdateManager::onDownloadReadyRead);
    connect(d->downloadReply, &QNetworkReply::downloadProgress, this, &UpdateManager::onDownloadProgress);
    connect(d->downloadReply, &QNetworkReply::finished, this, &UpdateManager::onDownloadFinished);
}

void UpdateManager::onDownloadReadyRead()
{
    if (d->downloadReply && d->downloadFile) {
        d->downloadFile->write(d->downloadReply->readAll());
    }
}

void UpdateManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void UpdateManager::onDownloadFinished()
{
    if (!d->downloadReply || !d->downloadFile) return;

    d->downloadFile->close();
    QString path = d->downloadFile->fileName();

    if (d->downloadReply->error() != QNetworkReply::NoError) {
        emit downloadFinished(false, path, d->downloadReply->errorString());
    } else {
        // Simplified Verification
        if (!d->expectedSha256.isEmpty()) {
            QCryptographicHash hash(QCryptographicHash::Sha256);
            QFile f(path);
            if (f.open(QIODevice::ReadOnly)) {
                hash.addData(&f);
                if (hash.result() != d->expectedSha256) {
                    emit downloadFinished(false, path, "SHA256 Mismatch");
                    return;
                }
            }
        }
        emit downloadFinished(true, path, QString());
    }
    
    d->downloadReply->deleteLater();
    d->downloadReply = nullptr;
}
