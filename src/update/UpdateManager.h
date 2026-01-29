#pragma once

#include <QObject>
#include <QUrl>

// Forward declaration of the private implementation class
class UpdateManagerPrivate;

class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = nullptr);
    ~UpdateManager() override;

    // Standard manifest-based check
    void checkForUpdates(const QUrl &manifestUrl);

    // GitHub API-based check
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
    void onChecksumReplyFinished();

private:
    // Pointer to the implementation details (The "Pimpl")
    UpdateManagerPrivate *d;
};
