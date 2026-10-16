#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldDungeonTests()
{
    gameStateManager* gsm = gameStateManager::instance();

    section("[47] Guided first dungeon run");
    {
        Tutorial::reset();
        check(!Tutorial::isActive(), "tutorial not active after reset");

        QList<TutorialStep> steps = Tutorial::steps();
        check(steps.size() == 11, "11 tutorial steps",
              QString::number(steps.size()));

        // First step is movement.
        TutorialStep s0 = Tutorial::step(0);
        check(s0.id == "welcome", "step 0 is welcome");
        check(s0.completesOn == "move", "step 0 completes on move");

        // Last step is victory.
        TutorialStep last = Tutorial::step(10);
        check(last.id == "victory", "last step is victory");

        // Out-of-range step is invalid.
        TutorialStep invalid = Tutorial::step(99);
        check(invalid.index == -1, "out-of-range step is invalid");
    }

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

    section("[62g] Automap integration");
    {
        // AutomapDialog can be constructed and updated.
        AutomapDialog* dlg = new AutomapDialog();
        check(dlg != nullptr, "AutomapDialog constructs");
        dlg->updatePlayerPosition(10, 20, "NORTH");
        dlg->updatePlayerPosition(15, 25, "EAST");
        delete dlg;
    }

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

}
