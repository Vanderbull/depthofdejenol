#ifndef audioManager_H
#define audioManager_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QMap>

class audioManager : public QObject {
    Q_OBJECT

public:
    static audioManager* instance();

    // Controls
    void playMusic(const QString& trackName);
    void playSound(const QString& effectName);
    void setMusicVolume(float volume); // 0.0 to 1.0
    float getMusicVolume() const { return m_musicOutput->volume(); }
    void setSfxVolume(float volume);
    void stopAllAudio();

private:
    explicit audioManager(QObject* parent = nullptr);
    static audioManager* m_instance;

    QMediaPlayer* m_musicPlayer;
    QAudioOutput* m_musicOutput;
    
    // Cache for sound effects to prevent lag on playback
    QMap<QString, QSoundEffect*> m_sfxCache;

    float m_sfxVolume = 0.75f;
};

#endif
