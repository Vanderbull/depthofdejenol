#include "AudioManager.h"
#include <QUrl>

AudioManager* AudioManager::m_instance = nullptr;

AudioManager* AudioManager::instance() {
    if (!m_instance) m_instance = new AudioManager();
    return m_instance;
}

AudioManager::AudioManager(QObject* parent) : QObject(parent) {
    m_musicPlayer = new QMediaPlayer(this);
    m_musicOutput = new QAudioOutput(this);
    m_musicPlayer->setAudioOutput(m_musicOutput);
    m_musicOutput->setVolume(0.5); // Default 50%
}

void AudioManager::playMusic(const QString& trackName) {
    // trackName example: "qrc:/assets/music/main_theme.mp3"
    m_musicPlayer->setSource(QUrl(trackName));
    m_musicPlayer->setLoops(QMediaPlayer::Infinite);
    m_musicPlayer->play();
}

void AudioManager::playSound(const QString& effectName) {
    // If not in cache, load it
    if (!m_sfxCache.contains(effectName)) {
        QSoundEffect* effect = new QSoundEffect(this);
        effect->setSource(QUrl(effectName));
        m_sfxCache.insert(effectName, effect);
    }

    QSoundEffect* sound = m_sfxCache.value(effectName);
    sound->setVolume(m_sfxVolume);
    sound->play();
}

void AudioManager::setMusicVolume(float volume) {
    m_musicOutput->setVolume(volume);
}

void AudioManager::setSfxVolume(float volume) {
    m_sfxVolume = volume;
    for (QSoundEffect* effect : m_sfxCache.values()) {
        effect->setVolume(volume);
    }
}
void AudioManager::stopAllAudio() {
    // 1. Stop the Background Music
    if (m_musicPlayer->playbackState() != QMediaPlayer::StoppedState) {
        m_musicPlayer->stop();
    }

    // 2. Stop all cached Sound Effects
    for (QSoundEffect* effect : m_sfxCache.values()) {
        if (effect->isPlaying()) {
            effect->stop();
        }
    }
}
