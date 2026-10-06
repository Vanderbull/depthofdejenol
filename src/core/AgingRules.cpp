#include "AgingRules.h"
#include <QRandomGenerator>

int AgingRules::maxAgeForRace(const QString& race) {
    // Mirrors gameStateManager::getRaceAgeLimits(); kept here so the rules are
    // unit-testable without a gameStateManager instance.
    static const QMap<QString, int> limits = {
        {"Human", 100}, {"Elf", 400}, {"Giant", 225},
        {"Gnome", 300}, {"Dwarf", 275}, {"Ogre", 265},
        {"Morloch", 175}, {"Osiri", 325}, {"Troll", 285}
    };
    return limits.value(race, 100);
}

int AgingRules::decayThresholdForRace(const QString& race) {
    return static_cast<int>(maxAgeForRace(race) * 0.7);
}

bool AgingRules::isPastMaxAge(const Character& c) {
    return c.age >= maxAgeForRace(c.race);
}

bool AgingRules::isDecaying(const Character& c) {
    return c.age >= decayThresholdForRace(c.race);
}

QStringList AgingRules::applyYearOfAging(Character& c) {
    QStringList messages;

    // Death at max age takes precedence over decay.
    if (isPastMaxAge(c)) {
        c.isAlive = false;
        c.hp = 0;
        c.addStatus(StatusFlag::Dead);
        messages.append(QString("%1 has died of old age at %2.").arg(c.name).arg(c.age));
        return messages;
    }

    // Past the decay threshold, a small chance each year to lose a point.
    if (isDecaying(c)) {
        if (QRandomGenerator::global()->bounded(100) < 10) {
            c.strength     = qMax(3, c.strength - 1);
            c.constitution = qMax(3, c.constitution - 1);
            c.dexterity    = qMax(3, c.dexterity - 1);
            messages.append(QString("%1 feels the weight of the years (STR/CON/DEX -1).").arg(c.name));
        }
    }

    return messages;
}
