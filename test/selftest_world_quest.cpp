#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldQuestTests()
{
    gameStateManager* gsm = gameStateManager::instance();

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

}
