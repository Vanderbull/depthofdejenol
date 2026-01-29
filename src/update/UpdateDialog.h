#pragma once

#include <QDialog>
#include <QUrl>

class QPushButton;
class QLabel;
class QTextEdit;
class QProgressBar;
class UpdateManager;

class UpdateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UpdateDialog(QWidget *parent = nullptr);
    ~UpdateDialog() override;
    // Manifest URL to check (example: https://example.com/depthofdejenol/manifest.json)
    void setManifestUrl(const QUrl &url);

private slots:
    void onCheckClicked();
    void onUpdateAvailable(const QString &newVersion, const QString &notes, const QUrl &patchUrl, const QByteArray &sha256Hex);
    void onNoUpdate();
    void onCheckingFailed(const QString &reason);
    void onDownloadClicked();
    void onDownloadProgress(qint64, qint64);
    void onDownloadFinished(bool ok, const QString &localFilePath, const QString &errorMessage);

private:
    void setUiStateIdle();
    void setUiStateChecking();
    void setUiStateUpdateFound();

    UpdateManager *m_updater = nullptr;
    QUrl m_manifestUrl;
    QUrl m_patchUrl;
    QByteArray m_patchSha256Hex;

    QLabel *m_statusLabel = nullptr;
    QTextEdit *m_notes = nullptr;
    QProgressBar *m_progress = nullptr;
    QPushButton *m_checkBtn = nullptr;
    QPushButton *m_downloadBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
};
