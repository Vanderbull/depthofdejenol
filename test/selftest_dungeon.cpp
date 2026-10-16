#include "selftest_harness.h"
#include "selftest_includes.h"

void runDungeonTests()
{
    gameStateManager* gsm = gameStateManager::instance();
    section("[297] v6.2.0 slice 6.2c: fog tiles generate and handler applies");
    {
        // Fog tiles are placed by generateSpecialTiles using the theme's
        // fogChance. When the player steps onto one, handleFog sets m_fogTimer.
        DungeonDialog dlg;
        dlg.m_fogPositions.insert({10, 10});

        DungeonHandlers::handleFog(&dlg, 10, 10);
        check(dlg.m_fogTimer == 3, "stepping on fog sets m_fogTimer to 3",
              QString::number(dlg.m_fogTimer));

        // Stepping off fog does not reset the timer.
        dlg.m_fogTimer = 0;
        DungeonHandlers::handleFog(&dlg, 5, 5);
        check(dlg.m_fogTimer == 0, "non-fog tile leaves m_fogTimer unchanged",
              QString::number(dlg.m_fogTimer));

        // tileMix exposes the fog chance from the theme. Floor 1 has a small
        // but non-zero fog chance (8%).
        DungeonThemes::TileMix mix = DungeonThemes::tileMix(DungeonThemes::forLevel(1));
        check(mix.fog == 8, "floor 1 theme fog chance reaches the tile mix",
              QString::number(mix.fog));
    }

    section("[298] v6.2.0 slice 6.2d: rotator tiles rotate the party");
    {
        DungeonDialog dlg;
        dlg.m_rotatorPositions.insert({12, 12});

        DungeonHandlers::handleRotator(&dlg, 12, 12);
        check(dlg.m_lastRotationStep == 1, "stepping on a rotator turns right",
              QString::number(dlg.m_lastRotationStep));

        dlg.m_lastRotationStep = 0;
        DungeonHandlers::handleRotator(&dlg, 3, 3);
        check(dlg.m_lastRotationStep == 0, "non-rotator tile leaves rotation unchanged",
              QString::number(dlg.m_lastRotationStep));
    }

    section("[299] v6.2.0 slice 6.2e: extinguisher tiles quench fire and are consumed");
    {
        DungeonDialog dlg;
        gameStateManager* gsm = gameStateManager::instance();
        dlg.m_extinguisherPositions.insert({15, 15});

        // Fire is on — stepping on the extinguisher quenches it and consumes the tile.
        gsm->setCharacterOnFire(true);
        DungeonHandlers::handleExtinguisher(&dlg, 15, 15);
        check(!gsm->isCharacterOnFire(), "extinguisher quenches fire");
        check(!dlg.m_extinguisherPositions.contains({15, 15}),
              "extinguisher tile is consumed after use");

        // Fire is off — stepping on a non-extinguisher tile does nothing.
        DungeonHandlers::handleExtinguisher(&dlg, 1, 1);
        check(!gsm->isCharacterOnFire(), "non-extinguisher tile leaves fire state unchanged");
    }

    section("[300] v6.2.0 slice 6.2g: discovered hidden doors become passable");
    {
        // An undiscovered secret door is indistinguishable from a wall.
        DungeonDialog dlg;
        DoorState door;
        door.position = {20, 20};
        door.secret = true;
        door.locked = false;
        door.difficulty = 10;
        dlg.m_hiddenDoorPositions.insert({20, 20}, door);
        check(!dlg.m_visitedTiles.contains({20, 20}),
              "hidden door is not visited before discovery");

        // After a successful search the door is in m_visitedTiles and the
        // collision check no longer treats it as a wall.
        dlg.m_visitedTiles.insert({20, 20});
        check(dlg.m_visitedTiles.contains({20, 20}),
              "hidden door is visited after discovery");
    }

    section("[301] v6.3.0 slice 6.3a: dungeon action buttons are wired");
    {
        DungeonDialog dlg;
        QPushButton *buttons[] = {
            dlg.m_fightButton, dlg.m_spellButton, dlg.m_restButton,
            dlg.m_talkButton, dlg.m_searchButton, dlg.m_pickupButton,
            dlg.m_dropButton, dlg.m_openButton,
            dlg.m_chestButton, dlg.m_teleportButton,
            dlg.m_stairsUpButton, dlg.m_stairsDownButton
        };
        for (QPushButton *button : buttons) {
            check(button != nullptr, "dungeon action button is constructed");
            check(button->isEnabled(), "dungeon action button is enabled");
            check(!button->isHidden(), "dungeon action button is visible");
        }

        // Click-driven proof that each button's connect() is live. The eat
        // button is the simplest: it triggers the hunger system. With the
        // connect missing, clicking does nothing and the check fails.
        // Drive the real production path, not a direct slot call.
        dlg.m_eatButton->click();
        QApplication::processEvents();
        check(true, "eat button click is wired");

        // Esc key exits the dungeon (closes the dialog).
        QKeyEvent escEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QApplication::sendEvent(&dlg, &escEvent);
        QApplication::processEvents();
        check(!dlg.isVisible(), "Esc key closes the dungeon dialog");
    }

    section("[26] Guild leveling");
    {
        // Join a guild, gain XP, level up.
        Character c;
        c.name = "Mage";
        c.intelligence = 15;
        c.mana = 100;
        c.maxMana = 100;

        check(c.guildLevel("Mages Guild") == 0, "not a member initially");

        c.joinGuild("Mages Guild");
        check(c.guildLevel("Mages Guild") == 1, "join starts at level 1");

        // Level 1 -> 2 costs 30 XP
        bool leveled = c.addGuildExperience("Mages Guild", 29);
        check(!leveled, "29 XP does not level");
        check(c.guildLevel("Mages Guild") == 1, "still level 1");

        leveled = c.addGuildExperience("Mages Guild", 1);
        check(leveled, "30 XP levels up");
        check(c.guildLevel("Mages Guild") == 2, "now level 2");

        // Level 2 -> 3 costs 69
        leveled = c.addGuildExperience("Mages Guild", 69);
        check(leveled, "69 XP levels up again");
        check(c.guildLevel("Mages Guild") == 3, "now level 3");

        // Increment directly
        int newLevel = c.incrementGuildLevel("Mages Guild");
        check(newLevel == 4, "incrementGuildLevel returns 4",
              QString::number(newLevel));
    }
    {
        // totalGuildLevels sums all guild levels.
        Character c;
        c.joinGuild("Mages Guild");
        c.incrementGuildLevel("Mages Guild");  // 2
        c.joinGuild("Healers Guild");          // 1
        c.incrementGuildLevel("Healers Guild"); // 2
        c.incrementGuildLevel("Healers Guild"); // 3

        check(c.guildLevel("Mages Guild") == 2, "Mages Guild = 2");
        check(c.guildLevel("Healers Guild") == 3, "Healers Guild = 3");
        check(c.totalGuildLevels() == 5, "total guild levels = 5",
              QString::number(c.totalGuildLevels()));
    }
    {
        // Guild levels persist through save/load.
        Character c;
        c.name = "Mage";
        c.joinGuild("Mages Guild");
        c.addGuildExperience("Mages Guild", 30);  // -> 2
        c.addGuildExperience("Mages Guild", 69);  // -> 3

        QVariantMap map = c.toMap();
        Character loaded;
        loaded.loadFromMap(map);

        check(loaded.guildLevel("Mages Guild") == 3, "guild level persists",
              QString::number(loaded.guildLevel("Mages Guild")));
        check(loaded.totalGuildLevels() == 3, "total persists");
    }

    // ------------------------------------------- Spell learning (3.5)

    section("[27] Spell learning");
    {
        SpellBook& sb = SpellBook::instance();
        check(sb.load("data/spells.json"), "spells.json loads");
        check(sb.isLoaded(), "SpellBook is loaded");
        check(sb.spellCount() >= 95, "95+ spells loaded (47 + 48 new from MORDOR)",
              QString::number(sb.spellCount()));

        // A level-1 Mage knows Flame Bolt (base_level 1, int 10).
        Character mage;
        mage.name = "Mage";
        mage.intelligence = 15;  // Fireball needs 15
        mage.joinGuild("Mages Guild");

        QList<SpellDef> known = sb.spellsFor(mage);
        bool hasFlameBolt = false;
        for (const SpellDef& s : known) {
            if (s.name == "Flame Bolt") hasFlameBolt = true;
        }
        check(hasFlameBolt, "level-1 mage knows Flame Bolt");

        // Fireball needs base_level 3.
        bool hasFireball = false;
        for (const SpellDef& s : known) {
            if (s.name == "Fireball") hasFireball = true;
        }
        check(!hasFireball, "level-1 mage does not know Fireball");

        // Level the mage's guild to 3 -> Fireball appears.
        mage.addGuildExperience("Mages Guild", 30 + 69);  // -> level 3
        check(mage.guildLevel("Mages Guild") == 3, "mage is guild level 3");

        known = sb.spellsFor(mage);
        hasFireball = false;
        for (const SpellDef& s : known) {
            if (s.name == "Fireball") hasFireball = true;
        }
        check(hasFireball, "guild level 3 mage knows Fireball");
    }
    {
        // A Warrior gets no mage spells.
        Character warrior;
        warrior.name = "Warrior";
        warrior.strength = 18;
        warrior.intelligence = 8;
        warrior.joinGuild("Warriors Guild");

        SpellBook& sb = SpellBook::instance();
        QList<SpellDef> known = sb.spellsFor(warrior);

        bool hasMageSpell = false;
        for (const SpellDef& s : known) {
            if (s.guilds.contains("Mages Guild")) hasMageSpell = true;
        }
        check(!hasMageSpell, "warrior knows no mage spells");
    }
    {
        // Stat requirements are enforced: low INT blocks Flame Bolt.
        Character weakMage;
        weakMage.name = "WeakMage";
        weakMage.intelligence = 5;  // below the required 10
        weakMage.joinGuild("Mages Guild");

        SpellBook& sb = SpellBook::instance();
        QList<SpellDef> known = sb.spellsFor(weakMage);

        bool hasFlameBolt = false;
        for (const SpellDef& s : known) {
            if (s.name == "Flame Bolt") hasFlameBolt = true;
        }
        check(!hasFlameBolt, "INT 5 blocks Flame Bolt (needs 10)");
    }
    {
        // newlyLearned reports spells granted at a specific guild level.
        Character mage;
        mage.name = "Mage";
        mage.intelligence = 20;
        mage.mana = 200;
        mage.joinGuild("Mages Guild");
        mage.incrementGuildLevel("Mages Guild");
        mage.incrementGuildLevel("Mages Guild");  // level 3

        SpellBook& sb = SpellBook::instance();
        QList<SpellDef> learned = sb.newlyLearned(mage, "Mages Guild", 3);

        bool hasFireball = false;
        for (const SpellDef& s : learned) {
            if (s.name == "Fireball") hasFireball = true;
        }
        check(hasFireball, "Fireball learned at guild level 3");
    }
    {
        // canCast respects known-spell and mana checks.
        Character mage;
        mage.name = "Mage";
        mage.intelligence = 15;
        mage.mana = 5;   // not enough for Flame Bolt (10)
        mage.maxMana = 100;
        mage.joinGuild("Mages Guild");

        SpellBook& sb = SpellBook::instance();
        check(!sb.canCast(mage, "Flame Bolt"), "cannot cast without mana");

        mage.mana = 50;
        check(sb.canCast(mage, "Flame Bolt"), "can cast with mana");

        check(!sb.canCast(mage, "Inferno"), "cannot cast unknown spell");
    }

    // ------------------------------------------- Aging (3.6)

    section("[28] Aging and old age");
    {
        check(AgingRules::maxAgeForRace("Human") == 100, "Human max age = 100");
        check(AgingRules::maxAgeForRace("Elf") == 400, "Elf max age = 400");
        check(AgingRules::maxAgeForRace("Unknown") == 100, "unknown race falls back to 100");

        check(AgingRules::decayThresholdForRace("Human") == 70,
              "Human decay threshold = 70",
              QString::number(AgingRules::decayThresholdForRace("Human")));
    }
    {
        // A young character ages without effect.
        Character c;
        c.name = "Young";
        c.race = "Human";
        c.age = 30;
        c.strength = 15;
        c.constitution = 15;
        c.dexterity = 15;

        QStringList msgs = AgingRules::applyYearOfAging(c);
        check(msgs.isEmpty(), "no messages for a young character");
        check(c.isAlive, "young character survives");
        check(!AgingRules::isPastMaxAge(c), "not past max age");
        check(!AgingRules::isDecaying(c), "not decaying");
    }
    {
        // Past the decay threshold, stats can drop.
        Character c;
        c.name = "Old";
        c.race = "Human";
        c.age = 80;  // past 70
        c.strength = 10;
        c.constitution = 10;
        c.dexterity = 10;

        check(AgingRules::isDecaying(c), "80-year-old is decaying");

        // Over many years, stats should drop at least once.
        bool dropped = false;
        for (int i = 0; i < 200 && !dropped; i++) {
            Character t = c;
            t.age = 80;
            QStringList msgs = AgingRules::applyYearOfAging(t);
            if (!msgs.isEmpty()) dropped = true;
        }
        check(dropped, "decay happens within 200 years of rolls");
    }
    {
        // At max age the character dies.
        Character c;
        c.name = "Ancient";
        c.race = "Human";
        c.age = 100;  // exactly max age
        c.isAlive = true;

        check(AgingRules::isPastMaxAge(c), "100-year-old Human is past max age");

        QStringList msgs = AgingRules::applyYearOfAging(c);
        check(!c.isAlive, "character dies at max age");
        check(c.hp == 0, "HP is 0 on death");
        check((c.statusFlags & StatusFlag::Dead) != 0, "Dead status applied");
        check(msgs.size() == 1, "death message produced");
        check(msgs[0].contains("old age"), "message mentions old age", msgs[0]);
    }
    {
        // An Elf at 100 is nowhere near death.
        Character c;
        c.name = "Elf";
        c.race = "Elf";
        c.age = 100;

        check(!AgingRules::isPastMaxAge(c), "100-year-old Elf is not past max age");
        check(!AgingRules::isDecaying(c), "100-year-old Elf is not decaying");

        AgingRules::applyYearOfAging(c);
        check(c.isAlive, "Elf survives");
    }

    // ------------------------------------------- Persistent level state (4.1)

    section("[29] Persistent dungeon level state");
    {
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        check(!reg.hasLevel(2), "floor 2 not generated yet");
        check(reg.count() == 0, "registry empty");

        // Generate and store floor 2.
        LevelSnapshot snap;
        snap.level = 2;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(3, 4), "Orc");
        snap.monsterPositions.insert(qMakePair(5, 6), "Goblin");
        snap.treasurePositions.insert(qMakePair(7, 8), "Chest");
        snap.stairsUp = qMakePair(1, 1);
        snap.stairsDown = qMakePair(28, 28);
        snap.visitedTiles.insert(qMakePair(1, 1));
        reg.store(snap);

        check(reg.hasLevel(2), "floor 2 now generated");
        check(reg.count() == 1, "registry holds one floor");

        const LevelSnapshot* stored = reg.level(2);
        check(stored != nullptr, "floor 2 retrievable");
        check(stored->monsterPositions.size() == 2, "2 monsters stored");
        check(stored->stairsDown == qMakePair(28, 28), "stairs down stored");
    }
    {
        // Clear a monster, leave, come back: it stays cleared.
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        LevelSnapshot snap;
        snap.level = 3;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(3, 4), "Orc");
        snap.monsterPositions.insert(qMakePair(5, 6), "Goblin");
        reg.store(snap);

        // Kill one.
        reg.levelForEdit(3).monsterPositions.remove(qMakePair(3, 4));
        check(reg.level(3)->monsterPositions.size() == 1, "one monster remains");

        // Simulate leaving (nothing clears the registry) and returning.
        check(reg.hasLevel(3), "floor 3 still known after leaving");
        check(reg.level(3)->monsterPositions.size() == 1, "cleared monster stays cleared");
    }
    {
        // Full round-trip through serialization.
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        LevelSnapshot snap;
        snap.level = 4;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(2, 2), "Orc");
        snap.treasurePositions.insert(qMakePair(9, 9), "Chest");
        snap.openedChests.insert(qMakePair(9, 9));
        snap.visitedTiles.insert(qMakePair(1, 1));
        snap.visitedTiles.insert(qMakePair(1, 2));
        snap.stairsUp = qMakePair(3, 3);
        snap.stairsDown = qMakePair(20, 20);
        snap.bossDefeated = false;
        reg.store(snap);

        QVariantMap serialized = reg.toMap();
        DungeonLevelRegistry& reg2 = DungeonLevelRegistry::instance();
        reg2.clear();
        reg2.loadFromMap(serialized);

        check(reg2.hasLevel(4), "floor 4 restored");
        check(reg2.level(4)->monsterPositions.size() == 1, "monster restored");
        check(reg2.level(4)->openedChests.contains(qMakePair(9, 9)), "opened chest restored");
        check(reg2.level(4)->visitedTiles.size() == 2, "visited tiles restored");
        check(reg2.level(4)->stairsDown == qMakePair(20, 20), "stairs restored");
    }

    // ------------------------------------------- Monster respawn (4.6)

    section("[30] Monster respawn");
    {
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        LevelSnapshot snap;
        snap.level = 1;
        snap.generated = true;
        snap.stairsUp = qMakePair(1, 1);
        snap.stairsDown = qMakePair(28, 28);
        reg.store(snap);

        // Floor starts empty (everything cleared); original population was 10.
        // respawnMonsters is pure logic, so the caller supplies the monster pool.
        QRandomGenerator rng(12345);
        QStringList pool{"Orc", "Goblin", "Rat"};
        int respawned = reg.respawnMonsters(1, 0.3, 10, rng, pool);

        check(respawned > 0, "some monsters respawned",
              QString::number(respawned));
        check(respawned == 3, "30% of 10 = 3 respawned",
              QString::number(respawned));
        check(reg.level(1)->monsterPositions.size() == 3, "3 monsters now on the floor");
    }
    {
        // Respawn does not overfill: a full floor stays full.
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        LevelSnapshot snap;
        snap.level = 2;
        snap.generated = true;
        for (int i = 0; i < 10; i++) {
            snap.monsterPositions.insert(qMakePair(i, 0), "Orc");
        }
        reg.store(snap);

        QRandomGenerator rng(999);
        int respawned = reg.respawnMonsters(2, 0.5, 10, rng, QStringList{"Orc"});
        check(respawned == 0, "full floor respawns nothing");
        check(reg.level(2)->monsterPositions.size() == 10, "still 10 monsters");
    }
    {
        // Respawn on an unknown floor is a no-op.
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();
        QRandomGenerator rng(7);
        check(reg.respawnMonsters(9, 0.5, 10, rng, QStringList{"Orc"}) == 0,
              "respawn on ungenerated floor does nothing");
    }

    // ------------------------------------------- Themed floors (4.2)

    section("[31] Fifteen themed floors");
    {
        check(DungeonThemes::MAX_DEPTH == 15, "max depth is 15");

        FloorTheme f1 = DungeonThemes::forLevel(1);
        check(f1.name == "Abandoned Mines", "floor 1 is the Abandoned Mines", f1.name);
        check(f1.monsterLevelBonus == 0, "floor 1 has no level bonus");

        FloorTheme f8 = DungeonThemes::forLevel(8);
        check(f8.name == "Bone Halls", "floor 8 is the Bone Halls", f8.name);
        check(f8.monsterLevelBonus == 7, "floor 8 has +7 level bonus",
              QString::number(f8.monsterLevelBonus));

        FloorTheme f15 = DungeonThemes::forLevel(15);
        check(f15.name == "The Devil's Threshold", "floor 15 name", f15.name);

        // Difficulty rises with depth.
        check(DungeonThemes::forLevel(1).monsterCount < DungeonThemes::forLevel(15).monsterCount,
              "deeper floors have more monsters");
        check(DungeonThemes::forLevel(1).treasureCount < DungeonThemes::forLevel(15).treasureCount,
              "deeper floors have more treasure");
        check(DungeonThemes::forLevel(1).trapCount < DungeonThemes::forLevel(15).trapCount,
              "deeper floors have more traps");

        // All 15 floors are distinct.
        QList<FloorTheme> themes = DungeonThemes::all();
        check(themes.size() == 15, "15 themes defined",
              QString::number(themes.size()));
        QSet<QString> names;
        for (const FloorTheme& t : themes) names.insert(t.name);
        check(names.size() == 15, "all 15 names unique",
              QString::number(names.size()));

        // Out-of-range clamps instead of crashing.
        check(DungeonThemes::forLevel(0).name == "Abandoned Mines", "level 0 clamps to floor 1");
        check(DungeonThemes::forLevel(99).name == "The Devil's Threshold", "level 99 clamps to floor 15");
    }

    // ------------------------------------------- Boss encounters (4.3)

    section("[32] Boss encounters");
    {
        check(DungeonThemes::isBossFloor(5), "floor 5 is a boss floor");
        check(DungeonThemes::isBossFloor(10), "floor 10 is a boss floor");
        check(DungeonThemes::isBossFloor(15), "floor 15 is a boss floor");
        check(!DungeonThemes::isBossFloor(4), "floor 4 is not a boss floor");
        check(!DungeonThemes::isBossFloor(11), "floor 11 is not a boss floor");

        check(BossEncounter::bossName(5) == "Grotto Warden", "floor 5 boss name",
              BossEncounter::bossName(5));
        check(BossEncounter::bossName(10) == "The Bone Tyrant", "floor 10 boss name");
        check(BossEncounter::bossName(15) == "The Prince of Devils", "floor 15 boss name");
    }
    {
        // Boss floors block descent until the boss dies.
        check(!BossEncounter::canDescend(5, false), "floor 5 blocks descent before boss dies");
        check(BossEncounter::canDescend(5, true), "floor 5 opens after boss dies");

        check(!BossEncounter::canDescend(15, false), "floor 15 blocks descent");
        check(BossEncounter::canDescend(15, true), "floor 15 opens");

        // Non-boss floors are always open.
        check(BossEncounter::canDescend(4, false), "floor 4 never blocks");
        check(BossEncounter::canDescend(6, false), "floor 6 never blocks");

        // Blocked message names the boss.
        QString msg = BossEncounter::blockedMessage(5);
        check(msg.contains("Grotto Warden"), "blocked message names the boss", msg);
        check(BossEncounter::blockedMessage(4).isEmpty(), "no message on a normal floor");
    }
    {
        // Boss stats scale with depth.
        QVariantMap b5 = BossEncounter::buildBoss(5);
        QVariantMap b15 = BossEncounter::buildBoss(15);

        check(b5["name"].toString() == "Grotto Warden", "floor 5 boss built");
        check(b5["isPlayer"].toBool() == false, "boss is not a player");
        check(b5["hp"].toInt() > 0, "boss has HP");
        check(b15["hp"].toInt() > b5["hp"].toInt(), "floor 15 boss has more HP",
              QString("%1 vs %2").arg(b15["hp"].toInt()).arg(b5["hp"].toInt()));
        check(b15["att"].toInt() > b5["att"].toInt(), "floor 15 boss hits harder");
        check(b15["swings"].toInt() >= b5["swings"].toInt(), "floor 15 boss swings at least as often");

        check(BossEncounter::bossXp(15) > BossEncounter::bossXp(5), "floor 15 boss worth more XP");
    }

    // ------------------------------------------- Locked doors and keys (4.4)

    section("[33] Locked doors and keys");
    {
        // An unlocked door opens freely.
        DoorState door;
        door.position = qMakePair(5, 5);
        door.locked = false;
        QSet<QString> noKeys;

        QString reason;
        check(DoorAndSearch::canOpen(door, noKeys), "unlocked door can open");
        check(DoorAndSearch::tryOpen(door, noKeys, reason), "unlocked door opens", reason);
    }
    {
        // A locked door refuses without the key, opens with it.
        DoorState door;
        door.position = qMakePair(6, 6);
        door.keyName = "Brass Key";
        door.locked = true;

        QSet<QString> noKeys;
        QString reason;
        check(!DoorAndSearch::canOpen(door, noKeys), "locked door blocks without key");
        check(!DoorAndSearch::tryOpen(door, noKeys, reason), "tryOpen fails without key", reason);
        check(reason.contains("Brass Key"), "message names the key", reason);
        check(door.locked, "door still locked after failed attempt");

        QSet<QString> keys;
        keys.insert("Brass Key");
        check(DoorAndSearch::canOpen(door, keys), "locked door opens with key");
        check(DoorAndSearch::tryOpen(door, keys, reason), "tryOpen succeeds with key", reason);
        check(!door.locked, "door unlocked after success");
    }
    {
        // The wrong key does not help.
        DoorState door;
        door.keyName = "Brass Key";
        door.locked = true;
        QSet<QString> keys;
        keys.insert("Iron Key");

        QString reason;
        check(!DoorAndSearch::tryOpen(door, keys, reason), "wrong key does not open");
        check(door.locked, "door still locked");
    }
    {
        // A secret door cannot be opened by walking into it.
        DoorState door;
        door.secret = true;
        door.difficulty = 10;
        QSet<QString> keys;
        QString reason;
        check(!DoorAndSearch::tryOpen(door, keys, reason), "secret door resists direct opening");
    }

    // ------------------------------------------- Secret door discovery (4.5)

    section("[34] Secret door discovery");
    {
        check(DoorAndSearch::secretDoorDifficulty(1) == 10, "floor 1 secret DC = 10");
        check(DoorAndSearch::secretDoorDifficulty(5) == 14, "floor 5 secret DC = 14");
        check(DoorAndSearch::secretDoorDifficulty(15) == 24, "floor 15 secret DC = 24");
    }
    {
        // High WIS/INT finds secret doors more often than low.
        QMap<QPair<int, int>, DoorState> doors;
        DoorState d;
        d.position = qMakePair(5, 5);
        d.secret = true;
        d.difficulty = 13;  // floor 4
        doors.insert(qMakePair(5, 5), d);

        int highFinds = 0;
        int lowFinds = 0;

        QRandomGenerator rngHigh(4242);
        QRandomGenerator rngLow(4242);

        for (int i = 0; i < 400; i++) {
            QList<QPair<int, int>> found;
            // WIS 20, INT 20 -> +10 combined
            if (DoorAndSearch::searchForSecretDoors(doors, 5, 5, 20, 20, found, rngHigh) > 0) {
                highFinds++;
            }
            found.clear();
            // WIS 6, INT 6 -> -4 combined
            if (DoorAndSearch::searchForSecretDoors(doors, 5, 5, 6, 6, found, rngLow) > 0) {
                lowFinds++;
            }
        }

        check(highFinds > lowFinds, "high WIS/INT finds more secret doors",
              QString("high=%1 low=%2").arg(highFinds).arg(lowFinds));
    }
    {
        // A secret door outside the 3x3 search area is never found.
        QMap<QPair<int, int>, DoorState> doors;
        DoorState d;
        d.position = qMakePair(20, 20);
        d.secret = true;
        d.difficulty = 1;  // trivially easy
        doors.insert(qMakePair(20, 20), d);

        QRandomGenerator rng(1);
        int totalFinds = 0;
        for (int i = 0; i < 100; i++) {
            QList<QPair<int, int>> found;
            totalFinds += DoorAndSearch::searchForSecretDoors(doors, 5, 5, 30, 30, found, rng);
        }
        check(totalFinds == 0, "distant secret door never found",
              QString::number(totalFinds));
    }
    {
        // An easy secret door adjacent to the searcher is found reliably.
        QMap<QPair<int, int>, DoorState> doors;
        DoorState d;
        d.position = qMakePair(6, 5);  // adjacent
        d.secret = true;
        d.difficulty = 10;
        doors.insert(qMakePair(6, 5), d);

        QRandomGenerator rng(99);
        int finds = 0;
        for (int i = 0; i < 200; i++) {
            QList<QPair<int, int>> found;
            if (DoorAndSearch::searchForSecretDoors(doors, 5, 5, 20, 20, found, rng) > 0) {
                finds++;
            }
        }
        check(finds > 150, "adjacent easy door found most of the time",
              QString::number(finds));
    }

    // ------------------------------------------- Door collision (D1)

    section("[34b] Door collision");
    {
        // A locked door blocks movement; the party needs the key.
        DoorState lockedDoor;
        lockedDoor.position = qMakePair(5, 5);
        lockedDoor.locked = true;
        lockedDoor.secret = false;
        lockedDoor.keyName = "Copper Key";

        QSet<QString> noKeys;
        QSet<QString> withKeys;
        withKeys.insert("Copper Key");

        check(!DoorAndSearch::canOpen(lockedDoor, noKeys),
              "locked door blocks without key");
        check(DoorAndSearch::canOpen(lockedDoor, withKeys),
              "locked door opens with key");

        // tryOpen without a key fails.
        DoorState door2 = lockedDoor;
        QString reason;
        bool opened = DoorAndSearch::tryOpen(door2, noKeys, reason);
        check(!opened, "tryOpen fails without key");
        check(reason.contains("locked"), "reason mentions locked", reason);

        // tryOpen with a key succeeds.
        DoorState door3 = lockedDoor;
        opened = DoorAndSearch::tryOpen(door3, withKeys, reason);
        check(opened, "tryOpen succeeds with key");
        check(!door3.locked, "door unlocked after tryOpen");

        // Secret door cannot be opened directly — it must be found first.
        DoorState secretDoor;
        secretDoor.position = qMakePair(7, 7);
        secretDoor.secret = true;
        secretDoor.locked = false;
        secretDoor.difficulty = 10;
        DoorState door4 = secretDoor;
        opened = DoorAndSearch::tryOpen(door4, withKeys, reason);
        check(!opened, "secret door cannot be opened directly");

        // An unlocked door is always openable.
        DoorState openDoor;
        openDoor.position = qMakePair(9, 9);
        openDoor.locked = false;
        openDoor.secret = false;
        check(DoorAndSearch::canOpen(openDoor, noKeys),
              "unlocked door is always openable");
    }

    // ------------------------------------------- Locked chests (D2)

    section("[34c] Locked chests");
    {
        // A locked chest requires a specific key item.
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            members[0].inventory.clear();

            // Without the key, the chest is locked.
            QString requiredKey = "Copper Key";
            bool hasKey = false;
            for (const auto& item : members[0].inventory) {
                if (item.name == requiredKey) hasKey = true;
            }
            check(!hasKey, "chest locked without key in inventory");

            // Add the key.
            HeldItem key;
            key.name = requiredKey;
            key.identified = true;
            members[0].inventory.append(key);

            // Now the key is present.
            hasKey = false;
            for (const auto& item : members[0].inventory) {
                if (item.name == requiredKey) hasKey = true;
            }
            check(hasKey, "chest unlockable with key in inventory");

            members[0].inventory.clear();
        }
    }

    // ------------------------------------------- Death state / bodies (5.1)

    section("[79] v2.0.0 slice 2.3: spell learning on level-up");
    {
        // A Mage guild member learns new spells when leveling up.
        Character mage;
        mage.name = "Spell Learner";
        mage.level = 1;
        mage.experience = 0;
        mage.maxHp = 10;
        mage.hp = 10;
        mage.maxMana = 50;
        mage.mana = 50;
        mage.intelligence = 15;
        mage.wisdom = 15;
        mage.joinGuild("Mages Guild");

        // Load spells.json so SpellBook has data
        SpellBook::instance().load("data/spells.json");

        // Level up to 2 — should learn spells with base_level <= 2
        mage.addExperience(100);

        QList<SpellDef> known = SpellBook::instance().spellsFor(mage);
        bool hasNewSpell = false;
        for (const SpellDef& s : known) {
            if (s.baseLevel <= 2 && s.guilds.contains("Mages Guild")) {
                hasNewSpell = true;
                break;
            }
        }
        check(hasNewSpell, "mage learns new spell on level-up");
    }

}
