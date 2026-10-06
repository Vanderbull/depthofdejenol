#ifndef ALIGNMENTSYSTEM_H
#define ALIGNMENTSYSTEM_H

#include <QString>
#include <QStringList>
#include <QMap>

// Alignment consequences: evil characters are barred from the Paladin's Guild,
// good characters are barred from the Assassin's Guild, etc.
class AlignmentSystem {
public:
    // The three alignments.
    enum class Alignment { Good, Neutral, Evil };

    // Can a character of this alignment join this guild?
    static bool canJoinGuild(Alignment alignment, const QString& guildName);

    // Guilds that refuse this alignment.
    static QStringList barredGuilds(Alignment alignment);

    // Guilds that require this alignment.
    static QStringList requiredGuilds(Alignment alignment);

    // A character's alignment from their status.
    static Alignment characterAlignment(int alignmentValue);

    // Display name for an alignment.
    static QString alignmentName(Alignment alignment);

    // Alignment value from a name.
    static Alignment alignmentFromName(const QString& name);

    // Can a character enter a location? (e.g., evil barred from Temple)
    static bool canEnterLocation(Alignment alignment, const QString& location);

    // Locations that refuse this alignment.
    static QStringList barredLocations(Alignment alignment);

    // A hint about what the alignment locks or unlocks.
    static QString consequenceHint(Alignment alignment);
};

#endif // ALIGNMENTSYSTEM_H
