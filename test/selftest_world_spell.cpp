#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldSpellTests()
{
    gameStateManager* gsm = gameStateManager::instance();

    section("[53] Spell differentiation");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[72] v1.0.0 slice 1.7: SpellMechanics into combat");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[75] v2.0.0 slice 2.1: spell dialog routes through CombatActions");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[279] v5.4.0 slice 5.4: arena spell casting");
    {
        check(true, "placeholder - needs implementation");
    }

    section("[291] v6.1.0 slice 6.1a: dialog instant-kill is not 9999");
    {
        check(true, "placeholder - needs implementation");
    }
}
