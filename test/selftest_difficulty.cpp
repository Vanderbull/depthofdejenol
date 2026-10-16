#include "selftest_harness.h"
#include "src/core/MonsterBalance.h"
#include "src/core/LevelTable.h"
#include "src/core/Endgame.h"

// Helhetstest: simulera ett helt spel från New Game till win.
// Mäter svårighet per nivå och verifierar att spelet är vinnbart.

void runDifficultyTests() {
    section("[D1] Helhetstest: svårighetskurva");

    // 1) Kontrollera att spelet är vinnbart från början till slut
    bool winnable = MonsterBalance::isRunWinnable();
    check(winnable, "spelet är vinnbart från början till slut");

    // 2) Visa svårighet per nivå
    for (int floor = 1; floor <= 15; ++floor) {
        int recLevel = MonsterBalance::recommendedLevel(floor);
        int monsterHp = MonsterBalance::representativeMonsterHp(floor);
        int monsterAtt = MonsterBalance::representativeMonsterAtt(floor);
        bool floorWinnable = MonsterBalance::isFloorWinnable(floor, recLevel);
        check(floorWinnable,
              QString("floor %1 är vinnbar").arg(floor),
              QString("lvl %1, HP %2, att %3").arg(recLevel).arg(monsterHp).arg(monsterAtt));
    }

    // 3) Total speltid
    int totalMinutes = MonsterBalance::estimatedRunMinutes();
    double hours = totalMinutes / 60.0;
    check(hours >= 10.0 && hours <= 15.0,
          "speltiden är inom målet (10-15 timmar)",
          QString("%1 timmar").arg(hours, 0, 'f', 1));

    // 4) Party-storlek påverkar svårighet
    check(MonsterBalance::isPartyReady(4, 1, 1), "party på 4 är redo för floor 1");
    check(!MonsterBalance::isPartyReady(1, 1, 1), "party på 1 är inte redo för floor 1");

    // 5) NG+ skalning
    check(Endgame::ngPlusMonsterMultiplier(0) == 1.0, "NG+0 har 1.0x multiplikator");
    check(Endgame::ngPlusMonsterMultiplier(1) > 1.0, "NG+1 har >1.0x multiplikator");
    check(Endgame::ngPlusMonsterMultiplier(2) > Endgame::ngPlusMonsterMultiplier(1),
          "NG+2 är svårare än NG+1");
}
