#include "audioManager.h"
#include <QUrl>

audioManager* audioManager::m_instance = nullptr;

audioManager* audioManager::instance() {
    if (!m_instance) m_instance = new audioManager();
    return m_instance;
}

audioManager::audioManager(QObject* parent) : QObject(parent) {
    m_musicPlayer = new QMediaPlayer(this);
    m_musicOutput = new QAudioOutput(this);
    m_musicPlayer->setAudioOutput(m_musicOutput);
    m_musicOutput->setVolume(0.5); // Default 50%
}

void audioManager::playMusic(const QString& trackName) {
    // trackName example: "qrc:/assets/music/main_theme.mp3"
    m_musicPlayer->setSource(QUrl(trackName));
    m_musicPlayer->setLoops(QMediaPlayer::Infinite);
    m_musicPlayer->play();
}

void audioManager::playSound(const QString& effectName) {
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

void audioManager::setMusicVolume(float volume) {
    m_musicOutput->setVolume(volume);
}

void audioManager::setSfxVolume(float volume) {
    m_sfxVolume = volume;
    for (QSoundEffect* effect : m_sfxCache.values()) {
        effect->setVolume(volume);
    }
}
void audioManager::stopAllAudio() {
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
