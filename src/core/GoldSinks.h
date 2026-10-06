#ifndef GOLDSINKS_H
#define GOLDSINKS_H

#include <QString>
#include <QList>
#include <QVariantMap>

// Gold sinks: resurrection, identification, uncursing, guild leveling.
class GoldSinks {
public:
    // Resurrection cost for a character at a level, body in town or dungeon.
    static int resurrectionCost(int characterLevel, bool bodyInDungeon);

    // Rescue party cost for a depth.
    static int rescueCost(int depth);

    // Identification cost for an item.
    static int identificationCost();

    // Uncurse cost for an item.
    static int uncurseCost();

    // Guild level-up cost for a guild at a level.
    static int guildLevelCost(const QString& guildName, int currentLevel);

    // Rest cost per hour per member.
    static int restCostPerHour();

    // Cure poison cost.
    static int curePoisonCost();

    // Cure blindness cost.
    static int cureBlindnessCost();

    // Total gold sunk in a category.
    static int totalForCategory(const QString& category);

    // All gold sink categories with their costs.
    static QList<QVariantMap> allSinks();

    // A description of what each sink does.
    static QString sinkDescription(const QString& category);
};

#endif // GOLDSINKS_H
