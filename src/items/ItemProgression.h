#ifndef ITEMPROGRESSION_H
#define ITEMPROGRESSION_H

#include <QString>
#include <QStringList>
#include <QList>

// Item progression curve: Bronze → Iron → Steel → Adamantite → Mithril.
class ItemProgression {
public:
    enum class Tier { Bronze, Iron, Steel, Adamantite, Mithril };

    // Display name for a tier.
    static QString tierName(Tier tier);

    // Tier from a name.
    static Tier tierFromName(const QString& name);

    // All tiers in order.
    static QList<Tier> allTiers();

    // Tier for a dungeon floor.
    static Tier tierForFloor(int floor);

    // Minimum floor where a tier appears.
    static int minFloorForTier(Tier tier);

    // Maximum floor where a tier appears.
    static int maxFloorForTier(Tier tier);

    // Stat multiplier for a tier (1.0 = Bronze).
    static double statMultiplier(Tier tier);

    // Cost multiplier for a tier (1.0 = Bronze).
    static double costMultiplier(Tier tier);

    // Is this tier available on this floor?
    static bool isAvailable(Tier tier, int floor);

    // A short description of the tier.
    static QString tierDescription(Tier tier);

    // All items of a tier (by name prefix).
    static QStringList itemsOfTier(const QStringList& itemNames, Tier tier);
};

#endif // ITEMPROGRESSION_H
