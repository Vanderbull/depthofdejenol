#include "selftest.h"
#include "selftest_harness.h"
#include "selftest_includes.h"

void runCoreTests();
void runCombatTests();
void runDungeonTests();
void runProgressionTests();
void runWorldTests();

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
    runWorldTests();

    // -------------------------------------------------------------- cleanup
    QFile::remove(savePath());

    // --------------------------------------------------------------- report
    out("");
    out("====================");
    out(QString("%1 passed, %2 failed").arg(g_passed).arg(g_failed));

    qInstallMessageHandler(nullptr);
    return g_failed == 0 ? 0 : 1;
}
