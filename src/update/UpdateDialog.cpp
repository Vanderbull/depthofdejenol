#include "UpdateDialog.h"
#include "UpdateManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QApplication>
#include <QDesktopServices>
#include <QProcess>
#include <QFileInfo>
#include <QCoreApplication>

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Check for Updates"));
    resize(600, 320);

    m_updater = new UpdateManager(this);

    m_statusLabel = new QLabel(tr("Press \"Check\" to look for updates."), this);
    m_notes = new QTextEdit(this);
    m_notes->setReadOnly(true);
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_progress->setTextVisible(true);

    m_checkBtn = new QPushButton(tr("Check"), this);
    m_downloadBtn = new QPushButton(tr("Download & Apply"), this);
    m_downloadBtn->setEnabled(false);
    m_closeBtn = new QPushButton(tr("Close"), this);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addWidget(m_checkBtn);
    btnRow->addWidget(m_downloadBtn);
    btnRow->addStretch();
    btnRow->addWidget(m_closeBtn);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(m_statusLabel);
    main->addWidget(m_notes, 1);
    main->addWidget(m_progress);
    main->addLayout(btnRow);

    connect(m_checkBtn, &QPushButton::clicked, this, &UpdateDialog::onCheckClicked);
    connect(m_downloadBtn, &QPushButton::clicked, this, &UpdateDialog::onDownloadClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    connect(m_updater, &UpdateManager::checkingStarted, this, &UpdateDialog::setUiStateChecking);
    connect(m_updater, &UpdateManager::updateAvailable, this, &UpdateDialog::onUpdateAvailable);
    connect(m_updater, &UpdateManager::noUpdateAvailable, this, &UpdateDialog::onNoUpdate);
    connect(m_updater, &UpdateManager::checkingFailed, this, &UpdateDialog::onCheckingFailed);

    connect(m_updater, &UpdateManager::downloadProgress, this, &UpdateDialog::onDownloadProgress);
    connect(m_updater, &UpdateManager::downloadFinished, this, &UpdateDialog::onDownloadFinished);

    setUiStateIdle();
}

UpdateDialog::~UpdateDialog() = default;

void UpdateDialog::setManifestUrl(const QUrl &url)
{
    m_manifestUrl = url;
}

void UpdateDialog::onCheckClicked()
{
    if (m_manifestUrl.isValid()) {
        m_notes->clear();
        m_progress->setValue(0);
        m_updater->checkForUpdates(m_manifestUrl);
        return;
    }

    // Fallback: check the repository's latest GitHub release
    m_notes->clear();
    m_progress->setValue(0);
    // Default to this project — change owner/repo if you want a different source
    m_updater->checkForUpdatesFromGitHub(QStringLiteral("Vanderbull"), QStringLiteral("depthofdejenol"));
}

void UpdateDialog::setUiStateChecking()
{
    m_statusLabel->setText(tr("Checking for updates..."));
    m_checkBtn->setEnabled(false);
    m_downloadBtn->setEnabled(false);
}

void UpdateDialog::setUiStateIdle()
{
    m_statusLabel->setText(tr("Press \"Check\" to look for updates."));
    m_notes->clear();
    m_progress->setValue(0);
    m_checkBtn->setEnabled(true);
    m_downloadBtn->setEnabled(false);
    m_closeBtn->setEnabled(true);
}

void UpdateDialog::setUiStateUpdateFound()
{
    m_statusLabel->setText(tr("Update available"));
    m_checkBtn->setEnabled(true);
    m_downloadBtn->setEnabled(true);
    m_closeBtn->setEnabled(true);
}

void UpdateDialog::onUpdateAvailable(const QString &newVersion, const QString &notes, const QUrl &patchUrl, const QByteArray &sha256Hex)
{
    m_statusLabel->setText(tr("Update available: %1").arg(newVersion));
    m_notes->setPlainText(notes);
    m_patchUrl = patchUrl;
    m_patchSha256Hex = sha256Hex;
    m_downloadBtn->setEnabled(true);
    m_checkBtn->setEnabled(true);
}

void UpdateDialog::onNoUpdate()
{
    m_statusLabel->setText(tr("You have the latest version."));
    m_notes->clear();
    m_checkBtn->setEnabled(true);
    m_downloadBtn->setEnabled(false);
}

void UpdateDialog::onCheckingFailed(const QString &reason)
{
    m_statusLabel->setText(tr("Update check failed: %1").arg(reason));
    m_checkBtn->setEnabled(true);
    m_downloadBtn->setEnabled(false);
}

void UpdateDialog::onDownloadClicked()
{
    if (!m_patchUrl.isValid()) return;
    m_progress->setValue(0);
    m_statusLabel->setText(tr("Downloading patch..."));
    m_downloadBtn->setEnabled(false);
    m_checkBtn->setEnabled(false);
    m_updater->downloadPatch(m_patchUrl, m_patchSha256Hex);
}

void UpdateDialog::onDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        int pct = static_cast<int>((received * 100) / total);
        m_progress->setValue(pct);
    } else {
        m_progress->setValue(0);
    }
}

void UpdateDialog::onDownloadFinished(bool ok, const QString &localFilePath, const QString &errorMessage)
{
    if (!ok) {
        m_statusLabel->setText(tr("Download failed: %1").arg(errorMessage));
        m_checkBtn->setEnabled(true);
        m_downloadBtn->setEnabled(true);
        return;
    }

    m_statusLabel->setText(tr("Download complete. Applying patch..."));

    // APPLY STRATEGY:
    // - If the patch is a ZIP that contains files to overwrite, invoke a platform tool to extract to application dir.
    // - This example uses 'unzip -o' if available; you can replace with a built-in extraction library.
    QString appDir = QCoreApplication::applicationDirPath();
    QFileInfo fi(localFilePath);
    if (fi.suffix().compare(QStringLiteral("zip"), Qt::CaseInsensitive) == 0) {
        // Call system unzip if present
        QStringList args;
#if defined(Q_OS_WIN)
        // On Windows, try powershell Expand-Archive
        QString psCmd = QStringLiteral("Expand-Archive -Force -LiteralPath '%1' -DestinationPath '%2'").arg(localFilePath, appDir);
        QString program = QStringLiteral("powershell");
        QStringList pArgs = {QStringLiteral("-Command"), psCmd};
        bool started = QProcess::startDetached(program, pArgs);
        if (!started) {
            m_statusLabel->setText(tr("Failed to start extraction tool. Please extract %1 into %2 manually.").arg(localFilePath, appDir));
            m_checkBtn->setEnabled(true);
            return;
        }
#else
        QString program = QStringLiteral("unzip");
        args << QStringLiteral("-o") << localFilePath << QStringLiteral("-d") << appDir;
        bool started = QProcess::startDetached(program, args);
        if (!started) {
            m_statusLabel->setText(tr("Failed to start 'unzip'. Please install unzip or extract %1 into %2 manually.").arg(localFilePath, appDir));
            m_checkBtn->setEnabled(true);
            return;
        }
#endif
        m_statusLabel->setText(tr("Patch applied. Restart the application to finish update."));
        m_closeBtn->setText(tr("Close"));
        m_checkBtn->setEnabled(true);
        return;
    } else {
        m_statusLabel->setText(tr("Unknown patch format. Please extract %1 into %2 manually.").arg(localFilePath, appDir));
        m_checkBtn->setEnabled(true);
        m_downloadBtn->setEnabled(true);
    }
}
