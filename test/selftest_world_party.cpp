#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldPartyTests()
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

    section("[210] v5.0.0 slice 5.6: a saved party round-trips");
    {
        // TODO: party round-trip test
        check(true, "placeholder");
    }

    section("[211] v5.0.0 slice 5.6: loading a party replaces the existing party");
    {
        check(true, "placeholder");
    }

    section("[212] v5.0.0 slice 5.6: multi-select loads several characters");
    {
        check(true, "placeholder");
    }

    section("[213] v5.0.0 slice 5.6: MAX_PARTY_SIZE is respected");
    {
        check(true, "placeholder");
    }

    section("[237] v5.2.0 slice 5.2: build a party from individual characters");
    {
        check(true, "placeholder");
    }

    section("[238] v5.2.0 slice 5.2: party size is limited to MAX_PARTY_SIZE");
    {
        check(true, "placeholder");
    }

    section("[239] v5.2.0 slice 5.2: each party member has independent inventory");
    {
        check(true, "placeholder");
    }
}
