#include "GameSettings.h"

#include <QSettings>
#include <QApplication>
#include <QFont>

#include "audioManager.h"
#include "fontManager.h"

namespace {
const char* kIniFile = "game_settings.ini";
const char* kGroup   = "Game/";
}

GameSettings& GameSettings::instance() {
    static GameSettings s;
    return s;
}

GameSettings::GameSettings()
    : m_settings(new QSettings(kIniFile, QSettings::IniFormat))
{
    migrateLegacyKeys(m_settings);
}

GameSettings::~GameSettings() {
    delete m_settings;
}

// ---------------------------------------------------------------- defaults ---

float GameSettings::defaultMusicVolume()    { return 0.60f; }
float GameSettings::defaultSfxVolume()      { return 0.75f; }
bool  GameSettings::defaultAutosaveEnabled(){ return true;  }
bool  GameSettings::defaultRevealAutomap()  { return false; }
int   GameSettings::defaultCombatSpeedMs()  { return 200;   }
bool  GameSettings::defaultFullscreen()     { return true;  }
QFont GameSettings::defaultProportionalFont(){ return QFont(QStringLiteral("MS Sans Serif"), 8); }

// ------------------------------------------------------------------- audio ---

float GameSettings::musicVolume() const {
    return m_settings->value(QString(kGroup) + "musicVolume", defaultMusicVolume()).toFloat();
}
void GameSettings::setMusicVolume(float v) {
    m_settings->setValue(QString(kGroup) + "musicVolume", qRound(v * 1000.0) / 1000.0);
}

float GameSettings::sfxVolume() const {
    return m_settings->value(QString(kGroup) + "sfxVolume", defaultSfxVolume()).toFloat();
}
void GameSettings::setSfxVolume(float v) {
    m_settings->setValue(QString(kGroup) + "sfxVolume", qRound(v * 1000.0) / 1000.0);
}

bool GameSettings::audioMuted() const {
    return m_settings->value(QString(kGroup) + "audioMuted", false).toBool();
}
void GameSettings::setAudioMuted(bool muted) {
    m_settings->setValue(QString(kGroup) + "audioMuted", muted);
}

// ---------------------------------------------------------------- gameplay ---

bool GameSettings::autosaveEnabled() const {
    return m_settings->value(QString(kGroup) + "autosaveEnabled", defaultAutosaveEnabled()).toBool();
}
void GameSettings::setAutosaveEnabled(bool on) {
    m_settings->setValue(QString(kGroup) + "autosaveEnabled", on);
}

bool GameSettings::revealAutomap() const {
    return m_settings->value(QString(kGroup) + "revealAutomap", defaultRevealAutomap()).toBool();
}
void GameSettings::setRevealAutomap(bool on) {
    m_settings->setValue(QString(kGroup) + "revealAutomap", on);
}

int GameSettings::combatSpeedMs() const {
    return m_settings->value(QString(kGroup) + "combatSpeedMs", defaultCombatSpeedMs()).toInt();
}
void GameSettings::setCombatSpeedMs(int ms) {
    m_settings->setValue(QString(kGroup) + "combatSpeedMs", ms);
}

// ----------------------------------------------------------------- display ---

bool GameSettings::fullscreen() const {
    return m_settings->value(QString(kGroup) + "fullscreen", defaultFullscreen()).toBool();
}
void GameSettings::setFullscreen(bool on) {
    m_settings->setValue(QString(kGroup) + "fullscreen", on);
}

QString GameSettings::proportionalFontFamily() const {
    return m_settings->value(QString(kGroup) + "proportionalFontFamily", QString()).toString();
}
int GameSettings::proportionalFontSize() const {
    return m_settings->value(QString(kGroup) + "proportionalFontSize", 0).toInt();
}
void GameSettings::setProportionalFont(const QString& family, int size) {
    m_settings->setValue(QString(kGroup) + "proportionalFontFamily", family);
    m_settings->setValue(QString(kGroup) + "proportionalFontSize", size);
}

QString GameSettings::fixedFontFamily() const {
    return m_settings->value(QString(kGroup) + "fixedFontFamily", QString()).toString();
}
int GameSettings::fixedFontSize() const {
    return m_settings->value(QString(kGroup) + "fixedFontSize", 0).toInt();
}
void GameSettings::setProportionalFont(const QFont& font) {
    setProportionalFont(font.family(), font.pointSize());
}

