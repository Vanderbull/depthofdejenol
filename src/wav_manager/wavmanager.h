#ifndef WAVMANAGER_H
#define WAVMANAGER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QUrl>
#include <QSoundEffect>
#include <QDebug>
#include <QDir>
#include <QStringList>

class WavManager : public QObject
{
    Q_OBJECT
public:
    // Constructor. Takes the path to the folder containing the WAV files.
    explicit WavManager(const QString &folderPath, QObject *parent = nullptr);
    // Loads all .wav files from the specified folder.
    bool loadWavFiles();
    // Plays a sound file based on its filename (without path and without the .wav extension).
    // Returns true if the file was found and playback started, otherwise false.
    bool playWav(const QString &fileName);
    // Returns a list of the loaded filenames (without the .wav extension).
    QStringList availableWavs() const;
private:
    // The folder path to search in
    QString m_folderPath;
    // Hash table to store filename (key) and its URL (value)
    QHash<QString, QUrl> m_wavFiles;
    // QSoundEffect object for playback. Best for short, uncompressed effects.
    QSoundEffect m_soundEffect;
signals:
    // Optional: A signal that could be emitted if sound playback status changes.
    // Note: Simple QSoundEffect might not be ideal for tracking 'finished' status reliably.
    void playbackStatusChanged(const QString &fileName, QSoundEffect::Status status);
};

#endif // WAVMANAGER_H
