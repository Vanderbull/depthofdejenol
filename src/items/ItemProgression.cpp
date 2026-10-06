#include "ItemProgression.h"

QString ItemProgression::tierName(Tier tier) {
    switch (tier) {
    case Tier::Bronze: return "Bronze";
    case Tier::Iron: return "Iron";
    case Tier::Steel: return "Steel";
    case Tier::Adamantite: return "Adamantite";
    case Tier::Mithril: return "Mithril";
    }
    return "Unknown";
}

ItemProgression::Tier ItemProgression::tierFromName(const QString& name) {
    if (name == "Bronze") return Tier::Bronze;
    if (name == "Iron") return Tier::Iron;
    if (name == "Steel") return Tier::Steel;
    if (name == "Adamantite") return Tier::Adamantite;
    if (name == "Mithril") return Tier::Mithril;
    return Tier::Bronze;
}

QList<ItemProgression::Tier> ItemProgression::allTiers() {
    return {Tier::Bronze, Tier::Iron, Tier::Steel, Tier::Adamantite, Tier::Mithril};
}

ItemProgression::Tier ItemProgression::tierForFloor(int floor) {
    if (floor <= 3) return Tier::Bronze;
    if (floor <= 6) return Tier::Iron;
    if (floor <= 10) return Tier::Steel;
    if (floor <= 13) return Tier::Adamantite;
    return Tier::Mithril;
}

int ItemProgression::minFloorForTier(Tier tier) {
    switch (tier) {
    case Tier::Bronze: return 1;
    case Tier::Iron: return 3;
    case Tier::Steel: return 6;
    case Tier::Adamantite: return 10;
    case Tier::Mithril: return 13;
    }
    return 1;
}

int ItemProgression::maxFloorForTier(Tier tier) {
    switch (tier) {
    case Tier::Bronze: return 3;
    case Tier::Iron: return 6;
    case Tier::Steel: return 10;
    case Tier::Adamantite: return 13;
    case Tier::Mithril: return 15;
    }
    return 15;
}

double ItemProgression::statMultiplier(Tier tier) {
    switch (tier) {
    case Tier::Bronze: return 1.0;
    case Tier::Iron: return 1.5;
    case Tier::Steel: return 2.0;
    case Tier::Adamantite: return 3.0;
    case Tier::Mithril: return 4.0;
    }
    return 1.0;
}

double ItemProgression::costMultiplier(Tier tier) {
    switch (tier) {
    case Tier::Bronze: return 1.0;
    case Tier::Iron: return 2.0;
    case Tier::Steel: return 4.0;
    case Tier::Adamantite: return 8.0;
    case Tier::Mithril: return 16.0;
    }
    return 1.0;
}

bool ItemProgression::isAvailable(Tier tier, int floor) {
    return floor >= minFloorForTier(tier) && floor <= maxFloorForTier(tier);
}

QString ItemProgression::tierDescription(Tier tier) {
    switch (tier) {
    case Tier::Bronze: return "Basic equipment for beginners. Found on floors 1-3.";
    case Tier::Iron: return "Standard equipment for adventurers. Found on floors 3-6.";
    case Tier::Steel: return "Quality equipment for experienced parties. Found on floors 6-10.";
    case Tier::Adamantite: return "Rare equipment for deep delvers. Found on floors 10-13.";
    case Tier::Mithril: return "Legendary equipment for the Prince's court. Found on floors 13-15.";
    }
    return "";
}

QStringList ItemProgression::itemsOfTier(const QStringList& itemNames, Tier tier) {
    QStringList result;
    QString prefix = tierName(tier);
    for (const QString& name : itemNames) {
        if (name.startsWith(prefix, Qt::CaseInsensitive)) {
            result.append(name);
        }
    }
    return result;
}
