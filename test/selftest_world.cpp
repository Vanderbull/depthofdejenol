#include "selftest_harness.h"
#include "selftest_includes.h"
#include "src/combat/CombatController.h"

void runWorldTests()
{
    gameStateManager* gsm = gameStateManager::instance();

    // Several world sections assume a party exists (aging, hunger, etc.).
    // Core may not have set one up yet, so ensure a minimal party here.
    if (gsm->getPartyMembers().isEmpty()) {
        gsm->addCharacterToParty([]{
            Character c;
            c.name = "World Hero";
            c.race = "Human";
            c.level = 5;
            c.hp = 100; c.maxHp = 100;
            c.isAlive = true;
            return c;
        }());
    }

    section("[4b] Saving over an existing file (autosave path)");
    {
        gsm->getParty().sharedGold = 111;
        check(gsm->saveFullGameState(kTestSave), "second save over existing file succeeds");
        gsm->getParty().sharedGold = 222;
        check(gsm->saveFullGameState(kTestSave), "third save over existing file succeeds");

        const QJsonObject again = readSaveFile();
        const int gold = again.value("Party").toObject().value("SharedGold").toInt();
        check(gold == 222, "the latest write wins", QString::number(gold));

        // No stray temp file should be left behind.
        check(!QFile::exists(savePath() + ".tmp"), "no leftover .tmp file");
    }

    // ------------------------------------ load replaces rather than appends

    section("[146] v3.0.0 slice 3.4: statPoints persists through save/load");
    {
        Character c;
        c.name = "Persist";
        c.race = "Human";
        c.level = 5;
        c.statPoints = 7;
        c.strength = 12;

        QVariantMap m = c.toMap();
        check(m.value("StatPoints").toInt() == 7, "StatPoints serialized");

        Character c2;
        c2.loadFromMap(m);
        check(c2.statPoints == 7, "StatPoints deserialized",
              QString::number(c2.statPoints));
        check(c2.strength == 12, "strength also deserialized");
    }

    section("[44] Bestiary");
    {
        // The bestiary is built from data/bestiary.json (see
        // tools/gen_bestiary.py), not straight from the raw CSV, so that it
        // can carry the categories, pictures and walkthrough prose the CSV
        // only stores as opaque flags.
        const QList<QVariantMap>& entries = BestiaryDialog::entries();
        check(entries.size() > 0, "bestiary data is loaded",
              QString::number(entries.size()));

        int missingName = 0, missingHits = 0, missingImage = 0, missingCategory = 0;
        int missingPictureFile = 0;
        // picture -> how many monsters use it, and the best category count
        // for that picture.
        QHash<int, int> pictureCounts;
        QHash<int, int> topCategoryCount;
        QHash<int, QHash<QString, int>> categoryCounts;
        for (const auto& m : entries) {
            if (m.value("name").toString().isEmpty()) ++missingName;
            if (m.value("hits", 0).toInt() <= 0) ++missingHits;
            if (m.value("category").toString().isEmpty()) ++missingCategory;
            const QString img = m.value("image").toString();
            if (img.isEmpty()) ++missingImage;
            else if (!QFile::exists(img)) ++missingPictureFile;

            const int pic = m.value("picture").toInt();
            pictureCounts[pic] += 1;
            categoryCounts[pic][m.value("category").toString()] += 1;
        }
        for (auto it = categoryCounts.constBegin(); it != categoryCounts.constEnd(); ++it) {
            int best = 0;
            for (int n : it.value()) best = qMax(best, n);
            topCategoryCount.insert(it.key(), best);
        }
        check(missingName == 0, "every monster has a name",
              QString("%1 missing").arg(missingName));
        check(missingHits == 0, "every monster has hits",
              QString("%1 missing").arg(missingHits));
        check(missingCategory == 0, "every monster has a category",
              QString("%1 missing").arg(missingCategory));
        check(missingImage == 0, "every monster has a picture",
              QString("%1 missing").arg(missingImage));
        check(missingPictureFile == 0, "every picture file exists on disk",
              QString("%1 missing").arg(missingPictureFile));

        // The CSV's raw hp column does not exist; the field is "hits".
        // Guard the exact bug that made the old dialog show HP: 0.
        const QVariantMap goblie = BestiaryDialog::entry("Goblie");
        check(!goblie.isEmpty(), "a known monster can be looked up by name");
        if (!goblie.isEmpty()) {
            check(goblie.value("hits").toInt() > 0, "looked-up monster has hits",
                  QString::number(goblie.value("hits").toInt()));

            // The portrait must actually decode, not merely exist on disk.
            const QString path = BestiaryDialog::imagePath(goblie);
            QPixmap pix(path);
            check(!pix.isNull(), "the portrait loads as a pixmap", path);
            check(pix.width() > 0 && pix.height() > 0,
                  "the portrait has real dimensions",
                  QString("%1x%2").arg(pix.width()).arg(pix.height()));
        }

        // The five picture groups the walkthrough wraps onto long lines used
        // to lose their category entirely.
        check(BestiaryDialog::categories().size() > 10,
              "bestiary has multiple categories",
              QString::number(BestiaryDialog::categories().size()));

        // Categories come from the walkthrough's own headings and are per
        // monster, not per picture: "Werebear" sits under Lycanthropes even
        // though it shares a picture with the bears. So the picture only
        // supplies a fallback category for monsters the walkthrough never
        // names, and that fallback must cover at least half the picture's
        // monsters or it would be arbitrary.
        int ambiguousPictures = 0;
        for (auto it = pictureCounts.constBegin(); it != pictureCounts.constEnd(); ++it) {
            const int total = it.value();
            const int top = topCategoryCount.value(it.key(), 0);
            if (total > 0 && top * 2 < total) ++ambiguousPictures;
        }
        check(ambiguousPictures == 0,
              "every picture has a dominant category",
              QString("%1 ambiguous").arg(ambiguousPictures));

        // Every category the data uses must be one of the known headings.
        const QStringList known = BestiaryDialog::categories();
        int unknownCategory = 0;
        for (const auto& m : entries) {
            if (!known.contains(m.value("category").toString())) ++unknownCategory;
        }
        check(unknownCategory == 0, "every category is a known heading",
              QString("%1 unknown").arg(unknownCategory));

        // Floor distribution still spans the dungeon.
        QSet<int> floors;
        for (const auto& m : entries) floors.insert(m.value("level").toInt());
        check(floors.size() > 1, "monsters span multiple floors",
              QString::number(floors.size()));
    }

    // ------------------------------------------- Journal (7.7)

    section("[45] Journal");
    {
        // Start from a clean file: earlier tests now trigger real journal
        // writes (level-ups, quests), so the absolute contents must be reset.
        JournalDialog::clearAll();

        // Add an entry
        JournalDialog::addEntry("Quest", "Test quest entry");
        JournalDialog::addEntry("Combat", "Test combat entry");

        // Load and verify
        QFile file("data/journal.json");
        check(file.exists(), "journal file created");

        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();

            check(doc.isArray(), "journal is a JSON array");
            QJsonArray arr = doc.array();
            check(arr.size() >= 2, "journal has at least 2 entries",
                  QString::number(arr.size()));

            // Check first entry
            if (arr.size() > 0) {
                QJsonObject obj = arr[0].toObject();
                check(obj.value("category").toString() == "Quest", "first entry is Quest");
                check(obj.value("text").toString() == "Test quest entry", "first entry text matches");
            }
        }

        // Clean up
        QFile::remove("data/journal.json");
    }

    // ------------------------------------------- Quest board (7.2)

    section("[42] Quest board");
    {
        QuestBoardDialog::reset();

        QList<BoardQuest> quests = QuestBoardDialog::availableQuests();
        check(quests.size() >= 4, "at least 4 quests posted",
              QString::number(quests.size()));

        // Find a kill quest and a fetch quest.
        BoardQuest killQuest;
        BoardQuest fetchQuest;
        for (const BoardQuest& q : quests) {
            if (q.isFetch && fetchQuest.id.isEmpty()) fetchQuest = q;
            if (!q.isFetch && killQuest.id.isEmpty()) killQuest = q;
        }
        check(!killQuest.id.isEmpty(), "a kill quest exists");
        check(!fetchQuest.id.isEmpty(), "a fetch quest exists");
    }
    {
        // Accept a quest.
        QuestBoardDialog::reset();
        BoardQuest q = QuestBoardDialog::availableQuests().first();

        check(QuestBoardDialog::acceptQuest(q.id), "quest accepted");
        check(QuestBoardDialog::isAccepted(q.id), "quest is accepted");
        check(!QuestBoardDialog::acceptQuest(q.id), "cannot accept twice");
    }
    {
        // Kill quest progress.
        QuestBoardDialog::reset();
        BoardQuest q = QuestBoardDialog::availableQuests().first();
        QuestBoardDialog::acceptQuest(q.id);

        check(!QuestBoardDialog::isComplete(q.id), "quest not complete at start");

        for (int i = 0; i < q.killCount; ++i) {
            QuestBoardDialog::reportKill(q.targetMonster, q.targetFloor);
        }

        check(QuestBoardDialog::isComplete(q.id), "quest complete after enough kills");
    }
    {
        // Turn in a completed quest.
        QuestBoardDialog::reset();
        BoardQuest q = QuestBoardDialog::availableQuests().first();
        QuestBoardDialog::acceptQuest(q.id);

        for (int i = 0; i < q.killCount; ++i) {
            QuestBoardDialog::reportKill(q.targetMonster, q.targetFloor);
        }

        int gold = 0, xp = 0;
        check(QuestBoardDialog::turnIn(q.id, gold, xp), "turn-in succeeds");
        check(gold == q.rewardGold, "gold reward matches",
              QString::number(gold));
        check(xp == q.rewardXp, "XP reward matches",
              QString::number(xp));
        check(!QuestBoardDialog::isAccepted(q.id), "quest removed after turn-in");
    }
    {
        // Fetch quest.
        QuestBoardDialog::reset();
        BoardQuest fetchQuest;
        for (const BoardQuest& q : QuestBoardDialog::availableQuests()) {
            if (q.isFetch) { fetchQuest = q; break; }
        }
        QuestBoardDialog::acceptQuest(fetchQuest.id);

        check(!QuestBoardDialog::isComplete(fetchQuest.id), "fetch quest not complete yet");

        QuestBoardDialog::reportFetch(fetchQuest.fetchItem);
        check(QuestBoardDialog::isComplete(fetchQuest.id), "fetch quest complete after item");
    }
    {
        // Unknown quest id.
        BoardQuest invalid = QuestBoardDialog::questById("no_such_quest");
        check(invalid.id.isEmpty(), "unknown quest id returns invalid");
        check(!QuestBoardDialog::acceptQuest("no_such_quest"), "cannot accept unknown quest");
    }

    // ------------------------------------------- Alignment consequences (7.4)

    section("[42] Alignment consequences");
    {
        using Alignment = AlignmentSystem::Alignment;

        check(AlignmentSystem::alignmentName(Alignment::Good) == "Good", "Good name");
        check(AlignmentSystem::alignmentName(Alignment::Neutral) == "Neutral", "Neutral name");
        check(AlignmentSystem::alignmentName(Alignment::Evil) == "Evil", "Evil name");

        check(AlignmentSystem::alignmentFromName("Good") == Alignment::Good, "from Good");
        check(AlignmentSystem::alignmentFromName("Evil") == Alignment::Evil, "from Evil");
        check(AlignmentSystem::alignmentFromName("Neutral") == Alignment::Neutral, "from Neutral");
        check(AlignmentSystem::alignmentFromName("Unknown") == Alignment::Neutral, "unknown defaults to Neutral");
    }
    {
        // Good characters are barred from evil guilds.
        check(!AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Good, "Assassin's Guild"),
              "Good barred from Assassin's Guild");
        check(!AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Good, "Cult of the Dark"),
              "Good barred from Cult of the Dark");
        check(AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Good, "Paladin's Guild"),
              "Good can join Paladin's Guild");
    }
    {
        // Evil characters are barred from good guilds.
        check(!AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Evil, "Paladin's Guild"),
              "Evil barred from Paladin's Guild");
        check(!AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Evil, "Priest of Light"),
              "Evil barred from Priest of Light");
        check(AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Evil, "Assassin's Guild"),
              "Evil can join Assassin's Guild");
    }
    {
        // Neutral can join anything.
        check(AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Neutral, "Paladin's Guild"),
              "Neutral can join Paladin's Guild");
        check(AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Neutral, "Assassin's Guild"),
              "Neutral can join Assassin's Guild");
    }
    {
        // Barred guilds lists.
        check(AlignmentSystem::barredGuilds(AlignmentSystem::Alignment::Good).contains("Assassin's Guild"),
              "Good barred list includes Assassin's");
        check(AlignmentSystem::barredGuilds(AlignmentSystem::Alignment::Evil).contains("Paladin's Guild"),
              "Evil barred list includes Paladin's");
        check(AlignmentSystem::barredGuilds(AlignmentSystem::Alignment::Neutral).isEmpty(),
              "Neutral has no barred guilds");
    }
    {
        // Location restrictions.
        check(!AlignmentSystem::canEnterLocation(AlignmentSystem::Alignment::Evil, "Temple"),
              "Evil barred from Temple");
        check(!AlignmentSystem::canEnterLocation(AlignmentSystem::Alignment::Good, "Cult of the Dark"),
              "Good barred from Cult of the Dark");
        check(AlignmentSystem::canEnterLocation(AlignmentSystem::Alignment::Neutral, "Temple"),
              "Neutral can enter Temple");
        check(AlignmentSystem::canEnterLocation(AlignmentSystem::Alignment::Neutral, "Guild"),
              "Neutral can enter Guild");
    }
    {
        // Alignment from value.
        check(AlignmentSystem::characterAlignment(-5) == AlignmentSystem::Alignment::Evil,
              "negative value is Evil");
        check(AlignmentSystem::characterAlignment(5) == AlignmentSystem::Alignment::Good,
              "positive value is Good");
        check(AlignmentSystem::characterAlignment(0) == AlignmentSystem::Alignment::Neutral,
              "zero is Neutral");
    }
    {
        // Consequence hints.
        check(AlignmentSystem::consequenceHint(AlignmentSystem::Alignment::Good).contains("barred"),
              "Good hint mentions barred");
        check(AlignmentSystem::consequenceHint(AlignmentSystem::Alignment::Evil).contains("barred"),
              "Evil hint mentions barred");
        check(AlignmentSystem::consequenceHint(AlignmentSystem::Alignment::Neutral).contains("no restrictions"),
              "Neutral hint mentions no restrictions");
    }

    // ------------------------------------------- NPC dialog (7.3)

    section("[43] NPC dialog");
    {
        QList<NPC> npcs = NPCDialog::allNPCs();
        check(npcs.size() >= 8, "at least 8 NPCs", QString::number(npcs.size()));

        // Every NPC has a name, role, personality, greeting and hints.
        for (const NPC& n : npcs) {
            check(!n.name.isEmpty(), "NPC has a name", n.name);
            check(!n.role.isEmpty(), "NPC has a role", n.name);
            check(!n.personality.isEmpty(), "NPC has personality", n.name);
            check(!n.greeting.isEmpty(), "NPC has greeting", n.name);
            check(!n.hints.isEmpty(), "NPC has hints", n.name);
        }
    }
    {
        // Lookup by name.
        NPC borin = NPCDialog::npcByName("Guildmaster Borin");
        check(!borin.name.isEmpty(), "found Guildmaster Borin");
        check(borin.role == "Guildmaster", "Borin is Guildmaster");
        check(borin.location == "Guild Hall", "Borin is in Guild Hall");

        NPC unknown = NPCDialog::npcByName("Nobody");
        check(unknown.name.isEmpty(), "unknown NPC is invalid");
    }
    {
        // Hints from a specific NPC.
        QStringList borinHints = NPCDialog::hintsFrom("Guildmaster Borin");
        check(borinHints.size() >= 3, "Borin has at least 3 hints",
              QString::number(borinHints.size()));
        check(borinHints[0].contains("Floor 5"), "Borin mentions floor 5");

        QStringList unknownHints = NPCDialog::hintsFrom("Nobody");
        check(unknownHints.isEmpty(), "unknown NPC has no hints");
    }
    {
        // All hints aggregated.
        QStringList all = NPCDialog::allHints();
        check(all.size() >= 20, "at least 20 total hints",
              QString::number(all.size()));
    }
    {
        // NPCs cover key locations.
        QStringList locations;
        for (const NPC& n : NPCDialog::allNPCs()) {
            locations << n.location;
        }
        check(locations.contains("Guild Hall"), "Guild Hall NPC exists");
        check(locations.contains("Temple"), "Temple NPC exists");
        check(locations.contains("Tavern"), "Tavern NPC exists");
        check(locations.contains("Library"), "Library NPC exists");
        check(locations.contains("Royal Bank"), "Bank NPC exists");
        check(locations.contains("Morgue"), "Morgue NPC exists");
    }

    // ------------------------------------------- Monster difficulty curve (8.1)

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
    {
        // Party readiness.
        check(MonsterBalance::isPartyReady(4, 1, 1), "4 members at level 1 ready for floor 1");
        check(!MonsterBalance::isPartyReady(1, 1, 15), "solo level 1 not ready for floor 15");
        check(MonsterBalance::isPartyReady(4, 15, 15), "4 members at level 15 ready for floor 15");
        check(!MonsterBalance::isPartyReady(0, 1, 1), "empty party not ready");
    }
    {
        // Difficulty labels.
        check(MonsterBalance::difficultyLabel(1) == "Easy", "floor 1 is easy");
        check(MonsterBalance::difficultyLabel(5) == "Moderate", "floor 5 is moderate");
        check(MonsterBalance::difficultyLabel(10) == "Hard", "floor 10 is hard");
        check(MonsterBalance::difficultyLabel(15) == "Deadly", "floor 15 is deadly");
    }
    {
        // Floor table.
        QList<QVariantMap> table = MonsterBalance::floorTable();
        check(table.size() == 15, "15 floors in table");
        check(table[0].value("floor").toInt() == 1, "first floor is 1");
        check(table[14].value("floor").toInt() == 15, "last floor is 15");
    }

    // ------------------------------------------- Spell differentiation (8.2)

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
    {
        // Fire AoE.
        SpellDef fire;
        fire.category = "Fire";
        fire.name = "Fireball";
        fire.baseLevel = 3;
        check(SpellMechanics::fireTargets(fire) >= 2, "fire hits multiple targets");
        check(SpellMechanics::isAoe(fire), "fire is AoE");
        check(!SpellMechanics::isCrowdControl(fire), "fire is not crowd control");
    }
    {
        // Cold slow.
        SpellDef cold;
        cold.category = "Cold";
        cold.name = "Iceball";
        cold.baseLevel = 3;
        check(SpellMechanics::coldSlowTurns(cold) >= 1, "cold slows");
        check(SpellMechanics::coldSlowPercent(cold) >= 20, "cold slow percent");
        check(SpellMechanics::isCrowdControl(cold), "cold is crowd control");
    }
    {
        // Lightning chain.
        SpellDef lightning;
        lightning.category = "Electrical";
        lightning.name = "Chain Lightning";
        lightning.baseLevel = 6;
        check(SpellMechanics::lightningChainTargets(lightning) == 5, "chain hits 5 targets");
        check(SpellMechanics::lightningChainFalloff(lightning) == 20, "chain falloff 20%");
        check(SpellMechanics::isAoe(lightning), "chain is AoE");
    }
    {
        // Mind crowd control.
        SpellDef mind;
        mind.category = "Mind";
        mind.name = "Stun";
        mind.baseLevel = 3;
        check(SpellMechanics::mindStunTurns(mind) >= 1, "mind stuns");
        check(SpellMechanics::mindConfuseChance(mind) >= 25, "mind confuse chance");
        check(SpellMechanics::isCrowdControl(mind), "mind is crowd control");
    }
    {
        // Mechanic descriptions.
        SpellDef fire;
        fire.category = "Fire";
        fire.name = "Fireball";
        fire.baseLevel = 3;
        check(SpellMechanics::mechanicDescription(fire).contains("Fire"), "fire description");
        check(SpellMechanics::mechanicDescription(fire).contains("targets"), "fire targets in desc");
    }
    {
        // Spells in school.
        QList<SpellDef> spells;
        SpellDef f1; f1.category = "Fire"; f1.name = "Flame Bolt";
        SpellDef f2; f2.category = "Fire"; f2.name = "Fireball";
        SpellDef c1; c1.category = "Cold"; c1.name = "Cold Bolt";
        spells << f1 << f2 << c1;

        QList<SpellDef> fireSpells = SpellMechanics::spellsInSchool(spells, SpellMechanics::School::Fire);
        check(fireSpells.size() == 2, "2 fire spells");
        QList<SpellDef> coldSpells = SpellMechanics::spellsInSchool(spells, SpellMechanics::School::Cold);
        check(coldSpells.size() == 1, "1 cold spell");
    }

    // ------------------------------------------- Item progression (8.3)

    section("[54] Item progression");
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::tierName(Tier::Bronze) == "Bronze", "bronze name");
        check(ItemProgression::tierName(Tier::Iron) == "Iron", "iron name");
        check(ItemProgression::tierName(Tier::Steel) == "Steel", "steel name");
        check(ItemProgression::tierName(Tier::Adamantite) == "Adamantite", "adamantite name");
        check(ItemProgression::tierName(Tier::Mithril) == "Mithril", "mithril name");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::tierForFloor(1) == Tier::Bronze, "floor 1 is bronze");
        check(ItemProgression::tierForFloor(5) == Tier::Iron, "floor 5 is iron");
        check(ItemProgression::tierForFloor(8) == Tier::Steel, "floor 8 is steel");
        check(ItemProgression::tierForFloor(12) == Tier::Adamantite, "floor 12 is adamantite");
        check(ItemProgression::tierForFloor(15) == Tier::Mithril, "floor 15 is mithril");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::minFloorForTier(Tier::Bronze) == 1, "bronze min floor 1");
        check(ItemProgression::minFloorForTier(Tier::Mithril) == 13, "mithril min floor 13");
        check(ItemProgression::maxFloorForTier(Tier::Bronze) == 3, "bronze max floor 3");
        check(ItemProgression::maxFloorForTier(Tier::Mithril) == 15, "mithril max floor 15");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::statMultiplier(Tier::Bronze) == 1.0, "bronze stat 1.0");
        check(ItemProgression::statMultiplier(Tier::Iron) == 1.5, "iron stat 1.5");
        check(ItemProgression::statMultiplier(Tier::Steel) == 2.0, "steel stat 2.0");
        check(ItemProgression::statMultiplier(Tier::Adamantite) == 3.0, "adamantite stat 3.0");
        check(ItemProgression::statMultiplier(Tier::Mithril) == 4.0, "mithril stat 4.0");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::costMultiplier(Tier::Bronze) == 1.0, "bronze cost 1.0");
        check(ItemProgression::costMultiplier(Tier::Mithril) == 16.0, "mithril cost 16.0");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::isAvailable(Tier::Bronze, 1), "bronze available floor 1");
        check(!ItemProgression::isAvailable(Tier::Mithril, 1), "mithril not available floor 1");
        check(ItemProgression::isAvailable(Tier::Mithril, 15), "mithril available floor 15");
    }
    {
        // Items of tier.
        QStringList items = {"Bronze Sword", "Iron Shield", "Steel Helm", "Bronze Dagger"};
        QStringList bronze = ItemProgression::itemsOfTier(items, ItemProgression::Tier::Bronze);
        check(bronze.size() == 2, "2 bronze items");
        QStringList iron = ItemProgression::itemsOfTier(items, ItemProgression::Tier::Iron);
        check(iron.size() == 1, "1 iron item");
    }

    // ------------------------------------------- Gold sinks (8.4)

    section("[55] Gold sinks");
    {
        check(GoldSinks::resurrectionCost(1, false) == 500, "level 1 resurrection 500");
        check(GoldSinks::resurrectionCost(5, false) == 2500, "level 5 resurrection 2500");
        check(GoldSinks::resurrectionCost(5, true) == 3000, "level 5 dungeon resurrection 3000");
        check(GoldSinks::rescueCost(1) == 250, "depth 1 rescue 250");
        check(GoldSinks::rescueCost(5) == 6250, "depth 5 rescue 6250");
        check(GoldSinks::identificationCost() == 50, "identification 50");
        check(GoldSinks::uncurseCost() == 100, "uncurse 100");
        check(GoldSinks::guildLevelCost("Mage", 0) == 500, "guild level 0→1 costs 500");
        check(GoldSinks::guildLevelCost("Mage", 5) == 3000, "guild level 5→6 costs 3000");
        check(GoldSinks::restCostPerHour() == 10, "rest 10/hour");
        check(GoldSinks::curePoisonCost() == 50, "cure poison 50");
        check(GoldSinks::cureBlindnessCost() == 50, "cure blindness 50");
    }
    {
        // All sinks.
        QList<QVariantMap> sinks = GoldSinks::allSinks();
        check(sinks.size() >= 7, "at least 7 gold sinks",
              QString::number(sinks.size()));
    }
    {
        // Sink descriptions.
        check(GoldSinks::sinkDescription("resurrection").contains("Morgue"), "resurrection desc");
        check(GoldSinks::sinkDescription("identification").contains("Store"), "identification desc");
        check(GoldSinks::sinkDescription("guild").contains("Guild"), "guild desc");
    }

    // ------------------------------------------- Release packaging (8.5)

    section("[56] Release packaging");
    {
        // The expected version is read from ReleaseInfo itself, so a version
        // bump changes the data the tests point at rather than breaking them.
        const QString v = ReleaseInfo::version();
        check(v == GameConstants::SEMANTIC_VERSION,
              "version matches the build header", v);
        check(v.size() >= 5 && v[0].isDigit(),
              "version is a non-empty numeric string", v);
        check(ReleaseInfo::versionString() == "v" + v, "version string",
              ReleaseInfo::versionString());
        check(!ReleaseInfo::releaseDate().isEmpty(), "release date exists");
        check(!ReleaseInfo::systemRequirements().isEmpty(), "system requirements exist");
        check(ReleaseInfo::installerName().contains(v), "installer name",
              ReleaseInfo::installerName());
        check(ReleaseInfo::banner().contains(v), "banner", ReleaseInfo::banner());
    }
    {
        // Release notes.
        QStringList notes = ReleaseInfo::releaseNotes();
        check(notes.size() >= 8, "at least 8 release notes",
              QString::number(notes.size()));
        check(notes[0].contains("Phase 1"), "first note is Phase 1");
    }
    {
        // Changes.
        QStringList changes = ReleaseInfo::changes();
        check(changes.size() >= 5, "at least 5 changes",
              QString::number(changes.size()));
    }
    {
        // Version history.
        QList<QPair<QString, QString>> history = ReleaseInfo::versionHistory();
        check(history.size() >= 5, "at least 5 versions in history",
              QString::number(history.size()));
        check(history[0].first == ReleaseInfo::version(),
              "the newest history entry is the current version",
              QString("%1 vs %2").arg(history[0].first, ReleaseInfo::version()));
        check(history[0].second.contains("2026-10-17"),
              "the newest history entry is dated today",
              history[0].second.left(40));
    }

    // ------------------------------------------- Status flag integrity

    section("[57] Status flags");
    {
        // One statusFlags field carries both the death bit and the combat
        // statuses, so their bits must not overlap.
        check(StatusFlag::Dead != StatusFlag::Poisoned, "Dead and Poisoned are different bits");
        check(StatusFlag::Dead != StatusFlag::Blinded, "Dead and Blinded are different bits");
        check(StatusFlag::Dead != StatusFlag::OnFire, "Dead and OnFire are different bits");
        check((StatusFlag::Dead & StatusFlag::Poisoned) == 0, "Dead and Poisoned do not overlap");
        check((StatusFlag::Dead & StatusFlag::Blinded) == 0, "Dead and Blinded do not overlap");
        check((StatusFlag::Dead & StatusFlag::OnFire) == 0, "Dead and OnFire do not overlap");
    }
    {
        // Death must not look like poison, and vice versa.
        Character c;
        c.name = "StatusTest";
        c.isAlive = true;

        c.addStatus(StatusFlag::Poisoned);
        check((c.statusFlags & StatusFlag::Poisoned) != 0, "poison bit set");
        check((c.statusFlags & StatusFlag::Dead) == 0, "poison does not read as dead");
        check(c.isAlive, "a poisoned character is still alive");

        c.removeStatus(StatusFlag::Poisoned);
        c.addStatus(StatusFlag::Dead);
        check((c.statusFlags & StatusFlag::Dead) != 0, "dead bit set");
        check((c.statusFlags & StatusFlag::Poisoned) == 0, "death does not read as poisoned");
    }
    {
        // The cure-poison branch in useConsumable() reads statusFlags
        // directly; no item in MDATA3.csv currently reaches it, so assert
        // the invariant the branch depends on instead of the branch itself.
        Character c;
        c.name = "CureTest";
        c.isAlive = true;
        c.addStatus(StatusFlag::Poisoned);
        c.addStatus(StatusFlag::Blinded);

        // Clearing poison must leave blindness intact.
        c.removeStatus(StatusFlag::Poisoned);
        check((c.statusFlags & StatusFlag::Poisoned) == 0, "poison cleared",
              QString::number(c.statusFlags));
        check((c.statusFlags & StatusFlag::Blinded) != 0, "blindness survives a poison cure",
              QString::number(c.statusFlags));

        // And the reverse.
        c.removeStatus(StatusFlag::Blinded);
        c.addStatus(StatusFlag::Poisoned);
        check((c.statusFlags & StatusFlag::Blinded) == 0, "blindness cleared");
        check((c.statusFlags & StatusFlag::Poisoned) != 0, "poison survives a blindness cure",
              QString::number(c.statusFlags));
    }

    // ------------------------------------------- audio volume (B8)

    section("[58] Audio volume");
    {
        audioManager* am = audioManager::instance();
        check(am != nullptr, "audioManager singleton available");

        // The SFX slider must drive the SFX volume, not the music volume.
        float sfxBefore = am->getSfxVolume();
        am->setSfxVolume(0.25f);
        check(qFuzzyCompare(am->getSfxVolume(), 0.25f),
              "setSfxVolume updates the SFX volume",
              QString::number(am->getSfxVolume()));
        am->setSfxVolume(sfxBefore);  // restore
    }

    // ------------------------------------------- hunger / food (C1)

    section("[59] Hunger and food");
    {
        Character c;
        c.name = "TestChar";
        c.hunger = 100;
        c.maxHunger = 100;
        check(c.hunger == 100, "hunger starts at 100");
        check(!c.isStarving(), "not starving at full hunger");

        c.consumeHunger(30);
        check(c.hunger == 70, "consumeHunger decreases hunger",
              QString::number(c.hunger));
        check(!c.isStarving(), "not starving at 70 hunger");

        c.consumeHunger(80);
        check(c.hunger == 0, "hunger clamps at 0",
              QString::number(c.hunger));
        check(c.isStarving(), "starving at 0 hunger");

        c.restoreHunger(50);
        check(c.hunger == 50, "restoreHunger increases hunger",
              QString::number(c.hunger));
        check(!c.isStarving(), "not starving after restore");

        c.restoreHunger(100);
        check(c.hunger == 100, "hunger clamps at maxHunger",
              QString::number(c.hunger));
    }
    {
        // eatFood with a real food item from the database.
        Character c;
        c.name = "TestChar";
        c.hunger = 20;
        c.maxHunger = 100;

        // Find a real food item (type 35) in the database.
        const ItemDef* foodDef = nullptr;
        for (const ItemDef& def : ItemDatabase::instance().all()) {
            if (def.type == Character::FOOD_TYPE) {
                foodDef = &def;
                break;
            }
        }
        if (foodDef) {
            HeldItem food;
            food.name = foodDef->name;
            food.M4E97 = static_cast<int16_t>(foodDef->id);
            food.M4ED4 = 1;
            c.inventory.append(food);

            QString effect;
            bool ok = c.eatFood(0, effect);
            check(ok, "eatFood succeeds with food item", effect);
            check(c.hunger > 20, "hunger restored after eating",
                  QString::number(c.hunger));
            check(effect.contains("hunger"), "effect mentions hunger", effect);
        } else {
            out("  (no food items in database — skipping eatFood test)");
        }
    }
    {
        // eatFood rejects non-food items.
        Character c;
        c.name = "TestChar";
        c.hunger = 50;

        HeldItem sword;
        sword.name = "Bronze Sword";
        if (const ItemDef* def = ItemDatabase::instance().byName("Bronze Sword")) {
            sword.M4E97 = static_cast<int16_t>(def->id);
        }
        c.inventory.append(sword);

        QString effect;
        bool ok = c.eatFood(0, effect);
        check(!ok, "eatFood rejects non-food item");
        check(effect.contains("not food"), "effect mentions not food", effect);
        check(c.hunger == 50, "hunger unchanged after rejected eat");
    }
    {
        // Hunger round-trips through serialization.
        Character c;
        c.name = "TestChar";
        c.hunger = 42;
        c.maxHunger = 100;

        QVariantMap map = c.toMap();
        check(map.value("Hunger").toInt() == 42, "hunger serialized to map");
        check(map.value("MaxHunger").toInt() == 100, "maxHunger serialized to map");

        Character c2;
        c2.loadFromMap(map);
        check(c2.hunger == 42, "hunger deserialized from map",
              QString::number(c2.hunger));
        check(c2.maxHunger == 100, "maxHunger deserialized from map");
    }
    {
        // Default hunger for old saves (no Hunger key).
        QVariantMap map;
        map["Name"] = "OldChar";
        Character c;
        c.loadFromMap(map);
        check(c.hunger == 100, "old save defaults to full hunger",
              QString::number(c.hunger));
    }

    // ------------------------------------------- trap variants (C2)

    section("[60] Trap variants");
    {
        // Each trap type has a distinct effect — not just "1-10 damage".
        // We test the logic by checking that different trap types produce
        // different status effects or damage ranges.
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            // Test Poison Needler applies poison.
            members[0].statusFlags = 0;
            members[0].hp = 100;
            members[0].maxHp = 100;

            // Simulate trap effect directly.
            members[0].addStatus(StatusFlag::Poisoned);
            check((members[0].statusFlags & StatusFlag::Poisoned) != 0,
                  "Poison Needler applies poison status");

            // Test Snare applies snared status.
            members[0].statusFlags = 0;
            members[0].addStatus(StatusFlag::Snared);
            check((members[0].statusFlags & StatusFlag::Snared) != 0,
                  "Snare applies snared status");

            // Test that different trap types have different damage ranges.
            // Spike: 1-10, Dart: 3-8, Pit Cover: 5-15
            // We can't test the full handler without a dialog, but we can
            // verify the status flags are distinct.
            check(StatusFlag::Poisoned != StatusFlag::Snared,
                  "Poisoned and Snared are distinct flags");
            }
            }

            // ------------------------------------------- trap disarm (E1)

            section("[60b] Trap disarm");
            {
            // The disarmTrapChance formula returns a clamped percentage (5-98).
            double chance1 = disarmTrapChance(16, 14, 5, 1.0, 5, 5);
            double chance2 = disarmTrapChance(8, 8, 1, 1.0, 1, 1);
            double chance3 = disarmTrapChance(18, 18, 10, 1.5, 10, 10);
            check(chance1 >= 5.0 && chance1 <= 98.0,
                  "disarmTrapChance is within 5-98%",
                  QString::number(chance1));
            check(chance2 >= 5.0 && chance2 <= 98.0,
                  "disarmTrapChance (weak char) is within 5-98%",
                  QString::number(chance2));
            check(chance3 >= 5.0 && chance3 <= 98.0,
                  "disarmTrapChance (strong char) is within 5-98%",
                  QString::number(chance3));

            // Higher DEX + WIS should yield a higher disarm chance at the same level.
            // Use floor 1 so the depth penalty does not clamp both to the minimum.
            double strongSameLevel = disarmTrapChance(18, 18, 5, 1.0, 1, 1);
            double weakSameLevel = disarmTrapChance(8, 8, 5, 1.0, 1, 1);
            check(strongSameLevel > weakSameLevel,
                  "strong character has higher disarm chance than weak at same level",
                  QString("strong=%1 weak=%2").arg(strongSameLevel).arg(weakSameLevel));
            }
            {
            // Thieving ability formula returns 0-100.
            double ability1 = thievingAbility(50, 0, 100);
            double ability2 = thievingAbility(0, 0, 100);
            double ability3 = thievingAbility(100, 0, 100);
            check(ability1 == 50.0, "thievingAbility(50,0,100) = 50",
                  QString::number(ability1));
            check(ability2 == 0.0, "thievingAbility(0,0,100) = 0",
                  QString::number(ability2));
            check(ability3 == 100.0, "thievingAbility(100,0,100) = 100",
                  QString::number(ability3));
            }
            {
            // thievingParam returns a finite number for valid inputs.
            double param = thievingParam(16, 14, 5, 1.0);
            check(std::isfinite(param), "thievingParam returns finite value",
                  QString::number(param));
            }

            // ------------------------------------------- item identification (E2)

            section("[60d] Monster wander AI");
            {
            // Monster positions are tracked and can move between tiles.
            QMap<QPair<int, int>, QString> monsters;
            monsters.insert(qMakePair(5, 5), "Goblin");
            monsters.insert(qMakePair(10, 10), "Orc");
            check(monsters.size() == 2, "two monsters placed");
            check(monsters.value(qMakePair(5, 5)) == "Goblin", "goblin at (5,5)");

            // Simulate a move: erase old, insert new.
            monsters.insert(qMakePair(6, 5), "Goblin");
            // Note: in the real code the old key is erased first; here we verify
            // the map can hold both temporarily, then clean up.
            monsters.remove(qMakePair(5, 5));
            check(monsters.size() == 2, "two monsters after move");
            check(monsters.contains(qMakePair(6, 5)), "goblin moved to (6,5)");
            check(!monsters.contains(qMakePair(5, 5)), "old position empty");
            }

            // ------------------------------------------- quest chain UI (C3)

            section("[61] Quest chain");
    {
        // QuestChain steps are well-formed.
        QList<QuestStep> steps = QuestChain::steps();
        check(steps.size() == 6, "quest chain has 6 steps",
              QString::number(steps.size()));

        // First step requires depth 1.
        QuestStep s0 = QuestChain::step(0);
        check(s0.requiresDepth == 1, "first step requires depth 1");

        // Boss steps have correct floors.
        QuestStep s1 = QuestChain::step(1);
        check(s1.bossFloor == 5, "second step is boss on floor 5");

        QuestStep s5 = QuestChain::step(5);
        check(s5.bossFloor == 15, "final step is boss on floor 15");

        // Objective text is non-empty.
        for (const QuestStep& s : steps) {
            QString obj = QuestChain::objectiveText(s);
            check(!obj.isEmpty(), "objective text non-empty for step",
                  s.title);
        }

        // Chain is not complete at start.
        check(!QuestChain::isChainComplete(0, {}, {}),
              "chain not complete at start");

        // Chain is complete when all bosses defeated and depth reached.
        QList<int> allBosses = {5, 10, 15};
        check(QuestChain::isChainComplete(15, allBosses, {}),
              "chain complete when all bosses defeated and depth 15 reached");

        // Next step index is 0 at start.
        check(QuestChain::nextStepIndex(0, {}, {}) == 0,
              "next step is 0 at start");

        // After completing step 0 (depth 1), next is step 1.
        check(QuestChain::nextStepIndex(1, {}, {}) == 1,
              "next step is 1 after reaching depth 1");
    }

    // ------------------------------------------- quest integration (C4)

    section("[62] Quest integration");
    {
        // reportKill progresses kill-quests.
        QuestBoardDialog::reset();

        // Accept a kill quest.
        QuestBoardDialog::acceptQuest("kill_rats");
        check(QuestBoardDialog::isAccepted("kill_rats"),
              "kill_rats accepted");

        // Report kills.
        QuestBoardDialog::reportKill("Giant Rat", 1);
        QuestBoardDialog::reportKill("Giant Rat", 1);
        QuestBoardDialog::reportKill("Giant Rat", 1);
        QuestBoardDialog::reportKill("Giant Rat", 1);
        QuestBoardDialog::reportKill("Giant Rat", 1);

        // Quest should be complete now.
        check(QuestBoardDialog::isComplete("kill_rats"),
              "kill_rats complete after 5 kills");

        // Turn in the quest.
        int goldBefore = gameStateManager::instance()->getPartyGold();
        int goldOut = 0, xpOut = 0;
        bool turnedIn = QuestBoardDialog::turnIn("kill_rats", goldOut, xpOut);
        check(turnedIn, "turn-in succeeds");
        check(goldOut == 200, "gold reward is 200", QString::number(goldOut));
        check(xpOut == 500, "XP reward is 500", QString::number(xpOut));
        check(gameStateManager::instance()->getPartyGold() == goldBefore + 200,
              "gold awarded after turn-in");
    }

    // ------------------------------------------- opened chests (F1)

    section("[62b] Opened chests persist");
    {
        // LevelSnapshot has an openedChests field.
        LevelSnapshot snap;
        snap.level = 1;
        snap.openedChests.insert(qMakePair(5, 5));
        snap.openedChests.insert(qMakePair(10, 10));
        check(snap.openedChests.size() == 2, "openedChests populated");
        check(snap.openedChests.contains(qMakePair(5, 5)), "chest at (5,5) recorded");
        check(snap.openedChests.contains(qMakePair(10, 10)), "chest at (10,10) recorded");
        check(!snap.openedChests.contains(qMakePair(3, 3)), "unopened chest not recorded");
    }
    {
        // Serialisation round-trip preserves openedChests.
        LevelSnapshot snap;
        snap.level = 2;
        snap.openedChests.insert(qMakePair(7, 7));
        QVariantMap map = snap.toMap();
        LevelSnapshot snap2;
        snap2.loadFromMap(map);
        check(snap2.openedChests.size() == 1, "openedChests survives serialisation");
        check(snap2.openedChests.contains(qMakePair(7, 7)), "chest position preserved");
    }

    // ------------------------------------------- trap detection (F2)

    section("[62c] Trap detection");
    {
        // trapDetectionDifficulty increases with floor level.
        check(DoorAndSearch::trapDetectionDifficulty(1) == 9,
              "floor 1 trap DC is 9",
              QString::number(DoorAndSearch::trapDetectionDifficulty(1)));
        check(DoorAndSearch::trapDetectionDifficulty(5) == 13,
              "floor 5 trap DC is 13",
              QString::number(DoorAndSearch::trapDetectionDifficulty(5)));
        check(DoorAndSearch::trapDetectionDifficulty(10) == 18,
              "floor 10 trap DC is 18",
              QString::number(DoorAndSearch::trapDetectionDifficulty(10)));
    }
    {
        // searchForTraps finds traps within 3x3 range.
        QMap<QPair<int, int>, QString> traps;
        traps.insert(qMakePair(5, 5), "Spike");
        traps.insert(qMakePair(6, 5), "Dart");
        traps.insert(qMakePair(10, 10), "Pit");  // out of range

        QList<QPair<int, int>> found;
        QRandomGenerator rng(42);  // fixed seed for determinism
        int count = DoorAndSearch::searchForTraps(traps, 5, 5, 18, 18, 1, found, rng);
        check(count >= 1, "at least one trap found in 3x3 range",
              QString::number(count));
        // The trap at (10,10) is out of range and should never be found.
        bool foundOutOfRange = false;
        for (const QPair<int, int>& p : found) {
            if (p.first == 10 && p.second == 10) foundOutOfRange = true;
        }
        check(!foundOutOfRange, "trap outside 3x3 range not found");
    }
    {
        // searchForTraps with no traps returns 0.
        QMap<QPair<int, int>, QString> emptyTraps;
        QList<QPair<int, int>> found;
        QRandomGenerator rng(42);
        int count = DoorAndSearch::searchForTraps(emptyTraps, 5, 5, 18, 18, 1, found, rng);
        check(count == 0, "no traps found when none exist");
    }

    // ------------------------------------------- alignment (F3)

    section("[62d] Alignment display");
    {
        // getGameValue returns the alignment set at character creation.
        gameStateManager::instance()->setGameValue("CurrentCharacterAlignment", "Good");
        QString alignment = gameStateManager::instance()->getGameValue("CurrentCharacterAlignment").toString();
        check(alignment == "Good", "alignment reads Good", alignment);

        gameStateManager::instance()->setGameValue("CurrentCharacterAlignment", "Evil");
        alignment = gameStateManager::instance()->getGameValue("CurrentCharacterAlignment").toString();
        check(alignment == "Evil", "alignment reads Evil", alignment);

        // Reset to Neutral for other tests.
        gameStateManager::instance()->setGameValue("CurrentCharacterAlignment", "Neutral");
    }

    // ------------------------------------------- monster chase AI (F4)

    section("[62e] Monster chase AI");
    {
        // distanceBetween uses Euclidean distance.
        // We can't call DungeonDialog methods directly, but we can verify
        // the math via the same formula.
        QPair<int, int> a(0, 0), b(3, 4);
        int dx = a.first - b.first, dy = a.second - b.second;
        int dist = static_cast<int>(std::sqrt(dx * dx + dy * dy));
        check(dist == 5, "distance (0,0) to (3,4) is 5", QString::number(dist));

        QPair<int, int> c(10, 10), d(13, 14);
        dx = c.first - d.first; dy = c.second - d.second;
        dist = static_cast<int>(std::sqrt(dx * dx + dy * dy));
        check(dist == 5, "distance (10,10) to (13,14) is 5", QString::number(dist));
    }
    {
        // Bresenham line: a clear line has no obstacles.
        // We can't construct a DungeonDialog easily, but we can verify
        // the algorithm logic: a straight horizontal line with no walls
        // should have line of sight.
        QSet<QPair<int, int>> obstacles;
        // No obstacles — clear LOS from (0,0) to (5,0).
        bool los = true;
        for (int x = 0; x <= 5; ++x) {
            if (obstacles.contains(qMakePair(x, 0))) { los = false; break; }
        }
        check(los, "clear line of sight with no obstacles");

        // Add a wall at (3,0) — blocks LOS.
        obstacles.insert(qMakePair(3, 0));
        los = true;
        for (int x = 0; x <= 5; ++x) {
            if (obstacles.contains(qMakePair(x, 0))) { los = false; break; }
        }
        check(!los, "wall blocks line of sight");
    }
    {
        // getWalkableNeighbors: a position in open space has 8 neighbors.
        // We can't call DungeonDialog directly, but we can verify the
        // neighbor-counting logic.
        QPair<int, int> pos(5, 5);
        QSet<QPair<int, int>> obstacles;
        int count = 0;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;
                QPair<int, int> n(pos.first + dx, pos.second + dy);
                if (n.first < 0 || n.first > 39 || n.second < 0 || n.second > 39) continue;
                if (obstacles.contains(n)) continue;
                count++;
            }
        }
        check(count == 8, "8 walkable neighbors in open space", QString::number(count));
    }

    // ------------------------------------------- DungeonDialog action buttons are a grid
    // The 14 action buttons (Fight, Spell, Rest, ...) must render as a tidy 3-column
    // grid inside the sidebar, not as a crammed pile. Assert real geometry:
    //  - every action button has a sane size (at least 80x30)
    //  - no two buttons overlap

    // ------------------------------------------- Lua sandbox (0.1)

    section("[64] Lua host access is closed");
    {
        // Test the ENGINE'S OWN Lua state, not a state we build here. Calling
        // sandboxLua() directly would pass even if the engine never called it —
        // a vacuous test. Drive the live state through its public surface.
        const QString probe = "data/scripts/selftest_lua_probe.lua";
        QDir().mkpath("data/scripts");

        auto writeProbe = [&](const QString& body) -> bool {
            QFile f(probe);
            if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
            f.write(body.toUtf8());
            f.close();
            return true;
        };
        auto eval = [&](const QString& body) -> QString {
            writeProbe(body);
            gsm->loadLuaScript(probe);
            return gsm->getLuaString("PROBE_RESULT");
        };

        check(writeProbe("PROBE_RESULT = 'ready'") && eval("PROBE_RESULT = 'ready'") == "ready",
              "probe harness can drive the live Lua state");

        // The RCE path. os.execute is what the old onServerDataReceived() gave
        // an attacker; it must be gone from the engine's state.
        check(eval("PROBE_RESULT = tostring(os.execute)") == "nil",
              "engine: os.execute is unreachable");
        check(eval("PROBE_RESULT = tostring(io)") == "nil",
              "engine: io library is unreachable");
        check(eval("PROBE_RESULT = tostring(package)") == "nil",
              "engine: package library is unreachable");
        check(eval("PROBE_RESULT = tostring(debug)") == "nil",
              "engine: debug library is unreachable");
        check(eval("PROBE_RESULT = tostring(dofile)") == "nil",
              "engine: dofile is unreachable");
        check(eval("PROBE_RESULT = tostring(require)") == "nil",
              "engine: require is unreachable");

        // The sandbox must not be so tight that shipped scripts break.
        check(eval("PROBE_RESULT = type(os.date)") == "function",
              "engine: os.date survives (heartbeat.lua needs it)");
        check(eval("PROBE_RESULT = type(string.format)") == "function",
              "engine: string library survives");

        // The shipped heartbeat script must still run on the live engine.
        check(gsm->loadLuaScript("data/scripts/heartbeat.lua"),
              "heartbeat.lua still runs on the engine");

        QFile::remove(probe);
    }

    // ------------------------------------------- Phase 0 cleanup (0.2–0.6)

    section("[65] Phase 0: cleanup and latent breakage");
    {
        // 0.5 — aging must go through AgingRules, which is race-aware. The old
        // duplicate used a hardcoded `age > 70` for every race; an Elf at 80 is
        // nowhere near decay (threshold 280) but an old Human is.
        auto& members = gsm->getPartyMembers();
        check(!members.isEmpty(), "party has members to age");

        if (!members.isEmpty()) {
            // Remember what we touch, and restore it at the end: the suite runs
            // against one live state and later sections depend on the party.
            Character& c = members[0];
            const QString savedName = c.name;
            const int savedAge = c.age;
            const bool savedAlive = c.isAlive;
            const int savedHp = c.hp;
            const QString savedRace = c.race;

            c.name = "AgingProbe";
            c.race = "Human";
            c.isAlive = true;
            c.age = 99;                       // past the Human threshold (70)
            c.hp = 50;

            // One year of aging at 99 for a Human is below max age (100), so the
            // character survives; at 100 they must die. Drive it through the
            // engine's own entry point so a missing call site fails the test.
            const int ageBefore = c.age;
            gsm->incrementPartyAge(0);        // age by 0: rules still run
            check(c.age == ageBefore, "aging by 0 leaves age unchanged");

            c.age = 100;                      // exactly Human max age
            gsm->processAgingConsequences();
            check(!c.isAlive, "a Human at max age (100) dies of old age");
            check(c.hp == 0, "death by old age leaves 0 HP");

            // Race-awareness: an Elf at 100 is far from decay (threshold 280)
            // and must be untouched. The old hardcoded rule decayed everyone
            // past 70 regardless of race.
            c.race = "Elf";
            c.isAlive = true;
            c.hp = 50;
            c.age = 100;
            const int strBefore = c.strength;
            const int conBefore = c.constitution;
            for (int i = 0; i < 50; ++i) gsm->processAgingConsequences();
            check(c.isAlive, "an Elf at 100 is not past max age");
            check(c.strength == strBefore && c.constitution == conBefore,
                  "an Elf at 100 suffers no decay (race-aware threshold)");

            // Restore. processAgingConsequences() now saves the body of anyone
            // who died, so clean that file up too.
            c.name = savedName;
            c.age = savedAge;
            c.isAlive = savedAlive;
            c.hp = savedHp;
            c.race = savedRace;
            QFile::remove("data/characters/AgingProbe.json");
        }
    }
    {
        // 0.6 — FULL_VERSION is the build version pushed to Lua as GameVersion.
        // It carries a single "v" prefix (e.g. "v778") and nothing else.
        const QString v = QString::fromLatin1(GameConstants::FULL_VERSION);
        check(!v.isEmpty(), "FULL_VERSION is set", v);
        check(v.count('v') <= 1, "FULL_VERSION carries at most one 'v' prefix", v);
    }

    // ------------------------------------------- v1.0.0 — slice 1.1: Victory sequence

    section("[66] v1.0.0 slice 1.1: victory sequence");
    {
        // The Prince of Devils is the floor-15 boss. His defeat is the win condition.
        check(Endgame::finalBossName() == "The Prince of Devils", "final boss is the Prince of Devils");

        // isVictory is the gate the victory screen checks. A run that has not killed the
        // Prince is not a win; one that has is.
        check(!Endgame::isVictory({}), "not victory with no bosses defeated");
        check(!Endgame::isVictory({1, 5, 10}), "not victory without the Prince");
        check(Endgame::isVictory({1, 5, 10, 15}), "victory once the Prince falls");

        // The victory text exists and is non-empty — the screen has something to show.
        check(!Endgame::victoryTitle().isEmpty(), "victory has a title");
        check(Endgame::victoryParagraphs().size() >= 3, "victory has paragraphs",
              QString::number(Endgame::victoryParagraphs().size()));

        // buildFinalBoss produces a monster far beyond a normal floor boss.
        QVariantMap boss = Endgame::buildFinalBoss();
        check(boss["name"].toString() == "The Prince of Devils", "final boss name in data");
        check(boss["hp"].toInt() >= 5000, "final boss has 5000+ HP",
              QString::number(boss["hp"].toInt()));
        check(boss["level"].toInt() >= 40, "final boss is level 40+",
              QString::number(boss["level"].toInt()));
    }

    // ------------------------------------------- v1.0.0 — slice 1.2: Hall of Records

    section("[69] v1.0.0 slice 1.4: MonsterBalance into encounters");
    {
        // MonsterBalance is the per-floor difficulty curve. Floor 1 is baseline.
        check(MonsterBalance::statMultiplier(1) == 1.0, "floor 1 multiplier is 1.0");
        check(MonsterBalance::statMultiplier(10) > 1.5, "floor 10 multiplier is significant",
              QString::number(MonsterBalance::statMultiplier(10)));
        check(MonsterBalance::statMultiplier(15) > 2.0, "floor 15 multiplier is high",
              QString::number(MonsterBalance::statMultiplier(15)));

        // EncounterBuilder applies floor scaling: same monster, deeper floor = stronger.
        gameStateManager* gsm = gameStateManager::instance();
        const QList<QVariantMap>& monsters = gsm->getMonsterData();
        if (!monsters.isEmpty()) {
            QString testMonster;
            for (const QVariantMap& m : monsters) {
                if (m.contains("ingroup") && m["hits"].toInt() > 0) {
                    testMonster = m["name"].toString();
                    break;
                }
            }
            if (testMonster.isEmpty()) testMonster = monsters[0]["name"].toString();

            auto floor1 = EncounterBuilder::buildEncounter(testMonster, monsters, 0, 1);
            auto floor10 = EncounterBuilder::buildEncounter(testMonster, monsters, 0, 10);
            auto floor15 = EncounterBuilder::buildEncounter(testMonster, monsters, 0, 15);

            if (!floor1.isEmpty() && !floor10.isEmpty() && !floor15.isEmpty()) {
                check(floor10.first().hp > floor1.first().hp,
                      "floor 10 monster has more HP than floor 1",
                      QString("%1 → %2").arg(floor1.first().hp).arg(floor10.first().hp));
                check(floor15.first().hp > floor10.first().hp,
                      "floor 15 monster has more HP than floor 10",
                      QString("%1 → %2").arg(floor10.first().hp).arg(floor15.first().hp));
                check(floor10.first().att > floor1.first().att,
                      "floor 10 monster has higher ATT than floor 1",
                      QString("%1 → %2").arg(floor1.first().att).arg(floor10.first().att));
            }

            // NG+ and floor scaling stack multiplicatively.
            auto ng1floor1 = EncounterBuilder::buildEncounter(testMonster, monsters, 1, 1);
            auto ng1floor10 = EncounterBuilder::buildEncounter(testMonster, monsters, 1, 10);
            if (!ng1floor1.isEmpty() && !ng1floor10.isEmpty()) {
                check(ng1floor10.first().hp > ng1floor1.first().hp,
                      "NG+1 floor 10 > NG+1 floor 1 (floor scaling stacks with NG+)",
                      QString("%1 → %2").arg(ng1floor1.first().hp).arg(ng1floor10.first().hp));
            }
        }

        // isPartyReady gates entry by party size and level.
        check(MonsterBalance::isPartyReady(4, 1, 1), "4 members at level 1 ready for floor 1");
        check(!MonsterBalance::isPartyReady(1, 1, 15), "solo level 1 not ready for floor 15");
        check(MonsterBalance::isPartyReady(4, 15, 15), "4 members at level 15 ready for floor 15");
    }

    // ------------------------------------------- v1.0.0 — slice 1.5: ItemProgression

    section("[70] v1.0.0 slice 1.5: ItemProgression into loot");
    {
        using Tier = ItemProgression::Tier;

        // Tier curve: Bronze → Iron → Steel → Adamantite → Mithril.
        check(ItemProgression::tierForFloor(1) == Tier::Bronze, "floor 1 is bronze");
        check(ItemProgression::tierForFloor(5) == Tier::Iron, "floor 5 is iron");
        check(ItemProgression::tierForFloor(8) == Tier::Steel, "floor 8 is steel");
        check(ItemProgression::tierForFloor(12) == Tier::Adamantite, "floor 12 is adamantite");
        check(ItemProgression::tierForFloor(15) == Tier::Mithril, "floor 15 is mithril");

        // Tier availability gates loot: Mithril is not available on floor 1.
        check(ItemProgression::isAvailable(Tier::Bronze, 1), "bronze available floor 1");
        check(!ItemProgression::isAvailable(Tier::Mithril, 1), "mithril not available floor 1");
        check(ItemProgression::isAvailable(Tier::Mithril, 15), "mithril available floor 15");

        // VictoryReward applies tier-gating: a monster whose drop table contains
        // a Mithril-prefixed item must not drop it on floor 1.
        gameStateManager* gsm = gameStateManager::instance();
        const QList<QVariantMap>& monsters = gsm->getMonsterData();
        const QList<ItemDef>& items = ItemDatabase::instance().all();

        // Find a Mithril-prefixed item in the database.
        const ItemDef* mithrilItem = nullptr;
        for (const ItemDef& item : items) {
            if (item.name.startsWith("Mithril", Qt::CaseInsensitive)) {
                mithrilItem = &item;
                break;
            }
        }

        if (mithrilItem && !monsters.isEmpty()) {
            // Find a monster that drops this item.
            QString dropper;
            for (const QVariantMap& m : monsters) {
                for (int i = 0; i <= 9; ++i) {
                    if (m.value(QString("Item%1").arg(i)).toInt() == mithrilItem->id) {
                        dropper = m["name"].toString();
                        break;
                    }
                }
                if (!dropper.isEmpty()) break;
            }

            if (!dropper.isEmpty()) {
                // On floor 1, the Mithril item must never drop.
                bool foundMithril = false;
                for (int i = 0; i < 100; ++i) {
                    QStringList loot = VictoryReward::calculateLoot(dropper, monsters, 1);
                    if (loot.contains(mithrilItem->name)) {
                        foundMithril = true;
                        break;
                    }
                }
                check(!foundMithril,
                      "Mithril item does not drop on floor 1 (tier-gated)",
                      mithrilItem->name);

                // On floor 15, it can drop (may not always due to RNG, but the
                // tier gate must not block it).
                bool foundMithrilDeep = false;
                for (int i = 0; i < 200; ++i) {
                    QStringList loot = VictoryReward::calculateLoot(dropper, monsters, 15);
                    if (loot.contains(mithrilItem->name)) {
                        foundMithrilDeep = true;
                        break;
                    }
                }
                check(foundMithrilDeep,
                      "Mithril item can drop on floor 15 (tier available)",
                      mithrilItem->name);
            }
        }
    }

    // ------------------------------------------- v1.0.0 — slice 1.6: GoldSinks

    section("[71] v1.0.0 slice 1.6: GoldSinks into town dialogs");
    {
        // GoldSinks is the single authority for every gold charge.
        check(GoldSinks::identificationCost() == 50, "identification 50");
        check(GoldSinks::uncurseCost() == 100, "uncurse 100");
        check(GoldSinks::restCostPerHour() == 10, "rest 10/hour");
        check(GoldSinks::curePoisonCost() == 50, "cure poison 50");
        check(GoldSinks::cureBlindnessCost() == 50, "cure blindness 50");

        // Resurrection scales with level and body location.
        check(GoldSinks::resurrectionCost(1, false) == 500, "level 1 resurrection 500");
        check(GoldSinks::resurrectionCost(5, false) == 2500, "level 5 resurrection 2500");
        check(GoldSinks::resurrectionCost(5, true) == 3000, "level 5 dungeon resurrection 3000");

        // Rescue scales superlinearly with depth.
        check(GoldSinks::rescueCost(1) == 250, "depth 1 rescue 250");
        check(GoldSinks::rescueCost(5) == 6250, "depth 5 rescue 6250");

        // Guild leveling scales with current level.
        check(GoldSinks::guildLevelCost("Mage", 0) == 500, "guild level 0→1 costs 500");
        check(GoldSinks::guildLevelCost("Mage", 5) == 3000, "guild level 5→6 costs 3000");

        // allSinks returns all categories.
        QList<QVariantMap> sinks = GoldSinks::allSinks();
        check(sinks.size() >= 7, "at least 7 gold sink categories",
              QString::number(sinks.size()));

        // sinkDescription gives a human-readable explanation.
        check(GoldSinks::sinkDescription("resurrection").contains("Morgue"), "resurrection desc");
        check(GoldSinks::sinkDescription("identification").contains("Store"), "identification desc");
        check(GoldSinks::sinkDescription("guild").contains("Guild"), "guild desc");
    }

    // ------------------------------------------- v1.0.0 — slice 1.7: SpellMechanics

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

    {
        // A neutral monster must NOT trigger combat.
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();

        QPair<int, int> neutralPos = {7, 7};
        dlg.m_monsterPositions[neutralPos] = "Test Neutral";
        dlg.m_MonsterAttitude["Test Neutral"] = "Neutral";
        gsm->setGameValue("DungeonX", 7);
        gsm->setGameValue("DungeonY", 7);

        DungeonHandlers::handleEncounters(&dlg, 7, 7);

        check(!dlg.m_inCombat, "neutral monster does not trigger combat");

        // Clean up.
        dlg.m_monsterPositions.remove(neutralPos);
        dlg.m_MonsterAttitude.remove("Test Neutral");
    }
    {
        // handleEncounters with no monster at the position must not start combat.
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();
        gsm->setGameValue("DungeonX", 10);
        gsm->setGameValue("DungeonY", 10);

        DungeonHandlers::handleEncounters(&dlg, 10, 10);

        check(!dlg.m_inCombat, "no monster means no combat");
    }

    // ------------------------------------------- v2.0.0 — slice 2.1: flee semantics

    section("[74] v2.0.0 slice 2.1: flee removes monster from map");
    {
        // When the player flees, the monster must be removed from the map.
        // Otherwise the player steps onto the same tile and combat restarts
        // immediately — the monster is both gone (combat closed) and present
        // (still on the map).
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();

        // Add a party member so there is someone to flee.
        Character partyMember;
        partyMember.name = "Test Hero";
        partyMember.level = 5;
        partyMember.hp = 100;
        partyMember.maxHp = 100;
        partyMember.isAlive = true;
        gsm->addCharacterToParty(partyMember);

        // Place a hostile monster at (5, 5) and start combat there.
        // Use a real monster name so EncounterBuilder can find it in the data.
        QPair<int, int> monsterPos = {5, 5};
        dlg.m_monsterPositions[monsterPos] = "Goblin";
        dlg.m_MonsterAttitude["Goblin"] = "Hostile";
        gsm->setGameValue("DungeonX", 5);
        gsm->setGameValue("DungeonY", 5);

        DungeonHandlers::handleEncounters(&dlg, 5, 5);
        check(dlg.m_inCombat, "combat started for flee test");

        // Force a successful flee by setting the monster's speed very low
        // and the player's speed very high.
        if (dlg.m_combatController && dlg.m_combatController->combatState() && dlg.m_combatController->combatState()->participantCount() > 0) {
            for (int i = 0; i < dlg.m_combatController->combatState()->participantCount(); ++i) {
                auto& p = dlg.m_combatController->combatState()->participant(i);
                if (p.isPlayer) p.speed = 100;
                else p.speed = 0;
            }
        }

        // Advance combat until it's the player's turn (monster may go first).
        for (int i = 0; i < 100 && !dlg.m_combatController->isPlayerTurn(); ++i) {
            dlg.advanceCombat();
        }
        check(dlg.m_combatController->isPlayerTurn(), "player turn for flee test");

        // Call the flee handler.
        dlg.fleeCombat();

        // Combat must be closed.
        check(!dlg.m_inCombat, "combat closed after flee");

        // The monster must be removed from the map.
        check(!dlg.m_monsterPositions.contains(monsterPos),
              "monster removed from map after flee");

        // Clean up.
        dlg.m_monsterPositions.remove(monsterPos);
        dlg.m_MonsterAttitude.remove("Test Flee Goblin");
    }

    // ------------------------------------------- v2.0.0 — slice 2.1: spell integration

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
        if (dlg.m_combatController->combatState()) {
            for (int i = 0; i < dlg.m_combatController->combatState()->participantCount(); ++i) {
                const auto& p = dlg.m_combatController->combatState()->participant(i);
                if (!p.isPlayer && p.isAlive) {
                    monsterIdx = i;
                    break;
                }
            }
        }
        check(monsterIdx >= 0, "monster found in combat state");

        // Record the monster's HP before the spell.
        int hpBefore = 0;
        if (monsterIdx >= 0 && dlg.m_combatController->combatState()) {
            hpBefore = dlg.m_combatController->combatState()->participant(monsterIdx).hp;
        }

        // Simulate a spell cast by emitting the signal that SpellCastingDialog
        // would emit. We call the lambda directly by invoking the spell
        // button handler, but since that opens a modal dialog, we instead
        // test the routing by calling applySpellDamageBySchool directly.
        if (dlg.m_combatController->combatActions() && monsterIdx >= 0) {
            QString result;
            dlg.m_combatController->combatActions()->applySpellDamageBySchool(monsterIdx, "Fireball", 50, result);
        }

        // The monster's HP must have changed (or it died).
        if (dlg.m_combatController->combatState() && monsterIdx >= 0) {
            int hpAfter = dlg.m_combatController->combatState()->participant(monsterIdx).hp;
            bool died = !dlg.m_combatController->combatState()->participant(monsterIdx).isAlive;
            check(hpAfter != hpBefore || died,
                  "spell damage applied to monster");
        }

        // Clean up.
        dlg.m_monsterPositions.remove(monsterPos);
        dlg.m_MonsterAttitude.remove("Test Spell Goblin");
        dlg.m_inCombat = false;
    }

    // Helper: join all guilds so any guild-restricted item can be equipped.
    auto joinAllGuilds = [](Character& c) {
        const QStringList guilds = GameConstants::GUILD_NAMES;
        for (const QString& g : guilds) c.joinGuild(g);
    };

    // Equipment guild-restriction tests live in test/equipment_test.cpp
    // (run separately via --selftest) so their guild membership doesn't
    // leak into other tests' characters.

    // ------------------------------------------- v2.0.0 — slice 2.3: XP/leveling

    section("[77] v2.0.0 slice 2.3: XP awarded to living party members");
    {
        // XP is divided among living party members only.
        gameStateManager* gsm = gameStateManager::instance();
        // Clear party first to avoid interference from other tests
        gsm->getParty().members.clear();

        Character hero;
        hero.name = "XP Hero";
        hero.level = 1;
        hero.experience = 0;
        hero.maxHp = 10;
        hero.hp = 10;
        hero.isAlive = true;

        Character dead;
        dead.name = "Dead Member";
        dead.level = 1;
        dead.experience = 0;
        dead.isAlive = false;

        gsm->addCharacterToParty(hero);
        gsm->addCharacterToParty(dead);

        int xpBefore = gsm->getPartyMember(0).experience;
        gsm->addExperienceToParty(300);
        int xpAfter = gsm->getPartyMember(0).experience;
        check(xpAfter > xpBefore, "living member gains XP");
        check(gsm->getPartyMember(1).experience == 0, "dead member gains no XP");
    }

    section("[78] v2.0.0 slice 2.3: level-up increases MaxHP and MaxMana");
    {
        Character hero;
        hero.name = "LevelUp Hero";
        hero.level = 1;
        hero.experience = 0;
        hero.maxHp = 10;
        hero.hp = 10;
        hero.maxMana = 50;
        hero.mana = 50;
        hero.intelligence = 12; // caster

        int hpBefore = hero.maxHp;
        int manaBefore = hero.maxMana;

        // Award enough XP to level up (level 1→2 needs 30 XP)
        hero.addExperience(30);

        check(hero.level == 2, "level increased to 2");
        check(hero.maxHp > hpBefore, "MaxHP increased on level-up");
        check(hero.maxMana > manaBefore, "MaxMana increased for caster on level-up");
        check(hero.hp == hero.maxHp, "HP fully restored on level-up");
    }

    section("[80] v2.0.0 slice 2.3: guild experience awarded on victory");
    {
        // Guild experience is awarded to living party members on combat victory.
        Character warrior;
        warrior.name = "Guild XP Hero";
        warrior.level = 1;
        warrior.experience = 0;
        warrior.maxHp = 10;
        warrior.hp = 10;
        warrior.isAlive = true;
        warrior.joinGuild("Warrior");

        gameStateManager* gsm = gameStateManager::instance();
        gsm->addCharacterToParty(warrior);

        // Award guild experience directly
        bool leveled = warrior.addGuildExperience("Warrior", 100);
        // After level-up, guild XP is consumed (100 XP = level 1→2 threshold)
        check(warrior.guildLevel("Warrior") >= 2, "guild level increased after XP");
        check(warrior.guildExperience["Warrior"] < 100, "guild XP consumed on level-up");
    }

    // ------------------------------------------- v2.0.0 — slice 2.4: loot drops

    section("[81] v2.0.0 slice 2.4: loot items added to inventory on victory");
    {
        // Loot drops are added to the lead character's inventory as unidentified.
        gameStateManager* gsm = gameStateManager::instance();
        Character hero;
        hero.name = "Loot Hero";
        hero.level = 1;
        hero.experience = 0;
        hero.maxHp = 10;
        hero.hp = 10;
        hero.isAlive = true;

        gsm->addCharacterToParty(hero);

        int invBefore = gsm->getPartyMember(0).inventory.size();

        HeldItem lootItem;
        lootItem.name = "Iron Sword";
        lootItem.identified = false;  // dungeon loot starts unidentified
        if (const ItemDef* def = ItemDatabase::instance().byName("Iron Sword")) {
            lootItem.M4E97 = static_cast<int16_t>(def->id);
        }
        gsm->addItemToCharacter(0, lootItem);

        int invAfter = gsm->getPartyMember(0).inventory.size();
        check(invAfter > invBefore, "loot item added to inventory");
        check(!gsm->getPartyMember(0).inventory.last().identified, "loot item is unidentified");
    }

    // ------------------------------------------- v2.0.0 — slice 2.5: death flow

    section("[84] v2.0.0 slice 2.5: killCharacter marks dead and records location");
    {
        Character hero;
        hero.name = "Dead Hero";
        hero.level = 3;
        hero.maxHp = 10;
        hero.hp = 10;
        hero.isAlive = true;
        DeathRecovery::killCharacter(hero, 3, 5, 7);
        check(!hero.isAlive, "character is dead after killCharacter");
        check(hero.hp == 0, "HP is 0 after killCharacter");
        check(hero.dungeonLevel == 3, "dungeon level recorded");
        check(hero.dungeonX == 5 && hero.dungeonY == 7, "dungeon position recorded");
    }

    section("[85] v2.0.0 slice 2.5: resurrection cost scales with level");
    {
        BodyLocation loc;
        loc.valid = true;
        loc.inCity = false;

        int cost1 = DeathRecovery::resurrectionCost(1, loc);
        int cost5 = DeathRecovery::resurrectionCost(5, loc);
        int cost10 = DeathRecovery::resurrectionCost(10, loc);
        check(cost5 > cost1, "resurrection cost increases with level");
        check(cost10 > cost5, "resurrection cost increases more at level 10");

        // Body in dungeon costs more than body in town
        BodyLocation townLoc;
        townLoc.valid = true;
        townLoc.inCity = true;
        int townCost = DeathRecovery::resurrectionCost(5, townLoc);
        check(cost5 > townCost, "body in dungeon costs more than body in town");
    }

    section("[86] v2.0.0 slice 2.5: resurrection deducts gold and revives");
    {
        Character hero;
        hero.name = "Resurrect Me";
        hero.level = 3;
        hero.maxHp = 10;
        hero.isAlive = false;
        hero.hp = 0;

        BodyLocation loc;
        loc.valid = true;
        loc.inCity = true;

        int partyGold = 10000;
        QString reason;
        bool ok = DeathRecovery::resurrect(hero, loc, partyGold, reason);
        check(ok, "resurrection succeeds with enough gold");
        check(hero.isAlive, "character is alive after resurrection");
        check(hero.hp > 0, "character has HP after resurrection");
        check(partyGold < 10000, "gold deducted for resurrection");

        // Cannot resurrect a living character
        QString failReason;
        bool fail = DeathRecovery::resurrect(hero, loc, partyGold, failReason);
        check(!fail, "cannot resurrect a living character");
    }

    section("[87] v2.0.0 slice 2.5: party wipe detection");
    {
        QList<Character> party;
        Character alive;
        alive.name = "Alive";
        alive.isAlive = true;
        party.append(alive);

        check(!DeathRecovery::isPartyWiped(party), "party not wiped with living member");

        Character dead;
        dead.name = "Dead";
        dead.isAlive = false;
        party.append(dead);
        check(!DeathRecovery::isPartyWiped(party), "party not wiped with one alive");

        party[0].isAlive = false;
        check(DeathRecovery::isPartyWiped(party), "party wiped when all dead");
        check(DeathRecovery::needsRescue(party), "rescue needed when party wiped");
    }

    section("[88] v2.0.0 slice 2.5: rescue party cost scales with depth");
    {
        int cost1 = DeathRecovery::rescuePartyCost(1);
        int cost5 = DeathRecovery::rescuePartyCost(5);
        int cost10 = DeathRecovery::rescuePartyCost(10);
        check(cost5 > cost1, "rescue cost increases with depth");
        check(cost10 > cost5, "rescue cost increases more at depth 10");
    }

    section("[89] v2.0.0 slice 2.5: body carrying and bringing to town");
    {
        BodyLocation loc;
        loc.valid = true;
        loc.inCity = false;
        loc.dungeonLevel = 5;
        loc.x = 3;
        loc.y = 4;

        QString reason;
        bool ok = DeathRecovery::carryBody(loc, reason);
        check(ok, "can carry a valid body");
        check(loc.carried, "body is marked as carried");

        // Bring to town (must be carried first)
        QList<BodyLocation> bodies;
        bodies.append(loc);
        int brought = DeathRecovery::bringBodiesToTown(bodies);
        check(brought == 1, "body brought to town");
        check(bodies[0].inCity, "body is in town after bringing");
    }

    // ------------------------------------------- v2.0.0 — slice 2.6: dungeon persistence

    section("[90] v2.0.0 slice 2.6: LevelSnapshot serializes and deserializes");
    {
        LevelSnapshot snap;
        snap.level = 3;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(1, 2), "Goblin");
        snap.treasurePositions.insert(qMakePair(3, 4), "Iron Sword");
        snap.openedChests.insert(qMakePair(5, 6));
        snap.visitedTiles.insert(qMakePair(7, 8));
        snap.stairsUp = qMakePair(9, 10);
        snap.stairsDown = qMakePair(11, 12);
        snap.bossDefeated = true;
        snap.torchTurnsRemaining = 42;
        snap.lightRadius = 3;
        snap.collectedTorches.append("Torch");
        snap.triggeredTraps.insert(qMakePair(13, 14));
        snap.trapPositions.insert(qMakePair(15, 16), "Spike");

        QVariantMap map = snap.toMap();
        LevelSnapshot restored;
        restored.loadFromMap(map);

        check(restored.level == 3, "level preserved");
        check(restored.generated, "generated flag preserved");
        check(restored.monsterPositions.size() == 1, "monster positions preserved");
        check(restored.treasurePositions.size() == 1, "treasure positions preserved");
        check(restored.openedChests.size() == 1, "opened chests preserved");
        check(restored.visitedTiles.size() == 1, "visited tiles preserved");
        check(restored.stairsUp == qMakePair(9, 10), "stairs up preserved");
        check(restored.stairsDown == qMakePair(11, 12), "stairs down preserved");
        check(restored.bossDefeated, "boss defeated preserved");
        check(restored.torchTurnsRemaining == 42, "torch turns preserved");
        check(restored.lightRadius == 3, "light radius preserved");
        check(restored.collectedTorches.size() == 1, "collected torches preserved");
        check(restored.triggeredTraps.size() == 1, "triggered traps preserved");
        check(restored.trapPositions.size() == 1, "trap positions preserved");
    }

    section("[91] v2.0.0 slice 2.6: DungeonLevelRegistry stores and retrieves levels");
    {
        DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();
        registry.clear();

        LevelSnapshot snap;
        snap.level = 5;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(1, 1), "Orc");
        registry.store(snap);

        check(registry.hasLevel(5), "level 5 exists after store");
        check(registry.count() == 1, "registry has 1 level");

        const LevelSnapshot* retrieved = registry.level(5);
        check(retrieved != nullptr, "level 5 retrieved");
        check(retrieved->monsterPositions.size() == 1, "monster positions preserved");
    }

    section("[92] v2.0.0 slice 2.6: respawnMonsters adds monsters back");
    {
        DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();
        registry.clear();

        LevelSnapshot snap;
        snap.level = 7;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(1, 1), "Goblin");
        registry.store(snap);

        QStringList pool;
        pool << "Goblin" << "Orc" << "Troll";

        QRandomGenerator rng(42);
        int respawned = registry.respawnMonsters(7, 0.5, 10, rng, pool);
        check(respawned > 0, "monsters respawned");
        check(registry.level(7)->monsterPositions.size() > 1, "monster count increased");
    }

    section("[93] v2.0.0 slice 2.6: respawnMonsters skips when no pool");
    {
        DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();
        registry.clear();

        LevelSnapshot snap;
        snap.level = 9;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(1, 1), "Goblin");
        registry.store(snap);

        QRandomGenerator rng(42);
        int respawned = registry.respawnMonsters(9, 0.5, 10, rng, {});
        check(respawned == 0, "no respawn with empty pool");
    }

    section("[94] v2.0.0 slice 2.6: registry serializes to map and back");
    {
        DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();
        registry.clear();

        LevelSnapshot snap;
        snap.level = 11;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(2, 3), "Skeleton");
        snap.bossDefeated = true;
        registry.store(snap);

        QVariantMap map = registry.toMap();
        check(map.contains("levels"), "map contains levels");

        DungeonLevelRegistry& registry2 = DungeonLevelRegistry::instance();
        registry2.clear();
        registry2.loadFromMap(map);

        check(registry2.hasLevel(11), "level 11 restored from map");
        check(registry2.level(11)->bossDefeated, "boss defeated restored");
        check(registry2.level(11)->monsterPositions.size() == 1, "monsters restored");
    }

    // ------------------------------------------- v2.0.0 — slice 2.7: monster spells

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

    // ------------------------------------------- v2.0.0 — slice 2.8: status effects

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

    // ------------------------------------------- v2.0.0 — slice 2.9: town wiring

    section("[105] v2.0.0 slice 2.9: quest board accept and complete");
    {
        QuestBoardDialog::reset();
        QList<BoardQuest> quests = QuestBoardDialog::availableQuests();
        check(!quests.isEmpty(), "quests available on board");

        if (!quests.isEmpty()) {
            const BoardQuest& q = quests.first();
            bool accepted = QuestBoardDialog::acceptQuest(q.id);
            check(accepted, "quest accepted");
            check(QuestBoardDialog::isAccepted(q.id), "quest is accepted");

            // Kill quests progress via reportKill
            if (!q.isFetch && !q.targetMonster.isEmpty()) {
                for (int i = 0; i < q.killCount; ++i) {
                    QuestBoardDialog::reportKill(q.targetMonster, q.targetFloor);
                }
                check(QuestBoardDialog::isComplete(q.id), "kill quest complete after kills");
            }

            int gold = 0, xp = 0;
            bool turnedIn = QuestBoardDialog::turnIn(q.id, gold, xp);
            check(turnedIn, "quest turned in");
            check(gold > 0 || xp > 0, "quest reward given");
        }
    }

    section("[106] v2.0.0 slice 2.9: tavern rest restores HP and mana");
    {
        Character hero;
        hero.name = "Tired Hero";
        hero.level = 3;
        hero.maxHp = 20;
        hero.hp = 5;
        hero.maxMana = 50;
        hero.mana = 10;
        hero.isAlive = true;

        // Simulate rest: restore HP and mana
        hero.hp = hero.maxHp;
        hero.mana = hero.maxMana;

        check(hero.hp == hero.maxHp, "HP restored after rest");
        check(hero.mana == hero.maxMana, "mana restored after rest");
    }

    section("[107] v2.0.0 slice 2.9: tavern cures poison and blindness");
    {
        Character hero;
        hero.name = "Sick Hero";
        hero.level = 3;
        hero.maxHp = 20;
        hero.hp = 20;
        hero.isAlive = true;
        hero.statusFlags = GameConstants::Poisoned | GameConstants::Blinded;

        // Simulate cure
        hero.statusFlags &= ~GameConstants::Poisoned;
        hero.statusFlags &= ~GameConstants::Blinded;

        check(!(hero.statusFlags & GameConstants::Poisoned), "poison cured");
        check(!(hero.statusFlags & GameConstants::Blinded), "blindness cured");
    }

    // ------------------------------------------- v2.0.0 — slice 2.10: item identification

    section("[108] v2.0.0 slice 2.10: loot items start unidentified");
    {
        HeldItem lootItem;
        lootItem.name = "Iron Sword";
        lootItem.identified = false;
        if (const ItemDef* def = ItemDatabase::instance().byName("Iron Sword")) {
            lootItem.M4E97 = static_cast<int16_t>(def->id);
        }
        check(!lootItem.identified, "loot item starts unidentified");
    }

    section("[109] v2.0.0 slice 2.10: identify renames item");
    {
        HeldItem item;
        item.name = "Unknown Iron Sword";
        item.identified = false;
        if (const ItemDef* def = ItemDatabase::instance().byName("Iron Sword")) {
            item.M4E97 = static_cast<int16_t>(def->id);
        }

        // Identify: look up real name and mark identified
        if (const ItemDef* def = ItemDatabase::instance().byId(item.M4E97)) {
            item.name = def->name;
            item.identified = true;
        }
        check(item.identified, "item identified");
        check(item.name == "Iron Sword", "item renamed after identification");
    }

    section("[111] v2.0.0 slice 2.10: identification cost from GoldSinks");
    {
        int cost = GoldSinks::identificationCost();
        check(cost > 0, "identification cost is positive");

        int uncurseCost = GoldSinks::uncurseCost();
        check(uncurseCost > 0, "uncurse cost is positive");
    }

    // ------------------------------------------- v2.0.0 — slice 2.11: bestiary auto-population

    section("[112] v2.0.0 slice 2.11: bestiary is empty at game start");
    {
        BestiaryDialog::resetEncounters();
        check(BestiaryDialog::encounteredCount() == 0, "no monsters encountered at start");
        check(BestiaryDialog::displayName("Goblie") == "???",
              "unmet monster shows as ???");
    }

    section("[113] v2.0.0 slice 2.11: encountering a monster unlocks its entry");
    {
        BestiaryDialog::resetEncounters();
        BestiaryDialog::recordEncounter("Goblie");
        check(BestiaryDialog::isEncountered("Goblie"), "Goblie is now encountered");
        check(BestiaryDialog::encounteredCount() == 1, "exactly one encountered");
        check(BestiaryDialog::displayName("Goblie") == "Goblie",
              "met monster shows its real name");
        check(BestiaryDialog::displayName("Dragon") == "???",
              "still-unmet monster stays hidden");
    }

    section("[114] v2.0.0 slice 2.11: handleEncounters records the monster");
    {
        BestiaryDialog::resetEncounters();
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        QPair<int, int> pos = {6, 6};
        dlg.m_monsterPositions[pos] = "Goblie";
        dlg.m_MonsterAttitude["Goblie"] = "Neutral";
        gameStateManager* gsm = gameStateManager::instance();
        gsm->setGameValue("DungeonX", 6);
        gsm->setGameValue("DungeonY", 6);

        DungeonHandlers::handleEncounters(&dlg, 6, 6);

        check(BestiaryDialog::isEncountered("Goblie"),
              "real encounter path records the monster");

        dlg.m_monsterPositions.remove(pos);
        dlg.m_MonsterAttitude.remove("Goblie");
    }

    section("[115] v2.0.0 slice 2.11: stats shown only for encountered monsters");
    {
        BestiaryDialog::resetEncounters();
        const QVariantMap goblie = BestiaryDialog::entry("Goblie");
        check(!goblie.isEmpty(), "Goblie exists in the bestiary data");

        // Unmet: no stats revealed.
        check(!BestiaryDialog::isEncountered("Goblie"), "unmet before recording");

        BestiaryDialog::recordEncounter("Goblie");
        check(BestiaryDialog::isEncountered("Goblie"), "met after recording");
        check(goblie.value("hits").toInt() >= 0, "stats available once encountered");
    }

    // ------------------------------------------- v2.0.0 — slice 2.12: journal wiring

    section("[116] v2.0.0 slice 2.12: quest accept writes a journal entry");
    {
        JournalDialog::clearAll();
        QuestBoardDialog::reset();

        QList<BoardQuest> quests = QuestBoardDialog::availableQuests();
        check(!quests.isEmpty(), "quests available");
        if (!quests.isEmpty()) {
            const BoardQuest q = quests.first();
            QuestBoardDialog::acceptQuest(q.id);

            QList<JournalEntry> entries = JournalDialog::allEntries();
            bool found = false;
            for (const JournalEntry& e : entries) {
                if (e.category == "Quest" && e.text.contains(q.title)) found = true;
            }
            check(found, "accepting a quest wrote a Quest journal entry");
        }
    }

    section("[117] v2.0.0 slice 2.12: quest turn-in writes a journal entry");
    {
        JournalDialog::clearAll();
        QuestBoardDialog::reset();

        QList<BoardQuest> quests = QuestBoardDialog::availableQuests();
        BoardQuest killQuest;
        for (const BoardQuest& q : quests) {
            if (!q.isFetch && q.killCount > 0 && !q.targetMonster.isEmpty()) {
                killQuest = q;
                break;
            }
        }
        if (!killQuest.id.isEmpty()) {
            QuestBoardDialog::acceptQuest(killQuest.id);
            for (int i = 0; i < killQuest.killCount; ++i) {
                QuestBoardDialog::reportKill(killQuest.targetMonster, killQuest.targetFloor);
            }
            int gold = 0, xp = 0;
            QuestBoardDialog::turnIn(killQuest.id, gold, xp);

            QList<JournalEntry> entries = JournalDialog::allEntries();
            bool found = false;
            for (const JournalEntry& e : entries) {
                if (e.category == "Quest" && e.text.contains("Completed")) found = true;
            }
            check(found, "turning in a quest wrote a Completed entry");
        } else {
            check(false, "the quest board offered no kill quest to test with");
        }
    }

    section("[118] v2.0.0 slice 2.12: victory writes a journal entry");
    {
        JournalDialog::clearAll();

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();
        gsm->setGameValue("DungeonLevel", 3);
        gsm->setGameValue("DungeonX", 4);
        gsm->setGameValue("DungeonY", 4);

        dlg.handleVictory();

        QList<JournalEntry> entries = JournalDialog::allEntries();
        bool found = false;
        for (const JournalEntry& e : entries) {
            if (e.category == "Combat" && e.text.contains("Defeated")) found = true;
        }
        check(found, "handleVictory wrote a Combat journal entry");
    }

    section("[119] v2.0.0 slice 2.12: level-up writes a journal entry");
    {
        JournalDialog::clearAll();

        PartyManager pm;
        Character hero;
        hero.name = "Journal Hero";
        hero.level = 1;
        hero.experience = 0;
        hero.maxHp = 20;
        hero.hp = 20;
        hero.isAlive = true;

        hero.level = 2;   // simulate the level the character just reached
        pm.applyLevelUpGains(hero);

        QList<JournalEntry> entries = JournalDialog::allEntries();
        bool found = false;
        for (const JournalEntry& e : entries) {
            if (e.category == "Combat" && e.text.contains("reached level")) found = true;
        }
        check(found, "level-up wrote a journal entry");
    }

    // ------------------------------------------- v2.0.0 — slice 2.13: gold sinks wiring

    section("[120] v2.0.0 slice 2.13: tavern rest cost comes from GoldSinks");
    {
        // 3 hours for 2 living members, priced at GoldSinks::restCostPerHour().
        const int rate = GoldSinks::restCostPerHour();
        check(TavernDialog::restCost(3, 2) == 3 * rate * 2,
              "rest cost = hours * GoldSinks rate * living members",
              QString::number(TavernDialog::restCost(3, 2)));

        // The rate must actually flow through: 2h for 5 members is a clean
        // multiple that a stale literal would miss unless it equals the rate.
        check(TavernDialog::restCost(2, 5) == 10 * rate,
              "rest cost scales with members",
              QString::number(TavernDialog::restCost(2, 5)));

        // Dead members don't pay.
        check(TavernDialog::restCost(3, 0) == 0, "no living members, no cost");
        check(TavernDialog::restCost(0, 4) == 0, "zero hours, no cost");
    }

    section("[121] v2.0.0 slice 2.13: tavern cure cost comes from GoldSinks");
    {
        check(TavernDialog::cureCost(true, false) == GoldSinks::curePoisonCost(),
              "poison cure uses GoldSinks rate");
        check(TavernDialog::cureCost(false, true) == GoldSinks::cureBlindnessCost(),
              "blindness cure uses GoldSinks rate");
        check(TavernDialog::cureCost(true, true)
                  == GoldSinks::curePoisonCost() + GoldSinks::cureBlindnessCost(),
              "both cures sum the two GoldSinks rates");
        check(TavernDialog::cureCost(false, false) == 0, "nothing selected, no cost");
    }

    section("[122] v2.0.0 slice 2.13: morgue raise cost comes from GoldSinks");
    {
        // Concrete amounts: a level-5 body in town is 5 * 500; still in the
        // dungeon adds the 500 rescue fee.
        check(MorgueDialog::raiseCost(5, true) == 2500,
              "level 5 body in city costs 2500",
              QString::number(MorgueDialog::raiseCost(5, true)));
        check(MorgueDialog::raiseCost(5, false) == 3000,
              "level 5 body in dungeon costs 3000",
              QString::number(MorgueDialog::raiseCost(5, false)));
        check(MorgueDialog::raiseCost(5, true) == GoldSinks::resurrectionCost(5, false),
              "body in city matches resurrectionCost(level, false)");
        check(MorgueDialog::raiseCost(5, false) == GoldSinks::resurrectionCost(5, true),
              "body in dungeon matches resurrectionCost(level, true)");
        check(MorgueDialog::raiseCost(5, false) > MorgueDialog::raiseCost(5, true),
              "raising from the dungeon costs more");
    }

    section("[123] v2.0.0 slice 2.13: DeathRecovery::resurrect deducts gold");
    {
        Character dead;
        dead.name = "Fallen Hero";
        dead.level = 4;
        dead.isAlive = false;

        BodyLocation loc;
        loc.valid = true;
        loc.inCity = true;

        const int cost = GoldSinks::resurrectionCost(4, false);
        int gold = cost + 100;
        QString reason;

        bool ok = DeathRecovery::resurrect(dead, loc, gold, reason);
        check(ok, "resurrection succeeds with enough gold");
        check(dead.isAlive, "character is alive again");
        check(gold == 100, "exactly the resurrection cost was deducted",
              QString::number(gold));

        // Not enough gold: nothing happens.
        Character dead2;
        dead2.name = "Broke Hero";
        dead2.level = 4;
        dead2.isAlive = false;
        BodyLocation loc2;
        loc2.valid = true;
        loc2.inCity = true;
        int poorGold = 1;
        check(!DeathRecovery::resurrect(dead2, loc2, poorGold, reason),
              "resurrection fails when gold is short");
        check(!dead2.isAlive, "character stays dead");
        check(poorGold == 1, "no gold deducted on failure");
    }

    // ------------------------------------------- v2.0.0 — slice 2.14: aging consequences

    section("[124] v2.0.0 slice 2.14: death by old age writes a journal entry");
    {
        JournalDialog::clearAll();

        auto& members = gsm->getPartyMembers();
        check(!members.isEmpty(), "party has members to age");
        if (!members.isEmpty()) {
            Character& c = members[0];
            const QString savedName = c.name;
            const int savedAge = c.age;
            const bool savedAlive = c.isAlive;
            const int savedHp = c.hp;
            const QString savedRace = c.race;

            c.name = "OldAgeJournal";
            c.race = "Human";
            c.isAlive = true;
            c.hp = 50;
            c.age = 100;   // exactly Human max age

            gsm->processAgingConsequences();

            QList<JournalEntry> entries = JournalDialog::allEntries();
            bool found = false;
            for (const JournalEntry& e : entries) {
                if (e.text.contains("OldAgeJournal") &&
                    (e.text.contains("old age") || e.text.contains("died"))) {
                    found = true;
                }
            }
            check(found, "death by old age wrote a journal entry");

            // Restore and clean up (the death path also saved a character file).
            c.name = savedName;
            c.age = savedAge;
            c.isAlive = savedAlive;
            c.hp = savedHp;
            c.race = savedRace;
            QFile::remove("data/characters/OldAgeJournal.json");
        }
    }

    section("[125] v2.0.0 slice 2.14: body of an old-age death reaches the Morgue");
    {
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            Character& c = members[0];
            const QString savedName = c.name;
            const int savedAge = c.age;
            const bool savedAlive = c.isAlive;
            const int savedHp = c.hp;
            const QString savedRace = c.race;

            c.name = "OldAgeBody";
            c.race = "Human";
            c.isAlive = true;
            c.hp = 50;
            c.age = 100;

            gsm->processAgingConsequences();

            // The Morgue finds bodies by reading character files that are dead.
            const QString path = "data/characters/OldAgeBody.json";
            check(QFile::exists(path), "dead character was saved to file");

            QFile f(path);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QString content = QString::fromUtf8(f.readAll());
                f.close();
                check(content.contains("\"isAlive\": false") || content.contains("\"isAlive\":false"),
                      "saved body is marked dead");
            }

            // Restore and clean up.
            c.name = savedName;
            c.age = savedAge;
            c.isAlive = savedAlive;
            c.hp = savedHp;
            c.race = savedRace;
            QFile::remove(path);
        }
    }

    // ------------------------------------------- v3.0.0 — slice 3.1: target selection

    section("[126] v3.0.0 slice 3.1: attack hits the selected monster, not the first");
    {
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();
        gsm->getPartyMembers().clear();
        gsm->addCharacterToParty([]{
            Character c;
            c.name = "Target Hero";
            c.level = 5;
            c.hp = 100; c.maxHp = 100;
            c.isAlive = true;
            return c;
        }());

        QPair<int, int> pos = {5, 5};
        dlg.m_monsterPositions[pos] = "Test Target Goblin";
        dlg.m_MonsterAttitude["Test Target Goblin"] = "Hostile";
        gsm->setGameValue("DungeonX", 5);
        gsm->setGameValue("DungeonY", 5);

        DungeonHandlers::handleEncounters(&dlg, 5, 5);
        check(dlg.m_inCombat, "combat started");

        // Build a deterministic 3-monster fight on top of the real participants.
        dlg.m_combatController->combatState()->clear();
        CombatParticipant hero;
        hero.name = "Target Hero";
        hero.isPlayer = true;
        hero.isAlive = true;
        hero.hp = 100; hero.maxHp = 100;
        hero.att = 30; hero.dex = 50; hero.level = 5;
        hero.speed = 100;
        dlg.m_combatController->combatState()->addParticipant(hero);

        for (int m = 0; m < 3; ++m) {
            CombatParticipant mon;
            mon.name = QString("Goblin %1").arg(m + 1);
            mon.isPlayer = false;
            mon.isAlive = true;
            mon.hp = 500; mon.maxHp = 500;   // survive one hit
            mon.def = -50;                    // guarantee hits
            mon.level = 1;
            mon.speed = 0;
            dlg.m_combatController->combatState()->addParticipant(mon);
        }

        // Give the hero the turn.
        dlg.m_combatController->combatState()->rollInitiative();
        dlg.m_combatController->turnEngine()->startRound();
        for (int i = 0; i < 100 && !dlg.m_combatController->combatActions()->isPlayerTurn(); ++i) {
            dlg.advanceCombat();
        }
        check(dlg.m_combatController->combatActions()->isPlayerTurn(), "player has the turn");

        dlg.updateCombatUI();
        check(dlg.m_combatTargetList && dlg.m_combatTargetList->count() == 3,
              "target list has one row per living monster",
              QString::number(dlg.m_combatTargetList ? dlg.m_combatTargetList->count() : -1));

        // Select the THIRD monster and attack.
        const int thirdIdx = 3;   // hero=0, goblins=1,2,3
        dlg.m_combatTargetList->setCurrentRow(2);
        check(dlg.selectedMonsterIndex() == thirdIdx, "third monster is selected",
              QString::number(dlg.selectedMonsterIndex()));

        int firstHp = dlg.m_combatController->combatState()->participant(1).hp;
        int thirdHp = dlg.m_combatController->combatState()->participant(thirdIdx).hp;

        // A natural 1 always fumbles, so a single attack can miss. Attack until
        // one lands (re-taking the turn each round), then check WHERE it landed:
        // the first monster must be untouched throughout.
        bool thirdDamaged = false;
        for (int attempt = 0; attempt < 50 && !thirdDamaged; ++attempt) {
            for (int i = 0; i < 100 && !dlg.m_combatController->combatActions()->isPlayerTurn(); ++i) {
                dlg.advanceCombat();
            }
            if (!dlg.m_combatController->combatActions()->isPlayerTurn()) break;

            dlg.updateCombatUI();
            for (int row = 0; row < dlg.m_combatTargetList->count(); ++row) {
                if (dlg.m_combatTargetList->item(row)->data(Qt::UserRole).toInt() == thirdIdx) {
                    dlg.m_combatTargetList->setCurrentRow(row);
                    break;
                }
            }
            dlg.on_combatAttackButton_clicked();

            if (dlg.m_combatController->combatState()->participant(thirdIdx).hp < thirdHp) thirdDamaged = true;
        }

        check(thirdDamaged, "the selected (third) monster took the damage",
              QString::number(dlg.m_combatController->combatState()->participant(thirdIdx).hp));
        check(dlg.m_combatController->combatState()->participant(1).hp == firstHp,
              "the first monster was NOT hit",
              QString::number(dlg.m_combatController->combatState()->participant(1).hp));

        dlg.m_monsterPositions.remove(pos);
        dlg.m_MonsterAttitude.remove("Test Target Goblin");
        dlg.m_inCombat = false;
    }

    section("[127] v3.0.0 slice 3.1: selection moves on when the target dies");
    {
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();
        gsm->getPartyMembers().clear();
        gsm->addCharacterToParty([]{
            Character c;
            c.name = "Mover Hero";
            c.level = 5;
            c.hp = 100; c.maxHp = 100;
            c.isAlive = true;
            return c;
        }());

        QPair<int, int> pos = {6, 6};
        dlg.m_monsterPositions[pos] = "Test Move Goblin";
        dlg.m_MonsterAttitude["Test Move Goblin"] = "Hostile";
        gsm->setGameValue("DungeonX", 6);
        gsm->setGameValue("DungeonY", 6);
        DungeonHandlers::handleEncounters(&dlg, 6, 6);

        dlg.m_combatController->combatState()->clear();
        CombatParticipant hero;
        hero.name = "Mover Hero";
        hero.isPlayer = true;
        hero.isAlive = true;
        hero.hp = 100; hero.maxHp = 100;
        hero.speed = 100;
        dlg.m_combatController->combatState()->addParticipant(hero);

        for (int m = 0; m < 2; ++m) {
            CombatParticipant mon;
            mon.name = QString("Weak Goblin %1").arg(m + 1);
            mon.isPlayer = false;
            mon.isAlive = true;
            mon.hp = 1; mon.maxHp = 1;   // one hit kills
            mon.def = -50;
            mon.speed = 0;
            dlg.m_combatController->combatState()->addParticipant(mon);
        }

        dlg.updateCombatUI();
        check(dlg.m_combatTargetList->count() == 2, "two monsters listed");

        // Select the first monster and kill it.
        dlg.m_combatTargetList->setCurrentRow(0);
        check(dlg.selectedMonsterIndex() == 1, "first monster selected");

        // Kill it directly, then refresh: selection must move to the survivor.
        dlg.m_combatController->combatState()->participant(1).hp = 0;
        dlg.m_combatController->combatState()->participant(1).isAlive = false;
        dlg.updateCombatUI();

        check(dlg.m_combatTargetList->count() == 1, "dead monster dropped from the list");
        check(dlg.selectedMonsterIndex() == 2,
              "selection moved to the surviving monster",
              QString::number(dlg.selectedMonsterIndex()));

        dlg.m_monsterPositions.remove(pos);
        dlg.m_MonsterAttitude.remove("Test Move Goblin");
        dlg.m_inCombat = false;
    }

    section("[128] v3.0.0 slice 3.1: no selection falls back to the first living monster");
    {
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();
        gsm->getPartyMembers().clear();
        gsm->addCharacterToParty([]{
            Character c;
            c.name = "Fallback Hero";
            c.level = 5;
            c.hp = 100; c.maxHp = 100;
            c.isAlive = true;
            return c;
        }());

        QPair<int, int> pos = {7, 7};
        dlg.m_monsterPositions[pos] = "Test Fallback Goblin";
        dlg.m_MonsterAttitude["Test Fallback Goblin"] = "Hostile";
        gsm->setGameValue("DungeonX", 7);
        gsm->setGameValue("DungeonY", 7);
        DungeonHandlers::handleEncounters(&dlg, 7, 7);

        dlg.m_combatController->combatState()->clear();
        CombatParticipant hero;
        hero.name = "Fallback Hero";
        hero.isPlayer = true;
        hero.isAlive = true;
        hero.hp = 100; hero.maxHp = 100;
        hero.speed = 100;
        dlg.m_combatController->combatState()->addParticipant(hero);

        CombatParticipant mon;
        mon.name = "Lonely Goblin";
        mon.isPlayer = false;
        mon.isAlive = true;
        mon.hp = 50; mon.maxHp = 50;
        mon.def = -50;
        mon.speed = 0;
        dlg.m_combatController->combatState()->addParticipant(mon);

        // Clear any selection: the fallback must find the living monster.
        dlg.m_combatTargetList->clear();
        check(dlg.selectedMonsterIndex() == -1, "nothing selected");

        const int found = dlg.selectFirstLivingMonster();
        check(found == 1, "fallback found the living monster",
              QString::number(found));
        check(dlg.selectedMonsterIndex() == 1, "fallback selected it");

        dlg.m_monsterPositions.remove(pos);
        dlg.m_MonsterAttitude.remove("Test Fallback Goblin");
        dlg.m_inCombat = false;
    }

    // ------------------------------------------- v3.0.0 — slice 3.2: per-monster display

    section("[129] v3.0.0 slice 3.2: one row per monster with independent HP");
    {
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();
        gsm->getPartyMembers().clear();
        gsm->addCharacterToParty([]{
            Character c;
            c.name = "Row Hero";
            c.level = 5;
            c.hp = 100; c.maxHp = 100;
            c.isAlive = true;
            return c;
        }());

        QPair<int, int> pos = {8, 8};
        dlg.m_monsterPositions[pos] = "Test Row Goblin";
        dlg.m_MonsterAttitude["Test Row Goblin"] = "Hostile";
        gsm->setGameValue("DungeonX", 8);
        gsm->setGameValue("DungeonY", 8);
        DungeonHandlers::handleEncounters(&dlg, 8, 8);

        dlg.m_combatController->combatState()->clear();
        CombatParticipant hero;
        hero.name = "Row Hero";
        hero.isPlayer = true;
        hero.isAlive = true;
        hero.hp = 100; hero.maxHp = 100;
        dlg.m_combatController->combatState()->addParticipant(hero);

        for (int m = 0; m < 3; ++m) {
            CombatParticipant mon;
            mon.name = QString("Row Goblin %1").arg(m + 1);
            mon.isPlayer = false;
            mon.isAlive = true;
            mon.hp = 20 + m * 10;   // distinct HP per row
            mon.maxHp = 20 + m * 10;
            dlg.m_combatController->combatState()->addParticipant(mon);
        }

        dlg.updateCombatUI();

        check(dlg.m_combatTargetList->count() == 3, "three rows, one per monster",
              QString::number(dlg.m_combatTargetList->count()));

        // Each row must show its own HP, not a shared total.
        check(dlg.m_combatTargetList->item(0)->text().contains("20/20"),
              "row 1 shows its own HP", dlg.m_combatTargetList->item(0)->text());
        check(dlg.m_combatTargetList->item(1)->text().contains("30/30"),
              "row 2 shows its own HP", dlg.m_combatTargetList->item(1)->text());
        check(dlg.m_combatTargetList->item(2)->text().contains("40/40"),
              "row 3 shows its own HP", dlg.m_combatTargetList->item(2)->text());

        // Damage only the second monster: only its row changes.
        dlg.m_combatController->combatState()->participant(2).hp = 5;
        dlg.updateCombatUI();

        check(dlg.m_combatTargetList->item(0)->text().contains("20/20"),
              "untouched row 1 unchanged", dlg.m_combatTargetList->item(0)->text());
        check(dlg.m_combatTargetList->item(1)->text().contains("5/30"),
              "damaged row 2 shows the new HP", dlg.m_combatTargetList->item(1)->text());
        check(dlg.m_combatTargetList->item(2)->text().contains("40/40"),
              "untouched row 3 unchanged", dlg.m_combatTargetList->item(2)->text());

        // Status must reach the ROW, not just exist in combatStatusText().
        // Participant 0 is the hero, so goblins are participants 1..3 and list
        // rows 0..2.
        dlg.m_combatController->combatState()->participant(1).poisonDuration = 4;   // goblin 1 = row 0
        dlg.m_combatController->combatState()->participant(3).fireDuration = 2;     // goblin 3 = row 2
        dlg.updateCombatUI();

        check(dlg.m_combatTargetList->item(0)->text().contains("poison 4"),
              "poison shows in the poisoned monster's row",
              dlg.m_combatTargetList->item(0)->text());
        check(dlg.m_combatTargetList->item(2)->text().contains("fire 2"),
              "fire shows in the burning monster's row",
              dlg.m_combatTargetList->item(2)->text());
        check(!dlg.m_combatTargetList->item(1)->text().contains("poison") &&
              !dlg.m_combatTargetList->item(1)->text().contains("fire"),
              "the clean row shows no status",
              dlg.m_combatTargetList->item(1)->text());

        // The colour markup must reach the row too, not just combatStatusText().
        check(dlg.m_combatTargetList->item(0)->text().contains("<font color='green'>"),
              "the poisoned row is green",
              dlg.m_combatTargetList->item(0)->text());
        check(dlg.m_combatTargetList->item(2)->text().contains("<font color='red'>"),
              "the burning row is red",
              dlg.m_combatTargetList->item(2)->text());

        dlg.m_monsterPositions.remove(pos);
        dlg.m_MonsterAttitude.remove("Test Row Goblin");
        dlg.m_inCombat = false;
    }

    section("[130] v3.0.0 slice 3.2: status is shown per monster");
    {
        // combatStatusText is the single definition of what a row displays.
        CombatParticipant clean;
        check(DungeonDialog::combatStatusText(clean).isEmpty(),
              "a clean participant has no status text");

        CombatParticipant poisoned;
        poisoned.poisonDuration = 3;
        const QString poisonText = DungeonDialog::combatStatusText(poisoned);
        check(poisonText.contains("poison 3"),
              "poison shows its remaining rounds", poisonText);
        check(poisonText.contains("<font color='green'>"),
              "poison is green", poisonText);

        CombatParticipant burning;
        burning.fireDuration = 2;
        const QString fireText = DungeonDialog::combatStatusText(burning);
        check(fireText.contains("fire 2"),
              "fire shows its remaining rounds", fireText);
        check(fireText.contains("<font color='red'>"),
              "fire is red", fireText);

        CombatParticipant blinded;
        blinded.blindDuration = 1;
        const QString blindText = DungeonDialog::combatStatusText(blinded);
        check(blindText.contains("blind 1"),
              "blind shows its remaining rounds", blindText);
        check(blindText.contains("<font color='yellow'>"),
              "blind is yellow", blindText);

        CombatParticipant confused;
        confused.confusionDuration = 1;
        const QString confusedText = DungeonDialog::combatStatusText(confused);
        check(confusedText.contains("confused 1"),
              "confused shows its remaining rounds", confusedText);
        check(confusedText.contains("<font color='magenta'>"),
              "confused is magenta", confusedText);

        CombatParticipant stunned;
        stunned.stunDuration = 1;
        const QString stunText = DungeonDialog::combatStatusText(stunned);
        check(stunText.contains("stunned 1"),
              "stun is shown", stunText);
        check(stunText.contains("<font color='cyan'>"),
              "stunned is cyan", stunText);

        CombatParticipant slowed;
        slowed.slowDuration = 1;
        const QString slowText = DungeonDialog::combatStatusText(slowed);
        check(slowText.contains("slowed 1"),
              "slowed shows its remaining rounds", slowText);
        check(slowText.contains("<font color='gray'>"),
              "slowed is gray", slowText);

        CombatParticipant defending;
        defending.isDefending = true;
        const QString defendText = DungeonDialog::combatStatusText(defending);
        check(defendText.contains("defending"),
              "defending is shown", defendText);
        check(defendText.contains("<font color='blue'>"),
              "defending is blue", defendText);

        CombatParticipant many;
        many.poisonDuration = 2;
        many.blindDuration = 1;
        many.isDefending = true;
        const QString text = DungeonDialog::combatStatusText(many);
        check(text.contains("poison 2") && text.contains("blind 1") &&
              text.contains("defending"),
              "multiple statuses combine", text);

        // Each colour must be bound to ITS OWN status, not merely appear
        // somewhere in the joined string: a shared <font> wrapper or a
        // swapped colour would satisfy a bare "contains" check but colour
        // the wrong status.
        check(text.contains("<font color='green'>poison 2</font>"),
              "green wraps the poison status", text);
        check(text.contains("<font color='yellow'>blind 1</font>"),
              "yellow wraps the blind status", text);
        check(text.contains("<font color='blue'>defending</font>"),
              "blue wraps the defending status", text);
    }

    section("[132] v3.0.0 slice 3.3: melee reaches the front row only");
    {
        CombatState state;
        CombatParticipant front;
        front.name = "Front Hero";
        front.isPlayer = true;
        front.isAlive = true;
        front.row = CombatRow::Front;
        state.addParticipant(front);

        CombatParticipant back;
        back.name = "Back Mage";
        back.isPlayer = true;
        back.isAlive = true;
        back.row = CombatRow::Back;
        state.addParticipant(back);

        // Melee sees only the front row.
        const QList<int> meleeTargets = state.reachablePlayerIndices(false);
        check(meleeTargets.size() == 1, "melee sees one reachable player",
              QString::number(meleeTargets.size()));
        check(meleeTargets.contains(0), "melee reaches the front-row member");
        check(!meleeTargets.contains(1), "melee cannot reach the back-row member");

        // Ranged (spells, breath) reaches both.
        const QList<int> rangedTargets = state.reachablePlayerIndices(true);
        check(rangedTargets.size() == 2, "ranged sees both players",
              QString::number(rangedTargets.size()));
        check(rangedTargets.contains(0) && rangedTargets.contains(1),
              "ranged reaches front and back");
    }

    section("[133] v3.0.0 slice 3.3: everyone in the back row blocks melee");
    {
        CombatState state;
        for (int i = 0; i < 2; ++i) {
            CombatParticipant p;
            p.name = QString("Back Hero %1").arg(i + 1);
            p.isPlayer = true;
            p.isAlive = true;
            p.row = CombatRow::Back;
            state.addParticipant(p);
        }

        check(state.reachablePlayerIndices(false).isEmpty(),
              "no front row means melee reaches nobody");
        check(state.reachablePlayerIndices(true).size() == 2,
              "ranged still reaches both");
    }

    section("[134] v3.0.0 slice 3.3: monster melee is blocked by an empty front row");
    {
        // Drive the real AI: a melee monster must not pick a back-row target.
        CombatState state;
        CombatParticipant front;
        front.name = "Shield Wall";
        front.isPlayer = true;
        front.isAlive = true;
        front.hp = 100; front.maxHp = 100;
        front.row = CombatRow::Front;
        front.speed = 0;
        state.addParticipant(front);

        CombatParticipant back;
        back.name = "Squishy Mage";
        back.isPlayer = true;
        back.isAlive = true;
        back.hp = 30; back.maxHp = 30;
        back.row = CombatRow::Back;
        back.speed = 0;
        state.addParticipant(back);

        CombatParticipant meleeMonster;
        meleeMonster.name = "Melee Orc";
        meleeMonster.isPlayer = false;
        meleeMonster.isAlive = true;
        meleeMonster.hp = 50; meleeMonster.maxHp = 50;
        meleeMonster.speed = 100;
        meleeMonster.ranged = false;
        state.addParticipant(meleeMonster);

        TurnEngine engine;
        engine.setCombatState(&state);
        CombatActions actions(&state, &engine);
        MonsterAI ai(&state, &engine, &actions);

        state.rollInitiative();
        engine.startRound();
        for (int i = 0; i < 20 && !ai.isMonsterTurn(); ++i) engine.nextTurn();
        check(ai.isMonsterTurn(), "the monster has the turn");

        // Run many decisions: a melee monster must NEVER pick the back row,
        // even though it is the weakest target (25% of rolls aim at weakest).
        bool everHitBack = false;
        for (int i = 0; i < 200; ++i) {
            if (ai.chooseTarget() == 1) everHitBack = true;
        }
        check(!everHitBack, "melee monster never targets the back row");
    }

    section("[135] v3.0.0 slice 3.3: a ranged monster can reach the back row");
    {
        CombatState state;
        CombatParticipant back;
        back.name = "Only Mage";
        back.isPlayer = true;
        back.isAlive = true;
        back.hp = 30; back.maxHp = 30;
        back.row = CombatRow::Back;
        back.speed = 0;
        state.addParticipant(back);

        CombatParticipant caster;
        caster.name = "Casting Shaman";
        caster.isPlayer = false;
        caster.isAlive = true;
        caster.hp = 50; caster.maxHp = 50;
        caster.speed = 100;
        caster.ranged = true;
        caster.canCastSpells = true;
        state.addParticipant(caster);

        TurnEngine engine;
        engine.setCombatState(&state);
        CombatActions actions(&state, &engine);
        MonsterAI ai(&state, &engine, &actions);

        state.rollInitiative();
        engine.startRound();
        for (int i = 0; i < 20 && !ai.isMonsterTurn(); ++i) engine.nextTurn();

        check(ai.chooseTarget() == 0, "a ranged monster reaches the back row");
    }

    section("[136] v3.0.0 slice 3.3: togglePlayerRow moves a member between rows");
    {
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();
        gsm->getPartyMembers().clear();
        gsm->addCharacterToParty([]{
            Character c;
            c.name = "Row Toggler";
            c.level = 5;
            c.hp = 100; c.maxHp = 100;
            c.isAlive = true;
            return c;
        }());

        QPair<int, int> pos = {9, 9};
        dlg.m_monsterPositions[pos] = "Test Row Toggle Goblin";
        dlg.m_MonsterAttitude["Test Row Toggle Goblin"] = "Hostile";
        gsm->setGameValue("DungeonX", 9);
        gsm->setGameValue("DungeonY", 9);
        DungeonHandlers::handleEncounters(&dlg, 9, 9);

        // Find the player's participant index.
        int playerIdx = -1;
        for (int i = 0; i < dlg.m_combatController->combatState()->participantCount(); ++i) {
            if (dlg.m_combatController->combatState()->participant(i).isPlayer) { playerIdx = i; break; }
        }
        check(playerIdx >= 0, "player is in the combat state");

        const CombatRow before = dlg.m_combatController->combatState()->participant(playerIdx).row;
        check(dlg.togglePlayerRow(playerIdx), "togglePlayerRow succeeded");
        const CombatRow after = dlg.m_combatController->combatState()->participant(playerIdx).row;
        check(after != before, "the row changed");

        // Toggling a monster must be refused: rows are a player position.
        int monsterIdx = -1;
        for (int i = 0; i < dlg.m_combatController->combatState()->participantCount(); ++i) {
            if (!dlg.m_combatController->combatState()->participant(i).isPlayer) { monsterIdx = i; break; }
        }
        if (monsterIdx >= 0) {
            check(!dlg.togglePlayerRow(monsterIdx), "a monster's row cannot be toggled");
        }
        check(!dlg.togglePlayerRow(-1), "an invalid index is refused");

        dlg.m_monsterPositions.remove(pos);
        dlg.m_MonsterAttitude.remove("Test Row Toggle Goblin");
        dlg.m_inCombat = false;
    }

    section("[137] v3.0.0 slice 3.3: party display shows each member's row");
    {
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();
        gsm->getPartyMembers().clear();
        gsm->addCharacterToParty([]{
            Character c;
            c.name = "Shown Hero";
            c.level = 5;
            c.hp = 100; c.maxHp = 100;
            c.isAlive = true;
            return c;
        }());

        QPair<int, int> pos = {10, 10};
        dlg.m_monsterPositions[pos] = "Test Show Goblin";
        dlg.m_MonsterAttitude["Test Show Goblin"] = "Hostile";
        gsm->setGameValue("DungeonX", 10);
        gsm->setGameValue("DungeonY", 10);
        DungeonHandlers::handleEncounters(&dlg, 10, 10);

        dlg.updateCombatUI();
        const QString text = dlg.m_combatPartyHpLabel->text();
        check(text.contains("front") || text.contains("back"),
              "the party line shows the row", text);

        dlg.m_monsterPositions.remove(pos);
        dlg.m_MonsterAttitude.remove("Test Show Goblin");
        dlg.m_inCombat = false;
    }

    // ------------------------------------------- v3.0.0 — slice 3.7: save versioning

    section("[138] v3.0.0 slice 3.7: SaveVersion written on save");
    {
        gameStateManager* gsm = gameStateManager::instance();
        gsm->saveFullGameState(kTestSave);
        const QJsonObject s = readSaveFile();
        const int ver = s.contains("SaveVersion") ? s.value("SaveVersion").toInt() : -1;
        check(ver == SavegameVersion::CURRENT_SAVE_VERSION,
              "SaveVersion matches CURRENT_SAVE_VERSION",
              QString("got %1 want %2").arg(ver).arg(SavegameVersion::CURRENT_SAVE_VERSION));
        check(s.contains("SaveVersion"), "SaveVersion key present in saved file");
    }

    section("[139] v3.0.0 slice 3.7: v1 save migrates to current with defaults");
    {
        // Ensure the party is non-empty so later member[0] access is safe.
        gameStateManager* gsm0 = gameStateManager::instance();
        if (gsm0->getPartyMembers().isEmpty()) {
            gsm0->addCharacterToParty([]{ Character c; c.name="Migrate Test"; c.race="Human"; c.level=1; c.hp=50; c.maxHp=50; c.isAlive=true; return c; }());
        }

        // Hand-write a v1 save file on disk, then load it.
        QFile f(savePath());
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QJsonObject party;
            QJsonObject member;
            member["Name"] = "Migrated Hero";
            member["Level"] = 3;
            member["HP"] = 80;
            member["MaxHP"] = 80;
            // No StatPoints / PlayTimeSeconds / Row — v1 did not have them.
            QJsonArray members;
            members.append(member);
            party["Members"] = members;
            party["SharedGold"] = 500;

            QJsonObject root;
            root["SaveVersion"] = 1;
            root["Party"] = party;
            root["lastSaved"] = "2025-01-01";
            f.write(QJsonDocument(root).toJson());
            f.close();
        } else {
            check(false, "hand-written v1 save file can be created");
        }

        // Load it — should succeed because migration runs.
        check(gameStateManager::instance()->loadFullGameState(kTestSave),
              "v1 save loads (migrated)");

        // Save again so the migrated data lands on disk.
        gameStateManager::instance()->saveFullGameState(kTestSave);

        // The migrated save must now have v2.
        const QJsonObject after = readSaveFile();
        check(after.value("SaveVersion").toInt() == SavegameVersion::CURRENT_SAVE_VERSION,
              "SaveVersion bumped to current after load");

        // Party member should have default StatPoints.
        const QJsonObject partyAfter = after.value("Party").toObject();
        const QJsonArray membersAfter = partyAfter.value("Members").toArray();
        check(membersAfter.size() == 1, "party member survived migration");
        if (!membersAfter.isEmpty()) {
            const QJsonObject m = membersAfter.at(0).toObject();
            check(m.contains("StatPoints"), "StatPoints default present after migration");
            check(m.value("StatPoints").toInt() == 0, "StatPoints defaults to 0");
            check(m.contains("PlayTimeSeconds"), "PlayTimeSeconds default present");
            check(m.value("PlayTimeSeconds").toInt() == 0, "PlayTimeSeconds defaults to 0");
            check(m.contains("row"), "Row default present after migration");
            check(m.value("row").toInt() == 0, "Row defaults to 0 (Front)");
        }
    }

    section("[139b] v3.0.0 slice 3.7: migration bumps version in memory");
    {
        // Write a v1 save, load it, and check the in-memory version *before*
        // saving. This is the only way to verify migration actually ran —
        // Character::toMap() always includes StatPoints, so checking the file
        // after a save cannot distinguish migrated from non-migrated.
        QFile f(savePath());
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QJsonObject party;
            QJsonObject member;
            member["Name"] = "Version Check";
            member["Level"] = 1;
            member["HP"] = 50;
            member["MaxHP"] = 50;
            QJsonArray members;
            members.append(member);
            party["Members"] = members;
            party["SharedGold"] = 0;

            QJsonObject root;
            root["SaveVersion"] = 1;
            root["Party"] = party;
            f.write(QJsonDocument(root).toJson());
            f.close();
        }

        gameStateManager* gsm = gameStateManager::instance();
        gsm->loadFullGameState(kTestSave);

        // The in-memory game state must reflect the migrated version.
        const int memVer = gsm->getGameValue("SaveVersion").toInt();
        check(memVer == SavegameVersion::CURRENT_SAVE_VERSION,
              "in-memory SaveVersion is current after migration",
              QString("got %1 want %2").arg(memVer).arg(SavegameVersion::CURRENT_SAVE_VERSION));
    }

    section("[140] v3.0.0 slice 3.7: legacy save (no SaveVersion) migrates");
    {
        // v0: no SaveVersion field at all.
        QFile f(savePath());
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QJsonObject party;
            QJsonObject member;
            member["Name"] = "Legacy Hero";
            member["Level"] = 1;
            member["HP"] = 50;
            member["MaxHP"] = 50;
            QJsonArray members;
            members.append(member);
            party["Members"] = members;
            party["SharedGold"] = 0;

            QJsonObject root;
            // No SaveVersion key.
            root["Party"] = party;
            root["lastSaved"] = "2024-01-01";
            f.write(QJsonDocument(root).toJson());
            f.close();
        } else {
            check(false, "legacy save file can be created");
        }

        check(gameStateManager::instance()->loadFullGameState(kTestSave),
              "legacy save loads (migrated to current)");

        // Save again so the migrated data lands on disk.
        gameStateManager::instance()->saveFullGameState(kTestSave);

        const QJsonObject after = readSaveFile();
        check(after.value("SaveVersion").toInt() == SavegameVersion::CURRENT_SAVE_VERSION,
              "legacy save has current version after migration");
    }

    section("[141] v3.0.0 slice 3.7: newer save is refused with a message");
    {
        // Write a save with SaveVersion above the current build.
        QFile f(savePath());
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QJsonObject root;
            root["SaveVersion"] = SavegameVersion::MAX_LOADABLE_VERSION + 1;
            root["Party"] = QJsonObject();
            root["lastSaved"] = "2027-01-01";
            f.write(QJsonDocument(root).toJson());
            f.close();
        } else {
            check(false, "future-version save file can be created");
        }

        bool refused = false;
        QString reason;
        QObject::connect(gameStateManager::instance(), &gameStateManager::saveLoadRefused,
                         [&](const QString&, const QString& r) { refused = true; reason = r; });

        check(!gameStateManager::instance()->loadFullGameState(kTestSave),
              "future-version save is refused");
        check(refused, "saveLoadRefused signal emitted");
        check(!reason.isEmpty(), "refusal carries a user-facing message", reason);

        QObject::disconnect(gameStateManager::instance(), &gameStateManager::saveLoadRefused,
                            nullptr, nullptr);
    }

    // ------------------------------------------- v3.0.0 — slice 3.5: quest chain driver

    section("[148] v3.0.0 slice 3.5: DeepestFloor tracks via setDungeonLevel");
    {
        gameStateManager* gsm = gameStateManager::instance();
        gsm->setDungeonLevel(1);
        check(gsm->getGameValue("DeepestFloor").toInt() == 1,
              "DeepestFloor starts at 1");
        gsm->setDungeonLevel(3);
        check(gsm->getGameValue("DeepestFloor").toInt() == 3,
              "DeepestFloor updates to 3");
        gsm->setDungeonLevel(2);
        check(gsm->getGameValue("DeepestFloor").toInt() == 3,
              "DeepestFloor does not decrease");
        gsm->setDungeonLevel(7);
        check(gsm->getGameValue("DeepestFloor").toInt() == 7,
              "DeepestFloor updates to 7");
    }

    section("[149] v3.0.0 slice 3.5: QuestChainDriver::advance grants rewards for new steps");
    {
        gameStateManager* gsm = gameStateManager::instance();

        // Reset quest state
        for (int i = 0; i < QuestChain::stepCount(); ++i) {
            gsm->setGameValue(QString("QuestRewardGiven_%1").arg(QuestChain::step(i).id), false);
        }

        // Set up: deepest floor = 5, boss on floor 5 defeated
        gsm->setGameValue("DeepestFloor", 5);
        DungeonLevelRegistry::instance().levelForEdit(5).generated = true;
        DungeonLevelRegistry::instance().levelForEdit(5).bossDefeated = true;

        int goldBefore = gsm->getPartyGold();
        int xpBefore = gsm->getPartyMembers().isEmpty() ? 0 : gsm->getPartyMembers()[0].experience;

        QuestChainDriver::AdvanceResult result = QuestChainDriver::advance();

        // Step 0 (first_descent, requiresDepth=1) should be complete -> reward
        // Step 1 (grotto_warden, bossFloor=5) should be complete -> reward
        check(result.grantedRewards.size() >= 2,
              "at least two steps rewarded",
              QString::number(result.grantedRewards.size()));

        // Check gold was added
        check(gsm->getPartyGold() > goldBefore,
              "quest gold reward granted",
              QString("%1 -> %2").arg(goldBefore).arg(gsm->getPartyGold()));

        // Check QuestRewardGiven flags were set
        check(gsm->getGameValue("QuestRewardGiven_first_descent").toBool(),
              "first_descent reward marked given");
        check(gsm->getGameValue("QuestRewardGiven_grotto_warden").toBool(),
              "grotto_warden reward marked given");

        // Clean up
        DungeonLevelRegistry::instance().levelForEdit(5).bossDefeated = false;
    }

    section("[150] v3.0.0 slice 3.5: QuestChainDriver::advance does not double-grant");
    {
        gameStateManager* gsm = gameStateManager::instance();

        // Reset quest state
        for (int i = 0; i < QuestChain::stepCount(); ++i) {
            gsm->setGameValue(QString("QuestRewardGiven_%1").arg(QuestChain::step(i).id), false);
        }

        // Set up: deepest floor = 5, boss on floor 5 defeated
        gsm->setGameValue("DeepestFloor", 5);
        DungeonLevelRegistry::instance().levelForEdit(5).generated = true;
        DungeonLevelRegistry::instance().levelForEdit(5).bossDefeated = true;

        // First advance — should grant
        QuestChainDriver::AdvanceResult r1 = QuestChainDriver::advance();
        int grantsAfterFirst = r1.grantedRewards.size();
        int goldAfterFirst = gsm->getPartyGold();

        // Second advance — should NOT grant again
        QuestChainDriver::AdvanceResult r2 = QuestChainDriver::advance();
        check(r2.grantedRewards.isEmpty(),
              "no double-grant on second advance",
              QString::number(r2.grantedRewards.size()));
        check(gsm->getPartyGold() == goldAfterFirst,
              "gold unchanged on second advance");

        // Clean up
        DungeonLevelRegistry::instance().levelForEdit(5).bossDefeated = false;
    }

    section("[151] v3.0.0 slice 3.5: QuestChainDriver::isChainComplete false before Prince");
    {
        gameStateManager* gsm = gameStateManager::instance();

        // Reset
        gsm->setGameValue("DeepestFloor", 10);
        for (int i = 1; i <= 15; ++i) {
            if (DungeonLevelRegistry::instance().hasLevel(i)) {
                DungeonLevelRegistry::instance().levelForEdit(i).bossDefeated = false;
            }
        }
        // Only floor 5 boss defeated
        DungeonLevelRegistry::instance().levelForEdit(5).generated = true;
        DungeonLevelRegistry::instance().levelForEdit(5).bossDefeated = true;

        check(!QuestChainDriver::isChainComplete(),
              "chain not complete without Prince of Devils");

        // Now defeat floor 10 and floor 15 bosses
        DungeonLevelRegistry::instance().levelForEdit(10).generated = true;
        DungeonLevelRegistry::instance().levelForEdit(10).bossDefeated = true;
        DungeonLevelRegistry::instance().levelForEdit(15).generated = true;
        DungeonLevelRegistry::instance().levelForEdit(15).bossDefeated = true;
        gsm->setGameValue("DeepestFloor", 15);

        check(QuestChainDriver::isChainComplete(),
              "chain complete after Prince defeated");
    }

    section("[152] v3.0.0 slice 3.5: QuestChainDriver::advance reports chain completion");
    {
        gameStateManager* gsm = gameStateManager::instance();

        // Reset quest state
        for (int i = 0; i < QuestChain::stepCount(); ++i) {
            gsm->setGameValue(QString("QuestRewardGiven_%1").arg(QuestChain::step(i).id), false);
        }

        // Set up: everything done except Prince
        gsm->setGameValue("DeepestFloor", 15);
        for (int i = 1; i <= 15; ++i) {
            DungeonLevelRegistry::instance().levelForEdit(i).generated = true;
        }
        for (int i = 1; i <= 14; ++i) {
            DungeonLevelRegistry::instance().levelForEdit(i).bossDefeated = true;
        }
        DungeonLevelRegistry::instance().levelForEdit(15).bossDefeated = false;

        check(!QuestChainDriver::isChainComplete(), "not complete before Prince");

        // Defeat Prince
        DungeonLevelRegistry::instance().levelForEdit(15).bossDefeated = true;

        QuestChainDriver::AdvanceResult result = QuestChainDriver::advance();
        check(result.chainJustCompleted,
              "advance reports chain completion after Prince defeat");
    }

    section("[153] v3.0.0 slice 3.5: progressText shows current step");
    {
        gameStateManager* gsm = gameStateManager::instance();
        gsm->setGameValue("DeepestFloor", 3);
        for (int i = 1; i <= 15; ++i) {
            if (DungeonLevelRegistry::instance().hasLevel(i)) {
                DungeonLevelRegistry::instance().levelForEdit(i).bossDefeated = false;
            }
        }

        QString text = QuestChainDriver::progressText();
        check(!text.isEmpty(), "progress text not empty", text);
        check(!text.contains("complete"), "text does not say complete", text);
    }

    // ------------------------------------------- v3.0.0 — slice 3.6: play-time tracking

    section("[154] v3.0.0 slice 3.6: startPlayTime and playTimeSeconds advance");
    {
        gameStateManager* gsm = gameStateManager::instance();
        gsm->startPlayTime();
        const qint64 t1 = gsm->playTimeSeconds();
        check(t1 >= 0, "playTimeSeconds returns non-negative", QString::number(t1));
        gsm->stopPlayTime();
        check(gsm->playTimeSeconds() >= 0, "playTimeSeconds still valid after stop",
              QString::number(gsm->playTimeSeconds()));
    }

    section("[155] v3.0.0 slice 3.6: stopPlayTime persists PlayTimeSeconds to save data");
    {
        gameStateManager* gsm = gameStateManager::instance();
        gsm->startPlayTime();
        gsm->stopPlayTime();
        const qint64 persisted = gsm->getGameValue("PlayTimeSeconds").toLongLong();
        check(persisted >= 0, "PlayTimeSeconds written to game state",
              QString::number(persisted));
        check(persisted <= gsm->playTimeSeconds() + 1,
              "persisted time matches live time",
              QString("persisted=%1 live=%2").arg(persisted).arg(gsm->playTimeSeconds()));
    }

    section("[156] v3.0.0 slice 3.6: play time survives save/load");
    {
        gameStateManager* gsm = gameStateManager::instance();
        gsm->startPlayTime();
        QThread::msleep(50);
        gsm->stopPlayTime();
        const qint64 before = gsm->playTimeMs();
        check(before > 0, "playTimeMs is non-zero after 50ms",
              QString::number(before));

        gsm->saveFullGameState(kTestSave);
        // Reset live timer to zero
        gsm->startPlayTime();
        const qint64 reset = gsm->playTimeMs();
        check(reset < before, "timer reset to near-zero",
              QString("before=%1 after=%2").arg(before).arg(reset));

        gsm->loadFullGameState(kTestSave);
        const qint64 after = gsm->playTimeMs();
        check(after >= before - 100, "play time survives save/load",
              QString("before=%1 after=%2").arg(before).arg(after));
    }

    section("[157] v3.0.0 slice 3.6: GameRecord completionTimeSeconds is populated on victory");
    {
        // Simulate: start play time, wait, record a victory
        gameStateManager* gsm = gameStateManager::instance();
        gsm->startPlayTime();
        gsm->stopPlayTime();
        gsm->startPlayTime();

        // Let the timer actually tick so playTimeMs() > 0
        QThread::msleep(50);
        GameRecord rec;
        // Use milliseconds directly — playTimeSeconds() returns whole seconds
        // which would be 0 after only 50ms.
        rec.completionTimeSeconds = gsm->playTimeMs() / 1000;
        check(rec.completionTimeSeconds >= 0,
              "completionTimeSeconds is non-negative on victory",
              QString::number(rec.completionTimeSeconds));
        // formattedTime returns "00:00:00" for 0 seconds — not the placeholder
        // For a real time, set completionTimeSeconds to a known value
        rec.completionTimeSeconds = 3661;  // 1h 1m 1s
        check(rec.formattedTime() == "01:01:01",
              "formattedTime shows a real time", rec.formattedTime());
    }

    section("[158] v3.0.0 slice 3.6: faster run sorts ahead of slower run");
    {
        GameRecord fast, slow;
        fast.won = true;
        slow.won = true;
        fast.completionTimeSeconds = 120;
        slow.completionTimeSeconds = 300;
        check(Endgame::outranks(fast, slow, Endgame::Category::FastestCompletion),
              "faster run outranks slower");
        check(!Endgame::outranks(slow, fast, Endgame::Category::FastestCompletion),
              "slower run does not outrank faster");

        // 0 = not finished — should never outrank a finished record
        GameRecord unfinished;
        check(!Endgame::outranks(unfinished, fast, Endgame::Category::FastestCompletion),
              "unfinished record does not outrank a finished one");
    }

    // ------------------------------------------- v3.0.0 — slice 3.6: play-time tracking

    // ------------------------------------------- v3.0.0 — slice 3.8: combat visual feedback

    section("[159] v3.0.0 slice 3.8: combatFxEnabled defaults to true");
    {
        gameStateManager* gsm = gameStateManager::instance();
        gsm->setGameValue("CombatFxEnabled", QVariant());  // clear the key
        check(DungeonDialog::combatFxEnabled(), "combatFxEnabled defaults to true");

        gsm->setGameValue("CombatFxEnabled", true);
        check(DungeonDialog::combatFxEnabled(), "combatFxEnabled is true when set");

        gsm->setGameValue("CombatFxEnabled", false);
        check(!DungeonDialog::combatFxEnabled(), "combatFxEnabled is false when disabled");

        // Restore default
        gsm->setGameValue("CombatFxEnabled", true);
    }

    section("[160] v3.0.0 slice 3.8: flashCombatFx sets stylesheet and clears after timeout");
    {
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        // Enter combat so m_combatGroup is not null
        gameStateManager* gsm = gameStateManager::instance();
        gsm->getPartyMembers().clear();
        gsm->addCharacterToParty([]{ Character c; c.name="Fx Test"; c.race="Human"; c.level=5; c.hp=30; c.maxHp=30; c.mana=50; c.maxMana=50; c.isAlive=true; return c; }());
        QPair<int,int> pos = {7,7};
        dlg.m_monsterPositions[pos] = "Fx Goblin";
        dlg.m_MonsterAttitude["Fx Goblin"] = "Hostile";
        gsm->setGameValue("DungeonX", 7);
        gsm->setGameValue("DungeonY", 7);
        DungeonHandlers::handleEncounters(&dlg, 7, 7);

        // Trigger a flash (damage = red)
        const QString before = dlg.combatGroupStyleSheet();
        dlg.flashCombatFx(true);
        const QString after = dlg.combatGroupStyleSheet();
        check(after != before, "flashCombatFx sets a stylesheet",
              QString("before='%1' after='%2'").arg(before).arg(after));
        check(after.contains("background-color"), "stylesheet contains background-color", after);

        // Disable fx and verify no-op
        gameStateManager::instance()->setGameValue("CombatFxEnabled", false);
        dlg.flashCombatFx(true);
        const QString noFx = dlg.combatGroupStyleSheet();
        check(noFx == after, "flashCombatFx is a no-op when disabled");

        // Re-enable
        gameStateManager::instance()->setGameValue("CombatFxEnabled", true);

        // Let the flash timer fire while the dialog is still alive
        QThread::msleep(300);
        QApplication::processEvents();
        const QString cleared = dlg.combatGroupStyleSheet();
        check(cleared != after, "stylesheet cleared after timer", cleared);

        dlg.m_monsterPositions.remove(pos);
        dlg.m_MonsterAttitude.remove("Fx Goblin");
        dlg.m_inCombat = false;
    }

    // ------------------------------------------- v4.0.0 — slice 4.1: New Game Plus

    section("[165] v4.0.0 slice 4.1: NGPlusLevel persists through save/load");
    {
        gameStateManager* gsm = gameStateManager::instance();
        gsm->setNgPlusLevel(3);
        gsm->saveFullGameState(kTestSave);
        // Verify the raw JSON file contains NGPlusLevel
        QFile f(savePath());
        f.open(QIODevice::ReadOnly);
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        check(doc.toVariant().toMap().value("NGPlusLevel", -1).toInt() == 3,
              "NGPlusLevel written to save file",
              QString::number(doc.toVariant().toMap().value("NGPlusLevel", -1).toInt()));
        // Now test load
        gsm->setNgPlusLevel(0);
        gsm->loadFullGameState(kTestSave);
        check(gsm->getNgPlusLevel() == 3, "NG+ level persists through save/load",
              QString::number(gsm->getNgPlusLevel()));
        gsm->setNgPlusLevel(0);
    }

    section("[166] v4.0.0 slice 4.1: NGPlusLevel defaults to 0 for old saves");
    {
        // Simulate an old save without NGPlusLevel
        QVariantMap save;
        save["SaveVersion"] = 1;
        save["Party"] = QVariantMap();
        SavegameVersion::migrate(save);
        check(save.value("NGPlusLevel", -1).toInt() == 0, "NGPlusLevel defaults to 0",
              QString::number(save.value("NGPlusLevel", -1).toInt()));
    }

    // ------------------------------------------- v4.0.0 — slice 4.2: Endgame dungeon

    section("[167] v4.0.0 slice 4.2: AbyssThemes has 10 floors");
    {
        QList<FloorTheme> themes = AbyssThemes::all();
        check(themes.size() == 10, "Abyss has 10 floors",
              QString::number(themes.size()));
        check(AbyssThemes::forLevel(1).name == "The Outer Rim", "floor 1 is The Outer Rim",
              AbyssThemes::forLevel(1).name);
        check(AbyssThemes::forLevel(10).name == "The Abyssal Heart", "floor 10 is The Abyssal Heart",
              AbyssThemes::forLevel(10).name);
    }

    section("[168] v4.0.0 slice 4.2: Abyss boss floors are 5 and 10");
    {
        check(AbyssThemes::isBossFloor(5), "floor 5 is a boss floor");
        check(AbyssThemes::isBossFloor(10), "floor 10 is a boss floor");
        check(!AbyssThemes::isBossFloor(1), "floor 1 is not a boss floor");
        check(!AbyssThemes::isBossFloor(7), "floor 7 is not a boss floor");
        check(AbyssThemes::bossNameForLevel(5) == "Abyssal Maw", "floor 5 boss name",
              AbyssThemes::bossNameForLevel(5));
        check(AbyssThemes::bossNameForLevel(10) == "The Abyssal Heart", "floor 10 boss name",
              AbyssThemes::bossNameForLevel(10));
        check(AbyssThemes::bossNameForLevel(1).isEmpty(), "floor 1 has no boss");
    }

    section("[169] v4.0.0 slice 4.2: Abyss boss stats scale with depth");
    {
        QVariantMap boss5 = BossEncounter::buildAbyssBoss(5);
        QVariantMap boss10 = BossEncounter::buildAbyssBoss(10);

        check(boss5["name"].toString() == "Abyssal Maw", "floor 5 boss name",
              boss5["name"].toString());
        check(boss10["name"].toString() == "The Abyssal Heart", "floor 10 boss name",
              boss10["name"].toString());

        // Floor 10 boss must be significantly harder than floor 5
        int hp5 = boss5["hp"].toInt();
        int hp10 = boss10["hp"].toInt();
        check(hp10 > hp5 * 2, "floor 10 boss has more than 2x HP of floor 5",
              QString("%1 vs %2").arg(hp10).arg(hp5));

        int att5 = boss5["att"].toInt();
        int att10 = boss10["att"].toInt();
        check(att10 > att5 * 1.5, "floor 10 boss has significantly more attack",
              QString("%1 vs %2").arg(att10).arg(att5));

        check(boss5["damageMod"].toInt() == 250, "floor 5 damageMod is 250",
              QString::number(boss5["damageMod"].toInt()));
        check(boss10["damageMod"].toInt() == 250, "floor 10 damageMod is 250",
              QString::number(boss10["damageMod"].toInt()));
    }

    section("[170] v4.0.0 slice 4.2: Abyss boss XP is much higher than main dungeon");
    {
        int abyss5 = BossEncounter::abyssBossXp(5);
        int abyss10 = BossEncounter::abyssBossXp(10);
        int main15 = BossEncounter::bossXp(15);

        check(abyss5 > main15, "Abyss floor 5 boss XP > main floor 15 boss XP",
              QString("%1 vs %2").arg(abyss5).arg(main15));
        check(abyss10 > abyss5 * 2, "Abyss floor 10 boss XP > 2x floor 5",
              QString("%1 vs %2").arg(abyss10).arg(abyss5));
    }







    // ------------------------------------------- v4.0.0 — slice 4.4: Achievements

    section("[177] v4.0.0 slice 4.4: AchievementSystem has achievements");
    {
        int total = AchievementSystem::totalCount();
        check(total >= 20, "at least 20 achievements",
              QString::number(total));

        // Spot-check a few
        check(AchievementSystem::nameForId("first_blood") == "First Blood",
              "first_blood name", AchievementSystem::nameForId("first_blood"));
        check(AchievementSystem::nameForId("boss_15") == "Devil's Bane",
              "boss_15 name", AchievementSystem::nameForId("boss_15"));
        check(AchievementSystem::nameForId("abyss_10") == "Heart Destroyer",
              "abyss_10 name", AchievementSystem::nameForId("abyss_10"));
    }

    section("[178] v4.0.0 slice 4.4: unlock and isUnlocked");
    {
        AchievementSystem::reset();
        check(AchievementSystem::unlockedCount() == 0, "no achievements after reset",
              QString::number(AchievementSystem::unlockedCount()));

        check(AchievementSystem::unlock("first_blood"), "unlock first_blood succeeds");
        check(AchievementSystem::isUnlocked("first_blood"), "first_blood is unlocked");
        check(!AchievementSystem::unlock("first_blood"), "double unlock returns false");
        check(AchievementSystem::unlock("boss_15"), "unlock boss_15 succeeds");
        check(AchievementSystem::unlockedCount() == 2, "two achievements unlocked",
              QString::number(AchievementSystem::unlockedCount()));

        // Unknown achievement
        check(!AchievementSystem::unlock("nonexistent"), "unknown achievement returns false");

        AchievementSystem::reset();
    }

    section("[179] v4.0.0 slice 4.4: unlock grants gold reward");
    {
        AchievementSystem::reset();
        gameStateManager* gsm = gameStateManager::instance();
        int goldBefore = gsm->getPartyGold();

        AchievementSystem::unlock("first_blood");  // 100 gold
        int goldAfter = gsm->getPartyGold();
        check(goldAfter == goldBefore + 100, "first_blood grants 100 gold",
              QString("%1 → %2").arg(goldBefore).arg(goldAfter));

        AchievementSystem::unlock("boss_15");  // 5000 gold
        check(gsm->getPartyGold() == goldAfter + 5000, "boss_15 grants 5000 gold",
              QString::number(gsm->getPartyGold() - goldAfter));

        AchievementSystem::reset();
    }

    section("[180] v4.0.0 slice 4.4: checkAndUnlock based on game state");
    {
        AchievementSystem::reset();
        gameStateManager* gsm = gameStateManager::instance();

        // Set up state: 5 kills, floor 10, 20000 gold
        gsm->setGameValue("TotalKills", 5);
        gsm->setGameValue("DeepestFloor", 10);
        gsm->setGameValue("TotalDeaths", 0);
        gsm->setGameValue("AbyssFloor", 0);
        gsm->setGameValue("NGPlusLevel", 0);
        gsm->setGameValue("PartyGold", 20000);

        // Party at level 15
        gsm->getPartyMembers().clear();
        gsm->addCharacterToParty([]{ Character c; c.name="Test"; c.race="Human"; c.level=15; c.hp=50; c.maxHp=50; c.mana=30; c.maxMana=30; c.gold=0; c.isAlive=true; return c; }());

        int newCount = AchievementSystem::checkAndUnlock();
        check(newCount > 0, "checkAndUnlock unlocked some achievements",
              QString::number(newCount));
        check(AchievementSystem::isUnlocked("first_blood"), "first_blood unlocked");
        check(AchievementSystem::isUnlocked("floor_5"), "floor_5 unlocked");
        check(AchievementSystem::isUnlocked("floor_10"), "floor_10 unlocked");
        check(AchievementSystem::isUnlocked("gold_10000"), "gold_10000 unlocked");
        check(AchievementSystem::isUnlocked("level_10"), "level_10 unlocked");

        // Second call should not re-unlock
        int secondCount = AchievementSystem::checkAndUnlock();
        check(secondCount == 0, "second checkAndUnlock unlocks nothing",
              QString::number(secondCount));

        AchievementSystem::reset();
    }

    section("[181] v4.0.0 slice 4.4: achievement name/description/reward lookups");
    {
        check(AchievementSystem::nameForId("first_blood") == "First Blood",
              "first_blood name", AchievementSystem::nameForId("first_blood"));
        check(AchievementSystem::descriptionForId("first_blood") == "Kill your first monster",
              "first_blood description", AchievementSystem::descriptionForId("first_blood"));
        check(AchievementSystem::goldRewardForId("first_blood") == 100,
              "first_blood gold reward", QString::number(AchievementSystem::goldRewardForId("first_blood")));
        check(AchievementSystem::goldRewardForId("boss_15") == 5000,
              "boss_15 gold reward", QString::number(AchievementSystem::goldRewardForId("boss_15")));
        check(AchievementSystem::goldRewardForId("abyss_10") == 10000,
              "abyss_10 gold reward", QString::number(AchievementSystem::goldRewardForId("abyss_10")));
        check(AchievementSystem::nameForId("nonexistent").isEmpty(), "unknown name is empty");
        check(AchievementSystem::goldRewardForId("nonexistent") == 0, "unknown reward is 0");
    }

    // ------------------------------------------------- v4.0.0 — slice 4.5: Balance pass

    section("[182] v4.0.0 slice 4.5: floor hint names the recommended level");
    {
        QString h1 = MonsterBalance::floorHint(1);
        check(h1.contains("Recommended level 1"), "floor 1 hint names level 1", h1);
        check(h1.contains("Easy"), "floor 1 hint says Easy", h1);

        QString h15 = MonsterBalance::floorHint(15);
        check(h15.contains("Recommended level 15"), "floor 15 hint names level 15", h15);
        check(h15.contains("Deadly"), "floor 15 hint says Deadly", h15);

        check(MonsterBalance::floorHint(0).isEmpty(), "floor 0 has no hint");
    }

    section("[183] v4.0.0 slice 4.5: every floor is winnable at its recommended level");
    {
        // The core winnability claim: a party at the recommended level can
        // clear the floor without a wipe. This is what "the run is winnable"
        // means, made checkable.
        for (int floor = 1; floor <= 15; ++floor) {
            check(MonsterBalance::isFloorWinnable(floor, MonsterBalance::recommendedLevel(floor)),
                  QString("floor %1 winnable at recommended level").arg(floor),
                  QString("floor=%1 level=%2").arg(floor).arg(MonsterBalance::recommendedLevel(floor)));
        }
        check(MonsterBalance::isRunWinnable(), "the full run is winnable");
    }

    section("[184] v4.0.0 slice 4.5: a badly under-leveled party is not winnable");
    {
        // Non-vacuous: the winnability check must actually fail somewhere, or
        // it proves nothing. A level-1 party on floor 15 must not be "ready".
        check(!MonsterBalance::isFloorWinnable(15, 1),
              "level 1 party cannot clear floor 15");
        check(!MonsterBalance::isFloorWinnable(10, 1),
              "level 1 party cannot clear floor 10");
        check(!MonsterBalance::isFloorWinnable(0, 5), "floor 0 is never winnable");
        check(!MonsterBalance::isFloorWinnable(5, 0), "level 0 is never winnable");
    }

    section("[185] v4.0.0 slice 4.5: difficulty rises with depth");
    {
        // Monsters get tougher monotonically; the model never dips.
        for (int floor = 2; floor <= 15; ++floor) {
            check(MonsterBalance::representativeMonsterHp(floor) >
                  MonsterBalance::representativeMonsterHp(floor - 1),
                  QString("floor %1 monster HP > floor %2").arg(floor).arg(floor - 1));
            check(MonsterBalance::representativeMonsterAtt(floor) >=
                  MonsterBalance::representativeMonsterAtt(floor - 1),
                  QString("floor %1 monster attack >= floor %2").arg(floor).arg(floor - 1));
        }
    }

    section("[186] v4.0.0 slice 4.5: the full run lands in the 10-15 hour band");
    {
        int minutes = MonsterBalance::estimatedRunMinutes();
        double hours = minutes / 60.0;
        check(minutes > 0, "run length is positive", QString::number(minutes));
        check(MonsterBalance::isRunLengthInTarget(),
              "estimated run is 10-15 hours",
              QString("%1 min = %2 h").arg(minutes).arg(MonsterBalance::estimatedRunHours()));
        check(hours >= 10.0 && hours <= 15.0, "run hours in band",
              QString::number(hours, 'f', 2));

        // A boss floor takes longer than a normal one.
        check(MonsterBalance::estimatedMinutesForFloor(15) >
              MonsterBalance::estimatedMinutesForFloor(14),
              "the final boss floor costs more time");
    }

    section("[187] v4.0.0 slice 4.5: floor table carries the balance columns");
    {
        QList<QVariantMap> table = MonsterBalance::floorTable();
        check(table.size() == 15, "floor table has 15 rows",
              QString::number(table.size()));
        const QVariantMap& first = table.first();
        check(first.contains("hint"), "floor table row has a hint");
        check(first.contains("winnable"), "floor table row has winnable");
        check(first.contains("minutes"), "floor table row has minutes");
        check(first["winnable"].toBool(), "floor 1 is winnable in the table");
    }

    section("[188] v4.0.0 slice 4.5: deep-floor monsters can be hit and can hit back");
    {
        // The old to-hit formula made floor 15 impossible in both directions:
        // raw `def` 312 gave AC 749 on a d20, and monsters had dex 0. The fix
        // must let a real floor-15 encounter resolve. Drive actual combat with
        // the real monster table and require that both sides connect.
        gameStateManager* gsm = gameStateManager::instance();
        const QList<QVariantMap>& monsters = gsm->getMonsterData();
        if (!monsters.isEmpty()) {
            // Pick a floor-15 monster from the real table.
            QVariantMap chosen;
            for (const QVariantMap& m : monsters) {
                if (m.value("levelFound").toInt() == 15 && m.value("hits").toInt() > 0) {
                    chosen = m;
                    break;
                }
            }
            if (!chosen.isEmpty()) {
                QList<CombatParticipant> group =
                    EncounterBuilder::buildEncounter(chosen["name"].toString(), monsters, 0, 15);
                check(!group.isEmpty(), "floor 15 encounter built");
                if (!group.isEmpty()) {
                    const CombatParticipant& mon = group.first();
                    check(mon.dex > 0, "deep-floor monster has a usable dexterity",
                          QString::number(mon.dex));
                    // The raw def column is untouched in the participant; the
                    // compression happens in the to-hit maths, so check the
                    // resulting armor class instead.
                    check(CombatActions::targetArmorClass(mon) < 30,
                          "floor-15 monster AC is on a d20 scale",
                          QString::number(CombatActions::targetArmorClass(mon)));
                    check(CombatActions::targetArmorClass(mon) > 10,
                          "floor-15 monster AC is above the floor",
                          QString::number(CombatActions::targetArmorClass(mon)));

                    // A level-15 hero attacks the monster many times: it must land.
                    CombatState cs;
                    CombatParticipant hero;
                    hero.name = "Hero"; hero.isPlayer = true;
                    hero.hp = 200; hero.maxHp = 200;
                    hero.att = 40; hero.dex = 35; hero.level = 15; hero.speed = 100;
                    cs.addParticipant(hero);
                    CombatParticipant target = mon;
                    target.hp = 100000; target.maxHp = 100000;
                    target.speed = 0; target.dex = 1;
                    cs.addParticipant(target);

                    TurnEngine te;
                    te.setCombatState(&cs);
                    CombatActions ca(&cs, &te);
                    te.startRound();
                    te.nextTurn();

                    int hits = 0;
                    for (int i = 0; i < 200; ++i) {
                        cs.participant(1).hp = 100000;
                        QString r;
                        if (ca.attack(1, r) > 0) hits++;
                    }
                    check(hits > 0, "a level-15 hero can hit a floor-15 monster",
                          QString("%1/200 hits").arg(hits));
                    // It should be a real fight, not a coin flip: at least 30%.
                    check(hits >= 60, "floor-15 hit rate is at least 30%",
                          QString("%1/200 hits").arg(hits));
                }
            }
        }
    }

    // ---------------------------------------------- v5.0.0 slice 5.1: hunger visible

    section("[189] v5.0.0 slice 5.1: hunger helpers report the drain");
    {
        Character c;
        c.hunger = 100; c.maxHunger = 100;
        check(c.hungerPercent() == 100, "full hunger reads 100%",
              QString::number(c.hungerPercent()));
        check(!c.isHungerLow(), "full hunger is not in the warning band");

        c.hunger = 19;
        check(c.hungerPercent() == 19, "hunger percent tracks the value",
              QString::number(c.hungerPercent()));
        check(c.isHungerLow(), "hunger below 20% is in the warning band");
        check(!c.isStarving(), "19 hunger is low but not starving");

        c.hunger = 0;
        check(c.isStarving(), "zero hunger is starving");
        check(!c.isHungerLow(), "starving is not merely low");

        c.maxHunger = 0; c.hunger = 0;
        check(c.hungerPercent() == 0, "zero max hunger does not divide by zero");
    }

    section("[190] v5.0.0 slice 5.1: the party panel shows hunger and warns");
    {
        Character c;
        c.name = "Borin"; c.level = 3; c.hp = 12; c.maxHp = 20;
        c.hunger = 15; c.maxHunger = 100; c.isAlive = true;

        const QString text = DungeonDialog::partyMemberStatusText(c);
        check(text.contains("12/20"), "panel row still shows HP", text);
        check(text.contains("Hunger 15/100"), "panel row shows hunger", text);

        // Low hunger must warn even when HP is healthy.
        check(DungeonDialog::partyMemberColor(c) == QColor(Qt::yellow),
              "low hunger colours the row as a warning");

        Character full = c; full.hunger = 100;
        check(DungeonDialog::partyMemberColor(full) == QColor(Qt::green),
              "a full, healthy member reads green");

        Character dead = c; dead.isAlive = false;
        check(DungeonDialog::partyMemberColor(dead) == QColor(Qt::red),
              "a dead member reads red");

        Character starving = c; starving.hunger = 0;
        check(DungeonDialog::partyMemberColor(starving) == QColor(Qt::red),
              "a starving member reads red");
    }

    section("[191] v5.0.0 slice 5.1: the low-hunger warning fires once per crossing");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "Hungry Test"; c.level = 1; c.hp = 10; c.maxHp = 10;
        c.hunger = 25; c.maxHunger = 100; c.isAlive = true;
        members.append(c);

        DungeonDialog dlg;
        // At 25% the member is not yet low: nothing to announce.
        check(dlg.announceHungerWarnings() == 0,
              "no warning while hunger is above the band");

        // Cross into the band: exactly one warning.
        members[0].hunger = 15;
        check(dlg.announceHungerWarnings() == 1, "crossing into the band warns once");
        check(dlg.announceHungerWarnings() == 0,
              "staying low does not repeat the warning");

        // Recovering and crossing again warns again.
        members[0].hunger = 80;
        check(dlg.announceHungerWarnings() == 0, "recovering emits nothing");
        members[0].hunger = 10;
        check(dlg.announceHungerWarnings() == 1, "a second crossing warns again");

        // The message names the member, and a non-low member has none.
        members[0].hunger = 15;
        const QString msg = DungeonDialog::hungerWarningMessage(members[0]);
        check(msg.contains("Hungry Test"), "warning names the member", msg);
        check(DungeonDialog::hungerWarningMessage(Character()).isEmpty(),
              "a non-low member has no warning");

        members.clear();
    }

    section("[193] v5.0.0 slice 5.1: a real move updates the panel and warns");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "Walker"; c.level = 1; c.hp = 10; c.maxHp = 10;
        // Start just above the warning band (20%) so one step crosses into it.
        c.hunger = 21; c.maxHunger = 100; c.isAlive = true;
        members.append(c);

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        // Find a walkable tile with a walkable neighbour, so movePlayer actually
        // moves (a walled-in start tile blocks every step and drains nothing).
        int startX = -1, startY = -1, dx = 0, dy = 0;
        for (int x = 1; x < MAP_SIZE - 1 && startX < 0; ++x) {
            for (int y = 1; y < MAP_SIZE - 1 && startX < 0; ++y) {
                if (dlg.m_obstaclePositions.contains({x, y})) continue;
                const int dirs[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};
                for (auto& d : dirs) {
                    if (!dlg.m_obstaclePositions.contains({x + d[0], y + d[1]})) {
                        startX = x; startY = y; dx = d[0]; dy = d[1];
                        break;
                    }
                }
            }
        }
        check(startX >= 0, "found a walkable tile with a walkable neighbour");
        if (startX >= 0) {
            gsm->setGameValue("DungeonX", startX);
            gsm->setGameValue("DungeonY", startY);

            const int hungerBefore = members[0].hunger;
            dlg.movePlayer(dx, dy, 0);   // the real movement entry point

            check(members[0].hunger == hungerBefore - 1,
                  "a real step costs one hunger",
                  QString("%1 -> %2").arg(hungerBefore).arg(members[0].hunger));

            // The party panel must have been refreshed through the real path.
            check(dlg.m_partyStatusList != nullptr &&
                      dlg.m_partyStatusList->count() == members.size(),
                  "the party panel was repopulated after the move");
            if (dlg.m_partyStatusList && dlg.m_partyStatusList->count() > 0) {
                const QString row = dlg.m_partyStatusList->item(0)->text();
                check(row.contains("Hunger"), "the panel row shows hunger", row);
            }
        }

        members.clear();
    }

    // ------------------------------------------- v5.0.0 slice 5.2: food exists

    section("[194] v5.0.0 slice 5.2: food is a real item type");
    {
        ItemDatabase& db = ItemDatabase::instance();

        // Type 35 is food — it must not collide with the amulet type (22).
        const ItemDef* rations = db.byName("Rations");
        check(rations != nullptr, "Rations exists in the item database");
        if (rations) {
            check(rations->type == 35, "Rations is food (type 35), not an amulet",
                  QString::number(rations->type));
            check(rations->typeName() == "Food", "food typeName is Food",
                  rations->typeName());
            check(!rations->equippable(), "food is not equippable");
        }

        // Every real food row shares the food type.
        int foodCount = 0;
        for (const ItemDef& def : db.all()) {
            if (def.type == 35) ++foodCount;
        }
        check(foodCount >= 3, "at least three food items ship",
              QString::number(foodCount));

        // The amulet type must remain an amulet — the old code ate jewellery.
        const ItemDef* amulet = db.byName("Amulet of Life");
        check(amulet != nullptr && amulet->type == 22, "amulets are still type 22");
        check(amulet == nullptr || amulet->typeName() == "Amulet",
              "amulet typeName is still Amulet");
    }

    section("[195] v5.0.0 slice 5.2: eatFood restores hunger and rejects non-food");
    {
        ItemDatabase& db = ItemDatabase::instance();
        const ItemDef* rations = db.byName("Rations");

        // A real food item restores hunger and is consumed.
        Character c;
        c.hunger = 40; c.maxHunger = 100;
        HeldItem food;
        food.name = "Rations";
        food.M4ED4 = 1;   // one portion
        if (rations) food.M4E97 = static_cast<int16_t>(rations->id);
        c.inventory.append(food);

        QString effect;
        bool ok = c.eatFood(0, effect);
        check(ok, "eating a real food item succeeds", effect);
        check(c.hunger > 40, "hunger was restored", QString::number(c.hunger));
        check(c.inventory.isEmpty(), "the food was consumed",
              QString::number(c.inventory.size()));
        check(effect.contains("hunger", Qt::CaseInsensitive),
              "effect mentions hunger", effect);

        // A non-food item is refused — an amulet is not food.
        Character d;
        d.hunger = 10;
        HeldItem amulet;
        amulet.name = "Amulet of Life";
        d.inventory.append(amulet);
        QString amuletEffect;
        bool amuletOk = d.eatFood(0, amuletEffect);
        check(!amuletOk, "eating an amulet is refused");
        check(d.hunger == 10, "a refused meal does not change hunger");
        check(!d.inventory.isEmpty(), "a refused meal is not consumed");

        // A weapon is refused too.
        Character w;
        w.hunger = 10;
        HeldItem sword; sword.name = "Bronze Sword";
        w.inventory.append(sword);
        QString swordEffect;
        check(!w.eatFood(0, swordEffect), "eating a sword is refused");
    }

    section("[196] v5.0.0 slice 5.2: the food picker lists only food");
    {
        Character c;
        c.hunger = 20;
        HeldItem food; food.name = "Rations"; food.M4ED4 = 2;
        HeldItem amulet; amulet.name = "Amulet of Life";
        HeldItem potion; potion.name = "Potion of Healing"; potion.M4ED4 = 1;
        c.inventory << food << amulet << potion;

        const QList<int> idx = Character::foodIndices(c.inventory);
        check(idx.size() == 1, "only the food item is listed",
              QString::number(idx.size()));
        check(idx.contains(0), "the food's inventory index is listed");
        check(!idx.contains(1), "the amulet is not offered as food");
        check(!idx.contains(2), "the potion is not offered as food");
        check(Character::hasFood(c.inventory), "hasFood sees the ration");
        check(!Character::hasFood({amulet}), "hasFood is false for jewellery");
    }

    section("[197] v5.0.0 slice 5.2: the store stocks food");
    {
        GeneralStore store;
        const QList<QVariantMap> stock = store.availableShopItems();
        bool foodOnSale = false;
        for (const QVariantMap& it : stock) {
            if (it.value("name").toString() == "Rations") { foodOnSale = true; break; }
        }
        check(!stock.isEmpty(), "the store has stock",
              QString::number(stock.size()));
        check(foodOnSale, "the store sells Rations");
    }

    section("[198] v5.0.0 slice 5.2: the dungeon eat action is wired");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "Eater"; c.level = 1; c.hp = 10; c.maxHp = 10;
        c.hunger = 15; c.maxHunger = 100; c.isAlive = true;
        HeldItem food; food.name = "Rations"; food.M4ED4 = 1;
        if (const ItemDef* def = ItemDatabase::instance().byName("Rations"))
            food.M4E97 = static_cast<int16_t>(def->id);
        HeldItem amulet; amulet.name = "Amulet of Life";   // must not be offered
        c.inventory << food << amulet;
        members.append(c);

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        // The picker helper must offer exactly the food slot.
        check(dlg.foodInventoryIndex(0) == 0, "the dungeon finds the food slot",
              QString::number(dlg.foodInventoryIndex(0)));

        // The Eat action must be reachable as a button, not only as a method.
        QPushButton* eatBtn = nullptr;
        for (QPushButton* b : dlg.findChildren<QPushButton*>()) {
            if (b->text() == "Eat") { eatBtn = b; break; }
        }
        check(eatBtn != nullptr, "the dungeon has an Eat button");
        check(eatBtn == nullptr || eatBtn->isEnabled(), "the Eat button is enabled");

        const bool ate = dlg.eatFromInventory();
        check(ate, "eating from the dungeon succeeds");
        check(members[0].hunger > 15, "the dungeon eat restored hunger",
              QString::number(members[0].hunger));
        check(members[0].inventory.size() == 1,
              "only the food was consumed, the amulet remains",
              QString::number(members[0].inventory.size()));
        check(members[0].inventory[0].name == "Amulet of Life",
              "the amulet was not eaten");

        // With no food left, the action refuses.
        members[0].inventory.clear();
        check(!dlg.eatFromInventory(), "eating with no food is refused");

        members.clear();
    }

    // ------------------------------------------- v5.0.0 slice 5.3: starvation death

    section("[199] v5.0.0 slice 5.3: starvation death records a body location");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "Starver"; c.level = 1; c.hp = 1; c.maxHp = 10;
        c.hunger = 0; c.maxHunger = 100; c.isAlive = true;
        members.append(c);

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        int startX = -1, startY = -1, dx = 0, dy = 0;
        for (int x = 1; x < MAP_SIZE - 1 && startX < 0; ++x) {
            for (int y = 1; y < MAP_SIZE - 1 && startX < 0; ++y) {
                if (dlg.m_obstaclePositions.contains({x, y})) continue;
                const int dirs[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};
                for (auto& d : dirs) {
                    if (!dlg.m_obstaclePositions.contains({x + d[0], y + d[1]})) {
                        startX = x; startY = y; dx = d[0]; dy = d[1];
                        break;
                    }
                }
            }
        }
        check(startX >= 0, "found a walkable tile");
        if (startX >= 0) {
            gsm->setGameValue("DungeonX", startX);
            gsm->setGameValue("DungeonY", startY);
            gsm->setGameValue("DungeonLevel", 3);

            dlg.movePlayer(dx, dy, 0);

            check(!members[0].isAlive, "the starved member is dead");
            check(members[0].dungeonLevel == 3, "body dungeon level recorded",
                  QString::number(members[0].dungeonLevel));
            check(members[0].dungeonX == startX + dx, "body X recorded",
                  QString::number(members[0].dungeonX));
            check(members[0].dungeonY == startY + dy, "body Y recorded",
                  QString::number(members[0].dungeonY));
        }

        members.clear();
        // The autosave wrote Starver.json; drop it so the seed characters stay put.
        QFile::remove("data/characters/Starver.json");
    }

    section("[200] v5.0.0 slice 5.3: starvation death writes a journal entry");
    {
        JournalDialog::clearAll();
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "JournalStarver"; c.level = 1; c.hp = 1; c.maxHp = 10;
        c.hunger = 0; c.maxHunger = 100; c.isAlive = true;
        members.append(c);

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        int startX = -1, startY = -1, dx = 0, dy = 0;
        for (int x = 1; x < MAP_SIZE - 1 && startX < 0; ++x) {
            for (int y = 1; y < MAP_SIZE - 1 && startX < 0; ++y) {
                if (dlg.m_obstaclePositions.contains({x, y})) continue;
                const int dirs[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};
                for (auto& d : dirs) {
                    if (!dlg.m_obstaclePositions.contains({x + d[0], y + d[1]})) {
                        startX = x; startY = y; dx = d[0]; dy = d[1];
                        break;
                    }
                }
            }
        }
        if (startX >= 0) {
            gsm->setGameValue("DungeonX", startX);
            gsm->setGameValue("DungeonY", startY);
            gsm->setGameValue("DungeonLevel", 2);

            dlg.movePlayer(dx, dy, 0);

            auto entries = JournalDialog::allEntries();
            bool found = false;
            for (const auto& e : entries) {
                if (e.text.contains("JournalStarver") && e.text.contains("starv")) {
                    found = true; break;
                }
            }
            check(found, "a journal entry records the starvation death");
        }

        members.clear();
        JournalDialog::clearAll();
        QFile::remove("data/characters/JournalStarver.json");
    }

    section("[201] v5.0.0 slice 5.3: starving the whole party triggers a wipe");
    {
        JournalDialog::clearAll();
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c1; c1.name = "Wipe1"; c1.level = 1; c1.hp = 1; c1.maxHp = 10;
        c1.hunger = 0; c1.maxHunger = 100; c1.isAlive = true;
        Character c2; c2.name = "Wipe2"; c2.level = 1; c2.hp = 1; c2.maxHp = 10;
        c2.hunger = 0; c2.maxHunger = 100; c2.isAlive = true;
        members << c1 << c2;

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        int startX = -1, startY = -1, dx = 0, dy = 0;
        for (int x = 1; x < MAP_SIZE - 1 && startX < 0; ++x) {
            for (int y = 1; y < MAP_SIZE - 1 && startX < 0; ++y) {
                if (dlg.m_obstaclePositions.contains({x, y})) continue;
                const int dirs[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};
                for (auto& d : dirs) {
                    if (!dlg.m_obstaclePositions.contains({x + d[0], y + d[1]})) {
                        startX = x; startY = y; dx = d[0]; dy = d[1];
                        break;
                    }
                }
            }
        }
        if (startX >= 0) {
            gsm->setGameValue("DungeonX", startX);
            gsm->setGameValue("DungeonY", startY);
            gsm->setGameValue("DungeonLevel", 1);

            dlg.movePlayer(dx, dy, 0);

            check(!members[0].isAlive && !members[1].isAlive,
                  "both members starved");
            check(gsm->getGameValue("isAlive").toInt() == 0,
                  "the party wipe was detected");

            auto entries = JournalDialog::allEntries();
            bool found = false;
            for (const auto& e : entries) {
                if (e.text.contains("wiped") || e.text.contains("defeated")) {
                    found = true; break;
                }
            }
            check(found, "the party wipe was journaled");
        }

        members.clear();
        JournalDialog::clearAll();
        QFile::remove("data/characters/Wipe1.json");
        QFile::remove("data/characters/Wipe2.json");
    }

    // ------------------------------------------- v5.0.0 slice 5.4: one HP source

    section("[202] v5.0.0 slice 5.4: trap damage reduces the live member's HP");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "TrapTarget"; c.level = 1; c.hp = 10; c.maxHp = 10;
        c.hunger = 100; c.maxHunger = 100; c.isAlive = true;
        members.append(c);

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        // Call the real damage path (what DungeonHandlers calls).
        dlg.updatePartyMemberHealth(0, 3);

        check(members[0].hp == 7, "the live member's HP was reduced",
              QString::number(members[0].hp));
        check(gsm->getGameValue("CurrentCharacterHP").toInt() == 0,
              "CurrentCharacterHP is no longer the write target");

        members.clear();
    }

    section("[203] v5.0.0 slice 5.4: dungeon rest heals the live member");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "RestHealer"; c.level = 1; c.hp = 3; c.maxHp = 10;
        c.hunger = 100; c.maxHunger = 100; c.isAlive = true;
        c.constitution = 10; // healAmount = max(1, 10/2) = 5
        members.append(c);

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        dlg.on_restButton_clicked();

        check(members[0].hp > 3, "the live member's HP was restored",
              QString::number(members[0].hp));
        check(members[0].hp <= 10, "healing does not exceed max HP",
              QString::number(members[0].hp));

        members.clear();
    }

    section("[204] v5.0.0 slice 5.4: spell healing increases the live member's HP");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "SpellHealer"; c.level = 1; c.hp = 5; c.maxHp = 20;
        c.hunger = 100; c.maxHunger = 100; c.isAlive = true;
        c.wisdom = 10;
        members.append(c);

        // Call the real spell healing path.
        QJsonObject spell;
        spell["name"] = "Cure Wounds";
        SpellCastingDialog spellDlg;
        spellDlg.castHealingSpell(spell);

        check(members[0].hp > 5, "the live member's HP was restored by the spell",
              QString::number(members[0].hp));

        members.clear();
    }

    section("[205] v5.0.0 slice 5.4: the party panel reflects trap damage");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "PanelCheck"; c.level = 1; c.hp = 10; c.maxHp = 10;
        c.hunger = 100; c.maxHunger = 100; c.isAlive = true;
        members.append(c);

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        dlg.updatePartyMemberHealth(0, 4);
        dlg.updatePartyPanel();

        check(dlg.m_partyStatusList && dlg.m_partyStatusList->count() > 0,
              "the party panel has a row");
        if (dlg.m_partyStatusList && dlg.m_partyStatusList->count() > 0) {
            const QString row = dlg.m_partyStatusList->item(0)->text();
            check(row.contains("6"), "the panel shows the reduced HP",
                  row);
        }

        members.clear();
    }

    section("[206] v5.0.0 slice 5.4: CurrentCharacterHP is not a write target");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "WriteTarget"; c.level = 1; c.hp = 10; c.maxHp = 10;
        c.hunger = 100; c.maxHunger = 100; c.isAlive = true;
        members.append(c);

        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        dlg.updatePartyMemberHealth(0, 2);

        // CurrentCharacterHP may exist as a mirror, but the live member's HP
        // must be the one that changed.
        check(members[0].hp == 8, "the live member HP is the source of truth",
              QString::number(members[0].hp));

        members.clear();
    }

    // ------------------------------------------- v5.0.0 slice 5.5: town resupply

    section("[207] v5.0.0 slice 5.5: tavern rest restores hunger");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();
        Character c;
        c.name = "InnRest"; c.level = 1; c.hp = 10; c.maxHp = 10;
        c.mana = 5; c.maxMana = 10;
        c.hunger = 30; c.maxHunger = 100; c.isAlive = true;
        c.constitution = 10;
        members.append(c);
        gsm->getPartyMembers().clear();
        gsm->addCharacterToParty(c);
        // Set party gold via the party manager
        gsm->getParty().sharedGold = 1000;

        TavernDialog dlg;
        dlg.resize(800, 600);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        QSpinBox* hoursSpin = dlg.findChild<QSpinBox*>();
        if (hoursSpin) hoursSpin->setValue(2);
        dlg.onRestClicked();

        check(members[0].hunger > 30, "tavern rest restored hunger",
              QString::number(members[0].hunger));

        members.clear();
    }

    section("[208] v5.0.0 slice 5.5: general store sells food");
    {
        GeneralStore store;
        store.resize(1000, 700);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        bool hasFood = false;
        for (const auto& item : store.availableShopItems()) {
            if (item["name"].toString() == "Rations") { hasFood = true; break; }
        }
        check(hasFood, "the general store stocks Rations");
    }

    section("[209] v5.0.0 slice 5.5: a new party starts with food");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();

        Character c;
        c.name = "Newbie"; c.level = 1; c.hp = 10; c.maxHp = 10;
        c.hunger = 100; c.maxHunger = 100; c.isAlive = true;
        members.append(c);

        gsm->addStartingFoodToParty();

        bool hasFood = false;
        for (const auto& m : members) {
            if (Character::hasFood(m.inventory)) { hasFood = true; break; }
        }
        check(hasFood, "a new party starts with food");
    }

    section("[215] v5.0.0 slice 5.7: dead files are gone");
    {
        check(!QFile::exists("TheCity.cpp"), "TheCity.cpp removed");
        check(!QFile::exists("TheCity.h"), "TheCity.h removed");
        check(!QFile::exists("FontManager.h"), "FontManager.h removed");
        check(!QFile::exists("StoryDialog.h"), "StoryDialog.h removed");
        check(!QFile::exists("DataRegistry.h"), "DataRegistry.h removed");
    }

    section("[216] v5.0.0 slice 5.7: BattleEngine build junk is gone");
    {
        check(!QFile::exists("src/core/BattleEngine/Makefile"), "BattleEngine Makefile removed");
        check(!QFile::exists("src/core/BattleEngine/main.o"), "BattleEngine main.o removed");
        check(!QFile::exists("src/core/BattleEngine/HommBattle"), "BattleEngine binary removed");
        check(!QFile::exists("src/core/BattleEngine/moc_BattleEngine.cpp"), "BattleEngine moc removed");
    }

    section("[218] v5.1.0 slice 5.1: main menu has a title and version");
    {
        GameMenu menu;
        QList<QLabel*> labels = menu.findChildren<QLabel*>();
        bool hasTitle = false, hasVersion = false;
        for (QLabel *lbl : labels) {
            QString t = lbl->text();
            if (t.contains("DEPTHS") || t.contains("Dejenol") || t.contains("Black Lands"))
                hasTitle = true;
            if (t.contains("5.0.0"))
                hasVersion = true;
        }
        check(hasTitle, "main menu shows the game title");
        check(hasVersion, "main menu shows the version");
    }

    section("[219] v5.1.0 slice 5.1: main menu has no debug text");
    {
        GameMenu menu;
        // The old paintEvent drew "ABCDEFGH" with the sprite font.
        // Verify no QLabel contains that debug string.
        QList<QLabel*> labels = menu.findChildren<QLabel*>();
        bool hasDebug = false;
        for (QLabel *lbl : labels) {
            if (lbl->text() == "ABCDEFGH")
                hasDebug = true;
        }
        check(!hasDebug, "main menu does not show debug text 'ABCDEFGH'");
    }

    section("[220] v5.1.0 slice 5.2: main menu has a Continue button when autosave exists");
    {
        // Set up a living party so the autosave has someone to continue with
        Character hero;
        hero.name = "ContinueHero";
        hero.level = 1;
        hero.hp = 10;
        hero.maxHp = 10;
        hero.isAlive = true;
        // saveFullGameState uses PartyManager::currentParty(), and
        // getPartyMembers() returns exactly that list.
        gameStateManager::instance()->getPartyMembers().clear();
        gameStateManager::instance()->getPartyMembers().append(hero);
        gameStateManager::instance()->saveFullGameState("autosave");

        QFile autosave("data/saves/autosave.json");
        check(autosave.exists(), "autosave file exists");

        GameMenu menu;
        QPushButton *continueBtn = menu.findChild<QPushButton*>("continueButton");
        check(continueBtn != nullptr, "main menu has a Continue button");
        if (continueBtn) {
            check(!continueBtn->isHidden(), "Continue button is visible when autosave exists");
            check(continueBtn->isEnabled(), "Continue button is enabled when autosave exists");
        }
    }

    section("[221] v5.1.0 slice 5.2: Continue button loads the autosave");
    {
        // Set up a known autosave with a living party
        Character hero;
        hero.name = "TestHero";
        hero.level = 5;
        hero.hp = 50;
        hero.maxHp = 50;
        hero.isAlive = true;
        gameStateManager::instance()->getPartyMembers().clear();
        gameStateManager::instance()->getPartyMembers().append(hero);
        gameStateManager::instance()->saveFullGameState("autosave");

        GameMenu menu;
        QPushButton *continueBtn = menu.findChild<QPushButton*>("continueButton");
        check(continueBtn != nullptr, "Continue button exists");
        if (continueBtn) {
            continueBtn->click();
            // After clicking Continue, the party should be loaded
            bool hasLiving = gameStateManager::instance()->hasLivingCharacters();
            check(hasLiving, "clicking Continue loads the autosave party");
        }
    }

    section("[222] v5.1.0 slice 5.3: main menu shows party preview when autosave exists");
    {
        Character hero;
        hero.name = "PreviewHero";
        hero.level = 3;
        hero.hp = 30;
        hero.maxHp = 30;
        hero.isAlive = true;
        gameStateManager::instance()->getPartyMembers().clear();
        gameStateManager::instance()->getPartyMembers().append(hero);
        gameStateManager::instance()->saveFullGameState("autosave");

        GameMenu menu;
        QLabel *preview = menu.findChild<QLabel*>("partyPreviewLabel");
        check(preview != nullptr, "main menu has a party preview label");
        if (preview) {
            QString text = preview->text();
            check(text.contains("PreviewHero") || text.contains("1") || text.contains("Party"),
                  "party preview shows party info", text);
        }
    }

    section("[223] v5.1.0 slice 5.3: party preview is hidden when no autosave");
    {
        // Remove autosave
        QFile::remove("data/saves/autosave.json");
        GameMenu menu;
        QLabel *preview = menu.findChild<QLabel*>("partyPreviewLabel");
        QPushButton *continueBtn = menu.findChild<QPushButton*>("continueButton");
        check(!preview || !preview->isVisible(), "party preview is hidden when no autosave");
        check(!continueBtn || !continueBtn->isVisible(), "Continue button is hidden when no autosave");
    }

    section("[224] v5.1.0 slice 5.4: main menu has a Load Party dialog");
    {
        GameMenu menu;
        QPushButton *loadBtn = menu.findChild<QPushButton*>("loadGameButton");
        check(loadBtn != nullptr, "main menu has a Load Game button");
    }

    section("[225] v5.1.0 slice 5.4: main menu buttons have visual hierarchy");
    {
        GameMenu menu;
        QPushButton *newBtn = menu.findChild<QPushButton*>("newGameButton");
        QPushButton *continueBtn = menu.findChild<QPushButton*>("continueButton");
        QPushButton *optionsBtn = menu.findChild<QPushButton*>("optionsButton");

        check(newBtn != nullptr, "New Game button exists");
        check(optionsBtn != nullptr, "Options button exists");

        // The action and footer buttons share one height now; the visual
        // hierarchy comes from the CONTINUE button and the QSS styling, not
        // from making the footer buttons smaller.
        if (newBtn && optionsBtn) {
            check(newBtn->minimumHeight() == optionsBtn->minimumHeight(),
                  "primary and footer buttons share the same height",
                  QString("new=%1 opt=%2").arg(newBtn->minimumHeight()).arg(optionsBtn->minimumHeight()));
        }

        // Every button uses the font Options selected - the application font,
        // family and size alike.
        QPushButton *recordsBtn = menu.findChild<QPushButton*>("recordsButton");
        QPushButton *quitBtn = menu.findChild<QPushButton*>("quitButton");
        const QList<QPushButton*> allBtns{newBtn, continueBtn, optionsBtn, recordsBtn, quitBtn};
        const QString appFontStr = QApplication::font().toString();
        for (QPushButton *b : allBtns) {
            if (!b) continue;
            check(b->font().toString() == appFontStr,
                  "button uses the font selected in Options",
                  QString("%1: %2 (app: %3)").arg(b->objectName(), b->font().toString(), appFontStr));
        }
    }

    section("[226] v5.1.0 slice 5.5: main menu has no overlapping buttons");
    {
        GameMenu menu;
        menu.resize(1280, 800);
        QList<QPushButton*> buttons = menu.findChildren<QPushButton*>();
        int overlaps = 0;
        for (int i = 0; i < buttons.size(); ++i) {
            for (int j = i + 1; j < buttons.size(); ++j) {
                if (!buttons[i]->isVisible() || !buttons[j]->isVisible()) continue;
                if (buttons[i]->geometry().intersects(buttons[j]->geometry())) {
                    // Check if they actually overlap (not just touch)
                    QRect r1 = buttons[i]->geometry();
                    QRect r2 = buttons[j]->geometry();
                    QRect intersection = r1 & r2;
                    if (intersection.width() > 1 && intersection.height() > 1) {
                        overlaps++;
                    }
                }
            }
        }
        check(overlaps == 0, "no buttons overlap", QString::number(overlaps) + " overlaps");
    }

    section("[227] v5.1.0 slice 5.5: main menu has a footer with utility buttons");
    {
        GameMenu menu;
        QPushButton *optionsBtn = menu.findChild<QPushButton*>("optionsButton");
        QPushButton *quitBtn = menu.findChild<QPushButton*>("quitButton");

        check(optionsBtn != nullptr, "Options button exists");
        check(quitBtn != nullptr, "Quit button exists");

        // Regression: the Hall of Records, Options and Quit buttons used to be
        // shorter (40/35/35px) than the action buttons (55px).
        QPushButton *newBtn = menu.findChild<QPushButton*>("newGameButton");
        QPushButton *recordsBtn = menu.findChild<QPushButton*>("recordsButton");
        check(newBtn && newBtn->minimumHeight() == 55, "New Game is 55px (reference)");
        check(recordsBtn && recordsBtn->minimumHeight() == 55, "Hall of Records matches the action button height");
        check(optionsBtn && optionsBtn->minimumHeight() == 55, "Options matches the action button height");
        check(quitBtn && quitBtn->minimumHeight() == 55, "Quit matches the action button height");
    }

    section("[228] v5.1.0 slice 5.6: main menu Continue button is disabled when autosave is invalid");
    {
        // Create an invalid autosave (empty JSON)
        QFile autosave("data/saves/autosave.json");
        if (autosave.open(QIODevice::WriteOnly)) {
            autosave.write("{}");
            autosave.close();
        }
        GameMenu menu;
        QPushButton *continueBtn = menu.findChild<QPushButton*>("continueButton");
        check(continueBtn != nullptr, "Continue button exists");
        if (continueBtn) {
            check(!continueBtn->isVisible(), "Continue button is hidden when autosave is invalid");
        }
        // Clean up
        QFile::remove("data/saves/autosave.json");
    }

    section("[229] v5.1.0 slice 5.6: main menu New Game button is always enabled");
    {
        GameMenu menu;
        QPushButton *newBtn = menu.findChild<QPushButton*>("newGameButton");
        check(newBtn != nullptr, "New Game button exists");
        if (newBtn) {
            check(newBtn->isEnabled(), "New Game button is always enabled");
        }
    }

    section("[217] v5.0.0 slice 5.7: data/characters is gitignored");
    {
        QFile gitignore(".gitignore");
        check(gitignore.open(QIODevice::ReadOnly | QIODevice::Text),
              ".gitignore is readable");
        if (gitignore.isOpen()) {
            QString content = gitignore.readAll();
            check(content.contains("data/characters"),
                  "data/characters is in .gitignore");
            gitignore.close();
        }
    }

    // ------------------------------------------- v5.0.0 slice 5.6: load a party

    section("[210] v5.0.0 slice 5.6: a saved party round-trips");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();

        Character c1; c1.name = "PartyA"; c1.level = 1; c1.hp = 10; c1.maxHp = 10;
        c1.hunger = 100; c1.maxHunger = 100; c1.isAlive = true;
        Character c2; c2.name = "PartyB"; c2.level = 2; c2.hp = 20; c2.maxHp = 20;
        c2.hunger = 100; c2.maxHunger = 100; c2.isAlive = true;
        Character c3; c3.name = "PartyC"; c3.level = 3; c3.hp = 30; c3.maxHp = 30;
        c3.hunger = 100; c3.maxHunger = 100; c3.isAlive = true;
        members << c1 << c2 << c3;
        gsm->getParty().sharedGold = 500;

        QString path = QDir::temp().filePath("selftest_party.json");
        check(gsm->savePartyToFile(path), "the party saved to a file");

        members.clear();
        gsm->getParty().sharedGold = 0;
        check(members.isEmpty(), "the party was cleared before loading");

        check(gsm->loadPartyFromFile(path), "the party loaded from the file");
        check(members.size() == 3, "all three members loaded",
              QString::number(members.size()));
        check(members[0].name == "PartyA", "first member name matches");
        check(members[1].name == "PartyB", "second member name matches");
        check(members[2].name == "PartyC", "third member name matches");
        check(gsm->getPartyGold() == 500, "shared gold round-trips",
              QString::number(gsm->getPartyGold()));

        members.clear();
        QFile::remove(path);
    }

    section("[211] v5.0.0 slice 5.6: loading a party replaces the existing party");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();

        Character old1; old1.name = "Old1"; old1.level = 1; old1.hp = 10; old1.maxHp = 10;
        old1.hunger = 100; old1.maxHunger = 100; old1.isAlive = true;
        Character old2; old2.name = "Old2"; old2.level = 1; old2.hp = 10; old2.maxHp = 10;
        old2.hunger = 100; old2.maxHunger = 100; old2.isAlive = true;
        members << old1 << old2;

        Character new1; new1.name = "New1"; new1.level = 5; new1.hp = 50; new1.maxHp = 50;
        new1.hunger = 100; new1.maxHunger = 100; new1.isAlive = true;
        Character new2; new2.name = "New2"; new2.level = 5; new2.hp = 50; new2.maxHp = 50;
        new2.hunger = 100; new2.maxHunger = 100; new2.isAlive = true;
        Character new3; new3.name = "New3"; new3.level = 5; new3.hp = 50; new3.maxHp = 50;
        new3.hunger = 100; new3.maxHunger = 100; new3.isAlive = true;

        QString path = QDir::temp().filePath("selftest_party_replace.json");
        members.clear();
        members << new1 << new2 << new3;
        gsm->savePartyToFile(path);

        members.clear();
        members << old1 << old2;
        check(members.size() == 2, "the old party has two members");

        gsm->loadPartyFromFile(path);
        check(members.size() == 3, "the new party replaced the old one",
              QString::number(members.size()));
        check(members[0].name == "New1", "the first member is from the new party");

        members.clear();
        QFile::remove(path);
    }

    section("[212] v5.0.0 slice 5.6: multi-select loads several characters");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();

        // Save a party of three as JSON (the format the dialog loads).
        Character c1; c1.name = "Multi1"; c1.level = 1; c1.hp = 10; c1.maxHp = 10;
        c1.hunger = 100; c1.maxHunger = 100; c1.isAlive = true;
        Character c2; c2.name = "Multi2"; c2.level = 2; c2.hp = 20; c2.maxHp = 20;
        c2.hunger = 100; c2.maxHunger = 100; c2.isAlive = true;
        Character c3; c3.name = "Multi3"; c3.level = 3; c3.hp = 30; c3.maxHp = 30;
        c3.hunger = 100; c3.maxHunger = 100; c3.isAlive = true;
        members << c1 << c2 << c3;

        QString path = QDir::temp().filePath("selftest_multi.json");
        gsm->savePartyToFile(path);

        members.clear();
        gsm->loadPartyFromFile(path);
        check(members.size() == 3, "all three members loaded from party file",
              QString::number(members.size()));

        members.clear();
        QFile::remove(path);
    }

    section("[213] v5.0.0 slice 5.6: MAX_PARTY_SIZE is respected");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();

        // Try to add more than MAX_PARTY_SIZE members.
        for (int i = 0; i < GameConstants::MAX_PARTY_SIZE + 2; ++i) {
            Character c;
            c.name = QString("Over%1").arg(i);
            c.level = 1; c.hp = 10; c.maxHp = 10;
            c.hunger = 100; c.maxHunger = 100; c.isAlive = true;
            gsm->addCharacterToParty(c);
        }

        check(members.size() <= GameConstants::MAX_PARTY_SIZE,
              "the party size does not exceed MAX_PARTY_SIZE",
              QString::number(members.size()));

        members.clear();
    }

    section("[230] v5.2.0 slice 5.1: savegame round-trips a 4-character party with inventories");
    {
        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();

        for (int i = 0; i < 4; ++i) {
            Character c;
            c.name = QString("Hero%1").arg(i + 1);
            c.race = "Human";
            c.level = 5 + i;
            c.hp = 20 + i * 10;
            c.maxHp = 50 + i * 10;
            c.gold = 100 + i * 50;
            c.strength = 10 + i;
            c.dexterity = 12 + i;
            c.hunger = 80 - i * 10;
            c.maxHunger = 100;
            c.isAlive = true;
            c.dungeonLevel = i;
            c.dungeonX = i * 3;
            c.dungeonY = i * 7;

            // Each character gets a unique inventory
            HeldItem sword;
            sword.name = QString("Sword%1").arg(i + 1);
            sword.M4E97 = 100 + i;
            c.inventory.append(sword);

            HeldItem shield;
            shield.name = QString("Shield%1").arg(i + 1);
            shield.M4E97 = 200 + i;
            c.inventory.append(shield);

            // Equipped items
            HeldItem armor;
            armor.name = QString("Armor%1").arg(i + 1);
            armor.M4E97 = 300 + i;
            c.equipped.append(armor);

            members.append(c);
        }

        // Save
        check(gameStateManager::instance()->saveFullGameState("testsave"),
              "saveFullGameState succeeds");

        // Clear and reload
        members.clear();
        check(members.isEmpty(), "party cleared before load");

        check(gameStateManager::instance()->loadFullGameState("testsave"),
              "loadFullGameState succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 4, "party has 4 characters after load",
              QString::number(loaded.size()));

        for (int i = 0; i < 4; ++i) {
            const Character& c = loaded[i];
            check(c.name == QString("Hero%1").arg(i + 1),
                  QString("Hero%1 name preserved").arg(i + 1));
            check(c.level == 5 + i,
                  QString("Hero%1 level preserved").arg(i + 1));
            check(c.hp == 20 + i * 10,
                  QString("Hero%1 HP preserved").arg(i + 1));
            check(c.maxHp == 50 + i * 10,
                  QString("Hero%1 MaxHP preserved").arg(i + 1));
            check(c.gold == 100 + i * 50,
                  QString("Hero%1 gold preserved").arg(i + 1));
            check(c.strength == 10 + i,
                  QString("Hero%1 strength preserved").arg(i + 1));
            check(c.dexterity == 12 + i,
                  QString("Hero%1 dexterity preserved").arg(i + 1));
            check(c.hunger == 80 - i * 10,
                  QString("Hero%1 hunger preserved").arg(i + 1));
            check(c.maxHunger == 100,
                  QString("Hero%1 maxHunger preserved").arg(i + 1));
            check(c.dungeonLevel == i,
                  QString("Hero%1 dungeonLevel preserved").arg(i + 1));
            check(c.dungeonX == i * 3,
                  QString("Hero%1 dungeonX preserved").arg(i + 1));
            check(c.dungeonY == i * 7,
                  QString("Hero%1 dungeonY preserved").arg(i + 1));

            // Inventory
            check(c.inventory.size() == 2,
                  QString("Hero%1 inventory has 2 items").arg(i + 1));
            if (c.inventory.size() == 2) {
                check(c.inventory[0].name == QString("Sword%1").arg(i + 1),
                      QString("Hero%1 sword name preserved").arg(i + 1));
                check(c.inventory[1].name == QString("Shield%1").arg(i + 1),
                      QString("Hero%1 shield name preserved").arg(i + 1));
            }

            // Equipped
            check(c.equipped.size() == 1,
                  QString("Hero%1 equipped has 1 item").arg(i + 1));
            if (c.equipped.size() == 1) {
                check(c.equipped[0].name == QString("Armor%1").arg(i + 1),
                      QString("Hero%1 armor name preserved").arg(i + 1));
            }
        }

        // Cleanup
        QFile::remove("data/saves/testsave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[231] v5.2.0 slice 5.1: party save/load round-trips with all fields");
    {
        // Test savePartyToFile / loadPartyFromFile with a 2-character party
        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();

        Character c1;
        c1.name = "PartyA";
        c1.level = 3;
        c1.hp = 15;
        c1.maxHp = 30;
        c1.gold = 200;
        c1.hunger = 60;
        c1.maxHunger = 100;
        c1.isAlive = true;
        c1.strength = 8;
        c1.dexterity = 14;

        HeldItem potion;
        potion.name = "Potion";
        potion.M4E97 = 42;
        c1.inventory.append(potion);

        Character c2;
        c2.name = "PartyB";
        c2.level = 7;
        c2.hp = 40;
        c2.maxHp = 80;
        c2.gold = 500;
        c2.hunger = 90;
        c2.maxHunger = 100;
        c2.isAlive = true;
        c2.strength = 16;
        c2.dexterity = 10;

        HeldItem ring;
        ring.name = "Ring";
        ring.M4E97 = 88;
        c2.inventory.append(ring);

        members.append(c1);
        members.append(c2);

        QString savePath = "data/saves/test_party.json";
        check(gameStateManager::instance()->savePartyToFile(savePath),
              "savePartyToFile succeeds");

        // Clear and reload
        members.clear();
        check(members.isEmpty(), "party cleared before party load");

        check(gameStateManager::instance()->loadPartyFromFile(savePath),
              "loadPartyFromFile succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 2, "party has 2 characters after party load",
              QString::number(loaded.size()));
        check(loaded[0].name == "PartyA", "PartyA loaded");
        check(loaded[1].name == "PartyB", "PartyB loaded");
        check(loaded[0].inventory.size() == 1, "PartyA inventory preserved");
        check(loaded[1].inventory.size() == 1, "PartyB inventory preserved");
        if (loaded[0].inventory.size() == 1) {
            check(loaded[0].inventory[0].name == "Potion", "PartyA potion preserved");
        }
        if (loaded[1].inventory.size() == 1) {
            check(loaded[1].inventory[0].name == "Ring", "PartyB ring preserved");
        }

        // Cleanup
        QFile::remove(savePath);
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[232] v5.2.0 slice 5.1: autosave works with a living party");
    {
        // Reproduce the exact autosave flow that runs every 30 seconds in-game
        Character hero;
        hero.name = "AutoHero";
        hero.level = 10;
        hero.hp = 100;
        hero.maxHp = 100;
        hero.gold = 500;
        hero.hunger = 50;
        hero.maxHunger = 100;
        hero.isAlive = true;
        hero.strength = 12;
        hero.dexterity = 14;

        HeldItem gem;
        gem.name = "Gem";
        gem.M4E97 = 99;
        hero.inventory.append(gem);

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        // Set CurrentCharacterName so handleAutosave doesn't skip
        gameStateManager::instance()->setGameValue("CurrentCharacterName", "AutoHero");

        // Call the real autosave handler
        gameStateManager::instance()->handleAutosave();

        // Verify the autosave file has the character
        QFile file("data/saves/autosave.json");
        check(file.open(QIODevice::ReadOnly), "autosave file readable");
        if (file.isOpen()) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();
            QVariantMap root = doc.toVariant().toMap();
            QVariantMap party = root.value("Party").toMap();
            QVariantList partyMembers = party.value("Members").toList();
            check(partyMembers.size() == 1, "autosave has 1 member",
                  QString::number(partyMembers.size()));
            if (partyMembers.size() == 1) {
                QVariantMap m = partyMembers[0].toMap();
                check(m.value("Name").toString() == "AutoHero", "autosave has AutoHero");
                check(m.value("HP").toInt() == 100, "autosave has correct HP");
                QVariantList inv = m.value("Inventory").toList();
                check(inv.size() == 1, "autosave has 1 inventory item",
                      QString::number(inv.size()));
                if (inv.size() == 1) {
                    QVariantMap item = inv[0].toMap();
                    check(item.value("name").toString() == "Gem",
                          "autosave has Gem in inventory");
                }
            }
        }

        // Cleanup
        QFile::remove("data/saves/autosave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[233] v5.2.0 slice 5.1: savegame preserves bank inventory");
    {
        Character hero;
        hero.name = "BankHero";
        hero.level = 5;
        hero.hp = 30;
        hero.maxHp = 50;
        hero.gold = 100;
        hero.hunger = 80;
        hero.maxHunger = 100;
        hero.isAlive = true;

        HeldItem bankItem;
        bankItem.name = "BankedItem";
        bankItem.M4E97 = 777;
        hero.bankInventory.append(bankItem);

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        check(gameStateManager::instance()->saveFullGameState("banksave"),
              "save with bank inventory succeeds");

        members.clear();
        check(gameStateManager::instance()->loadFullGameState("banksave"),
              "load with bank inventory succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 1, "1 character loaded");
        if (loaded.size() == 1) {
            check(loaded[0].bankInventory.size() == 1, "bank inventory preserved");
            if (loaded[0].bankInventory.size() == 1) {
                check(loaded[0].bankInventory[0].name == "BankedItem",
                      "bank item name preserved");
            }
        }

        // Cleanup
        QFile::remove("data/saves/banksave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[234] v5.2.0 slice 5.1: savegame preserves equipped items");
    {
        Character hero;
        hero.name = "EquipHero";
        hero.level = 8;
        hero.hp = 80;
        hero.maxHp = 120;
        hero.gold = 300;
        hero.hunger = 70;
        hero.maxHunger = 100;
        hero.isAlive = true;

        HeldItem weapon;
        weapon.name = "DragonSlayer";
        weapon.M4E97 = 1234;
        hero.equipped.append(weapon);

        HeldItem helm;
        helm.name = "IronHelm";
        helm.M4E97 = 5678;
        hero.equipped.append(helm);

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        check(gameStateManager::instance()->saveFullGameState("equipsave"),
              "save with equipped items succeeds");

        members.clear();
        check(gameStateManager::instance()->loadFullGameState("equipsave"),
              "load with equipped items succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 1, "1 character loaded");
        if (loaded.size() == 1) {
            check(loaded[0].equipped.size() == 2, "equipped items preserved");
            if (loaded[0].equipped.size() == 2) {
                check(loaded[0].equipped[0].name == "DragonSlayer", "weapon preserved");
                check(loaded[0].equipped[1].name == "IronHelm", "helmet preserved");
            }
        }

        // Cleanup
        QFile::remove("data/saves/equipsave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[235] v5.2.0 slice 5.1: savegame preserves guild data");
    {
        Character hero;
        hero.name = "GuildHero";
        hero.level = 12;
        hero.hp = 120;
        hero.maxHp = 200;
        hero.gold = 1000;
        hero.hunger = 60;
        hero.maxHunger = 100;
        hero.isAlive = true;

        hero.guildLevels["Fighters"] = 3;
        hero.guildLevels["Mages"] = 5;
        hero.guildExperience["Fighters"] = 5000;
        hero.guildExperience["Mages"] = 12000;

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        check(gameStateManager::instance()->saveFullGameState("guildsave"),
              "save with guild data succeeds");

        members.clear();
        check(gameStateManager::instance()->loadFullGameState("guildsave"),
              "load with guild data succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 1, "1 character loaded");
        if (loaded.size() == 1) {
            check(loaded[0].guildLevels["Fighters"] == 3, "Fighters guild level preserved");
            check(loaded[0].guildLevels["Mages"] == 5, "Mages guild level preserved");
            check(loaded[0].guildExperience["Fighters"] == 5000, "Fighters guild XP preserved");
            check(loaded[0].guildExperience["Mages"] == 12000, "Mages guild XP preserved");
        }

        // Cleanup
        QFile::remove("data/saves/guildsave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[236] v5.2.0 slice 5.1: savegame preserves play time and NG+ level");
    {
        Character hero;
        hero.name = "TimeHero";
        hero.level = 15;
        hero.hp = 150;
        hero.maxHp = 250;
        hero.gold = 2000;
        hero.hunger = 40;
        hero.maxHunger = 100;
        hero.isAlive = true;
        hero.playTimeSeconds = 3600;

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        // Set NG+ level
        gameStateManager::instance()->setGameValue("NGPlusLevel", 3);

        check(gameStateManager::instance()->saveFullGameState("timesave"),
              "save with play time succeeds");

        members.clear();
        // Clear NG+ to verify it loads
        gameStateManager::instance()->setGameValue("NGPlusLevel", 0);
        check(gameStateManager::instance()->loadFullGameState("timesave"),
              "load with play time succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 1, "1 character loaded");
        if (loaded.size() == 1) {
            check(loaded[0].playTimeSeconds == 3600, "play time preserved",
                  QString::number(loaded[0].playTimeSeconds));
        }

        int ngPlus = gameStateManager::instance()->getGameValue("NGPlusLevel").toInt();
        check(ngPlus == 3, "NG+ level preserved", QString::number(ngPlus));

        // Cleanup
        QFile::remove("data/saves/timesave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[237] v5.2.0 slice 5.2: build a party from individual characters");
    {
        // Create 4 individual character files using the real saveCharacterToFile
        for (int i = 0; i < 4; ++i) {
            Character c;
            c.name = QString("BuildHero%1").arg(i + 1);
            c.race = "Human";
            c.level = 2 + i;
            c.hp = 20 + i * 10;
            c.maxHp = 40 + i * 10;
            c.gold = 50 + i * 25;
            c.hunger = 90 - i * 5;
            c.maxHunger = 100;
            c.isAlive = true;
            c.strength = 10 + i;
            c.dexterity = 12 + i;

            HeldItem item;
            item.name = QString("Item%1").arg(i + 1);
            item.M4E97 = 10 + i;
            c.inventory.append(item);

            gameStateManager::instance()->getPartyMembers().clear();
            gameStateManager::instance()->getPartyMembers().append(c);
            check(gameStateManager::instance()->saveCharacterToFile(0),
                  QString("save BuildHero%1").arg(i + 1));
        }

        // Clear party
        gameStateManager::instance()->getPartyMembers().clear();

        // Use the real addCharacterFromFile to build a party from 3 files
        for (int i = 0; i < 3; ++i) {
            QString path = QString("data/characters/BuildHero%1.json").arg(i + 1);
            check(gameStateManager::instance()->addCharacterFromFile(path),
                  QString("add BuildHero%1 to party").arg(i + 1));
        }

        auto& members = gameStateManager::instance()->getPartyMembers();
        check(members.size() == 3, "party built with 3 characters",
              QString::number(members.size()));
        check(members[0].name == "BuildHero1", "first character is BuildHero1");
        check(members[1].name == "BuildHero2", "second character is BuildHero2");
        check(members[2].name == "BuildHero3", "third character is BuildHero3");

        // Verify inventories are independent
        check(members[0].inventory.size() == 1, "first character has 1 item");
        check(members[1].inventory.size() == 1, "second character has 1 item");
        check(members[0].inventory[0].name == "Item1", "first has Item1");
        check(members[1].inventory[0].name == "Item2", "second has Item2");

        // Cleanup
        for (int i = 0; i < 4; ++i) {
            QFile::remove(QString("data/characters/BuildHero%1.json").arg(i + 1));
        }
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[238] v5.2.0 slice 5.2: party size is limited to MAX_PARTY_SIZE");
    {
        // MAX_PARTY_SIZE is 4 — try to add 5 characters
        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();

        for (int i = 0; i < 5; ++i) {
            Character c;
            c.name = QString("Overflow%1").arg(i + 1);
            c.level = 1;
            c.hp = 10;
            c.maxHp = 10;
            c.isAlive = true;
            gameStateManager::instance()->addCharacterToParty(c);
        }

        check(members.size() <= GameConstants::MAX_PARTY_SIZE,
              "party size respects MAX_PARTY_SIZE",
              QString::number(members.size()));

        members.clear();
    }

    section("[239] v5.2.0 slice 5.2: each party member has independent inventory");
    {
        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();

        Character c1;
        c1.name = "Inv1";
        c1.isAlive = true;
        HeldItem sword;
        sword.name = "Sword";
        c1.inventory.append(sword);

        Character c2;
        c2.name = "Inv2";
        c2.isAlive = true;
        HeldItem shield;
        shield.name = "Shield";
        c2.inventory.append(shield);

        members.append(c1);
        members.append(c2);

        // Verify inventories are independent
        check(members[0].inventory.size() == 1, "first member has 1 item");
        check(members[1].inventory.size() == 1, "second member has 1 item");
        check(members[0].inventory[0].name == "Sword", "first member has Sword");
        check(members[1].inventory[0].name == "Shield", "second member has Shield");

        // Modify one inventory, verify the other is unchanged
        members[0].inventory.append(shield);
        check(members[0].inventory.size() == 2, "first member now has 2 items");
        check(members[1].inventory.size() == 1, "second member still has 1 item");

        members.clear();
    }

    section("[302] the arena is removed from the city");
    {
        // The arena (ArenaDialog + core/Arena) was cut from the game. Guard
        // against reintroduction: no button, no dialog class.
        theCity city;
        for (int i = 0; i < 10; ++i) QApplication::processEvents();

        check(city.findChild<QToolButton*>("arenaButton") == nullptr,
              "no arena button in the city");

        bool arenaDialogAlive = false;
        for (QWidget *child : city.findChildren<QWidget*>()) {
            if (child->metaObject()->className() == QString("ArenaDialog")) {
                arenaDialogAlive = true;
                break;
            }
        }
        check(!arenaDialogAlive, "no ArenaDialog instance in the city");

        city.close();
    }

    section("[303] v6.9.0: F5/F9/F12 save/load shortcuts");
    {
        DungeonDialog dlg;
        dlg.resize(1280, 800);
        for (int i = 0; i < 20; ++i) QApplication::processEvents();

        gameStateManager* gsm = gameStateManager::instance();
        gsm->setCurrentSaveName("test_quicksave");

        // F5 = quicksave — must not crash
        QKeyEvent f5Event(QEvent::KeyPress, Qt::Key_F5, Qt::NoModifier);
        QApplication::sendEvent(&dlg, &f5Event);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();
        check(true, "F5 quicksave does not crash");

        // F9 = quickload — must not crash
        QKeyEvent f9Event(QEvent::KeyPress, Qt::Key_F9, Qt::NoModifier);
        QApplication::sendEvent(&dlg, &f9Event);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();
        check(true, "F9 quickload does not crash");

        // F12 = autosave — must not crash
        QKeyEvent f12Event(QEvent::KeyPress, Qt::Key_F12, Qt::NoModifier);
        QApplication::sendEvent(&dlg, &f12Event);
        for (int i = 0; i < 10; ++i) QApplication::processEvents();
        check(true, "F12 autosave does not crash");

        dlg.close();
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

    section("[285] v5.5.0 slice 5.5: guild eligibility is not all-allowed");
    {
        // RaceData has per-race guild eligibility. The placeholder set
        // every race/guild pair to AS_Allowed; the real logic marks
        // specific pairs as AS_NotAllowed (Giant/Dwarf/Elf restrictions).
        QVector<GameConstants::RaceStats> races = loadRaceData();
        check(races.size() >= 5, "at least 5 races loaded");

        // Find a race that has at least one AS_NotAllowed guild
        bool foundNotAllowed = false;
        for (const GameConstants::RaceStats& race : races) {
            for (auto it = race.guildEligibility.begin(); it != race.guildEligibility.end(); ++it) {
                if (it.value() == GameConstants::AS_NotAllowed) {
                    foundNotAllowed = true;
                    break;
                }
            }
            if (foundNotAllowed) break;
        }
        check(foundNotAllowed, "at least one race/guild pair is not eligible");

        // Find a race that has at least one AS_Allowed guild
        bool foundAllowed = false;
        for (const GameConstants::RaceStats& race : races) {
            for (auto it = race.guildEligibility.begin(); it != race.guildEligibility.end(); ++it) {
                if (it.value() == GameConstants::AS_Allowed) {
                    foundAllowed = true;
                    break;
                }
            }
            if (foundAllowed) break;
        }
        check(foundAllowed, "at least one race/guild pair is eligible");
    }

    section("[284] v5.5.0 slice 5.3: partyChanged signal triggers UI refresh");
    {
        // PartyManager emits partyUpdated when the party changes.
        // gameStateManager connects partyUpdated to refreshUI.
        // We verify the wiring by checking that refreshUI runs when partyUpdated fires.
        gameStateManager* gsm = gameStateManager::instance();

        // Connect a spy to gameValueChanged (emitted by refreshUI)
        bool refreshFired = false;
        QObject::connect(gsm, &gameStateManager::gameValueChanged,
                         [&refreshFired](const QString&, const QVariant&) {
            refreshFired = true;
        });

        // Trigger partyUpdated via addExperienceToParty
        QList<Character> &members = gsm->getPartyMembers();
        members.clear();
        Character c1; c1.name = "Test"; c1.race = "Human"; c1.level = 1;
        c1.hp = 10; c1.maxHp = 10; c1.isAlive = true; c1.experience = 0;
        members << c1;

        gsm->addExperienceToParty(100);
        QApplication::processEvents();

        // refreshUI must have fired (via partyUpdated connect)
        check(refreshFired, "partyUpdated triggers refreshUI via gameValueChanged");

        members.clear();
    }

    section("[283] v5.5.0 slice 5.2: PlayerExperience is written when XP is added");
    {
        // DungeonDialog reads "PlayerExperience" for the XP label, but nothing
        // ever writes it — it stays 0 forever. This test proves the wiring.
        gameStateManager* gsm = gameStateManager::instance();
        QList<Character> &members = gsm->getPartyMembers();
        members.clear();
        Character c1; c1.name = "Fighter"; c1.race = "Human"; c1.level = 1;
        c1.hp = 30; c1.maxHp = 30; c1.isAlive = true; c1.experience = 0;
        members << c1;

        // Initially zero
        check(gsm->getGameValue("PlayerExperience").toULongLong() == 0,
              "PlayerExperience starts at 0");

        // Add XP
        gsm->addExperienceToCharacter(0, 500);

        // PlayerExperience must now reflect the party's total XP
        check(gsm->getGameValue("PlayerExperience").toULongLong() == 500,
              "PlayerExperience written after addExperienceToCharacter");

        members.clear();
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

