#ifndef DEATHRECOVERY_H
#define DEATHRECOVERY_H

#include "character.h"
#include <QString>
#include <QStringList>
#include <QList>

// Where a fallen character's body is.
struct BodyLocation {
    bool valid = false;
    bool inCity = false;   // body has been carried back to town
    bool carried = false;  // a living member is currently carrying it
    int dungeonLevel = 0;
    int x = 0;
    int y = 0;
};

// Death, body carrying, resurrection and party-wipe rescue.
// Pure logic — the dialogs own the presentation.
class DeathRecovery {
public:
    // --- 5.1 Death state and body carrying ---

    // Mark a character dead and record where the body fell.
    static void killCharacter(Character& c, int level, int x, int y);

    // Can this body be carried? Bodies in town are already home.
    static bool canCarry(const BodyLocation& loc);

    // Carry a body: it becomes carried by the party and moves with them.
    static bool carryBody(BodyLocation& loc, QString& reason);

    // Drop a carried body at the current position.
    static void dropBody(BodyLocation& loc, int level, int x, int y);

    // The party has returned to town: every carried body arrives with them.
    // Returns the number of bodies brought home.
    static int bringBodiesToTown(QList<BodyLocation>& bodies);

    // --- 5.2 Morgue / resurrection ---

    // Cost to resurrect a character. Scales with level; bodies still in the
    // dungeon cost more (a rescue fee).
    static int resurrectionCost(int characterLevel, const BodyLocation& loc);

    // Attempt resurrection. Deducts gold via `partyGold` on success.
    // Returns false (and fills `reason`) when gold is short or the body is missing.
    static bool resurrect(Character& c, BodyLocation& loc, int& partyGold, QString& reason);

    // --- 5.3 Party wipe and rescue ---

    // Is the whole party down?
    static bool isPartyWiped(const QList<Character>& party);

    // A wiped party is stranded; a rescue party can be formed from town.
    static bool needsRescue(const QList<Character>& party);

    // Cost to hire a rescue party, scaled by how deep the bodies are.
    static int rescuePartyCost(int deepestBodyLevel);

    // Recover every body on the given floor after a successful rescue.
    // Returns how many bodies were recovered.
    static int recoverBodies(QList<BodyLocation>& bodies, int level);

    // Hardcore mode: dead characters are gone for good.
    static bool isPermanentlyDead(bool hardcoreMode) { return hardcoreMode; }
};

#endif // DEATHRECOVERY_H
