#include "selftest.h"
#include "selftest_harness.h"
#include "selftest_includes.h"

void runCoreTests();
void runCombatTests();
void runDungeonTests();
void runProgressionTests();
void runWorldPartyTests();
void runWorldSaveTests();
void runWorldDungeonTests();
void runWorldQuestTests();
void runWorldMonsterTests();
void runWorldSpellTests();
void runWorldItemTests();
void runWorldTownTests();
void runWorldUITests();
void runSpellMechanicTests();
void runOptionsTests();
void runDifficultyTests();

int runSelfTest()
{
    qInstallMessageHandler(quietMessageHandler);

    out("BlackLands self-test");
    out("====================");

    gameStateManager* gsm = gameStateManager::instance();

    runCoreTests();
    runCombatTests();
    runDungeonTests();
    runProgressionTests();
    runWorldPartyTests();
    runWorldSaveTests();
    runWorldDungeonTests();
    runWorldQuestTests();
    runWorldMonsterTests();
    runWorldSpellTests();
    runWorldItemTests();
    runWorldTownTests();
    runWorldUITests();
    runSpellMechanicTests();
    runOptionsTests();
    runDifficultyTests();

    // -------------------------------------------------------------- cleanup
    QFile::remove(savePath());

    // --------------------------------------------------------------- report
    out("");
    out("====================");
    out(QString("%1 passed, %2 failed").arg(g_passed).arg(g_failed));

    qInstallMessageHandler(nullptr);
    return g_failed == 0 ? 0 : 1;
}
