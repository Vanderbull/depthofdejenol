#ifndef SOUNDEFFECTS_H
#define SOUNDEFFECTS_H

#include <QObject>
#include <QString>
#include <QStringList>

// Sound effects for combat, dungeon and UI events.
// Wraps audioManager and maps event types to available WAV files.
class SoundEffects : public QObject {
    Q_OBJECT

public:
    enum class Type {
        Hit,
        Miss,
        CriticalHit,
        SpellCast,
        Death,
        Victory,
        LevelUp,
        Trap,
        DoorOpen,
        SecretDoor,
        Stairs,
        Rest,
        Gold,
        Click,
        Error
    };

    static SoundEffects* instance();

    // Play a sound effect by type. Returns true if the sound was found.
    bool play(Type type);

    // Play a sound by name (without .wav extension).
    bool play(const QString& name);

    // All available sound names.
    QStringList availableSounds() const;

    // Display name for a sound type.
    static QString typeName(Type type);

private:
    explicit SoundEffects(QObject* parent = nullptr);
    static SoundEffects* m_instance;

    // Map a sound type to a WAV file name.
    QString soundFileFor(Type type) const;
};

#endif // SOUNDEFFECTS_H
