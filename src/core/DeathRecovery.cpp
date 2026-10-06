#include "DeathRecovery.h"

// --- 5.1 Death state and body carrying ---

void DeathRecovery::killCharacter(Character& c, int level, int x, int y) {
    c.addStatus(StatusFlag::Dead);
    c.hp = 0;
    c.isAlive = false;
    // The body stays where it fell so it can be found again.
    c.dungeonLevel = level;
    c.dungeonX = x;
    c.dungeonY = y;
}

bool DeathRecovery::canCarry(const BodyLocation& loc) {
    return loc.valid && !loc.carried && !loc.inCity;
}

bool DeathRecovery::carryBody(BodyLocation& loc, QString& reason) {
    if (!loc.valid) {
        reason = "There is no body here to carry.";
        return false;
    }
    if (loc.inCity) {
        reason = "The body is already resting in town.";
        return false;
    }
    if (loc.carried) {
        reason = "You are already carrying that body.";
        return false;
    }
    loc.carried = true;
    reason = "You shoulder the body. It will come with you.";
    return true;
}

void DeathRecovery::dropBody(BodyLocation& loc, int level, int x, int y) {
    loc.carried = false;
    loc.inCity = false;
    loc.dungeonLevel = level;
    loc.x = x;
    loc.y = y;
}

int DeathRecovery::bringBodiesToTown(QList<BodyLocation>& bodies) {
    int brought = 0;
    for (BodyLocation& loc : bodies) {
        if (loc.valid && loc.carried) {
            loc.inCity = true;
            loc.carried = false;
            brought++;
        }
    }
    return brought;
}

// --- 5.2 Morgue / resurrection ---

int DeathRecovery::resurrectionCost(int characterLevel, const BodyLocation& loc) {
    int base = qMax(1, characterLevel) * 500;
    // A body still lying in the dungeon costs extra to recover.
    if (loc.valid && !loc.inCity) {
        base += 500;
    }
    return base;
}

bool DeathRecovery::resurrect(Character& c, BodyLocation& loc, int& partyGold, QString& reason) {
    if (c.isAlive) {
        reason = QString("%1 is not dead.").arg(c.name);
        return false;
    }
    if (!loc.valid) {
        reason = "The body cannot be found.";
        return false;
    }

    int cost = resurrectionCost(c.level, loc);
    if (partyGold < cost) {
        reason = QString("Resurrection costs %1 gold (you have %2).").arg(cost).arg(partyGold);
        return false;
    }

    partyGold -= cost;
    c.resurrect();
    loc.valid = false;
    loc.carried = false;
    loc.inCity = false;
    reason = QString("%1 breathes again.").arg(c.name);
    return true;
}

// --- 5.3 Party wipe and rescue ---

bool DeathRecovery::isPartyWiped(const QList<Character>& party) {
    if (party.isEmpty()) return false;
    for (const Character& c : party) {
        if (c.isAlive) return false;
    }
    return true;
}

bool DeathRecovery::needsRescue(const QList<Character>& party) {
    return isPartyWiped(party);
}

int DeathRecovery::rescuePartyCost(int deepestBodyLevel) {
    if (deepestBodyLevel < 1) deepestBodyLevel = 1;
    // Deeper rescues are far more expensive.
    return deepestBodyLevel * deepestBodyLevel * 250;
}

int DeathRecovery::recoverBodies(QList<BodyLocation>& bodies, int level) {
    int recovered = 0;
    for (BodyLocation& loc : bodies) {
        if (loc.valid && !loc.inCity && loc.dungeonLevel == level) {
            loc.inCity = true;
            loc.carried = false;
            recovered++;
        }
    }
    return recovered;
}
