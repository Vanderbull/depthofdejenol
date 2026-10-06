#include "GoldSinks.h"
#include "DeathRecovery.h"

int GoldSinks::resurrectionCost(int characterLevel, bool bodyInDungeon) {
    BodyLocation loc;
    loc.valid = true;
    loc.inCity = !bodyInDungeon;
    loc.dungeonLevel = bodyInDungeon ? 5 : 0;
    return DeathRecovery::resurrectionCost(characterLevel, loc);
}

int GoldSinks::rescueCost(int depth) {
    return DeathRecovery::rescuePartyCost(depth);
}

int GoldSinks::identificationCost() {
    return 50;
}

int GoldSinks::uncurseCost() {
    return 100;
}

int GoldSinks::guildLevelCost(const QString& guildName, int currentLevel) {
    Q_UNUSED(guildName);
    // Guild leveling costs 500 × current level.
    return 500 * (currentLevel + 1);
}

int GoldSinks::restCostPerHour() {
    return 10;
}

int GoldSinks::curePoisonCost() {
    return 50;
}

int GoldSinks::cureBlindnessCost() {
    return 50;
}

int GoldSinks::totalForCategory(const QString& category) {
    if (category == "resurrection") return resurrectionCost(5, false);
    if (category == "rescue") return rescueCost(5);
    if (category == "identification") return identificationCost();
    if (category == "uncurse") return uncurseCost();
    if (category == "guild") return guildLevelCost("Mage", 0);
    if (category == "rest") return restCostPerHour() * 8;
    if (category == "cure") return curePoisonCost() + cureBlindnessCost();
    return 0;
}

QList<QVariantMap> GoldSinks::allSinks() {
    QList<QVariantMap> sinks;

    auto add = [&sinks](const QString& cat, int cost, const QString& desc) {
        QVariantMap m;
        m["category"] = cat;
        m["cost"] = cost;
        m["description"] = desc;
        sinks.append(m);
    };

    add("resurrection", resurrectionCost(5, false), "Resurrect a level-5 character");
    add("rescue", rescueCost(5), "Rescue a party from floor 5");
    add("identification", identificationCost(), "Identify an unknown item");
    add("uncurse", uncurseCost(), "Remove a curse from an item");
    add("guild", guildLevelCost("Mage", 0), "Guild level-up from level 0");
    add("rest", restCostPerHour() * 8, "Rest 8 hours for one member");
    add("cure", curePoisonCost() + cureBlindnessCost(), "Cure poison and blindness");

    return sinks;
}

QString GoldSinks::sinkDescription(const QString& category) {
    if (category == "resurrection") return "Resurrect a dead character at the Morgue.";
    if (category == "rescue") return "Hire rescuers to recover a wiped party.";
    if (category == "identification") return "Identify an unknown item at the General Store.";
    if (category == "uncurse") return "Remove a curse from an item at the General Store.";
    if (category == "guild") return "Level up a guild at the Guild Hall.";
    if (category == "rest") return "Rest at the Tavern to restore HP and mana.";
    if (category == "cure") return "Cure poison or blindness at the Tavern.";
    return "Unknown gold sink.";
}
