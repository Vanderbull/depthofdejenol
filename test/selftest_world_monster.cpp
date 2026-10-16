#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldMonsterTests()
{
    gameStateManager* gsm = gameStateManager::instance();

    section("[52] Monster difficulty curve");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[62e] Monster chase AI");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[95] v2.0.0 slice 2.7: monster spellcaster casts via CombatActions");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[96] v2.0.0 slice 2.7: monster AI decides to cast spell");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[97] v2.0.0 slice 2.7: monster AI flees when critically wounded");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[98] v2.0.0 slice 2.7: boss never flees");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[99] v2.0.0 slice 2.8: poison DoT applied each round");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[100] v2.0.0 slice 2.8: blind reduces to-hit and expires");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[101] v2.0.0 slice 2.8: fire DoT and expires");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[104] v2.0.0 slice 2.8: poison kills at 0 HP");
    {
        check(true, "placeholder - needs implementation");
    }
}
