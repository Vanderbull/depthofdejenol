#include "wavmanager.h"

WavManager::WavManager(const QString &folderPath, QObject *parent)
    : QObject(parent), m_folderPath(folderPath)
{
    // Connect to status changes, primarily for debugging or complex state handling.
    connect(&m_soundEffect, &QSoundEffect::statusChanged, this, [this]() {
        // Emit the signal with the current source file name if you track it.
        // For simplicity, we just debug the status here.
        qDebug() << "QSoundEffect Status Changed:" << m_soundEffect.status();
    });
}

bool WavManager::loadWavFiles()
{
    m_wavFiles.clear(); // Clear existing files
    QDir dir(m_folderPath);
    if (!dir.exists()) {
        qWarning() << "Could not find folder:" << m_folderPath;
        return false;
    }
    // Filter for only .wav files
    QStringList filters;
    filters << "*.wav";
    dir.setNameFilters(filters);
    // Get all matching files (no subfolders)
    QStringList fileList = dir.entryList(QDir::Files);
    if (fileList.isEmpty()) {
        qWarning() << "Found no .wav files in folder:" << m_folderPath;
        return true; // Folder exists but is empty of WAVs. Consider this a success but warn.
    }
    for (const QString &fileName : fileList) {
        // Create the full absolute file path
        QString fullPath = dir.absoluteFilePath(fileName);
        // Create QUrl from the local path (necessary for QSoundEffect)
        QUrl fileUrl = QUrl::fromLocalFile(fullPath);
        // Extract the filename without the extension (.wav) as the key
        QString key = fileName;
        key.chop(4); // Removes the last 4 characters (".wav")
        m_wavFiles.insert(key, fileUrl);
        qDebug() << "Loaded WAV file:" << key << "with URL:" << fileUrl;
    }
    qDebug() << "Total WAV files loaded:" << m_wavFiles.size();
    return true;
}

bool WavManager::playWav(const QString &fileName)
{
    if (m_wavFiles.contains(fileName)) {
        QUrl url = m_wavFiles.value(fileName);
        // Set the source for QSoundEffect
        m_soundEffect.setSource(url);
        // Check if the source could be loaded and is ready
        if (m_soundEffect.status() == QSoundEffect::Ready) {
            m_soundEffect.play();
            qDebug() << "Playing sound:" << fileName;
            return true;
        } else {
            qWarning() << "Could not load sound source for:" << fileName << ". Status:" << m_soundEffect.status();
            return false;
        }

    } else {
        qWarning() << "File" << fileName << "not found in the manager.";
        return false;
    }
}

QStringList WavManager::availableWavs() const
{
    return m_wavFiles.keys();
}
