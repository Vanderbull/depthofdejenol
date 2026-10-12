#include "selftest_harness.h"
#include "selftest_includes.h"

void runProgressionTests()
{
    gameStateManager* gsm = gameStateManager::instance();
    section("[35] Death state and body carrying");
    {
        // A character dies; the body records where it fell.
        Character c;
        c.name = "Fallen";
        c.level = 3;
        c.hp = 10;
        c.maxHp = 30;

        DeathRecovery::killCharacter(c, 4, 12, 9);

        check(!c.isAlive, "character is dead");
        check(c.hp == 0, "HP is 0");
        check((c.statusFlags & StatusFlag::Dead) != 0, "Dead status applied");
        check(c.dungeonLevel == 4, "body remembers the floor",
              QString::number(c.dungeonLevel));
        check(c.dungeonX == 12 && c.dungeonY == 9, "body remembers the position");
    }
    {
        // A body in the dungeon can be carried.
        BodyLocation loc;
        loc.valid = true;
        loc.dungeonLevel = 4;

        check(DeathRecovery::canCarry(loc), "dungeon body can be carried");

        QString reason;
        check(DeathRecovery::carryBody(loc, reason), "carry succeeds", reason);
        check(loc.carried, "body is now carried");
        check(!DeathRecovery::canCarry(loc), "cannot carry it twice");
    }
    {
        // Carried bodies come home when the party reaches town.
        QList<BodyLocation> bodies;
        BodyLocation carried;
        carried.valid = true;
        carried.carried = true;
        carried.dungeonLevel = 5;
        bodies.append(carried);

        BodyLocation leftBehind;
        leftBehind.valid = true;
        leftBehind.dungeonLevel = 5;
        bodies.append(leftBehind);

        int brought = DeathRecovery::bringBodiesToTown(bodies);
        check(brought == 1, "one body brought home",
              QString::number(brought));
        check(bodies[0].inCity, "carried body is now in town");
        check(!bodies[0].carried, "no longer carried once home");
        check(!bodies[1].inCity, "body left behind stays in the dungeon");
    }
    {
        // Dropping a carried body puts it back where the party stands.
        BodyLocation loc;
        loc.valid = true;
        loc.carried = true;
        loc.dungeonLevel = 2;

        DeathRecovery::dropBody(loc, 3, 7, 8);
        check(!loc.carried, "body dropped");
        check(loc.dungeonLevel == 3, "body is on the current floor");
        check(loc.x == 7 && loc.y == 8, "body at the drop position");
        check(DeathRecovery::canCarry(loc), "dropped body can be picked up again");
    }

    // ------------------------------------------- Morgue / resurrection (5.2)

    section("[36] Morgue resurrection");
    {
        // Cost scales with level; a dungeon body costs extra.
        BodyLocation inTown;
        inTown.valid = true;
        inTown.inCity = true;

        BodyLocation inDungeon;
        inDungeon.valid = true;
        inDungeon.dungeonLevel = 5;

        check(DeathRecovery::resurrectionCost(1, inTown) == 500,
              "level 1 in town costs 500",
              QString::number(DeathRecovery::resurrectionCost(1, inTown)));
        check(DeathRecovery::resurrectionCost(5, inTown) == 2500,
              "level 5 in town costs 2500",
              QString::number(DeathRecovery::resurrectionCost(5, inTown)));
        check(DeathRecovery::resurrectionCost(5, inDungeon) == 3000,
              "level 5 in the dungeon costs 3000 (rescue fee)",
              QString::number(DeathRecovery::resurrectionCost(5, inDungeon)));
    }
    {
        // Resurrection fails when gold is short.
        Character c;
        c.name = "Fallen";
        c.level = 3;
        c.isAlive = false;

        BodyLocation loc;
        loc.valid = true;
        loc.inCity = true;

        int gold = 100;  // needs 1500
        QString reason;
        check(!DeathRecovery::resurrect(c, loc, gold, reason), "resurrect fails without gold", reason);
        check(!c.isAlive, "still dead");
        check(gold == 100, "gold untouched on failure");
        check(loc.valid, "body still present");
    }
    {
        // Resurrection succeeds with enough gold.
        Character c;
        c.name = "Fallen";
        c.level = 3;
        c.isAlive = false;
        c.hp = 0;

        BodyLocation loc;
        loc.valid = true;
        loc.inCity = true;

        int gold = 2000;
        QString reason;
        check(DeathRecovery::resurrect(c, loc, gold, reason), "resurrect succeeds", reason);
        check(c.isAlive, "character is alive");
        check(c.hp >= 1, "HP restored");
        check((c.statusFlags & StatusFlag::Dead) == 0, "Dead status cleared");
        check(gold == 500, "1500 gold spent (2000 - 1500)",
              QString::number(gold));
        check(!loc.valid, "body consumed");
    }
    {
        // Resurrecting a living character is refused.
        Character c;
        c.name = "Alive";
        c.isAlive = true;

        BodyLocation loc;
        loc.valid = true;

        int gold = 10000;
        QString reason;
        check(!DeathRecovery::resurrect(c, loc, gold, reason), "cannot resurrect the living", reason);
        check(gold == 10000, "gold untouched");
    }

    // ------------------------------------------- Party wipe / rescue (5.3)

    section("[37] Party wipe and rescue");
    {
        // A party with one survivor is not wiped.
        QList<Character> party;
        Character a; a.name = "A"; a.isAlive = true;
        Character b; b.name = "B"; b.isAlive = false;
        party.append(a);
        party.append(b);

        check(!DeathRecovery::isPartyWiped(party), "one survivor means not wiped");
        check(!DeathRecovery::needsRescue(party), "no rescue needed");
    }
    {
        // All dead: wiped and stranded.
        QList<Character> party;
        Character a; a.name = "A"; a.isAlive = false;
        Character b; b.name = "B"; b.isAlive = false;
        party.append(a);
        party.append(b);

        check(DeathRecovery::isPartyWiped(party), "whole party down");
        check(DeathRecovery::needsRescue(party), "rescue needed");
    }
    {
        // An empty party is not a wipe.
        QList<Character> empty;
        check(!DeathRecovery::isPartyWiped(empty), "empty party is not wiped");
    }
    {
        // Rescue cost scales with depth, superlinearly.
        check(DeathRecovery::rescuePartyCost(1) == 250, "depth 1 rescue costs 250",
              QString::number(DeathRecovery::rescuePartyCost(1)));
        check(DeathRecovery::rescuePartyCost(5) == 6250, "depth 5 rescue costs 6250",
              QString::number(DeathRecovery::rescuePartyCost(5)));
        check(DeathRecovery::rescuePartyCost(10) > DeathRecovery::rescuePartyCost(5),
              "deeper rescues cost more");
        check(DeathRecovery::rescuePartyCost(0) == 250, "depth 0 clamps to depth 1");
    }
    {
        // A rescue recovers bodies on the target floor only.
        QList<BodyLocation> bodies;
        BodyLocation b5a; b5a.valid = true; b5a.dungeonLevel = 5;
        BodyLocation b5b; b5b.valid = true; b5b.dungeonLevel = 5;
        BodyLocation b7;  b7.valid = true;  b7.dungeonLevel = 7;
        bodies.append(b5a);
        bodies.append(b5b);
        bodies.append(b7);

        int recovered = DeathRecovery::recoverBodies(bodies, 5);
        check(recovered == 2, "two bodies recovered from floor 5",
              QString::number(recovered));
        check(bodies[0].inCity && bodies[1].inCity, "both floor-5 bodies are home");
        check(!bodies[2].inCity, "the floor-7 body is untouched");
    }
    {
        // Hardcore mode means no resurrection at all.
        check(DeathRecovery::isPermanentlyDead(true), "hardcore is permanent");
        check(!DeathRecovery::isPermanentlyDead(false), "normal mode is not permanent");
    }

    // ------------------------------------------- Main quest chain (6.1)

    section("[38] Main quest chain");
    {
        check(QuestChain::stepCount() == 6, "6 quest steps",
              QString::number(QuestChain::stepCount()));

        QuestStep s0 = QuestChain::step(0);
        check(s0.id == "first_descent", "step 0 is first descent");
        check(s0.requiresDepth == 1, "step 0 needs depth 1");

        QuestStep s5 = QuestChain::step(5);
        check(s5.id == "prince_of_devils", "step 5 is the Prince");
        check(s5.bossFloor == 15, "step 5 needs the floor-15 boss");

        QuestStep invalid = QuestChain::step(99);
        check(invalid.index == -1, "out-of-range step is invalid");
    }
    {
        // Objective text describes the goal.
        check(QuestChain::objectiveText(QuestChain::step(0)).contains("level 1"),
              "depth objective mentions the floor");
        check(QuestChain::objectiveText(QuestChain::step(1)).contains("floor 5"),
              "boss objective mentions the floor");
    }
    {
        // A fresh party is on step 0 and the chain is incomplete.
        QList<int> noBosses;
        QStringList noItems;

        check(QuestChain::nextStepIndex(0, noBosses, noItems) == 0,
              "fresh party is on step 0");
        check(!QuestChain::isChainComplete(0, noBosses, noItems),
              "chain not complete");
    }
    {
        // Reaching floor 1 completes step 0; the party moves to step 1.
        QList<int> noBosses;
        QStringList noItems;

        check(QuestChain::nextStepIndex(1, noBosses, noItems) == 1,
              "reaching depth 1 advances to step 1");
    }
    {
        // Killing the floor-5 boss completes step 1.
        QList<int> bosses;
        bosses.append(5);
        QStringList noItems;

        check(QuestChain::isStepComplete(QuestChain::step(1), 5, bosses, noItems),
              "floor 5 boss completes step 1");
        check(QuestChain::nextStepIndex(5, bosses, noItems) == 2,
              "chain advances to step 2");
    }
    {
        // Steps cannot be skipped: depth 15 without the floor-5 boss still
        // leaves the party on step 1.
        QList<int> noBosses;
        QStringList noItems;

        check(QuestChain::nextStepIndex(15, noBosses, noItems) == 1,
              "missing boss keeps the party on step 1");
    }
    {
        // The full chain completes only when the Prince is dead.
        QList<int> bosses;
        bosses.append(5);
        bosses.append(10);
        QStringList noItems;

        check(!QuestChain::isChainComplete(15, bosses, noItems),
              "chain incomplete without the Prince");

        bosses.append(15);
        check(QuestChain::isChainComplete(15, bosses, noItems),
              "chain complete once the Prince is dead");
        check(QuestChain::nextStepIndex(15, bosses, noItems) == -1,
              "no steps left when complete");
    }

    // ------------------------------------------- Final boss / victory (6.2, 6.3)

    section("[39] Final boss and victory");
    {
        check(Endgame::finalBossName() == "The Prince of Devils", "final boss name");

        QVariantMap prince = Endgame::buildFinalBoss();
        QVariantMap floorBoss = BossEncounter::buildBoss(15);

        check(prince["name"].toString() == "The Prince of Devils", "prince built");
        check(prince["hp"].toInt() > floorBoss["hp"].toInt(),
              "the Prince outlasts the floor-15 boss",
              QString("%1 vs %2").arg(prince["hp"].toInt()).arg(floorBoss["hp"].toInt()));
        check(prince["att"].toInt() > floorBoss["att"].toInt(), "the Prince hits harder");
        check(prince["swings"].toInt() > floorBoss["swings"].toInt(), "the Prince swings more");
    }
    {
        // Victory requires the floor-15 boss to be down.
        QList<int> bosses;
        bosses.append(5);
        bosses.append(10);
        check(!Endgame::isVictory(bosses), "not victory without the Prince");

        bosses.append(15);
        check(Endgame::isVictory(bosses), "victory once the Prince falls");
    }
    {
        // Victory text exists and reads as a sequence.
        check(!Endgame::victoryTitle().isEmpty(), "victory has a title");
        check(Endgame::victoryParagraphs().size() >= 3, "victory has paragraphs",
              QString::number(Endgame::victoryParagraphs().size()));
    }

    // ------------------------------------------- Hall of Records (6.4)

    section("[40] Hall of Records");
    {
        GameRecord a;
        a.heroName = "A";
        a.highestLevel = 20;
        a.mostGold = 1000;
        a.deepestFloor = 10;

        GameRecord b;
        b.heroName = "B";
        b.highestLevel = 15;
        b.mostGold = 5000;
        b.deepestFloor = 15;

        check(Endgame::outranks(a, b, Endgame::Category::HighestLevel),
              "A outranks B on level");
        check(!Endgame::outranks(a, b, Endgame::Category::MostGold),
              "B outranks A on gold");
        check(Endgame::outranks(b, a, Endgame::Category::DeepestFloor),
              "B outranks A on depth");
    }
    {
        // Fastest completion: a win beats a non-win; faster beats slower.
        GameRecord wonFast;
        wonFast.won = true;
        wonFast.completionTimeSeconds = 3600;

        GameRecord wonSlow;
        wonSlow.won = true;
        wonSlow.completionTimeSeconds = 7200;

        GameRecord notWon;
        notWon.won = false;
        notWon.completionTimeSeconds = 100;

        check(Endgame::outranks(wonFast, wonSlow, Endgame::Category::FastestCompletion),
              "faster win outranks slower win");
        check(Endgame::outranks(wonSlow, wonFast, Endgame::Category::FastestCompletion) == false,
              "slower win does not outrank faster");
        check(Endgame::outranks(wonFast, notWon, Endgame::Category::FastestCompletion),
              "a win outranks a non-finish");
        check(!Endgame::outranks(notWon, wonFast, Endgame::Category::FastestCompletion),
              "a non-finish never outranks a win");
    }
    {
        // Ranking sorts best-first.
        QList<GameRecord> records;
        GameRecord r1; r1.heroName = "Low";  r1.highestLevel = 5;
        GameRecord r2; r2.heroName = "High"; r2.highestLevel = 30;
        GameRecord r3; r3.heroName = "Mid";  r3.highestLevel = 15;
        records << r1 << r2 << r3;

        QList<GameRecord> sorted = Endgame::ranked(records, Endgame::Category::HighestLevel);
        check(sorted[0].heroName == "High", "highest level ranked first", sorted[0].heroName);
        check(sorted[1].heroName == "Mid", "mid ranked second");
        check(sorted[2].heroName == "Low", "low ranked last");
    }
    {
        // Record serialization round-trip.
        GameRecord r;
        r.heroName = "Hero";
        r.highestLevel = 25;
        r.mostGold = 999999;
        r.deepestFloor = 15;
        r.completionTimeSeconds = 4530;
        r.won = true;

        QVariantMap map = r.toMap();
        GameRecord loaded;
        loaded.loadFromMap(map);

        check(loaded.heroName == "Hero", "hero name persists");
        check(loaded.highestLevel == 25, "level persists");
        check(loaded.mostGold == 999999, "gold persists");
        check(loaded.deepestFloor == 15, "depth persists");
        check(loaded.won, "win persists");
        check(loaded.completionTimeSeconds == 4530, "time persists");
    }
    {
        // Time formatting.
        GameRecord r;
        r.completionTimeSeconds = 3661;  // 1h 1m 1s
        check(r.formattedTime() == "01:01:01", "time formats as HH:MM:SS", r.formattedTime());

        GameRecord unfinished;
        check(unfinished.formattedTime() == "--:--:--", "unfinished shows placeholder");
    }
    {
        // Category names.
        check(Endgame::categoryName(Endgame::Category::HighestLevel) == "Highest Level",
              "level category name");
        check(Endgame::categoryName(Endgame::Category::FastestCompletion) == "Fastest Completion",
              "completion category name");
    }

    // ------------------------------------------- New Game Plus (6.5)

    section("[41] New Game Plus");
    {
        check(Endgame::ngPlusMonsterMultiplier(0) == 1.0, "NG+0 monsters are normal");
        check(Endgame::ngPlusMonsterMultiplier(1) == 1.5, "NG+1 monsters are 50% stronger",
              QString::number(Endgame::ngPlusMonsterMultiplier(1)));
        check(Endgame::ngPlusMonsterMultiplier(2) == 2.0, "NG+2 monsters are twice as strong");

        check(Endgame::ngPlusRewardMultiplier(0) == 1.0, "NG+0 rewards are normal");
        check(Endgame::ngPlusRewardMultiplier(1) == 1.25, "NG+1 rewards are 25% higher",
              QString::number(Endgame::ngPlusRewardMultiplier(1)));

        // Difficulty rises faster than rewards, so NG+ stays a challenge.
        check(Endgame::ngPlusMonsterMultiplier(2) > Endgame::ngPlusRewardMultiplier(2),
              "monsters scale faster than rewards");

        check(Endgame::ngPlusBanner(0).isEmpty(), "no banner at NG+0");
        check(Endgame::ngPlusBanner(1).contains("New Game +1"), "NG+1 banner",
              Endgame::ngPlusBanner(1));
    }

    // ------------------------------------------- Tavern / Inn (7.1)

    section("[42] Tavern and Inn");
    {
        auto *gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();

        // Ensure there is a member to work with — earlier sections may have cleared it.
        if (members.isEmpty()) {
            members.append([]{
                Character c;
                c.name = "Tavern Test";
                c.race = "Human";
                c.level = 5;
                c.hp = 5; c.maxHp = 30;
                c.mana = 10; c.maxMana = 50;
                c.isAlive = true;
                return c;
            }());
        }

        // Set up a damaged party
        members[0].hp = 5;
        members[0].maxHp = 30;
        members[0].mana = 10;
        members[0].maxMana = 50;
        members[0].addStatus(GameConstants::Poisoned);
        members[0].addStatus(GameConstants::Blinded);

        int goldBefore = gsm->getPartyGold();

        // Rest: 8 hours × 10 gold × 1 living member = 80 gold
        int restCost = 8 * 10 * 1;
        gsm->spendPartyGold(restCost);
        for (auto& c : members) {
            if (!c.isAlive) continue;
            c.hp = c.maxHp;
            c.mana = c.maxMana;
        }

        check(members[0].hp == members[0].maxHp, "rest restores HP",
              QString("%1 / %2").arg(members[0].hp).arg(members[0].maxHp));
        check(members[0].mana == members[0].maxMana, "rest restores mana",
              QString("%1 / %2").arg(members[0].mana).arg(members[0].maxMana));
        check(gsm->getPartyGold() == goldBefore - restCost, "rest deducts gold");

        // Cure: 50 + 50 = 100 gold
        int cureCost = 100;
        gsm->spendPartyGold(cureCost);
        for (int i = 0; i < members.size(); ++i) {
            if (!members[i].isAlive) continue;
            members[i].removeStatus(GameConstants::Poisoned);
            members[i].removeStatus(GameConstants::Blinded);
            gsm->setCharacterStatus(i, GameConstants::Poisoned, false);
            gsm->setCharacterStatus(i, GameConstants::Blinded, false);
        }

        check((members[0].statusFlags & GameConstants::Poisoned) == 0, "poison cured",
              QString::number(members[0].statusFlags));
        check((members[0].statusFlags & GameConstants::Blinded) == 0, "blindness cured",
              QString::number(members[0].statusFlags));
        check(gsm->getPartyGold() == goldBefore - restCost - cureCost, "cure deducts gold");
    }
    {
        // Advance time. incrementPartyAge skips placeholder slots, so give
        // the member a real name first.
        auto *gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        if (members.isEmpty()) {
            members.append([]{
                Character c;
                c.name = "Aging Test";
                c.race = "Human";
                c.level = 5;
                c.age = 18;
                c.hp = 30; c.maxHp = 30;
                c.mana = 50; c.maxMana = 50;
                c.isAlive = true;
                return c;
            }());
        }
        if (members[0].name == "Empty Slot") {
            members[0].name = "Aging Test";
        }
        int ageBefore = members[0].age;
        gsm->incrementPartyAge(1);
        check(members[0].age == ageBefore + 1, "time advances by 1 year",
              QString("%1 -> %2").arg(ageBefore).arg(members[0].age));
    }

    // ------------------------------------------- Character sheet (7.5)

    section("[43] Character sheet");
    {
        auto *gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();

        Character& c = members[0];
        c.name = "TestHero";
        c.race = "Human";
        c.level = 5;
        c.experience = 1000;
        c.hp = 20;
        c.maxHp = 40;
        c.mana = 30;
        c.maxMana = 60;
        c.gold = 500;

        // Effective stats
        check(c.effectiveStrength() >= 8, "effective strength is at least base");
        check(c.effectiveIntelligence() >= 8, "effective intelligence is at least base");
        check(c.effectiveStat("Strength") == c.effectiveStrength(), "effectiveStat matches");

        // Guilds
        c.guildLevels.clear();
        c.joinGuild("Warrior");
        c.joinGuild("Mage");
        check(c.guildLevel("Warrior") == 1, "Warrior guild level 1");
        check(c.guildLevel("Mage") == 1, "Mage guild level 1");
        check(c.totalGuildLevels() == 2, "total guild levels is 2");
        check(c.guildLevel("Paladin") == 0, "not in Paladin guild");

        // Increment guild level
        c.incrementGuildLevel("Warrior");
        check(c.guildLevel("Warrior") == 2, "Warrior guild level 2 after increment");
        check(c.totalGuildLevels() == 3, "total guild levels is 3");
    }
    {
        // Spell book integration
        SpellBook& sb = SpellBook::instance();
        if (sb.isLoaded()) {
            auto *gsm = gameStateManager::instance();
            Character& c = gsm->getPartyMembers()[0];
            // Guild names come from data/guilds.json and are what
            // data/spells.json lists ("Mages Guild", not "Mage").
            // Intelligence must clear the spells' required_stats, or every
            // guild level yields the same handful of level-1 spells.
            c.intelligence = 20;
            c.equipped.clear();

            c.joinGuild("Mages Guild");
            QList<SpellDef> noviceSpells = sb.spellsFor(c);

            c.incrementGuildLevel("Mages Guild");
            c.incrementGuildLevel("Mages Guild");
            c.incrementGuildLevel("Mages Guild");
            QList<SpellDef> spells = sb.spellsFor(c);

            check(spells.size() > 0, "Mage knows spells at guild level 4",
                  QString::number(spells.size()));
            check(spells.size() > noviceSpells.size(),
                  "a higher guild level unlocks more spells",
                  QString("%1 > %2").arg(spells.size()).arg(noviceSpells.size()));
        }
    }

    section("[43b] Character sheet dialog");
    {
        // Constructing the dialog used to crash: refreshSheet() cleared and
        // refilled the member combo, which emitted currentIndexChanged, which
        // is wired to onMemberChanged -> refreshSheet, recursing until the
        // stack overflowed. The dialog must survive construction.
        auto *gsm = gameStateManager::instance();
        if (gsm->getPartyMembers().isEmpty()) {
            Character filler;
            filler.name = "SheetTest";
            gsm->getPartyMembers().append(filler);
        }

        CharacterSheetDialog sheet;
        auto *combo = sheet.findChild<QComboBox*>();
        auto *nameLabel = sheet.findChild<QLabel*>();

        check(combo != nullptr, "character sheet has a member combo");
        if (combo) {
            check(combo->count() == gsm->getPartyMembers().size(),
                  "member combo lists every party member",
                  QString("%1 vs %2").arg(combo->count())
                      .arg(gsm->getPartyMembers().size()));
        }
        check(nameLabel != nullptr, "character sheet shows the member name");

        // The stats table must be filled, not merely present: an empty sheet
        // is the visible symptom of refreshSheet() bailing out early.
        QTableWidget *stats = nullptr;
        for (QTableWidget *t : sheet.findChildren<QTableWidget*>()) {
            if (t->columnCount() == 3 && t->rowCount() == 6) { stats = t; break; }
        }
        check(stats != nullptr, "character sheet has a six-row stats table");
        if (stats) {
            int filled = 0;
            for (int r = 0; r < stats->rowCount(); ++r) {
                const auto *nameCell = stats->item(r, 0);
                const auto *valueCell = stats->item(r, 1);
                if (nameCell && valueCell && !nameCell->text().isEmpty()) ++filled;
            }
            check(filled == 6, "every stat row is populated",
                  QString("%1 of 6").arg(filled));
            if (filled == 6) {
                check(stats->item(0, 0)->text() == QStringLiteral("Strength"),
                      "first stat row is Strength",
                      stats->item(0, 0)->text());
            }
        }
    }

    // ------------------------------------------- Bestiary (7.6)

    section("[67] v1.0.0 slice 1.2: Hall of Records");
    {
        // addGameRecord appends a record and emits the signal.
        gameStateManager* gsm = gameStateManager::instance();
        // Record the count before adding. The suite may run against a state that
        // already has records from a previous section, so compare relatively.
        const int before = gsm->getGameValue("HallOfRecords").toList().size();

        GameRecord rec;
        rec.heroName = "TestHero";
        rec.highestLevel = 12;
        rec.mostGold = 5000;
        rec.deepestFloor = 8;
        rec.completionTimeSeconds = 3600;
        rec.won = true;
        gsm->addGameRecord(rec);

        const QVariantList after = gsm->getGameValue("HallOfRecords").toList();
        check(after.size() == before + 1, "addGameRecord appends one record",
              QString("%1 → %2").arg(before).arg(after.size()));

        // The persisted record round-trips through toMap/loadFromMap.
        GameRecord loaded;
        loaded.loadFromMap(after.last().toMap());
        check(loaded.heroName == "TestHero", "record round-trips: hero name");
        check(loaded.highestLevel == 12, "record round-trips: level");
        check(loaded.mostGold == 5000, "record round-trips: gold");
        check(loaded.deepestFloor == 8, "record round-trips: floor");
        check(loaded.won, "record round-trips: won flag");
        check(loaded.formattedTime() == "01:00:00", "record formatted time",
              loaded.formattedTime());

        // Endgame::ranked sorts best-first for each category.
        QList<GameRecord> records;
        GameRecord a; a.heroName = "A"; a.highestLevel = 5;  a.mostGold = 100;  a.deepestFloor = 3;  a.completionTimeSeconds = 0; a.won = false;
        GameRecord b; b.heroName = "B"; b.highestLevel = 10; b.mostGold = 2000; b.deepestFloor = 12; b.completionTimeSeconds = 7200; b.won = true;
        GameRecord c; c.heroName = "C"; c.highestLevel = 8;  c.mostGold = 500;  c.deepestFloor = 8;  c.completionTimeSeconds = 3600; c.won = true;
        records << a << b << c;

        QList<GameRecord> byLevel = Endgame::ranked(records, Endgame::Category::HighestLevel);
        check(byLevel.first().heroName == "B", "ranked: highest level first",
              byLevel.first().heroName);

        QList<GameRecord> byGold = Endgame::ranked(records, Endgame::Category::MostGold);
        check(byGold.first().heroName == "B", "ranked: most gold first",
              byGold.first().heroName);

        QList<GameRecord> byFloor = Endgame::ranked(records, Endgame::Category::DeepestFloor);
        check(byFloor.first().heroName == "B", "ranked: deepest floor first",
              byFloor.first().heroName);

        // Fastest completion: only wins count, then shortest time.
        QList<GameRecord> bySpeed = Endgame::ranked(records, Endgame::Category::FastestCompletion);
        check(bySpeed.first().heroName == "C", "ranked: fastest completion first",
              bySpeed.first().heroName);

        // A win always outranks a non-win.
        GameRecord loser; loser.heroName = "Loser"; loser.highestLevel = 99; loser.mostGold = 99999; loser.deepestFloor = 15; loser.completionTimeSeconds = 100; loser.won = false;
        GameRecord winner; winner.heroName = "Winner"; winner.highestLevel = 1; winner.mostGold = 1; winner.deepestFloor = 1; winner.completionTimeSeconds = 5000; winner.won = true;
        check(Endgame::outranks(winner, loser, Endgame::Category::FastestCompletion),
              "a win outranks a non-win for fastest completion");
    }

    // ------------------------------------------- v1.0.0 — slice 1.3: New Game Plus

    section("[68] v1.0.0 slice 1.3: New Game Plus");
    {
        // NG+ multipliers: monsters +50%/cycle, rewards +25%/cycle.
        check(Endgame::ngPlusMonsterMultiplier(0) == 1.0, "NG+0 monsters are normal");
        check(Endgame::ngPlusMonsterMultiplier(1) == 1.5, "NG+1 monsters are 50% stronger",
              QString::number(Endgame::ngPlusMonsterMultiplier(1)));
        check(Endgame::ngPlusMonsterMultiplier(2) == 2.0, "NG+2 monsters are twice as strong");
        check(Endgame::ngPlusRewardMultiplier(0) == 1.0, "NG+0 rewards are normal");
        check(Endgame::ngPlusRewardMultiplier(1) == 1.25, "NG+1 rewards are 25% higher",
              QString::number(Endgame::ngPlusRewardMultiplier(1)));
        check(Endgame::ngPlusMonsterMultiplier(2) > Endgame::ngPlusRewardMultiplier(2),
              "monsters scale faster than rewards");

        // NG+ banner describes the difficulty.
        check(Endgame::ngPlusBanner(0).isEmpty(), "no banner for base game");
        check(Endgame::ngPlusBanner(1).contains("50%"), "NG+1 banner mentions 50%",
              Endgame::ngPlusBanner(1));

        // EncounterBuilder applies NG+ scaling to monster stats.
        gameStateManager* gsm = gameStateManager::instance();
        const QList<QVariantMap>& monsters = gsm->getMonsterData();
        if (!monsters.isEmpty()) {
            // Find a monster with known stats.
            QString testMonster;
            for (const QVariantMap& m : monsters) {
                if (m.contains("ingroup") && m["hits"].toInt() > 0) {
                    testMonster = m["name"].toString();
                    break;
                }
            }
            if (testMonster.isEmpty()) testMonster = monsters[0]["name"].toString();

            auto base = EncounterBuilder::buildEncounter(testMonster, monsters, 0);
            auto ng1 = EncounterBuilder::buildEncounter(testMonster, monsters, 1);
            auto ng2 = EncounterBuilder::buildEncounter(testMonster, monsters, 2);

            if (!base.isEmpty() && !ng1.isEmpty() && !ng2.isEmpty()) {
                check(ng1.first().hp > base.first().hp,
                      "NG+1 monster has more HP than base",
                      QString("%1 → %2").arg(base.first().hp).arg(ng1.first().hp));
                check(ng2.first().hp > ng1.first().hp,
                      "NG+2 monster has more HP than NG+1",
                      QString("%1 → %2").arg(ng1.first().hp).arg(ng2.first().hp));
                check(ng1.first().att > base.first().att,
                      "NG+1 monster has higher ATT than base",
                      QString("%1 → %2").arg(base.first().att).arg(ng1.first().att));
            }
        }

        // VictoryReward applies NG+ scaling to gold.
        if (!monsters.isEmpty()) {
            QString testMonster;
            for (const QVariantMap& m : monsters) {
                if (m.contains("ingroup") && m["goldFactor"].toInt() > 0) {
                    testMonster = m["name"].toString();
                    break;
                }
            }
            if (!testMonster.isEmpty()) {
                // Gold has a random component (1–10 × goldFactor × multiplier),
                // so a small sample can invert the comparison by chance. Sum
                // enough rolls that the NG+ multiplier (1.25×) dominates the
                // noise: at 2000 rolls the gap is ~15 sigma.
                int baseTotal = 0, ng1Total = 0;
                for (int i = 0; i < 2000; ++i) {
                    baseTotal += VictoryReward::calculateGold(testMonster, monsters, 0);
                    ng1Total += VictoryReward::calculateGold(testMonster, monsters, 1);
                }
                check(ng1Total > baseTotal,
                      "NG+1 gold reward is higher than base over 2000 rolls",
                      QString("%1 → %2").arg(baseTotal).arg(ng1Total));
            }
        }

        // NG+ level is stored in game state.
        gsm->setNgPlusLevel(0);
        check(gsm->getNgPlusLevel() == 0, "NG+ level starts at 0");
        gsm->setNgPlusLevel(1);
        check(gsm->getNgPlusLevel() == 1, "NG+ level can be set to 1");
        gsm->setNgPlusLevel(0);  // restore
    }

    // ------------------------------------------- v1.0.0 — slice 1.4: MonsterBalance

    section("[192] v5.0.0 slice 5.1: the character sheet shows a hunger row");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "Sheet Test"; c.level = 2; c.hp = 10; c.maxHp = 10;
        c.hunger = 42; c.maxHunger = 100;
        members.append(c);

        CharacterSheetDialog sheet;
        bool found = false;
        for (QLabel* l : sheet.findChildren<QLabel*>()) {
            if (l->text().contains("Hunger") && l->text().contains("42")) {
                found = true; break;
            }
        }
        check(found, "character sheet shows the member's hunger");

        members.clear();
    }

}
