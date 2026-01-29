#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTemporaryDir>

class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = nullptr);
    ~UpdateManager() override;

    // Existing manifest-based check
    void checkForUpdates(const QUrl &manifestUrl);

    // New: check the latest GitHub release for owner/repo (e.g., "Vanderbull","depthofdejenol")
    void checkForUpdatesFromGitHub(const QString &owner, const QString &repo);

    void downloadPatch(const QUrl &patchUrl, const QByteArray &expectedSha256Hex);

signals:
    void checkingStarted();
    void checkingFailed(const QString &reason);
    void updateAvailable(const QString &newVersion, const QString &notes, const QUrl &patchUrl, const QByteArray &sha256Hex);
    void noUpdateAvailable();

    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(bool ok, const QString &localFilePath, const QString &errorMessage);

private slots:
    void onManifestReplyFinished();
    void onDownloadReadyRead();
    void onDownloadFinished();
    void onDownloadProgress(qint64, qint64);
    // New: handle GitHub checksum asset reply (if any)
    void onChecksumReplyFinished();

private:
    QNetworkAccessManager m_net;
    QNetworkReply *m_manifestReply = nullptr;
    QNetworkReply *m_downloadReply = nullptr;
    QNetworkReply *m_checksumReply = nullptr;
    QFile *m_downloadFile = nullptr;
    QTemporaryDir m_tempDir;
    QByteArray m_expectedSha256;
};
