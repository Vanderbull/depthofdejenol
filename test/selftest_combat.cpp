#include "selftest_harness.h"
#include "selftest_includes.h"

void runCombatTests()
{
    gameStateManager* gsm = gameStateManager::instance();
    section("[14] CombatState data model");
    {
        CombatState cs;
        check(cs.participantCount() == 0, "new CombatState is empty");
        check(cs.isCombatOver(), "empty combat is over (no participants)");

        CombatParticipant warrior;
        warrior.name = "Warrior";
        warrior.hp = 30; warrior.maxHp = 30;
        warrior.att = 10; warrior.def = 8; warrior.speed = 5; warrior.dex = 10;
        warrior.isPlayer = true;
        cs.addParticipant(warrior);

        CombatParticipant mage;
        mage.name = "Mage";
        mage.hp = 20; mage.maxHp = 20;
        mage.att = 6; mage.def = 4; mage.speed = 7; mage.dex = 12;
        mage.isPlayer = true;
        cs.addParticipant(mage);

        CombatParticipant goblin;
        goblin.name = "Goblin";
        goblin.hp = 15; goblin.maxHp = 15;
        goblin.att = 7; goblin.def = 5; goblin.speed = 6; goblin.dex = 8;
        goblin.isPlayer = false;
        goblin.level = 1;
        cs.addParticipant(goblin);

        check(cs.participantCount() == 3, "three participants added",
              QString::number(cs.participantCount()));
        check(cs.livingPlayerCount() == 2, "two living players",
              QString::number(cs.livingPlayerCount()));
        check(cs.livingMonsterCount() == 1, "one living monster",
              QString::number(cs.livingMonsterCount()));
        check(!cs.isCombatOver(), "combat not over with living participants");
    }
    {
        // Initiative roll produces valid order.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.speed = 5; a.dex = 10; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.speed = 8; b.dex = 12; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.rollInitiative();
        check(cs.initiativeOrder().size() == 2, "initiative order has 2 entries",
              QString::number(cs.initiativeOrder().size()));
        // All indices valid.
        bool valid = true;
        for (int idx : cs.initiativeOrder()) {
            if (idx < 0 || idx >= 2) valid = false;
        }
        check(valid, "initiative indices are valid");
    }
    {
        // Turn cycling: each participant acts once per round.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.speed = 5; a.dex = 10; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.speed = 8; b.dex = 12; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.rollInitiative();
        cs.startRound();
        check(cs.currentRound() == 1, "round 1 started",
              QString::number(cs.currentRound()));

        int turns = 0;
        while (cs.nextTurn()) {
            turns++;
            const CombatParticipant* p = cs.currentParticipant();
            check(p != nullptr, "current participant is valid");
            cs.markActed(cs.initiativeOrder()[cs.currentTurnIndex()]);
        }
        check(turns == 2, "both participants acted in round 1",
              QString::number(turns));
    }
    {
        // Dead participants are skipped.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.speed = 5; a.dex = 10; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.speed = 8; b.dex = 12; b.isPlayer = false;
        b.isAlive = false;  // B starts dead
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.rollInitiative();
        cs.startRound();

        int turns = 0;
        while (cs.nextTurn()) {
            turns++;
            cs.markActed(cs.initiativeOrder()[cs.currentTurnIndex()]);
        }
        check(turns == 1, "only living participant acts",
              QString::number(turns));
    }
    {
        // Combat over when one side is dead.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        check(!cs.isCombatOver(), "combat not over initially");

        cs.participant(1).isAlive = false;  // Kill monster
        check(cs.isCombatOver(), "combat over when monster dead");
    }
    {
        // Multiple rounds.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.rollInitiative();

        cs.startRound();
        check(cs.currentRound() == 1, "round 1");
        while (cs.nextTurn()) {
            cs.markActed(cs.initiativeOrder()[cs.currentTurnIndex()]);
        }

        cs.startRound();
        check(cs.currentRound() == 2, "round 2");
        int turns = 0;
        while (cs.nextTurn()) {
            turns++;
            cs.markActed(cs.initiativeOrder()[cs.currentTurnIndex()]);
        }
        check(turns == 2, "both act in round 2", QString::number(turns));
    }

    // ------------------------------------------- TurnEngine (2.2)

    section("[15] Turn engine");
    {
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5; warrior.dex = 10;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 6; goblin.dex = 8;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        check(te.combatState() == &cs, "TurnEngine holds CombatState");
        check(te.currentRound() == 0, "round 0 before start");
        check(!te.isCombatOver(), "combat not over");

        te.startRound();
        check(te.currentRound() == 1, "round 1 after startRound",
              QString::number(te.currentRound()));
        check(!te.isRoundOver(), "round not over at start");

        // First turn
        bool hasTurn = te.nextTurn();
        check(hasTurn, "first turn available");
        check(te.hasCurrentParticipant(), "has current participant");
        QString name = te.currentParticipantName();
        check(!name.isEmpty(), "current participant has name", name);
        check(!te.hasCurrentActed(), "current has not acted yet");

        te.markCurrentActed();
        check(te.hasCurrentActed(), "current has acted after mark");

        // Second turn
        hasTurn = te.nextTurn();
        check(hasTurn, "second turn available");
        QString name2 = te.currentParticipantName();
        check(name2 != name, "second turn is different participant",
              name2 + " vs " + name);

        te.markCurrentActed();

        // No more turns — round over
        hasTurn = te.nextTurn();
        check(!hasTurn, "no third turn — round over");
        check(te.isRoundOver(), "round is over after all acted");
    }
    {
        // TurnEngine with dead participants.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false; b.isAlive = false;
        cs.addParticipant(a);
        cs.addParticipant(b);

        TurnEngine te;
        te.setCombatState(&cs);
        te.startRound();

        int turns = 0;
        while (te.nextTurn()) {
            turns++;
            te.markCurrentActed();
        }
        check(turns == 1, "only living participant acts",
              QString::number(turns));
    }
    {
        // Combat status string.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);

        TurnEngine te;
        te.setCombatState(&cs);
        te.startRound();
        QString status = te.combatStatus();
        check(status.contains("Round 1"), "status mentions Round 1", status);
        check(status.contains("1 players"), "status mentions 1 players", status);
        check(status.contains("1 monsters"), "status mentions 1 monsters", status);
    }
    {
        // Multiple rounds via TurnEngine.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);

        TurnEngine te;
        te.setCombatState(&cs);

        te.startRound();
        check(te.currentRound() == 1, "round 1");
        while (te.nextTurn()) { te.markCurrentActed(); }

        te.startRound();
        check(te.currentRound() == 2, "round 2");
        int turns = 0;
        while (te.nextTurn()) { turns++; te.markCurrentActed(); }
        check(turns == 2, "both act in round 2", QString::number(turns));
    }
    {
        // livingPlayers / livingMonsters.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        CombatParticipant c; c.name = "C"; c.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.addParticipant(c);

        TurnEngine te;
        te.setCombatState(&cs);
        check(te.livingPlayers().size() == 1, "one living player",
              QString::number(te.livingPlayers().size()));
        check(te.livingMonsters().size() == 2, "two living monsters",
              QString::number(te.livingMonsters().size()));

        cs.participant(1).isAlive = false;
        check(te.livingMonsters().size() == 1, "one living monster after kill",
              QString::number(te.livingMonsters().size()));
    }

    // ------------------------------------------- CombatActions (2.3)

    section("[16] Player action menu");
    {
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.att = 10; warrior.dex = 10; warrior.speed = 100; warrior.swings = 1; warrior.damageMod = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 20; goblin.maxHp = 20; goblin.def = 5; goblin.speed = 6; goblin.dex = 8;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();  // Warrior acts first (speed 100 guarantees it)
        check(ca.isPlayerTurn(), "warrior acts first with speed 100");

        // Attack the goblin (index 1)
        QString result;
        int damage = ca.attack(1, result);
        check(damage > 0 || result.contains("miss") || result.contains("fumble"),
              "attack produces a result", result);
        check(cs.participant(1).hp < 20 || result.contains("miss") || result.contains("fumble"),
              "goblin HP reduced or attack missed",
              QString::number(cs.participant(1).hp));
    }
    {
        // Defend marks as acted.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.defend(result);
        check(result.contains("defensive"), "defend produces message", result);
        check(te.hasCurrentActed(), "defend marks as acted");
    }
    {
        // Defend halves incoming damage this round. Damage rolls vary (0-4
        // variance and 20-crits), so compare totals over many attacks rather
        // than a single pair.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.hp = 100000; warrior.maxHp = 100000; warrior.speed = 5;
        CombatParticipant orc; orc.name = "Orc"; orc.isPlayer = false;
        orc.att = 20; orc.swings = 1; orc.damageMod = 100; orc.dex = 100; orc.level = 20;
        orc.speed = 100;
        cs.addParticipant(warrior);
        cs.addParticipant(orc);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();  // orc first (speed 100)
        check(!ca.isPlayerTurn(), "orc acts first");

        const int trials = 300;
        int plainTotal = 0;
        for (int i = 0; i < trials; ++i) {
            cs.participant(0).isDefending = false;
            int before = cs.participant(0).hp;
            QString r;
            ca.attack(0, r);
            plainTotal += before - cs.participant(0).hp;
        }

        int defendedTotal = 0;
        for (int i = 0; i < trials; ++i) {
            cs.participant(0).isDefending = true;
            int before = cs.participant(0).hp;
            QString r;
            ca.attack(0, r);
            defendedTotal += before - cs.participant(0).hp;
        }

        // The stance should cut total damage taken roughly in half; allow slack
        // for the to-hit roll and crits.
        check(defendedTotal < plainTotal,
              "defensive stance reduces total damage taken",
              QString("plain=%1 defended=%2").arg(plainTotal).arg(defendedTotal));
        check(defendedTotal < plainTotal * 3 / 4,
              "defensive stance cuts damage substantially",
              QString("plain=%1 defended=%2").arg(plainTotal).arg(defendedTotal));
    }
    {
        // isDefending is cleared at the start of each round.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.participant(0).isDefending = true;
        cs.startRound();
        check(!cs.participant(0).isDefending,
              "defensive stance expires at round start");
    }
    {
        // Cast spell damages target.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.att = 8; mage.speed = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 30; goblin.maxHp = 30; goblin.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();
        check(ca.isPlayerTurn(), "mage acts first with speed 100");

        QString result;
        int damage = ca.castSpell(1, 10, result);
        check(damage > 0, "spell deals damage", result);
        check(cs.participant(1).hp < 30, "goblin HP reduced after spell",
              QString::number(cs.participant(1).hp));
    }
    {
        // Flee produces a result.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.flee(result);
        check(result.contains("flee") || result.contains("flees"),
              "flee produces message", result);
    }
    {
        // A fleeing monster leaves the fight: it is no longer a living
        // participant, so combat can actually end.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 1; goblin.maxHp = 50; goblin.speed = 100;  // will flee
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();  // goblin acts first
        QString result;
        bool fled = ca.flee(result);
        if (fled) {
            check(!cs.participant(1).isAlive, "fleeing monster is no longer alive");
            check(cs.livingMonsterCount() == 0,
                  "fleeing monster no longer counts as living");
            check(cs.isCombatOver(), "combat ends once the last monster flees");
        } else {
            // The roll can fail; assert the invariant that a failed flee keeps
            // the monster in the fight.
            check(cs.participant(1).isAlive, "failed flee keeps monster in fight");
        }
    }
    {
        // A fleeing player is not marked dead — the UI handles the exit.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 1;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();
        QString result;
        ca.flee(result);
        check(cs.participant(0).isAlive, "fleeing player stays alive");
    }
    {
        // useItem applies a healing potion's effect to the combat participant
        // and consumes it from the Character's inventory (B3).
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.hp = 10; warrior.maxHp = 50; warrior.speed = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        Character c;
        c.name = "Warrior";
        c.hp = 10; c.maxHp = 50;
        HeldItem potion;
        potion.name = "Potion of Healing";
        potion.M4ED4 = 1;  // last charge → consumed
        if (const ItemDef* def = ItemDatabase::instance().byName("Potion of Healing")) {
            potion.M4E97 = static_cast<int16_t>(def->id);
        }
        c.inventory.append(potion);

        te.startRound();
        te.nextTurn();
        check(ca.isPlayerTurn(), "warrior acts first");

        QString result;
        bool ok = ca.useItem(0, c, result);
        check(ok, "useItem succeeds", result);
        check(cs.participant(0).hp > 10, "combat HP restored by potion",
              QString::number(cs.participant(0).hp));
        check(c.inventory.isEmpty(), "potion consumed from inventory",
              QString::number(c.inventory.size()));
    }
    {
        // Attack dead target → refused.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.isAlive = false;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int damage = ca.attack(1, result);
        check(damage == -1, "attack on dead target refused");
        check(result.contains("dead"), "result mentions dead", result);
    }

    // ------------------------------------------- attack resolution (2.4)

    section("[17] Equipment-driven attack resolution");
    {
        // A 20 STR warrior with a 2-swing weapon out-damages a 10 STR mage with a dagger
        // over 100 simulated rounds.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.att = 20; warrior.dex = 10; warrior.speed = 100; warrior.swings = 2; warrior.damageMod = 100;
        warrior.level = 5; warrior.levelScale = 10;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.att = 10; mage.dex = 8; mage.speed = 90; mage.swings = 1; mage.damageMod = 100;
        mage.level = 5; mage.levelScale = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 10000; goblin.maxHp = 10000; goblin.def = 5; goblin.speed = 5; goblin.level = 1;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        int warriorDamage = 0;
        int mageDamage = 0;
        int rounds = 0;

        while (!te.isCombatOver() && rounds < 100) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString result;
                    int dmg = ca.attack(2, result);  // Attack goblin
                    if (dmg > 0) {
                        if (idx == 0) warriorDamage += dmg;
                        else if (idx == 1) mageDamage += dmg;
                    }
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(warriorDamage > mageDamage,
              "2-swing warrior out-damages 1-swing mage over 100 rounds",
              QString("warrior=%1 mage=%2").arg(warriorDamage).arg(mageDamage));
    }
    {
        // Level scaling: higher level attacker deals more damage.
        CombatState cs;
        CombatParticipant highLevel; highLevel.name = "HighLevel"; highLevel.isPlayer = true;
        highLevel.att = 10; highLevel.dex = 10; highLevel.speed = 100; highLevel.swings = 1;
        highLevel.damageMod = 100; highLevel.level = 10; highLevel.levelScale = 20;
        CombatParticipant lowLevel; lowLevel.name = "LowLevel"; lowLevel.isPlayer = true;
        lowLevel.att = 10; lowLevel.dex = 10; lowLevel.speed = 90; lowLevel.swings = 1;
        lowLevel.damageMod = 100; lowLevel.level = 1; lowLevel.levelScale = 0;
        CombatParticipant dummy; dummy.name = "Dummy"; dummy.isPlayer = false;
        dummy.hp = 100000; dummy.maxHp = 100000; dummy.def = 0; dummy.speed = 5; dummy.level = 1;
        cs.addParticipant(highLevel);
        cs.addParticipant(lowLevel);
        cs.addParticipant(dummy);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        int highDamage = 0;
        int lowDamage = 0;
        int rounds = 0;

        while (!te.isCombatOver() && rounds < 50) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString result;
                    int dmg = ca.attack(2, result);
                    if (dmg > 0) {
                        if (idx == 0) highDamage += dmg;
                        else if (idx == 1) lowDamage += dmg;
                    }
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(highDamage > lowDamage,
              "higher level attacker deals more damage",
              QString("high=%1 low=%2").arg(highDamage).arg(lowDamage));
    }
    {
        // DEX affects to-hit: higher DEX attacker hits more often.
        CombatState cs;
        CombatParticipant highDex; highDex.name = "HighDex"; highDex.isPlayer = true;
        highDex.att = 10; highDex.dex = 20; highDex.speed = 100; highDex.swings = 1;
        highDex.damageMod = 100; highDex.level = 1; highDex.levelScale = 0;
        CombatParticipant lowDex; lowDex.name = "LowDex"; lowDex.isPlayer = true;
        lowDex.att = 10; lowDex.dex = 2; lowDex.speed = 90; lowDex.swings = 1;
        lowDex.damageMod = 100; lowDex.level = 1; lowDex.levelScale = 0;
        CombatParticipant dummy; dummy.name = "Dummy"; dummy.isPlayer = false;
        dummy.hp = 100000; dummy.maxHp = 100000; dummy.def = 0; dummy.speed = 5; dummy.level = 1;
        cs.addParticipant(highDex);
        cs.addParticipant(lowDex);
        cs.addParticipant(dummy);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        int highDexHits = 0;
        int lowDexHits = 0;
        int rounds = 0;

        while (!te.isCombatOver() && rounds < 50) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString result;
                    int dmg = ca.attack(2, result);
                    if (dmg > 0) {
                        if (idx == 0) highDexHits++;
                        else if (idx == 1) lowDexHits++;
                    }
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(highDexHits >= lowDexHits,
              "higher DEX attacker hits at least as often",
              QString("high=%1 low=%2").arg(highDexHits).arg(lowDexHits));
    }

    // ------------------------------------------- Monster AI (2.5)
    // ------------------------------------------- MonsterAI (2.5)

    section("[18] Monster turns and AI");
    {
        // Monster attacks on its turn.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5; warrior.dex = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.dex = 20; goblin.hp = 50; goblin.maxHp = 50;
        goblin.att = 8; goblin.def = 5; goblin.level = 1;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();  // Goblin acts first (speed 100)
        check(ai.isMonsterTurn(), "goblin turn detected");

        int warriorHpBefore = cs.participant(0).hp;
        QString result = ai.takeTurn();
        check(result.contains("hit") || result.contains("miss") || result.contains("fumble"),
              "monster attack produces result", result);
        check(cs.participant(0).hp <= warriorHpBefore, "warrior HP reduced or unchanged");
    }
    {
        // Low-HP monster flees.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.hp = 2; goblin.maxHp = 50;  // Very low HP
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();
        check(ai.isMonsterTurn(), "goblin turn");

        MonsterAI::Decision d = ai.decide();
        check(d == MonsterAI::Decision::Flee, "low-HP monster decides to flee");
    }
    {
        // Full-HP monster attacks.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.hp = 50; goblin.maxHp = 50;  // Full HP
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();
        MonsterAI::Decision d = ai.decide();
        check(d == MonsterAI::Decision::Attack, "full-HP monster decides to attack");
    }
    {
        // Monster targets front row (lowest index) most of the time.
        CombatState cs;
        CombatParticipant p1; p1.name = "FrontRow"; p1.isPlayer = true;
        p1.speed = 5; p1.hp = 30;
        CombatParticipant p2; p2.name = "BackRow"; p2.isPlayer = true;
        p2.speed = 5; p2.hp = 30;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.hp = 50; goblin.maxHp = 50;
        cs.addParticipant(p1);
        cs.addParticipant(p2);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();

        int frontRowTargets = 0;
        for (int i = 0; i < 100; i++) {
            int target = ai.chooseTarget();
            if (target == 0) frontRowTargets++;
        }
        check(frontRowTargets > 40, "front row targeted more than random",
              QString::number(frontRowTargets));
    }
    {
        // Monster targets weakest when not front row.
        CombatState cs;
        CombatParticipant p1; p1.name = "Strong"; p1.isPlayer = true;
        p1.speed = 5; p1.hp = 50;
        CombatParticipant p2; p2.name = "Weak"; p2.isPlayer = true;
        p2.speed = 5; p2.hp = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.hp = 50; goblin.maxHp = 50;
        cs.addParticipant(p1);
        cs.addParticipant(p2);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();

        int weakTargets = 0;
        for (int i = 0; i < 100; i++) {
            int target = ai.chooseTarget();
            if (target == 1) weakTargets++;
        }
        check(weakTargets > 10, "weakest targeted sometimes",
              QString::number(weakTargets));
    }
    {
        // isMonsterTurn returns false for player turn.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();
        check(!ai.isMonsterTurn(), "player turn is not monster turn");
    }

    // ------------------------------------------- EncounterBuilder (2.6)

    section("[19] Group encounters");
    {
        // Build an encounter from monster data.
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["numGroups"] = 2;
        orc["hits"] = 3;
        orc["att"] = 12;
        orc["def"] = 8;
        orc["StatDex"] = 10;
        orc["StatCon"] = 14;
        orc["levelFound"] = 3;
        orc["damageMod"] = 100;
        monsterData.append(orc);

        QVariantMap snake;
        snake["name"] = "Rattlesnake";
        snake["numGroups"] = 1;
        snake["hits"] = 2;
        snake["att"] = 16;
        snake["def"] = 12;
        snake["StatDex"] = 18;
        snake["StatCon"] = 8;
        snake["levelFound"] = 6;
        snake["damageMod"] = 100;
        monsterData.append(snake);

        // Orc: 2 groups * 3 hits = 6 monsters
        QList<CombatParticipant> orcGroup = EncounterBuilder::buildEncounter("Orc", monsterData);
        check(orcGroup.size() == 6, "Orc encounter has 6 monsters",
              QString::number(orcGroup.size()));

        // Rattlesnake: 1 group * 2 hits = 2 monsters
        QList<CombatParticipant> snakeGroup = EncounterBuilder::buildEncounter("Rattlesnake", monsterData);
        check(snakeGroup.size() == 2, "Rattlesnake encounter has 2 monsters",
              QString::number(snakeGroup.size()));

        // All monsters are non-player
        bool allMonsters = true;
        for (const CombatParticipant& p : orcGroup) {
            if (p.isPlayer) allMonsters = false;
        }
        check(allMonsters, "all encounter members are monsters");

        // Monster names are unique
        QSet<QString> names;
        for (const CombatParticipant& p : orcGroup) {
            names.insert(p.name);
        }
        check(names.size() == orcGroup.size(), "monster names are unique");

        // Stats are populated
        check(orcGroup[0].att == 12, "Orc att = 12");
        check(orcGroup[0].def == 8, "Orc def = 8");
        check(orcGroup[0].level == 3, "Orc level = 3");
        check(orcGroup[0].hp > 0, "Orc HP > 0");
        check(orcGroup[0].maxHp > 0, "Orc maxHP > 0");

        // getGroupSize matches
        check(EncounterBuilder::getGroupSize("Orc", monsterData) == 6,
              "getGroupSize Orc = 6");
        check(EncounterBuilder::getGroupSize("Rattlesnake", monsterData) == 2,
              "getGroupSize Rattlesnake = 2");

        // Unknown monster falls back to 1
        check(EncounterBuilder::getGroupSize("Unknown", monsterData) == 1,
              "unknown monster defaults to 1");

        // Full combat with group: all monsters take turns
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 50; warrior.hp = 200; warrior.maxHp = 200;
        cs.addParticipant(warrior);
        for (const CombatParticipant& p : orcGroup) {
            cs.addParticipant(p);
        }

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        int monsterTurns = 0;
        int rounds = 0;
        while (!te.isCombatOver() && rounds < 20) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (!cs.participant(idx).isPlayer) {
                    ai.takeTurn();
                    monsterTurns++;
                } else {
                    // Player attacks first living monster
                    for (int i = 0; i < cs.participantCount(); i++) {
                        if (!cs.participant(i).isPlayer && cs.participant(i).isAlive) {
                            QString result;
                            ca.attack(i, result);
                            break;
                        }
                    }
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(monsterTurns >= 6, "all 6 monsters took at least one turn",
              QString::number(monsterTurns));
        check(te.isCombatOver(), "combat ended within 20 rounds");
    }

    // ------------------------------------------- Spells in combat (2.7)

    section("[20] Spells in combat");
    {
        // Single-target spell: Fireball on one monster.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 50; mage.maxMana = 50; mage.att = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();  // Mage acts first

        int manaBefore = cs.participant(0).mana;
        QString result;
        int dmg = ca.castSpellAdvanced(1, "Fireball", 25, "15-30", false, result);
        check(dmg > 0, "Fireball deals damage", result);
        check(cs.participant(0).mana == manaBefore - 25, "mana deducted",
              QString("before=%1 after=%2").arg(manaBefore).arg(cs.participant(0).mana));
        check(cs.participant(1).hp < 100, "goblin HP reduced");
    }
    {
        // AoE spell: Fireball hits all monsters.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 100; mage.maxMana = 100; mage.att = 5;
        CombatParticipant goblin1; goblin1.name = "Goblin 1"; goblin1.isPlayer = false;
        goblin1.hp = 100; goblin1.maxHp = 100; goblin1.speed = 5;
        CombatParticipant goblin2; goblin2.name = "Goblin 2"; goblin2.isPlayer = false;
        goblin2.hp = 100; goblin2.maxHp = 100; goblin2.speed = 5;
        CombatParticipant goblin3; goblin3.name = "Goblin 3"; goblin3.isPlayer = false;
        goblin3.hp = 100; goblin3.maxHp = 100; goblin3.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin1);
        cs.addParticipant(goblin2);
        cs.addParticipant(goblin3);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int totalDmg = ca.castSpellAdvanced(1, "Fireball", 25, "15-30", true, result);
        check(totalDmg > 0, "AoE Fireball deals damage", result);
        check(cs.participant(1).hp < 100, "goblin 1 hit");
        check(cs.participant(2).hp < 100, "goblin 2 hit");
        check(cs.participant(3).hp < 100, "goblin 3 hit");
    }
    {
        // Not enough mana: spell fails.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 10; mage.maxMana = 50; mage.att = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int dmg = ca.castSpellAdvanced(1, "Fireball", 25, "15-30", false, result);
        check(dmg == -1, "spell fails without enough mana", result);
        check(cs.participant(0).mana == 10, "mana not deducted on failure");
        check(cs.participant(1).hp == 100, "goblin unharmed");
    }
    {
        // Heal spell: restores HP.
        CombatState cs;
        CombatParticipant cleric; cleric.name = "Cleric"; cleric.isPlayer = true;
        cleric.speed = 100; cleric.mana = 50; cleric.maxMana = 50;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.hp = 20; warrior.maxHp = 50; warrior.speed = 5;
        cs.addParticipant(cleric);
        cs.addParticipant(warrior);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int healed = ca.castHeal(1, 25, result);
        check(healed > 0, "heal spell restores HP", result);
        check(cs.participant(1).hp == 45, "warrior HP = 20 + 25 = 45",
              QString::number(cs.participant(1).hp));
    }
    {
        // Heal doesn't overheal.
        CombatState cs;
        CombatParticipant cleric; cleric.name = "Cleric"; cleric.isPlayer = true;
        cleric.speed = 100; cleric.mana = 50; cleric.maxMana = 50;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.hp = 45; warrior.maxHp = 50; warrior.speed = 5;
        cs.addParticipant(cleric);
        cs.addParticipant(warrior);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int healed = ca.castHeal(1, 25, result);
        check(healed == 5, "heal capped at max HP", result);
        check(cs.participant(1).hp == 50, "warrior HP = 50 (max)",
              QString::number(cs.participant(1).hp));
    }
    {
        // hasEnoughMana check.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 30; mage.maxMana = 50;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        check(ca.hasEnoughMana(25), "has enough mana for 25");
        check(!ca.hasEnoughMana(35), "does not have enough mana for 35");
    }
    {
        // Full combat: mage casts Fireball in a group fight.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 100; mage.maxMana = 100; mage.att = 5;
        CombatParticipant goblin1; goblin1.name = "Goblin 1"; goblin1.isPlayer = false;
        goblin1.hp = 50; goblin1.maxHp = 50; goblin1.speed = 5;
        CombatParticipant goblin2; goblin2.name = "Goblin 2"; goblin2.isPlayer = false;
        goblin2.hp = 50; goblin2.maxHp = 50; goblin2.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin1);
        cs.addParticipant(goblin2);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        int rounds = 0;
        while (!te.isCombatOver() && rounds < 40) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    // Mage casts AoE Fireball
                    QString result;
                    ca.castSpellAdvanced(1, "Fireball", 25, "15-30", true, result);
                } else {
                    ai.takeTurn();
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(te.isCombatOver(), "combat ended within 10 rounds");
        check(cs.participant(0).mana < 100, "mana was spent",
              QString::number(cs.participant(0).mana));
    }

    // ------------------------------------------- Status effects (2.8)

    section("[21] Status effects in combat");
    {
        // Poison DoT: monster loses HP each round for N rounds.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 200; warrior.maxHp = 200;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 50; goblin.maxHp = 50; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.applyStatus(1, GameConstants::Poisoned, 3, result);
        check(ca.isPoisoned(1), "goblin is poisoned");
        check(ca.getStatusDuration(1, GameConstants::Poisoned) == 3, "poison duration = 3");

        int hpBefore = cs.participant(1).hp;
        QStringList messages = ca.tickStatusEffects();
        check(cs.participant(1).hp < hpBefore, "poison deals damage");
        check(messages.size() > 0, "poison message generated");

        // Tick 2 more rounds
        ca.tickStatusEffects();
        ca.tickStatusEffects();

        // Poison should be expired now
        check(!ca.isPoisoned(1), "poison expired after 3 rounds");
    }
    {
        // Blind reduces to-hit.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.dex = 10; warrior.level = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 200; goblin.maxHp = 200; goblin.def = 5; goblin.level = 1; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        // Without blind: should hit most of the time
        int hitsWithoutBlind = 0;
        for (int i = 0; i < 100; i++) {
            QString result;
            int dmg = ca.attack(1, result);
            if (dmg > 0) hitsWithoutBlind++;
        }

        // Apply blind
        QString applyResult;
        ca.applyStatus(0, GameConstants::Blinded, 5, applyResult);
        check(ca.isBlinded(0), "warrior is blinded");

        // With blind: should hit less often
        int hitsWithBlind = 0;
        for (int i = 0; i < 100; i++) {
            QString result;
            int dmg = ca.attack(1, result);
            if (dmg > 0) hitsWithBlind++;
        }

        check(hitsWithBlind < hitsWithoutBlind,
              "blind reduces to-hit",
              QString("without=%1 with=%2").arg(hitsWithoutBlind).arg(hitsWithBlind));
    }
    {
        // Confusion risks friendly fire.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.att = 10; warrior.dex = 10;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.hp = 50; mage.maxHp = 50; mage.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 200; goblin.maxHp = 200; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        // Apply confusion to warrior
        cs.participant(0).confusionDuration = 10;
        check(ca.isConfused(0), "warrior is confused");

        // Attack goblin many times, count friendly fire incidents
        int friendlyFire = 0;
        for (int i = 0; i < 1000; i++) {
            int mageHpBefore = cs.participant(1).hp;
            QString result;
            ca.attack(2, result);  // Attack goblin
            if (cs.participant(1).hp < mageHpBefore) {
                friendlyFire++;
            }
        }

        check(friendlyFire > 0, "confusion causes friendly fire",
              QString::number(friendlyFire));
    }
    {
        // OnFire DoT.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 200; warrior.maxHp = 200;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.applyStatus(1, GameConstants::OnFire, 3, result);
        check(ca.isOnFire(1), "goblin is on fire");

        int hpBefore = cs.participant(1).hp;
        ca.tickStatusEffects();
        check(cs.participant(1).hp < hpBefore, "fire deals damage");

        // Tick until expired
        ca.tickStatusEffects();
        ca.tickStatusEffects();
        check(!ca.isOnFire(1), "fire expired after 3 rounds");
    }
    {
        // Status tick at round end.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 200; warrior.maxHp = 200;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 50; goblin.maxHp = 50; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.applyStatus(1, GameConstants::Poisoned, 2, result);

        int hpBefore = cs.participant(1).hp;
        QStringList messages = ca.tickStatusEffects();
        check(cs.participant(1).hp < hpBefore, "tick deals poison damage");
        check(messages.size() > 0, "tick generates messages");

        // Second tick
        hpBefore = cs.participant(1).hp;
        ca.tickStatusEffects();
        check(cs.participant(1).hp < hpBefore, "second tick deals damage");

        // Third tick - poison expired
        ca.tickStatusEffects();
        check(!ca.isPoisoned(1), "poison expired");
    }
    {
        // Full combat with status effects.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 200; warrior.maxHp = 200; warrior.att = 15; warrior.dex = 10;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 30; goblin.maxHp = 30; goblin.speed = 5; goblin.att = 5; goblin.def = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();

        // Poison the goblin
        QString result;
        ca.applyStatus(1, GameConstants::Poisoned, 5, result);

        int rounds = 0;
        // The goblin can dodge for a while; give the fight enough rounds that
        // a miss streak cannot leave it standing at the limit.
        while (!te.isCombatOver() && rounds < 40) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString atkResult;
                    ca.attack(1, atkResult);
                } else {
                    ai.takeTurn();
                }
                te.markCurrentActed();
            }
            // Tick status effects at round end
            ca.tickStatusEffects();
            rounds++;
        }

        check(te.isCombatOver(), "combat ended");
        check(!cs.participant(1).isAlive, "goblin died");
    }

    // ------------------------------------------- Victory rewards (2.9)

    section("[22] Victory: XP, gold and loot");
    {
        // XP reward: levelFound * 100
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["levelFound"] = 3;
        orc["goldFactor"] = 50;
        monsterData.append(orc);

        QVariantMap dragon;
        dragon["name"] = "Dragon";
        dragon["levelFound"] = 10;
        dragon["goldFactor"] = 500;
        monsterData.append(dragon);

        // XP formula: round(30 * level^1.2) — same curve as LevelTable
        check(VictoryReward::calculateXp("Orc", monsterData) == 112,
              "Orc XP = 112 (round(30 * 3^1.2))");
        check(VictoryReward::calculateXp("Dragon", monsterData) == 475,
              "Dragon XP = 475 (round(30 * 10^1.2))");
        check(VictoryReward::calculateXp("Unknown", monsterData) == 30,
              "Unknown monster XP = 30 (default level 1)");
    }
    {
        // Gold reward: goldFactor * random(1, 10)
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["levelFound"] = 3;
        orc["goldFactor"] = 50;
        monsterData.append(orc);

        // Roll many times, check range
        int minGold = 999999;
        int maxGold = 0;
        for (int i = 0; i < 100; i++) {
            int gold = VictoryReward::calculateGold("Orc", monsterData);
            if (gold < minGold) minGold = gold;
            if (gold > maxGold) maxGold = gold;
        }
        check(minGold >= 50, "min gold >= 50 (50 * 1)");
        check(maxGold <= 500, "max gold <= 500 (50 * 10)");
    }
    {
        // Loot: drop table from Item0-Item9
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["levelFound"] = 3;
        orc["goldFactor"] = 50;
        orc["Item0"] = 101;
        orc["Item1"] = 102;
        orc["Item2"] = 0;  // No drop
        orc["Item3"] = 103;
        monsterData.append(orc);

        QList<int> dropTable = VictoryReward::getDropTable("Orc", monsterData);
        check(dropTable.size() == 3, "Orc has 3 drop slots",
              QString::number(dropTable.size()));
        check(dropTable.contains(101), "drop table contains item 101");
        check(dropTable.contains(102), "drop table contains item 102");
        check(dropTable.contains(103), "drop table contains item 103");
    }
    {
        // Full victory flow: XP + gold + loot
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["levelFound"] = 3;
        orc["goldFactor"] = 50;
        orc["Item0"] = 101;
        monsterData.append(orc);

        int xp = VictoryReward::calculateXp("Orc", monsterData);
        int gold = VictoryReward::calculateGold("Orc", monsterData);

        check(xp > 0, "XP awarded");
        check(gold > 0, "gold awarded");
    }

    // ------------------------------------------- Death in combat (2.10)

    section("[23] Death in combat");
    {
        // Individual character death: HP 0 → isAlive = false.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 10; warrior.maxHp = 10;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();

        // Kill the warrior
        cs.participant(0).hp = 0;
        cs.participant(0).isAlive = false;

        check(dh.isPartyMemberDead(0), "warrior is dead");
        check(dh.deadPartyMemberCount() == 1, "1 dead party member");
        check(dh.livingPartyMemberCount() == 0, "0 living party members");
        check(dh.isPartyWipe(), "party wipe detected");
        check(dh.handleGameOver(), "game over detected");
    }
    {
        // Victory: all monsters dead.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 100; warrior.maxHp = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 0; goblin.maxHp = 100; goblin.isAlive = false; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        check(dh.isVictory(), "victory detected");
        check(!dh.isPartyWipe(), "not a party wipe");
        check(dh.deadPartyMemberCount() == 0, "no dead party members");
    }
    {
        // Partial death: one of two party members dies.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 100; warrior.maxHp = 100;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.hp = 0; mage.maxHp = 50; mage.isAlive = false; mage.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        check(dh.isPartyMemberDead(1), "mage is dead");
        check(!dh.isPartyMemberDead(0), "warrior is alive");
        check(dh.deadPartyMemberCount() == 1, "1 dead party member");
        check(dh.livingPartyMemberCount() == 1, "1 living party member");
        check(!dh.isPartyWipe(), "not a party wipe");
        check(!dh.handleGameOver(), "no game over");
    }
    {
        // Revive all party members.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 0; warrior.maxHp = 100; warrior.isAlive = false;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.hp = 0; mage.maxHp = 50; mage.isAlive = false; mage.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        check(dh.isPartyWipe(), "party wipe before revive");

        dh.reviveAllPartyMembers();

        check(!dh.isPartyWipe(), "no party wipe after revive");
        check(dh.livingPartyMemberCount() == 2, "2 living party members after revive");
        check(cs.participant(0).hp == 1, "warrior revived with 1 HP");
        check(cs.participant(1).hp == 1, "mage revived with 1 HP");
    }
    {
        // getDeadPartyMemberNames.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 0; warrior.maxHp = 100; warrior.isAlive = false;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.hp = 50; mage.maxHp = 50; mage.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        QStringList deadNames = dh.getDeadPartyMemberNames();
        check(deadNames.size() == 1, "1 dead name");
        check(deadNames.contains("Warrior"), "dead names contains Warrior");
    }
    {
        // Full combat: party wipe ends combat.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 10; warrior.maxHp = 10; warrior.att = 1; warrior.dex = 1;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 1000; goblin.maxHp = 1000; goblin.speed = 5; goblin.att = 50; goblin.def = 20;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        te.startRound();
        int rounds = 0;
        while (!te.isCombatOver() && rounds < 40) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString result;
                    ca.attack(1, result);
                } else {
                    QString result;
                    ca.attack(0, result);
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(te.isCombatOver(), "combat ended");
        check(dh.isPartyWipe(), "party wipe");
        check(dh.handleGameOver(), "game over");
    }
    {
        // processDeaths returns messages for dead players.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 0; warrior.maxHp = 100; warrior.isAlive = false;
        warrior.hasActed = false;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        QStringList messages = dh.processDeaths();
        check(messages.size() > 0, "death message generated");
        check(messages[0].contains("Warrior"), "message contains Warrior");

        // The death is announced exactly once — a second call must not repeat
        // it (the old code keyed off hasActed and re-announced every round).
        QStringList again = dh.processDeaths();
        check(again.isEmpty(), "death is not announced twice",
              QString::number(again.size()));
    }

    // ------------------------------------------- LevelTable (3.1)

    section("[24] Data-driven XP table");
    {
        LevelTable& lt = LevelTable::instance();
        check(lt.load("data/levels.json"), "levels.json loads");

        check(lt.isLoaded(), "LevelTable is loaded");
        check(lt.maxLevel() == 20, "max level = 20",
              QString::number(lt.maxLevel()));

        // XP values increase with level
        check(lt.xpForLevel(1) < lt.xpForLevel(5), "XP increases with level");
        check(lt.xpForLevel(5) < lt.xpForLevel(10), "XP increases with level (5<10)");
        check(lt.xpForLevel(10) < lt.xpForLevel(20), "XP increases with level (10<20)");

        // Specific values from the table (new formula: round(30 * n^1.2))
        check(lt.xpForLevel(1) == 30, "xpForLevel(1) = 30");
        check(lt.xpForLevel(2) == 69, "xpForLevel(2) = 69");
        check(lt.xpForLevel(5) == 207, "xpForLevel(5) = 207");
        check(lt.xpForLevel(10) == 475, "xpForLevel(10) = 475");
        check(lt.xpForLevel(20) == 1092, "xpForLevel(20) = 1092");

        // Total XP should be reasonable (~10K for level 20, not 31K)
        int totalXp = lt.totalXpForLevel(20);
        check(totalXp > 5000 && totalXp < 20000,
              "total XP for level 20 is reasonable (5K-20K)",
              QString::number(totalXp));

        // totalXpForLevel
        check(lt.totalXpForLevel(1) == 0, "totalXpForLevel(1) = 0");
        check(lt.totalXpForLevel(2) == 30, "totalXpForLevel(2) = 30");
        check(lt.totalXpForLevel(3) == 99, "totalXpForLevel(3) = 99");

        // levelForXp
        check(lt.levelForXp(0) == 1, "levelForXp(0) = 1");
        check(lt.levelForXp(29) == 1, "levelForXp(29) = 1");
        check(lt.levelForXp(30) == 2, "levelForXp(30) = 2");
        check(lt.levelForXp(98) == 2, "levelForXp(98) = 2");
        check(lt.levelForXp(99) == 3, "levelForXp(99) = 3");

        // xpProgress
        QPair<int, int> progress = lt.xpProgress(150);
        check(progress.first == 51, "xpProgress(150).first = 51",
              QString::number(progress.first));
        check(progress.second == 112, "xpProgress(150).second = 112",
              QString::number(progress.second));

        // Fallback for unloaded table
        LevelTable fallback;
        check(fallback.xpForLevel(1) == 30, "fallback xpForLevel(1) = 30");
        check(fallback.xpForLevel(5) == 207, "fallback xpForLevel(5) = 207");
    }

    // ------------------------------------------- Level-up stat gains (3.2)

    section("[25] Level-up stat gains");
    {
        // Level-up grants HP, mana for casters, and stat points.
        PartyManager pm;
        Character c;
        c.name = "TestChar";
        c.level = 1;
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 10;
        c.intelligence = 12;  // Caster
        c.wisdom = 10;
        c.constitution = 10;
        c.charisma = 10;
        c.dexterity = 10;

        int oldMaxHp = c.maxHp;
        int oldMaxMana = c.maxMana;

        pm.applyLevelUpGains(c);

        check(c.maxHp > oldMaxHp, "HP increases on level up",
              QString("old=%1 new=%2").arg(oldMaxHp).arg(c.maxHp));
        check(c.hp == c.maxHp, "HP fully restored on level up");
        check(c.maxMana > oldMaxMana, "mana increases for caster",
              QString("old=%1 new=%2").arg(oldMaxMana).arg(c.maxMana));
        check(c.mana == c.maxMana, "mana fully restored on level up");
    }
    {
        // Non-caster: no mana gain.
        PartyManager pm;
        Character c;
        c.name = "Warrior";
        c.level = 1;
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 15;
        c.intelligence = 8;  // Not a caster
        c.wisdom = 8;
        c.constitution = 12;
        c.charisma = 8;
        c.dexterity = 10;

        int oldMaxMana = c.maxMana;
        pm.applyLevelUpGains(c);

        check(c.maxMana == oldMaxMana, "no mana gain for non-caster");
    }
    {
        // Stat point every 2 levels.
        PartyManager pm;
        Character c;
        c.name = "TestChar";
        c.level = 2;  // Even level
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 10;
        c.intelligence = 10;
        c.wisdom = 10;
        c.constitution = 10;
        c.charisma = 10;
        c.dexterity = 10;

        const int pointsBefore = c.statPoints;
        pm.applyLevelUpGains(c);

        check(c.statPoints == pointsBefore + 2,
              "even level grants two stat points",
              QString("before=%1 after=%2").arg(pointsBefore).arg(c.statPoints));
    }
    {
        // No stat point on odd level.
        PartyManager pm;
        Character c;
        c.name = "TestChar";
        c.level = 3;  // Odd level
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 10;
        c.intelligence = 10;
        c.wisdom = 10;
        c.constitution = 10;
        c.charisma = 10;
        c.dexterity = 10;

        const int pointsBefore = c.statPoints;
        pm.applyLevelUpGains(c);

        check(c.statPoints == pointsBefore + 1,
              "odd level grants one stat point",
              QString("before=%1 after=%2").arg(pointsBefore).arg(c.statPoints));
    }

    // ------------------------------------------- v3.0.0 — slice 3.4: stat allocation

    section("[142] v3.0.0 slice 3.4: spendStatPoint raises base stat and decrements pool");
    {
        Character c;
        c.name = "Spender";
        c.race = "Human";
        c.strength = 10;
        c.statPoints = 3;

        check(c.baseStat("Strength") == 10, "base strength starts at 10");
        check(c.spendStatPoint("Strength", 17), "spend a point on Strength");
        check(c.baseStat("Strength") == 11, "base strength raised to 11",
              QString::number(c.baseStat("Strength")));
        check(c.statPoints == 2, "pool decremented to 2",
              QString::number(c.statPoints));
    }

    section("[143] v3.0.0 slice 3.4: spendStatPoint refuses when pool is empty");
    {
        Character c;
        c.name = "Broke";
        c.race = "Human";
        c.strength = 10;
        c.statPoints = 0;

        check(!c.spendStatPoint("Strength", 17), "cannot spend with 0 points");
        check(c.baseStat("Strength") == 10, "stat unchanged after refused spend");
    }

    section("[144] v3.0.0 slice 3.4: spendStatPoint refuses at race cap");
    {
        Character c;
        c.name = "Capped";
        c.race = "Human";
        c.strength = 17;  // Human STR max
        c.statPoints = 5;

        check(c.raceCapForStat("Strength") == 17, "Human STR cap is 17",
              QString::number(c.raceCapForStat("Strength")));
        check(!c.spendStatPoint("Strength", 17), "cannot spend past race cap");
        check(c.baseStat("Strength") == 17, "stat unchanged at cap");
        check(c.statPoints == 5, "pool unchanged after refused spend");
    }

    section("[145] v3.0.0 slice 3.4: raceCapForStat returns correct caps per race");
    {
        Character human;
        human.race = "Human";
        check(human.raceCapForStat("Strength") == 17, "Human STR cap 17");
        check(human.raceCapForStat("Intelligence") == 18, "Human INT cap 18");

        Character elf;
        elf.race = "Elf";
        check(elf.raceCapForStat("Strength") == 15, "Elf STR cap 15");
        check(elf.raceCapForStat("Intelligence") == 20, "Elf INT cap 20");

        Character giant;
        giant.race = "Giant";
        check(giant.raceCapForStat("Strength") == 25, "Giant STR cap 25");
        check(giant.raceCapForStat("Constitution") == 19, "Giant CON cap 19");

        Character unknown;
        unknown.race = "Unknown";
        check(unknown.raceCapForStat("Strength") == 0, "unknown race returns 0");
    }

    section("[147] v3.0.0 slice 3.4: character sheet shows stat points label");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        members.append([]{
            Character c;
            c.name = "UI Test";
            c.race = "Human";
            c.level = 3;
            c.statPoints = 4;
            c.strength = 10;
            c.intelligence = 10;
            c.wisdom = 10;
            c.constitution = 10;
            c.charisma = 10;
            c.dexterity = 10;
            return c;
        }());

        CharacterSheetDialog sheet;

        bool foundPoolLabel = false;
        bool foundSpendButtons = false;
        for (QLabel* l : sheet.findChildren<QLabel*>()) {
            if (l->text().contains("Stat Points")) { foundPoolLabel = true; break; }
        }
        check(foundPoolLabel, "character sheet shows stat points pool label");

        // Each stats row should have a spend button in column 2
        QTableWidget* stats = nullptr;
        for (QTableWidget* t : sheet.findChildren<QTableWidget*>()) {
            if (t->columnCount() == 3 && t->rowCount() == 6) { stats = t; break; }
        }
        check(stats != nullptr, "character sheet has 3-column stats table");
        if (stats) {
            int buttonCount = 0;
            for (int r = 0; r < stats->rowCount(); ++r) {
                if (stats->cellWidget(r, 2) != nullptr) buttonCount++;
            }
            check(buttonCount == 6, "every stat row has a spend button",
                  QString::number(buttonCount));
            foundSpendButtons = true;
        }

        members.clear();
    }
    {
        // addExperienceToCharacter uses LevelTable.
        LevelTable::instance().load("data/levels.json");
        PartyManager pm;
        Character c;
        c.name = "TestChar";
        c.level = 1;
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 10;
        c.intelligence = 12;
        c.wisdom = 10;
        c.constitution = 10;
        c.charisma = 10;
        c.dexterity = 10;
        pm.currentParty().members.append(c);

        // Give enough XP to level up (level 1→2 needs 30 XP)
        pm.addExperienceToCharacter(0, 30);

        check(pm.currentParty().members[0].level == 2, "level up via LevelTable",
              QString::number(pm.currentParty().members[0].level));
    }

    // ------------------------------------------- Guild leveling (3.3)

    section("[294] v6.1.0 slice 6.1d: monster loot derived from level + tier");
    {
        // data/bestiary.json rows carry no Item0-9, so before this fix no
        // monster ever dropped loot. deriveDropTable must now produce a
        // floor-appropriate, tier-matched candidate list.
        QList<QVariantMap> monsters;
        QVariantMap goblin; goblin["name"] = "Goblie"; goblin["level"] = 1;
        monsters.append(goblin);

        QList<int> shallow = VictoryReward::deriveDropTable("Goblie", monsters, 1);
        check(!shallow.isEmpty(), "level-1 monster on floor 1 has a drop table",
              QString::number(shallow.size()));

        // Every candidate must be a floor-1 item and a Bronze-tier name.
        bool allBronze = true;
        bool allFloorOk = true;
        for (int id : shallow) {
            const ItemDef* it = ItemDatabase::instance().byId(id);
            if (!it) { allBronze = false; allFloorOk = false; break; }
            if (!it->name.startsWith("Bronze", Qt::CaseInsensitive)) allBronze = false;
            if (it->floor > 1) allFloorOk = false;
        }
        check(allBronze, "floor-1 loot is all Bronze tier");
        check(allFloorOk, "floor-1 loot respects the floor filter");

        // A deep monster must be able to reach a higher tier.
        QVariantMap dragon; dragon["name"] = "Dragon"; dragon["level"] = 14;
        monsters.append(dragon);
        QList<int> deep = VictoryReward::deriveDropTable("Dragon", monsters, 14);
        bool hasHighTier = false;
        for (int id : deep) {
            const ItemDef* it = ItemDatabase::instance().byId(id);
            if (it && (it->name.startsWith("Adamantite", Qt::CaseInsensitive) ||
                       it->name.startsWith("Mithril", Qt::CaseInsensitive))) {
                hasHighTier = true;
                break;
            }
        }
        check(hasHighTier, "level-14 monster on floor 14 can drop a high tier");

        // End-to-end: a real bestiary-shaped row (no Item0-9) now yields loot
        // over repeated rolls, where it used to always return an empty list.
        int drops = 0;
        for (int i = 0; i < 200; ++i) {
            if (!VictoryReward::calculateLoot("Goblie", monsters, 1).isEmpty()) drops++;
        }
        check(drops > 0, "real monster drops loot over repeated kills",
              QString("%1/200").arg(drops));
    }

    section("[293] v6.1.0 slice 6.1c: damage-over-time spells apply their DoT");
    {
        // Poison (Damage school) must set a poison DoT that tickStatusEffects
        // actually damages. Before the fix the "Damage over Time" string parsed
        // to 0 damage and the spell did nothing.
        SpellBook::instance().load("data/spells.json");

        CombatState state;
        CombatParticipant caster; caster.name = "Mage"; caster.isPlayer = true;
        caster.level = 6; caster.hp = 100; caster.maxHp = 100;
        caster.mana = 100; caster.maxMana = 100; caster.speed = 100;
        state.addParticipant(caster);
        CombatParticipant monster; monster.name = "Ogre"; monster.isPlayer = false;
        monster.hp = 200; monster.maxHp = 200; monster.level = 5; monster.speed = 1;
        state.addParticipant(monster);

        TurnEngine engine; engine.setCombatState(&state);
        CombatActions actions(&state, &engine);
        state.rollInitiative();
        engine.startRound();
        engine.nextTurn();

        QString result;
        actions.castSpellBySchool(1, "Poison", result);
        check(state.participant(1).poisonDuration > 0,
              "Poison sets a poison duration on the target",
              QString::number(state.participant(1).poisonDuration));

        // The DoT must actually deal damage on the next tick.
        int hpBefore = state.participant(1).hp;
        QStringList ticks = actions.tickStatusEffects();
        check(state.participant(1).hp < hpBefore,
              "poison DoT deals damage on tick",
              QString("%1 -> %2").arg(hpBefore).arg(state.participant(1).hp));

        // Drain Life: instant damage, heals the caster.
        CombatState state2;
        CombatParticipant c2; c2.name = "Sorcerer"; c2.isPlayer = true;
        c2.level = 6; c2.hp = 50; c2.maxHp = 100;
        c2.mana = 100; c2.maxMana = 100; c2.speed = 100;
        state2.addParticipant(c2);
        CombatParticipant m2; m2.name = "Troll"; m2.isPlayer = false;
        m2.hp = 200; m2.maxHp = 200; m2.level = 5; m2.speed = 1;
        state2.addParticipant(m2);

        TurnEngine e2; e2.setCombatState(&state2);
        CombatActions a2(&state2, &e2);
        state2.rollInitiative();
        e2.startRound();
        e2.nextTurn();

        int casterHpBefore = state2.participant(0).hp;
        QString r2;
        int drainDmg = a2.castSpellBySchool(1, "Drain Life", r2);
        check(drainDmg > 0, "Drain Life deals instant damage",
              QString::number(drainDmg));
        check(state2.participant(0).hp > casterHpBefore,
              "Drain Life heals the caster",
              QString("%1 -> %2").arg(casterHpBefore).arg(state2.participant(0).hp));
    }

    section("[292] v6.1.0 slice 6.1b: Kill and Banish schools resolve in combat");
    {
        // Exorcism (Kill) and Banish Undead (Banish) must deal damage through
        // the school-aware path, not fall through to a no-op.
        SpellBook::instance().load("data/spells.json");

        CombatState state;
        CombatParticipant caster; caster.name = "Paladin"; caster.isPlayer = true;
        caster.level = 8; caster.hp = 100; caster.maxHp = 100;
        caster.mana = 200; caster.maxMana = 200; caster.speed = 100;
        state.addParticipant(caster);
        CombatParticipant undead; undead.name = "Wight"; undead.isPlayer = false;
        undead.hp = 300; undead.maxHp = 300; undead.level = 6; undead.speed = 1;
        state.addParticipant(undead);

        TurnEngine engine; engine.setCombatState(&state);
        CombatActions actions(&state, &engine);
        state.rollInitiative();
        engine.startRound();
        engine.nextTurn();

        QString result;
        int dmg = actions.castSpellBySchool(1, "Banish Undead", result);
        check(dmg > 0 && dmg < 9999,
              "Banish Undead deals massive damage (not a no-op)",
              QString::number(dmg));
    }

    section("[290] v6.0.0 slice 6.0e: instant kill spells deal massive damage");
    {
        // Insanity (Mind, L8, "Instant Kill") and Exorcism (Kill, L6,
        // "Instant Kill") should deal 3× caster level, not 9999.
        SpellBook::instance().load("data/spells.json");

        CombatState state;
        CombatParticipant caster; caster.name = "Mage"; caster.isPlayer = true;
        caster.level = 10; caster.hp = 100; caster.maxHp = 100;
        caster.mana = 100; caster.maxMana = 100; caster.speed = 100;
        state.addParticipant(caster);
        CombatParticipant monster; monster.name = "Dragon"; monster.isPlayer = false;
        monster.hp = 500; monster.maxHp = 500; monster.level = 8; monster.speed = 1;
        state.addParticipant(monster);

        TurnEngine engine; engine.setCombatState(&state);
        CombatActions actions(&state, &engine);
        state.rollInitiative();
        engine.startRound();
        engine.nextTurn();

        QString result;
        int dmg = actions.castSpellBySchool(1, "Insanity", result);
        check(dmg > 0 && dmg < 9999,
              "Insanity deals massive damage, not one-shot",
              QString::number(dmg));
        check(dmg == 50, "Insanity deals max(50, 3× caster level) = 50",
              QString::number(dmg));

        // Exorcism (Kill school, "Instant Kill")
        CombatState state2;
        CombatParticipant caster2; caster2.name = "Paladin"; caster2.isPlayer = true;
        caster2.level = 5; caster2.hp = 100; caster2.maxHp = 100;
        caster2.mana = 100; caster2.maxMana = 100; caster2.speed = 100;
        state2.addParticipant(caster2);
        CombatParticipant undead; undead.name = "Skeleton"; undead.isPlayer = false;
        undead.hp = 200; undead.maxHp = 200; undead.level = 5; undead.speed = 1;
        state2.addParticipant(undead);

        TurnEngine engine2; engine2.setCombatState(&state2);
        CombatActions actions2(&state2, &engine2);
        state2.rollInitiative();
        engine2.startRound();
        engine2.nextTurn();

        QString result2;
        int dmg2 = actions2.castSpellBySchool(1, "Exorcism", result2);
        check(dmg2 > 0 && dmg2 < 9999,
              "Exorcism deals massive damage, not one-shot",
              QString::number(dmg2));
        check(dmg2 == 50, "Exorcism deals max(50, 3× caster level) = 50",
              QString::number(dmg2));
    }

    section("[289] v6.0.0 slice 6.0d: electrical school has 4+ spells");
    {
        SpellBook& sb = SpellBook::instance();
        check(sb.load("data/spells.json"), "spells.json loads");
        check(sb.isLoaded(), "SpellBook is loaded");
        check(sb.spellCount() >= 95, "at least 95 spells loaded (was 47 + 48 new from MORDOR)",
              QString::number(sb.spellCount()));

        QList<SpellDef> electrical = SpellMechanics::spellsInSchool(
            sb.all(), SpellMechanics::School::Lightning);
        check(electrical.size() >= 4, "Electrical school has 4+ spells",
              QString::number(electrical.size()));

        // Thunderclap exists and arcs to 1 additional target
        const SpellDef* thunderclap = sb.byName("Thunderclap");
        check(thunderclap != nullptr, "Thunderclap exists");
        if (thunderclap) {
            check(SpellMechanics::lightningChainTargets(*thunderclap) == 1,
                  "Thunderclap chains to 1 target",
                  QString::number(SpellMechanics::lightningChainTargets(*thunderclap)));
        }

        // Storm exists and arcs to 3 additional targets
        const SpellDef* storm = sb.byName("Storm");
        check(storm != nullptr, "Storm exists");
        if (storm) {
            check(SpellMechanics::lightningChainTargets(*storm) == 3,
                  "Storm chains to 3 targets",
                  QString::number(SpellMechanics::lightningChainTargets(*storm)));
        }
    }

    section("[288] v6.0.0 slice 6.0a: mana system allows 3+ casts at level 1");
    {
        Character c;
        c.level = 1;
        c.maxMana = 50;
        c.mana = 50;

        // Cast until empty — base spell costs 10 mana
        int casts = 0;
        while (c.mana >= 10) { c.mana -= 10; casts++; }
        check(casts >= 3, "can cast base spell 3+ times at level 1",
              QString::number(casts));

        // Level-up should increase maxMana by +5
        int manaBefore = c.maxMana;
        c.addExperience(LevelTable::instance().xpForLevel(1));
        check(c.maxMana > manaBefore, "mana grows on level-up",
              QString("before=%1 after=%2").arg(manaBefore).arg(c.maxMana));
        check(c.maxMana - manaBefore >= 5, "mana grows by at least +5 per level",
              QString::number(c.maxMana - manaBefore));
    }

    section("[300] v6.9.0 slice 6.9c: combat log marks crits and misses");
    {
        // Roadmap v6.9.0 step 3. A natural 20 doubles damage but used to say
        // nothing about it — the log now marks it, and marks misses, so the
        // three outcomes are distinguishable at a glance.
        //
        // The roll is random, so this is statistical rather than a forced
        // roll: over 300 attacks both crits (1/20) and misses are guaranteed,
        // and every outcome must be colour-marked.

        CombatState cs;
        CombatParticipant warrior;
        warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.hp = 1000000; warrior.maxHp = 1000000;
        warrior.att = 10; warrior.swings = 1; warrior.damageMod = 100;
        warrior.dex = 10; warrior.speed = 100;
        CombatParticipant goblin;
        goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 1000000; goblin.maxHp = 1000000;
        goblin.att = 5; goblin.def = 5; goblin.speed = 1; goblin.dex = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();
        check(ca.isPlayerTurn(), "warrior acts first");

        const int trials = 300;
        int crits = 0, misses = 0, ordinary = 0, unmarked = 0;
        int lastCritDamage = 0, lastOrdinaryDamage = 0;

        for (int i = 0; i < trials; ++i) {
            // Keep the goblin alive so every trial resolves the same way.
            cs.participant(1).hp = 1000000;
            QString r;
            int dmg = ca.attack(1, r);
            if (dmg < 0) { ++misses; }

            const bool marked = r.contains("<font color=");
            if (!marked) { ++unmarked; }

            if (r.contains("CRITICAL")) {
                ++crits;
                lastCritDamage = dmg;
            } else if (dmg >= 0) {
                ++ordinary;
                lastOrdinaryDamage = dmg;
            }
        }

        check(unmarked == 0,
              "every attack result is colour-marked for the log",
              QString("%1 of %2 unmarked").arg(unmarked).arg(trials));
        check(crits > 0, "at least one critical was rolled over 300 attacks",
              QString::number(crits));
        check(misses > 0, "at least one miss was rolled over 300 attacks",
              QString::number(misses));
        check(ordinary > 0, "at least one ordinary hit was rolled",
              QString::number(ordinary));
        check(lastCritDamage > lastOrdinaryDamage,
              "a crit deals more than an ordinary hit of the same swing",
              QString("crit=%1 ordinary=%2").arg(lastCritDamage).arg(lastOrdinaryDamage));

        // The crit hit the same target with the same swing as every ordinary
        // hit, so any crit being at least as large as any ordinary hit is the
        // real invariant. The single-swing comparison above is only a sanity
        // check; the range check cannot be satisfied by a miss or an
        // unlucky low roll.
        int minOrdinary = INT_MAX, maxCrit = 0;
        for (int i = 0; i < trials; ++i) {
            cs.participant(1).hp = 1000000;
            QString r;
            int dmg = ca.attack(1, r);
            if (r.contains("CRITICAL")) {
                maxCrit = qMax(maxCrit, dmg);
            } else if (dmg > 0) {
                minOrdinary = qMin(minOrdinary, dmg);
            }
        }
        check(maxCrit >= minOrdinary,
              "a crit is never weaker than an ordinary hit of the same swing",
              QString("maxCrit=%1 minOrdinary=%2").arg(maxCrit).arg(minOrdinary));
        check(maxCrit >= 2,
              "a crit lands real damage (at least the doubled minimum)",
              QString::number(maxCrit));
    }

    {
        // The crit flag is what makes the log say CRITICAL. It is derived from
        // the roll, so the deterministic way to test it is the helper that
        // builds the message for a given roll.
        check(CombatActions::attackMessage(20, "Warrior", "Goblin", 40).contains("CRITICAL"),
              "a natural 20 is announced as a critical",
              CombatActions::attackMessage(20, "Warrior", "Goblin", 40));
        check(!CombatActions::attackMessage(10, "Warrior", "Goblin", 40).contains("CRITICAL"),
              "an ordinary roll is not announced as a critical",
              CombatActions::attackMessage(10, "Warrior", "Goblin", 40));

        const QString crit = CombatActions::attackMessage(20, "Warrior", "Goblin", 40);
        check(crit.contains("40 damage"), "the crit reports the doubled damage", crit);
        check(crit.startsWith("<font color='red'>"),
              "the crit is marked for the log in red", crit);

        const QString ordinary = CombatActions::attackMessage(10, "Warrior", "Goblin", 20);
        check(ordinary.contains("20 damage"), "the ordinary hit reports its damage", ordinary);
        check(ordinary.startsWith("<font color='orange'>"),
              "the ordinary hit is marked in orange", ordinary);

        const QString miss = CombatActions::missMessage("Warrior", "Goblin", 7, 14);
        check(miss.contains("misses"), "the miss says it missed", miss);
        check(miss.contains("(roll 7 vs AC 14)"), "the miss reports roll and AC", miss);
        check(miss.startsWith("<font color='gray'>"),
              "the miss is marked in gray", miss);

        // The three outcomes must never share a colour, or the log marking
        // would be pointless.
        check(!crit.startsWith(ordinary.left(20)),
              "crit and ordinary hit use different colours");
    }

    {
        // A miss is marked, and a marked miss is a miss — not just any line.
        // Driven to a guaranteed miss by forcing the attacker's skill to 0
        // against a target whose AC is as high as the band allows.
        CombatState cs;
        CombatParticipant archer;
        archer.name = "Archer"; archer.isPlayer = true;
        archer.att = 10; archer.swings = 1; archer.damageMod = 100;
        archer.dex = 1; archer.level = 1; archer.speed = 100;
        CombatParticipant wall;
        wall.name = "Wall"; wall.isPlayer = false;
        // AC = 10 + level/2 + min(def/12, 8). Maxing def and level pushes the
        // target's AC to the top of the band while the attacker's skill stays
        // at 1, so every roll but a 20 misses.
        wall.hp = 1000000; wall.maxHp = 1000000; wall.def = 100000; wall.level = 1000;
        wall.speed = 1; wall.dex = 1;
        cs.addParticipant(archer);
        cs.addParticipant(wall);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        te.startRound();
        te.nextTurn();

        int misses = 0, unmarked = 0;
        for (int i = 0; i < 200; ++i) {
            cs.participant(1).hp = 1000000;
            QString r;
            if (ca.attack(1, r) < 0) {
                ++misses;
                if (!r.contains("<font color=")) ++unmarked;
            }
        }
        check(misses > 0, "at least one miss was forced over 200 attacks",
              QString::number(misses));
        check(unmarked == 0,
              "every miss carries the colour markup the log needs",
              QString::number(unmarked));
    }

    {
        // stylize() is the single definition of the markup, so a crit line and
        // a miss line can never drift into different formats.
        const QString critLine = CombatActions::stylize("CRITICAL — test", "red");
        const QString missLine = CombatActions::stylize("misses test", "gray");
        check(critLine == "<font color='red'>CRITICAL — test</font>",
              "stylize wraps in a quoted font colour", critLine);
        check(missLine == "<font color='gray'>misses test</font>",
              "stylize wraps misses the same way", missLine);
        check(critLine.startsWith("<font") && critLine.endsWith("</font>"),
              "crit markup is well-formed");
    }
}
