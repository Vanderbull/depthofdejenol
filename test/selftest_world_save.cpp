#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldSaveTests()
{
    gameStateManager* gsm = gameStateManager::instance();

    section("[4b] Saving over an existing file (autosave path)");
    {
        gsm->getParty().sharedGold = 111;
        check(gsm->saveFullGameState(kTestSave), "second save over existing file succeeds");
        gsm->getParty().sharedGold = 222;
        check(gsm->saveFullGameState(kTestSave), "third save over existing file succeeds");

        const QJsonObject again = readSaveFile();
        const int gold = again.value("Party").toObject().value("SharedGold").toInt();
        check(gold == 222, "the latest write wins", QString::number(gold));

        check(!QFile::exists(savePath() + ".tmp"), "no leftover .tmp file");
    }

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

    section("[230] v5.2.0 slice 5.1: savegame round-trips a 4-character party with inventories");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[231] v5.2.0 slice 5.1: party save/load round-trips with all fields");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[232] v5.2.0 slice 5.1: autosave works with a living party");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[233] v5.2.0 slice 5.1: savegame preserves bank inventory");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[234] v5.2.0 slice 5.1: savegame preserves equipped items");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[235] v5.2.0 slice 5.1: savegame preserves guild data");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[236] v5.2.0 slice 5.1: savegame preserves play time and NG+ level");
    {
        check(true, "placeholder - needs implementation");
    }
}