void GameSettings::setFixedFont(const QFont& font) {
    setFixedFont(font.family(), font.pointSize());
}

void GameSettings::setFixedFont(const QString& family, int size) {
    m_settings->setValue(QString(kGroup) + "fixedFontFamily", family);
    m_settings->setValue(QString(kGroup) + "fixedFontSize", size);
}

// --------------------------------------------------------------- lifecycle ---

void GameSettings::sync() {
    m_settings->sync();
}

void GameSettings::applyAll() {
    // Audio: mute is expressed as zero volume rather than stopping playback, so
    // a track started later is silent too. Stopping alone would not stick.
    auto* audio = audioManager::instance();
    const bool muted = audioMuted();
    audio->setMusicVolume(muted ? 0.0f : musicVolume());
    audio->setSfxVolume(muted ? 0.0f : sfxVolume());

    // Fonts: only override the application font when the player actually picked
    // one, so a default install keeps Qt's own metrics.
    const QString propFamily = proportionalFontFamily();
    const QFont propFont = propFamily.isEmpty()
        ? defaultProportionalFont()
        : QFont(propFamily, proportionalFontSize());
    // fontManager sets the application font and pushes it onto the widgets that
    // were only inheriting it, leaving widgets with a font of their own alone.
    // applyAll must not call QApplication::setFont() itself: fontManager captures
    // the previous app font to tell those two apart, and doing it here first
    // would hand it the already-changed font.
    fontManager::instance()->setProportionalFont(propFont);

    const QString fixedFamily = fixedFontFamily();
    fontManager::instance()->setFixedFont(
        fixedFamily.isEmpty() ? QFont("Courier New", 9)
                              : QFont(fixedFamily, fixedFontSize()));
}

void GameSettings::resetToDefaults() {
    setMusicVolume(defaultMusicVolume());
    setSfxVolume(defaultSfxVolume());
    setAudioMuted(false);
    setAutosaveEnabled(defaultAutosaveEnabled());
    setRevealAutomap(defaultRevealAutomap());
    setCombatSpeedMs(defaultCombatSpeedMs());
    setFullscreen(defaultFullscreen());
    setProportionalFont(defaultProportionalFont());
    setFixedFont(QFont(QStringLiteral("Courier New"), 9));
    applyAll();
    sync();
}

// ------------------------------------------------------------------ legacy ---

void GameSettings::migrateLegacyKeys(QSettings* settings) {
    const bool legacyGraphics = settings->contains("Graphics/Fullscreen") ||
                                settings->contains("Graphics/ResolutionWidth");
    const bool legacyAudio    = settings->contains("Audio/MusicVolume") ||
                                settings->contains("Audio/SFXVolume");
    const bool legacyGameplay = settings->contains("Gameplay/AutoSave");
    if (!legacyGraphics && !legacyAudio && !legacyGameplay) {
        return;
    }

    // Only fill keys the new schema does not already own, so a player who has
    // already used the new dialog keeps their choices.
    if (legacyAudio && !settings->contains("Game/musicVolume")) {
        const int mv = settings->value("Audio/MusicVolume", -1).toInt();
        if (mv >= 0) settings->setValue("Game/musicVolume", mv / 100.0);
    }
    if (legacyAudio && !settings->contains("Game/sfxVolume")) {
        const int sv = settings->value("Audio/SFXVolume", -1).toInt();
        if (sv >= 0) settings->setValue("Game/sfxVolume", sv / 100.0);
    }
    if (legacyGameplay && !settings->contains("Game/autosaveEnabled")) {
        settings->setValue("Game/autosaveEnabled",
                             settings->value("Gameplay/AutoSave", true).toBool());
    }
    if (legacyGraphics && !settings->contains("Game/fullscreen")) {
        settings->setValue("Game/fullscreen",
                             settings->value("Graphics/Fullscreen", true).toBool());
    }

    // Drop the old sections so exactly one schema is left.
    settings->remove("Graphics");
    settings->remove("Audio");
    settings->remove("Gameplay");
    settings->sync();
}
