#include "SoundEffects.h"
#include "AudioManager.h"
#include <QFileInfo>
#include <QDir>

SoundEffects* SoundEffects::m_instance = nullptr;

SoundEffects* SoundEffects::instance() {
    if (!m_instance) m_instance = new SoundEffects();
    return m_instance;
}

SoundEffects::SoundEffects(QObject* parent) : QObject(parent) {}

bool SoundEffects::play(Type type) {
    QString file = soundFileFor(type);
    if (file.isEmpty()) return false;
    return play(file);
}

bool SoundEffects::play(const QString& name) {
    if (name.isEmpty()) return false;
    audioManager::instance()->playSound("resources/waves/" + name + ".wav");
    return true;
}

QStringList SoundEffects::availableSounds() const {
    QStringList sounds;
    QDir dir("resources/waves");
    if (dir.exists()) {
        QStringList filters;
        filters << "*.wav";
        for (const QString& file : dir.entryList(filters, QDir::Files)) {
            sounds << file.left(file.lastIndexOf('.'));
        }
    }
    return sounds;
}

QString SoundEffects::typeName(Type type) {
    switch (type) {
    case Type::Hit:         return "Hit";
    case Type::Miss:        return "Miss";
    case Type::CriticalHit: return "Critical Hit";
    case Type::SpellCast:   return "Spell Cast";
    case Type::Death:       return "Death";
    case Type::Victory:     return "Victory";
    case Type::LevelUp:     return "Level Up";
    case Type::Trap:        return "Trap";
    case Type::DoorOpen:    return "Door Open";
    case Type::SecretDoor:  return "Secret Door";
    case Type::Stairs:      return "Stairs";
    case Type::Rest:        return "Rest";
    case Type::Gold:        return "Gold";
    case Type::Click:       return "Click";
    case Type::Error:       return "Error";
    }
    return "Unknown";
}

QString SoundEffects::soundFileFor(Type type) const {
    // Map event types to available WAV files.
    // The "sc" files are short sound effects; the others are music tracks.
    switch (type) {
    case Type::Hit:         return "gen2sc";
    case Type::Miss:        return "gen3sc";
    case Type::CriticalHit: return "gen2sc";
    case Type::SpellCast:   return "gen3general";
    case Type::Death:       return "gen3sc";
    case Type::Victory:     return "gen2sc";
    case Type::LevelUp:     return "gen3general";
    case Type::Trap:        return "gen3sc";
    case Type::DoorOpen:    return "gen2sc";
    case Type::SecretDoor:  return "gen3general";
    case Type::Stairs:      return "gen2sc";
    case Type::Rest:        return "gen3general";
    case Type::Gold:        return "gen2sc";
    case Type::Click:       return "gen3sc";
    case Type::Error:       return "gen3sc";
    }
    return QString();
}
