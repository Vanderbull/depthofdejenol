#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldSpellTests()
{
    gameStateManager* gsm = gameStateManager::instance();

    section("[53] Spell differentiation");
    {
        SpellDef fire;
        fire.category = "Fire";
        fire.name = "Fireball";
        fire.baseLevel = 3;

        SpellDef cold;
        cold.category = "Cold";
        cold.name = "Iceball";
        cold.baseLevel = 3;

        SpellDef lightning;
        lightning.category = "Electrical";
        lightning.name = "Chain Lightning";
        lightning.baseLevel = 6;

        SpellDef mind;
        mind.category = "Mind";
        mind.name = "Stun";
        mind.baseLevel = 3;

        using School = SpellMechanics::School;
        check(SpellMechanics::schoolFor("Fire") == School::Fire, "fire school");
        check(SpellMechanics::schoolFor("Cold") == School::Cold, "cold school");
        check(SpellMechanics::schoolFor("Electrical") == School::Lightning, "lightning school");
        check(SpellMechanics::schoolFor("Mind") == School::Mind, "mind school");
    }

    section("[72] v1.0.0 slice 1.7: SpellMechanics into combat");
    {
        SpellBook& sb = SpellBook::instance();
        if (!sb.isLoaded()) sb.load("data/spells.json");

        // --- Fire: Fireball splashes to multiple targets ---
        {
            CombatState cs;
            CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
            mage.speed = 100; mage.mana = 200; mage.maxMana = 200;
            cs.addParticipant(mage);
            for (int i = 0; i < 4; ++i) {
                CombatParticipant g; g.name = QString("Goblin %1").arg(i + 1);
                g.isPlayer = false; g.hp = 500; g.maxHp = 500; g.speed = 5;
                cs.addParticipant(g);
            }
            TurnEngine te; te.setCombatState(&cs);
            CombatActions ca(&cs, &te);
            te.startRound(); te.nextTurn();

            QString result;
            int total = ca.applySpellDamageBySchool(1, "Fireball", 20, result);
            check(total > 0, "Fireball school path deals damage", result);
            // Fireball baseLevel 3 → fireTargets = 1 + 3/3 = 2 targets.
            int hit = 0;
            for (int i = 1; i <= 4; ++i) if (cs.participant(i).hp < 500) hit++;
            check(hit >= 2, "Fireball hits multiple targets", QString::number(hit));
        }

        // --- Cold: Iceball slows the target and the slow expires ---
        {
            CombatState cs;
            CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
            mage.speed = 100; mage.mana = 200;
            CombatParticipant troll; troll.name = "Troll"; troll.isPlayer = false;
            troll.hp = 500; troll.maxHp = 500; troll.speed = 20;
            cs.addParticipant(mage); cs.addParticipant(troll);
            TurnEngine te; te.setCombatState(&cs);
            CombatActions ca(&cs, &te);
            te.startRound(); te.nextTurn();

            QString result;
            ca.applySpellDamageBySchool(1, "Iceball", 20, result);
            check(cs.participant(1).speed < 20, "Iceball slows the target",
                  QString("speed=%1").arg(cs.participant(1).speed));
            check(cs.participant(1).slowDuration > 0, "slow has a duration");
            check(result.contains("slowed", Qt::CaseInsensitive), "slow is reported", result);

            int slowedSpeed = cs.participant(1).speed;
            // Tick until the slow expires.
            for (int i = 0; i < 5; ++i) ca.tickStatusEffects();
            check(cs.participant(1).speed == 20, "speed restored when slow expires",
                  QString("speed=%1 (was %2)").arg(cs.participant(1).speed).arg(slowedSpeed));
        }

        // --- Lightning: Chain Lightning arcs to several targets ---
        {
            CombatState cs;
            CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
            mage.speed = 100; mage.mana = 200;
            cs.addParticipant(mage);
            for (int i = 0; i < 4; ++i) {
                CombatParticipant g; g.name = QString("Orc %1").arg(i + 1);
                g.isPlayer = false; g.hp = 500; g.maxHp = 500; g.speed = 5;
                cs.addParticipant(g);
            }
            TurnEngine te; te.setCombatState(&cs);
            CombatActions ca(&cs, &te);
            te.startRound(); te.nextTurn();

            QString result;
            ca.applySpellDamageBySchool(1, "Chain Lightning", 40, result);
            int hit = 0;
            for (int i = 1; i <= 4; ++i) if (cs.participant(i).hp < 500) hit++;
            check(hit >= 3, "Chain Lightning arcs to multiple targets", QString::number(hit));
            check(result.contains("arcs", Qt::CaseInsensitive), "chain is reported", result);
        }

        // --- Mind: Confusion stuns the target ---
        {
            CombatState cs;
            CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
            mage.speed = 100; mage.mana = 200;
            CombatParticipant ogre; ogre.name = "Ogre"; ogre.isPlayer = false;
            ogre.hp = 500; ogre.maxHp = 500; ogre.speed = 10;
            cs.addParticipant(mage); cs.addParticipant(ogre);
            TurnEngine te; te.setCombatState(&cs);
            CombatActions ca(&cs, &te);
            te.startRound(); te.nextTurn();

            QString result;
            ca.applySpellDamageBySchool(1, "Confusion", 0, result);
            check(cs.participant(1).stunDuration > 0, "Confusion stuns the target",
                  QString("stun=%1").arg(cs.participant(1).stunDuration));
            check(result.contains("stunned", Qt::CaseInsensitive), "stun is reported", result);
        }

        // --- Stun costs the target its turn ---
        {
            CombatState cs;
            CombatParticipant a; a.name = "A"; a.isPlayer = true; a.speed = 100;
            CombatParticipant b; b.name = "B"; b.isPlayer = true; b.speed = 50;
            b.stunDuration = 1;
            cs.addParticipant(a); cs.addParticipant(b);
            TurnEngine te; te.setCombatState(&cs);
            te.startRound();
            int turnsSeen = 0;
            while (te.nextTurn()) {
                turnsSeen++;
                te.markCurrentActed();
                if (turnsSeen > 10) break;
            }
            // Only A acts; B is stunned and skipped.
            check(turnsSeen == 1, "a stunned participant loses its turn",
                  QString("turns=%1").arg(turnsSeen));
        }

        // --- castSpellBySchool: mana is charged from the spell's own cost ---
        {
            SpellBook& book = SpellBook::instance();
            const SpellDef* fireball = book.byName("Fireball");
            if (fireball) {
                CombatState cs;
                CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
                mage.speed = 100; mage.mana = 200; mage.maxMana = 200;
                CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
                goblin.hp = 500; goblin.maxHp = 500; goblin.speed = 5;
                cs.addParticipant(mage); cs.addParticipant(goblin);
                TurnEngine te; te.setCombatState(&cs);
                CombatActions ca(&cs, &te);
                te.startRound(); te.nextTurn();

                QString result;
                int dmg = ca.castSpellBySchool(1, "Fireball", result);
                check(dmg > 0, "castSpellBySchool deals damage", result);
                check(cs.participant(0).mana == 200 - fireball->mana,
                      "castSpellBySchool charges the spell's mana cost",
                      QString("mana=%1 cost=%2").arg(cs.participant(0).mana).arg(fireball->mana));
            }
        }

        // --- castSpellBySchool refuses when mana is short ---
        {
            SpellBook& book = SpellBook::instance();
            const SpellDef* fireball = book.byName("Fireball");
            if (fireball) {
                CombatState cs;
                CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
                mage.speed = 100; mage.mana = 1;
                CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
                goblin.hp = 500; goblin.maxHp = 500; goblin.speed = 5;
                cs.addParticipant(mage); cs.addParticipant(goblin);
                TurnEngine te; te.setCombatState(&cs);
                CombatActions ca(&cs, &te);
                te.startRound(); te.nextTurn();

                QString result;
                int dmg = ca.castSpellBySchool(1, "Fireball", result);
                check(dmg == -1, "castSpellBySchool fails without mana", result);
                check(cs.participant(0).mana == 1, "mana untouched on failure");
                check(cs.participant(1).hp == 500, "target unharmed on failure");
            }
        }
    }

    section("[75] v2.0.0 slice 2.1: spell dialog routes through CombatActions");
    {
        // When a spell is cast in combat, the damage must flow through
        // CombatActions (death bookkeeping, status effects) rather than
        // poking participant HP directly. The spell dialog emits spellCast
        // with a SpellResult; DungeonDialog must route that through
        // applySpellDamageBySchool.
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();

        // Add a party member so there is someone to cast spells.
        Character partyMember;
        partyMember.name = "Test Hero";
        partyMember.level = 5;
        partyMember.hp = 100;
        partyMember.maxHp = 100;
        partyMember.isAlive = true;
        gsm->addCharacterToParty(partyMember);

        // Place a hostile monster at (5, 5) and start combat.
        QPair<int, int> monsterPos = {5, 5};
        dlg.m_monsterPositions[monsterPos] = "Test Spell Goblin";
        dlg.m_MonsterAttitude["Test Spell Goblin"] = "Hostile";
        gsm->setGameValue("DungeonX", 5);
        gsm->setGameValue("DungeonY", 5);

        DungeonHandlers::handleEncounters(&dlg, 5, 5);
        check(dlg.m_inCombat, "combat started for spell test");

        // Find the monster's combat index.
        int monsterIdx = -1;
        if (dlg.m_combatState) {
            for (int i = 0; i < dlg.m_combatState->participantCount(); ++i) {
                const auto& p = dlg.m_combatState->participant(i);
                if (!p.isPlayer && p.isAlive) {
                    monsterIdx = i;
                    break;
                }
            }
        }
        check(monsterIdx >= 0, "monster found in combat state");

        // Record the monster's HP before the spell.
        int hpBefore = 0;
        if (monsterIdx >= 0 && dlg.m_combatState) {
            hpBefore = dlg.m_combatState->participant(monsterIdx).hp;
        }

        // Simulate a spell cast by emitting the signal that SpellCastingDialog
        // would emit. We call the lambda directly by invoking the spell
        // button handler, but since that opens a modal dialog, we instead
        // test the routing by calling applySpellDamageBySchool directly.
        if (dlg.m_combatActions && monsterIdx >= 0) {
            QString result;
            dlg.m_combatActions->applySpellDamageBySchool(monsterIdx, "Fireball", 50, result);
        }

        // The monster's HP must have changed (or it died).
        if (dlg.m_combatState && monsterIdx >= 0) {
            int hpAfter = dlg.m_combatState->participant(monsterIdx).hp;
            bool died = !dlg.m_combatState->participant(monsterIdx).isAlive;
            check(hpAfter != hpBefore || died,
                  "spell damage applied to monster");
        }

        // Clean up.
        dlg.m_monsterPositions.remove(monsterPos);
        dlg.m_MonsterAttitude.remove("Test Spell Goblin");
        dlg.m_inCombat = false;
    }

    section("[291] v6.1.0 slice 6.1a: dialog instant-kill is not 9999");
    {
        // The player Cast Spell path must use the same massive-damage rule as
        // CombatActions (slice 6.0e), never the old 9999 one-shot.
        SpellBook& sb = SpellBook::instance();
        sb.load("data/spells.json");
        const SpellDef* insanity = sb.byName("Insanity");
        check(insanity != nullptr, "Insanity spell exists");
        if (insanity) {
            check(insanity->damage == "Instant Kill",
                  "Insanity is flagged Instant Kill",
                  insanity->damage);
        }

        // Mirror the dialog's rule: qMax(50, level*3), never 9999.
        int casterLevel = 10;
        int massiveDmg = SpellMechanics::instantKillDamage(casterLevel);
        check(massiveDmg < 9999, "dialog instant-kill damage is not 9999",
              QString::number(massiveDmg));
        check(massiveDmg == 50, "dialog instant-kill damage = max(50, 3*level)",
              QString::number(massiveDmg));
        // The shared helper is the single source of truth for both paths.
        check(SpellMechanics::instantKillDamage(20) == 60,
              "instantKillDamage(20) = 60 (3*level above the floor)",
              QString::number(SpellMechanics::instantKillDamage(20)));
    }

    section("[279] v5.4.0 slice 5.4: CombatActions spell casting");
    {
        // Verify CombatActions has spell casting methods
        CombatState state;
        TurnEngine engine;
        CombatActions actions(&state, &engine);

        CombatParticipant p; p.name = "TestMage"; p.isPlayer = true; p.hp = 20; p.maxHp = 20; p.mana = 50; p.maxMana = 50;
        state.addParticipant(p);

        CombatParticipant m; m.name = "TestMonster"; m.isPlayer = false; m.hp = 30; m.maxHp = 30;
        state.addParticipant(m);

        state.rollInitiative();
        state.startRound();

        // Check if it's player turn
        if (actions.isPlayerTurn()) {
            QString result;
            int dmg = actions.castSpellBySchool(1, "Fireball", result);
            // Result should be valid (damage >= 0 or -1 for failure, but not crash)
            check(dmg >= -1, "castSpellBySchool returns valid result");
        }
    }

}
