#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldItemTests()
{
    gameStateManager* gsm = gameStateManager::instance();

    section("[54] Item progression");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[70] v1.0.0 slice 1.5: ItemProgression into loot");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[81] v2.0.0 slice 2.4: loot items added to inventory on victory");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[108] v2.0.0 slice 2.10: loot items start unidentified");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[109] v2.0.0 slice 2.10: identify renames item");
    {
        check(true, "placeholder - needs implementation");
    }
}
