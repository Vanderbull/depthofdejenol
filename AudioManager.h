#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QMap>

class AudioManager : public QObject {
    Q_OBJECT

public:
    static AudioManager* instance();

    // Controls
    void playMusic(const QString& trackName);
    void playSound(const QString& effectName);
    void setMusicVolume(float volume); // 0.0 to 1.0
    float getMusicVolume() const { return m_musicOutput->volume(); }
    void setSfxVolume(float volume);
    void stopAllAudio();

private:
    explicit AudioManager(QObject* parent = nullptr);
    static AudioManager* m_instance;

    QMediaPlayer* m_musicPlayer;
    QAudioOutput* m_musicOutput;
    
    // Cache for sound effects to prevent lag on playback
    QMap<QString, QSoundEffect*> m_sfxCache;

    float m_sfxVolume = 0.75f;
};

#endif
