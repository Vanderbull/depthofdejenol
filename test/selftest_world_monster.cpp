#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldMonsterTests()
{
    gameStateManager* gsm = gameStateManager::instance();

    section("[52] Monster difficulty curve");
    {
        check(MonsterBalance::recommendedLevel(1) == 1, "floor 1 needs level 1");
        check(MonsterBalance::recommendedLevel(15) == 15, "floor 15 needs level 15");
        check(MonsterBalance::statMultiplier(1) == 1.0, "floor 1 multiplier is 1.0");
        check(MonsterBalance::statMultiplier(15) > 2.0, "floor 15 multiplier is high",
              QString::number(MonsterBalance::statMultiplier(15)));
        check(MonsterBalance::scaledHp(1, 10) == 10, "floor 1 HP unchanged");
        check(MonsterBalance::scaledHp(15, 10) > 20, "floor 15 HP scaled",
              QString::number(MonsterBalance::scaledHp(15, 10)));
        check(MonsterBalance::scaledAttack(1, 5) == 5, "floor 1 attack unchanged");
        check(MonsterBalance::scaledAttack(15, 5) > 10, "floor 15 attack scaled");
        check(MonsterBalance::scaledXp(1, 100) == 100, "floor 1 XP unchanged");
        check(MonsterBalance::scaledXp(15, 100) > 200, "floor 15 XP scaled");
        check(MonsterBalance::scaledGold(1, 50) == 50, "floor 1 gold unchanged");
        check(MonsterBalance::scaledGold(15, 50) > 100, "floor 15 gold scaled");
    }

    section("[95] v2.0.0 slice 2.7: monster spellcaster casts via CombatActions");
    {
        // A spell-capable monster casts a spell through CombatActions.
        CombatState state;
        CombatParticipant monster;
        monster.name = "Dark Mage";
        monster.isPlayer = false;
        monster.isAlive = true;
        monster.hp = 30;
        monster.maxHp = 30;
        monster.canCastSpells = true;
        monster.level = 3;
        state.addParticipant(monster);

        CombatParticipant player;
        player.name = "Hero";
        player.isPlayer = true;
        player.isAlive = true;
        player.hp = 20;
        player.maxHp = 20;
        state.addParticipant(player);

        TurnEngine engine;
        CombatActions actions(&state, &engine);

        // Load spells so SpellBook has data
        SpellBook::instance().load("data/spells.json");

        // Cast a fire spell at the player
        QString result;
        int damage = actions.applySpellDamageBySchool(1, "Flame Bolt", 8, result);
        check(damage > 0, "monster spell deals damage");
        check(state.participant(1).hp < 20, "player HP reduced after spell");
    }

    section("[96] v2.0.0 slice 2.7: monster AI decides to cast spell");
    {
        // MonsterAI returns CastSpell decision for spellcasters.
        CombatState state;
        CombatParticipant monster;
        monster.name = "Spellcaster";
        monster.isPlayer = false;
        monster.isAlive = true;
        monster.hp = 30;
        monster.maxHp = 30;
        monster.canCastSpells = true;
        monster.level = 1;
        state.addParticipant(monster);

        CombatParticipant player;
        player.name = "Hero";
        player.isPlayer = true;
        player.isAlive = true;
        player.hp = 20;
        player.maxHp = 20;
        state.addParticipant(player);

        TurnEngine engine;
        engine.setCombatState(&state);
        CombatActions actions(&state, &engine);
        MonsterAI ai(&state, &engine, &actions);

        // Start a round and advance to the monster's turn
        state.rollInitiative();
        engine.startRound();
        // Advance until it's the monster's turn (initiative order is random)
        for (int i = 0; i < 10 && !ai.isMonsterTurn(); ++i) {
            engine.nextTurn();
        }

        // Force the AI to decide — with canCastSpells and full HP,
        // it should sometimes choose CastSpell (40% chance)
        bool sawCastSpell = false;
        for (int i = 0; i < 50; ++i) {
            MonsterAI::Decision d = ai.decide();
            if (d == MonsterAI::Decision::CastSpell) {
                sawCastSpell = true;
                break;
            }
        }
        check(sawCastSpell, "monster AI chooses CastSpell for spellcaster");
    }

    section("[97] v2.0.0 slice 2.7: monster AI flees when critically wounded");
    {
        // MonsterAI returns Flee decision when HP is critically low.
        CombatState state;
        CombatParticipant monster;
        monster.name = "Coward";
        monster.isPlayer = false;
        monster.isAlive = true;
        monster.hp = 2;
        monster.maxHp = 30;
        monster.canCastSpells = false;
        monster.level = 1;
        state.addParticipant(monster);

        CombatParticipant player;
        player.name = "Hero";
        player.isPlayer = true;
        player.isAlive = true;
        player.hp = 20;
        player.maxHp = 20;
        state.addParticipant(player);

        TurnEngine engine;
        engine.setCombatState(&state);
        CombatActions actions(&state, &engine);
        MonsterAI ai(&state, &engine, &actions);

        // Start a round and advance to the monster's turn
        state.rollInitiative();
        engine.startRound();
        // Advance until it's the monster's turn (initiative order is random)
        for (int i = 0; i < 10 && !ai.isMonsterTurn(); ++i) {
            engine.nextTurn();
        }

        MonsterAI::Decision d = ai.decide();
        check(d == MonsterAI::Decision::Flee, "monster flees when critically wounded");
    }

    section("[98] v2.0.0 slice 2.7: boss never flees");
    {
        // Bosses (level >= 5) always attack, never flee.
        CombatState state;
        CombatParticipant boss;
        boss.name = "Dragon";
        boss.isPlayer = false;
        boss.isAlive = true;
        boss.hp = 5;
        boss.maxHp = 100;
        boss.canCastSpells = false;
        boss.level = 10;
        state.addParticipant(boss);

        CombatParticipant player;
        player.name = "Hero";
        player.isPlayer = true;
        player.isAlive = true;
        player.hp = 20;
        player.maxHp = 20;
        state.addParticipant(player);

        TurnEngine engine;
        engine.setCombatState(&state);
        CombatActions actions(&state, &engine);
        MonsterAI ai(&state, &engine, &actions);

        // Start a round and advance to the monster's turn
        state.rollInitiative();
        engine.startRound();
        engine.nextTurn();

        MonsterAI::Decision d = ai.decide();
        check(d != MonsterAI::Decision::Flee, "boss never flees");
    }

    section("[99] v2.0.0 slice 2.8: poison DoT applied each round");
    {
        CombatState state;
        CombatParticipant player;
        player.name = "Poisoned Hero";
        player.isPlayer = true;
        player.isAlive = true;
        player.hp = 20;
        player.maxHp = 20;
        state.addParticipant(player);

        TurnEngine engine;
        engine.setCombatState(&state);
        CombatActions actions(&state, &engine);

        QString result;
        actions.applyStatus(0, GameConstants::Poisoned, 3, result);
        check(actions.hasStatus(0, GameConstants::Poisoned), "player is poisoned");

        int hpBefore = state.participant(0).hp;
        QStringList msgs = actions.tickStatusEffects();
        int hpAfter = state.participant(0).hp;
        check(hpAfter < hpBefore, "poison damage applied");
    }

    section("[100] v2.0.0 slice 2.8: blind reduces to-hit and expires");
    {
        CombatState state;
        CombatParticipant player;
        player.name = "Blinded Hero";
        player.isPlayer = true;
        player.isAlive = true;
        player.hp = 20;
        player.maxHp = 20;
        state.addParticipant(player);

        TurnEngine engine;
        engine.setCombatState(&state);
        CombatActions actions(&state, &engine);

        QString result;
        actions.applyStatus(0, GameConstants::Blinded, 2, result);
        check(actions.isBlinded(0), "player is blinded");

        actions.tickStatusEffects();  // duration 2 → 1
        check(actions.isBlinded(0), "still blinded after 1 round");

        actions.tickStatusEffects();  // duration 1 → 0
        check(!actions.isBlinded(0), "blind expired after 2 rounds");
    }

    section("[101] v2.0.0 slice 2.8: fire DoT and expires");
    {
        CombatState state;
        CombatParticipant player;
        player.name = "Burning Hero";
        player.isPlayer = true;
        player.isAlive = true;
        player.hp = 20;
        player.maxHp = 20;
        state.addParticipant(player);

        TurnEngine engine;
        engine.setCombatState(&state);
        CombatActions actions(&state, &engine);

        QString result;
        actions.applyStatus(0, GameConstants::OnFire, 2, result);
        check(actions.hasStatus(0, GameConstants::OnFire), "player is on fire");

        int hpBefore = state.participant(0).hp;
        actions.tickStatusEffects();
        check(state.participant(0).hp < hpBefore, "fire damage applied");

        actions.tickStatusEffects();  // expires
        check(!actions.hasStatus(0, GameConstants::OnFire), "fire expired");
    }

    section("[104] v2.0.0 slice 2.8: poison kills at 0 HP");
    {
        CombatState state;
        CombatParticipant player;
        player.name = "Doomed Hero";
        player.isPlayer = true;
        player.isAlive = true;
        player.hp = 1;   // any poison tick (1-3) is lethal, so the test is deterministic
        player.maxHp = 20;
        state.addParticipant(player);

        TurnEngine engine;
        engine.setCombatState(&state);
        CombatActions actions(&state, &engine);

        QString result;
        actions.applyStatus(0, GameConstants::Poisoned, 3, result);
        actions.tickStatusEffects();  // 1-3 damage, always kills at 1 HP
        check(!state.participant(0).isAlive, "poison kills at 0 HP");
    }

}
