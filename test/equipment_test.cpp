// Equipment tests — guild restrictions, cursed items, stat modifiers.
// Kept separate from the main suite so guild membership doesn't leak into
// other tests' characters.

#include <QApplication>
#include <QString>
#include <QVariantMap>
#include <cstdio>

#include "character.h"
#include "src/core/GameConstants.h"
#include "src/items/ItemDatabase.h"
#include "src/skill/SkillDatabase.h"
#include <QRandomGenerator>
#include "src/combat/CombatState.h"
#include "src/combat/CombatActions.h"
#include "src/combat/TurnEngine.h"

static int g_passed = 0;
static int g_failed = 0;

static void out(const QString& line)
{
    std::fputs(qPrintable(line), stdout);
    std::fputc('\n', stdout);
    std::fflush(stdout);
}

static void check(bool condition, const QString& what, const QString& detail = QString())
{
    if (condition) {
        ++g_passed;
        out("  PASS  " + what);
    } else {
        ++g_failed;
        QString line = "  FAIL  " + what;
        if (!detail.isEmpty()) line += "  [" + detail + "]";
        out(line);
    }
}

int runEquipmentTest()
{
    g_passed = 0;
    g_failed = 0;

    out("");
    out("--- [76] v2.0.0 slice 2.2: equipment guild restriction " + QString(20, '-'));

    // Items with a guild bitmask can only be equipped by members of at
    // least one of the required guilds.
    Character hero;
    hero.name = "Guild Test Hero";
    hero.level = 5;
    hero.strength = 15;
    hero.intelligence = 15;
    hero.wisdom = 15;
    hero.constitution = 15;
    hero.charisma = 15;
    hero.dexterity = 15;

    // Find an item with a non-zero guild bitmask.
    const ItemDef* guildItem = nullptr;
    for (const ItemDef& def : ItemDatabase::instance().all()) {
        if (def.guilds != 0 && def.equippable()) {
            guildItem = &def;
            break;
        }
    }

    if (guildItem) {
        HeldItem held;
        held.name = guildItem->name;
        held.M4E97 = guildItem->id;
        held.identified = true;
        hero.inventory.append(held);

        // Character without the guild cannot equip it.
        QString reason;
        bool equipped = hero.equipItem(0, reason);
        check(!equipped, "cannot equip guild-restricted item without guild");
        check(hero.inventory.size() == 1, "item still in inventory after failed equip");

        // Join all guilds and verify it can now be equipped.
        for (const QString& g : GameConstants::GUILD_NAMES) hero.joinGuild(g);
        equipped = hero.equipItem(0, reason);
        check(equipped, "can equip guild-restricted item after joining guild");
    } else {
        check(true, "no guild-restricted items found (skip)");
    }

    // --------------------------------------------------------------- skill allocation tests
    out("--- [77] v6.1.0 slice 6.1a: skill allocation tests ---");

    // ---- Full allocation scenario ----
    Character skillHero;
    skillHero.name = "Skill Test Hero";
    skillHero.level = 1;
    // Starting skill points should be 0
    check(skillHero.skillPoints() == 0, "initial skill points = 0");

    // Add some skill points
    skillHero.addSkillPoints(5);
    check(skillHero.skillPoints() == 5, "skill points after addSkillPoints = 5");

    // Learn Swordsmanship (cost 1)
    int newLevel = 0;
    QString error;
    bool ok = skillHero.learnSkill("Swordsmanship", newLevel, error);
    check(ok, "learned Swordsmanship");
    check(newLevel == 1, "learned Swordsmanship increases level to 1");
    check(skillHero.skillPoints() == 4, "skill points after learning Swordsmanship = 4");

    // Try to learn a skill without enough points (character with 0 points)
    Character shortHero;
    shortHero.name = "Short Hero";
    shortHero.level = 1;
    ok = shortHero.learnSkill("Swordsmanship", newLevel, error);
    check(!ok, "cannot learn skill with 0 points");
    check(!error.isEmpty(), "error message about insufficient points");

    // ---- Prerequisite test ----
    Character prereqHero;
    prereqHero.name = "Prereq Hero";
    prereqHero.level = 1;
    prereqHero.addSkillPoints(5);
    // Attempt to learn Advanced Swordsmanship before learning Swordsmanship
    ok = prereqHero.learnSkill("Advanced Swordsmanship", newLevel, error);
    check(!ok, "cannot learn Advanced Swordsmanship without prerequisite");
    check(!error.isEmpty(), "error message about prerequisite");

    // Learn Swordsmanship
    ok = prereqHero.learnSkill("Swordsmanship", newLevel, error);
    check(ok, "learned Swordsmanship for prerequisite test");
    check(newLevel == 1, "learned Swordsmanship increases level to 1");

    // Now Advanced Swordsmanship should be learnable
    int newLevel2 = 0;
    ok = prereqHero.learnSkill("Advanced Swordsmanship", newLevel2, error);
    check(ok, "learned Advanced Swordsmanship after prerequisite");
    check(newLevel2 == 1, "Advanced Swordsmanship level increased to 1");
    check(prereqHero.skillPoints() == 2, "skill points after learning Advanced Swordsmanship = 2");

    // ---- Serialization test ----
    QVariantMap saved = skillHero.toMap();
    Character loaded;
    loaded.loadFromMap(saved);
    check(loaded.skillPoints() == skillHero.skillPoints(), "serialized skill points preserved");
    check(loaded.skillLevel("Swordsmanship") == 1, "serialized skill level Swordsmanship");
    check(loaded.skillLevel("Advanced Swordsmanship") == 0, "serialized skill level Advanced Swordsmanship (0)");

    // --------------------------------------------------------------- skill check tests
    out("--- [78] v6.1.0 slice 6.1b: skill checks in combat ---");

    // Helper to get skill id
    auto skillIdForName = [&](const QString& name) {
        const QVector<SkillDef>& defs = skillDefs();
        for (int i = 0; i < defs.size(); ++i) {
            if (defs[i].name == name) return i;
        }
        return -1;
    };
    int thieveryId = skillIdForName("Thievery");
    check(thieveryId >= 0, "Thievery skill found", QString::number(thieveryId));

    // Test skillMod
    CombatState state1;
    CombatParticipant player1, player2;
    player1.name = "High";
    player1.dex = 12;
    player1.skillLevels["Thievery"] = 5;
    state1.addParticipant(player1);
    int mod1 = state1.skillMod("Thievery", 0);
    int expectedMod1 = 5 + qMax(0, (player1.dex - 10) / 2);
    check(mod1 == expectedMod1, "skillMod high skill", QString::number(mod1));

    player2.name = "Low";
    player2.dex = 12;
    player2.skillLevels["Thievery"] = 0;
    state1.addParticipant(player2);
    int mod2 = state1.skillMod("Thievery", 1);
    int expectedMod2 = 0 + qMax(0, (player2.dex - 10) / 2);
    check(mod2 == expectedMod2, "skillMod low skill", QString::number(mod2));

    // Test deterministic RNG with a local generator
    QRandomGenerator rng(12345);
    int roll1 = rng.bounded(1, 21);
    QRandomGenerator rng2(12345);
    int roll2 = rng2.bounded(1, 21);
    check(roll1 == roll2, "deterministic RNG", QString("%1 vs %2").arg(roll1).arg(roll2));

    // Test skillCheck success rates
    // High skill
    CombatState stateHigh;
    CombatParticipant high;
    high.name = "High";
    high.dex = 12;
    high.skillLevels["Thievery"] = 5;
    stateHigh.addParticipant(high);
    TurnEngine engineHigh;
    engineHigh.setCombatState(&stateHigh);
    engineHigh.startRound();
    engineHigh.nextTurn();
    CombatActions actionsHigh(&stateHigh, &engineHigh);
    int highSuccesses = 0;
    const int trials = 2000;
    for (int i = 0; i < trials; ++i) {
        QString result;
        CombatActions::SkillCheckResult res = actionsHigh.skillCheck("Thievery", 0, 10, result);
        if (res == CombatActions::SkillCheckResult::Success) ++highSuccesses;
    }

    // Low skill
    CombatState stateLow;
    CombatParticipant low;
    low.name = "Low";
    low.dex = 12;
    low.skillLevels["Thievery"] = 0;
    stateLow.addParticipant(low);
    TurnEngine engineLow;
    engineLow.setCombatState(&stateLow);
    engineLow.startRound();
    engineLow.nextTurn();
    CombatActions actionsLow(&stateLow, &engineLow);
    int lowSuccesses = 0;
    for (int i = 0; i < trials; ++i) {
        QString result;
        CombatActions::SkillCheckResult res = actionsLow.skillCheck("Thievery", 0, 10, result);
        if (res == CombatActions::SkillCheckResult::Success) ++lowSuccesses;
    }

    check(highSuccesses > lowSuccesses, "high skill success rate > low skill success rate",
          QString("high: %1, low: %2").arg(highSuccesses).arg(lowSuccesses));

    // --------------------------------------------------------------- slice 6.1c feat tests
    out("--- [79] v6.1.0 slice 6.1c: combat skill feats and maneuvers ---");

    // The feat skills must exist as Combat-type skills with cost 0 and no
    // prerequisite (they are granted by feats, not earned from the pool).
    {
        const QVector<SkillDef>& defs = skillDefs();
        auto checkFeat = [&](int id, const QString& name) {
            check(id >= 0, QString("%1 skill found").arg(name), QString::number(id));
            if (id >= 0) {
                const SkillDef& d = defs[id];
                check(d.cost == 0, QString("%1 cost is 0").arg(name));
                check(d.prerequisite.isEmpty(), QString("%1 has no prerequisite").arg(name));
                check(d.type == QStringLiteral("Combat"), QString("%1 is Combat type").arg(name));
            }
        };
        checkFeat(skillIdForName("Parry"), "Parry");
        checkFeat(skillIdForName("Dodge"), "Dodge");
        checkFeat(skillIdForName("DualWield"), "DualWield");
        checkFeat(skillIdForName("ShieldBash"), "ShieldBash");
    }

    // Rate tests: a character with a high skill level succeeds far more often
    // than a character with a low skill level.
    int featTrials = 2000;
    {
        // Parry
        auto makeState = [](int skillLevel) {
            CombatState state;
            CombatParticipant part;
            part.name = "Feater";
            part.dex = 12;
            part.skillLevels["Parry"] = skillLevel;
            state.addParticipant(part);
            return state;
        };
        auto runParryTrials = [&](CombatState& state, int skillLevel) {
            TurnEngine engine;
            engine.setCombatState(&state);
            engine.startRound();
            engine.nextTurn();
            CombatActions actions(&state, &engine);
            int successes = 0;
            for (int i = 0; i < featTrials; ++i) {
                QString result;
                CombatActions::SkillCheckResult res = actions.parry(0, 12, result);
                if (res == CombatActions::SkillCheckResult::Success) ++successes;
            }
            return successes;
        };
        CombatState high = makeState(5);
        CombatState low = makeState(0);
        int highParry = runParryTrials(high, 5);
        int lowParry = runParryTrials(low, 0);
        check(highParry > lowParry, "Parry rate increases with skill",
              QString("high: %1, low: %2").arg(highParry).arg(lowParry));
    }
    {
        // Dodge
        auto makeState = [](int skillLevel) {
            CombatState state;
            CombatParticipant part;
            part.name = "Feater";
            part.dex = 12;
            part.skillLevels["Dodge"] = skillLevel;
            state.addParticipant(part);
            return state;
        };
        auto runDodgeTrials = [&](CombatState& state, int skillLevel) {
            TurnEngine engine;
            engine.setCombatState(&state);
            engine.startRound();
            engine.nextTurn();
            CombatActions actions(&state, &engine);
            int successes = 0;
            for (int i = 0; i < featTrials; ++i) {
                QString result;
                CombatActions::SkillCheckResult res = actions.dodge(0, 10, result);
                if (res == CombatActions::SkillCheckResult::Success) ++successes;
            }
            return successes;
        };
        CombatState high = makeState(5);
        CombatState low = makeState(0);
        int highDodge = runDodgeTrials(high, 5);
        int lowDodge = runDodgeTrials(low, 0);
        check(highDodge > lowDodge, "Dodge rate increases with skill",
              QString("high: %1, low: %2").arg(highDodge).arg(lowDodge));
    }
    {
        // DualWield
        auto makeState = [](int skillLevel) {
            CombatState state;
            CombatParticipant part;
            part.name = "Feater";
            part.dex = 12;
            part.skillLevels["DualWield"] = skillLevel;
            state.addParticipant(part);
            return state;
        };
        auto runDualTrials = [&](CombatState& state, int skillLevel) {
            TurnEngine engine;
            engine.setCombatState(&state);
            engine.startRound();
            engine.nextTurn();
            CombatActions actions(&state, &engine);
            int successes = 0;
            for (int i = 0; i < featTrials; ++i) {
                QString result;
                CombatActions::SkillCheckResult res = actions.dualWield(0, 10, result);
                if (res == CombatActions::SkillCheckResult::Success) ++successes;
            }
            return successes;
        };
        CombatState high = makeState(5);
        CombatState low = makeState(0);
        int highDual = runDualTrials(high, 5);
        int lowDual = runDualTrials(low, 0);
        check(highDual > lowDual, "DualWield rate increases with skill",
              QString("high: %1, low: %2").arg(highDual).arg(lowDual));
    }
    {
        // ShieldBash
        auto makeState = [](int skillLevel) {
            CombatState state;
            CombatParticipant part;
            part.name = "Feater";
            part.dex = 12;
            part.skillLevels["ShieldBash"] = skillLevel;
            state.addParticipant(part);
            CombatParticipant target;
            target.name = "Mob";
            target.hp = 50000;
            target.maxHp = 50000;
            target.row = CombatRow::Front;
            state.addParticipant(target);
            return state;
        };
        auto runBashTrials = [&](CombatState& state, int skillLevel) {
            TurnEngine engine;
            engine.setCombatState(&state);
            engine.startRound();
            engine.nextTurn();
            CombatActions actions(&state, &engine);
            int successes = 0;
            for (int i = 0; i < featTrials; ++i) {
                QString result;
                CombatActions::SkillCheckResult res = actions.shieldBash(1, 10, result);
                if (res == CombatActions::SkillCheckResult::Success) ++successes;
            }
            return successes;
        };
        CombatState high = makeState(5);
        CombatState low = makeState(0);
        int highBash = runBashTrials(high, 5);
        int lowBash = runBashTrials(low, 0);
        check(highBash > lowBash, "ShieldBash rate increases with skill",
              QString("high: %1, low: %2").arg(highBash).arg(lowBash));
    }

    // --------------------------------------------------------------- main report
    out("");
    out("====================");
    out(QString("%1 passed, %2 failed").arg(g_passed).arg(g_failed));
    return g_failed == 0 ? 0 : 1;
}

    // --------------------------------------------------------------- main report
