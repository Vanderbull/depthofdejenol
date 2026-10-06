#include "AlignmentSystem.h"

AlignmentSystem::Alignment AlignmentSystem::characterAlignment(int alignmentValue) {
    if (alignmentValue < 0) return Alignment::Evil;
    if (alignmentValue > 0) return Alignment::Good;
    return Alignment::Neutral;
}

QString AlignmentSystem::alignmentName(Alignment alignment) {
    switch (alignment) {
    case Alignment::Good: return "Good";
    case Alignment::Neutral: return "Neutral";
    case Alignment::Evil: return "Evil";
    }
    return "Neutral";
}

AlignmentSystem::Alignment AlignmentSystem::alignmentFromName(const QString& name) {
    if (name == "Good") return Alignment::Good;
    if (name == "Evil") return Alignment::Evil;
    return Alignment::Neutral;
}

bool AlignmentSystem::canJoinGuild(Alignment alignment, const QString& guildName) {
    QStringList barred = barredGuilds(alignment);
    return !barred.contains(guildName);
}

QStringList AlignmentSystem::barredGuilds(Alignment alignment) {
    switch (alignment) {
    case Alignment::Good:
        // Good characters cannot join evil guilds.
        return {"Assassin's Guild", "Cult of the Dark", "Necromancer's Circle"};
    case Alignment::Evil:
        // Evil characters cannot join good guilds.
        return {"Paladin's Guild", "Priest of Light", "Temple of the Sun"};
    case Alignment::Neutral:
        // Neutral can join anything.
        return {};
    }
    return {};
}

QStringList AlignmentSystem::requiredGuilds(Alignment alignment) {
    switch (alignment) {
    case Alignment::Good:
        return {"Paladin's Guild", "Priest of Light"};
    case Alignment::Evil:
        return {"Assassin's Guild", "Cult of the Dark"};
    case Alignment::Neutral:
        return {};
    }
    return {};
}

bool AlignmentSystem::canEnterLocation(Alignment alignment, const QString& location) {
    QStringList barred = barredLocations(alignment);
    return !barred.contains(location);
}

QStringList AlignmentSystem::barredLocations(Alignment alignment) {
    switch (alignment) {
    case Alignment::Evil:
        // Evil characters are barred from holy places.
        return {"Temple", "Shrine of Light", "Paladin's Hall"};
    case Alignment::Good:
        // Good characters are barred from dark places.
        return {"Cult of the Dark", "Assassin's Den"};
    case Alignment::Neutral:
        return {};
    }
    return {};
}

QString AlignmentSystem::consequenceHint(Alignment alignment) {
    switch (alignment) {
    case Alignment::Good:
        return "Good: barred from the Assassin's Guild and dark places.";
    case Alignment::Evil:
        return "Evil: barred from the Paladin's Guild and holy places.";
    case Alignment::Neutral:
        return "Neutral: no restrictions.";
    }
    return "";
}
