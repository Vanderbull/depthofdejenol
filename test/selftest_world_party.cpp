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
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();

        Character c1; c1.name = "PartyA"; c1.level = 1; c1.hp = 10; c1.maxHp = 10;
        c1.hunger = 100; c1.maxHunger = 100; c1.isAlive = true;
        Character c2; c2.name = "PartyB"; c2.level = 2; c2.hp = 20; c2.maxHp = 20;
        c2.hunger = 100; c2.maxHunger = 100; c2.isAlive = true;
        Character c3; c3.name = "PartyC"; c3.level = 3; c3.hp = 30; c3.maxHp = 30;
        c3.hunger = 100; c3.maxHunger = 100; c3.isAlive = true;
        members << c1 << c2 << c3;
        gsm->getParty().sharedGold = 500;

        QString path = QDir::temp().filePath("selftest_party.json");
        check(gsm->savePartyToFile(path), "the party saved to a file");

        members.clear();
        gsm->getParty().sharedGold = 0;
        check(members.isEmpty(), "the party was cleared before loading");

        check(gsm->loadPartyFromFile(path), "the party loaded from the file");
        check(members.size() == 3, "all three members loaded",
              QString::number(members.size()));
        check(members[0].name == "PartyA", "first member name matches");
        check(members[1].name == "PartyB", "second member name matches");
        check(members[2].name == "PartyC", "third member name matches");
        check(gsm->getPartyGold() == 500, "shared gold round-trips",
              QString::number(gsm->getPartyGold()));

        members.clear();
        QFile::remove(path);
    }

    section("[211] v5.0.0 slice 5.6: loading a party replaces the existing party");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();

        Character old1; old1.name = "Old1"; old1.level = 1; old1.hp = 10; old1.maxHp = 10;
        old1.hunger = 100; old1.maxHunger = 100; old1.isAlive = true;
        Character old2; old2.name = "Old2"; old2.level = 1; old2.hp = 10; old2.maxHp = 10;
        old2.hunger = 100; old2.maxHunger = 100; old2.isAlive = true;
        members << old1 << old2;

        Character new1; new1.name = "New1"; new1.level = 5; new1.hp = 50; new1.maxHp = 50;
        new1.hunger = 100; new1.maxHunger = 100; new1.isAlive = true;
        Character new2; new2.name = "New2"; new2.level = 5; new2.hp = 50; new2.maxHp = 50;
        new2.hunger = 100; new2.maxHunger = 100; new2.isAlive = true;
        Character new3; new3.name = "New3"; new3.level = 5; new3.hp = 50; new3.maxHp = 50;
        new3.hunger = 100; new3.maxHunger = 100; new3.isAlive = true;

        QString path = QDir::temp().filePath("selftest_party_replace.json");
        members.clear();
        members << new1 << new2 << new3;
        gsm->savePartyToFile(path);

        members.clear();
        members << old1 << old2;
        check(members.size() == 2, "the old party has two members");

        gsm->loadPartyFromFile(path);
        check(members.size() == 3, "the new party replaced the old one",
              QString::number(members.size()));
        check(members[0].name == "New1", "the first member is from the new party");

        members.clear();
        QFile::remove(path);
    }

    section("[212] v5.0.0 slice 5.6: multi-select loads several characters");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();

        // Save a party of three as JSON (the format the dialog loads).
        Character c1; c1.name = "Multi1"; c1.level = 1; c1.hp = 10; c1.maxHp = 10;
        c1.hunger = 100; c1.maxHunger = 100; c1.isAlive = true;
        Character c2; c2.name = "Multi2"; c2.level = 2; c2.hp = 20; c2.maxHp = 20;
        c2.hunger = 100; c2.maxHunger = 100; c2.isAlive = true;
        Character c3; c3.name = "Multi3"; c3.level = 3; c3.hp = 30; c3.maxHp = 30;
        c3.hunger = 100; c3.maxHunger = 100; c3.isAlive = true;
        members << c1 << c2 << c3;

        QString path = QDir::temp().filePath("selftest_multi.json");
        gsm->savePartyToFile(path);

        members.clear();
        gsm->loadPartyFromFile(path);
        check(members.size() == 3, "all three members loaded from party file",
              QString::number(members.size()));

        members.clear();
        QFile::remove(path);
    }

    section("[213] v5.0.0 slice 5.6: MAX_PARTY_SIZE is respected");
    {
        gameStateManager* gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        members.clear();

        // Try to add more than MAX_PARTY_SIZE members.
        for (int i = 0; i < GameConstants::MAX_PARTY_SIZE + 2; ++i) {
            Character c;
            c.name = QString("Over%1").arg(i);
            c.level = 1; c.hp = 10; c.maxHp = 10;
            c.hunger = 100; c.maxHunger = 100; c.isAlive = true;
            gsm->addCharacterToParty(c);
        }

        check(members.size() <= GameConstants::MAX_PARTY_SIZE,
              "the party size does not exceed MAX_PARTY_SIZE",
              QString::number(members.size()));

        members.clear();
    }

    section("[237] v5.2.0 slice 5.2: build a party from individual characters");
    {
        // Create 4 individual character files using the real saveCharacterToFile
        for (int i = 0; i < 4; ++i) {
            Character c;
            c.name = QString("BuildHero%1").arg(i + 1);
            c.race = "Human";
            c.level = 2 + i;
            c.hp = 20 + i * 10;
            c.maxHp = 40 + i * 10;
            c.gold = 50 + i * 25;
            c.hunger = 90 - i * 5;
            c.maxHunger = 100;
            c.isAlive = true;
            c.strength = 10 + i;
            c.dexterity = 12 + i;

            HeldItem item;
            item.name = QString("Item%1").arg(i + 1);
            item.M4E97 = 10 + i;
            c.inventory.append(item);

            gameStateManager::instance()->getPartyMembers().clear();
            gameStateManager::instance()->getPartyMembers().append(c);
            check(gameStateManager::instance()->saveCharacterToFile(0),
                  QString("save BuildHero%1").arg(i + 1));
        }

        // Clear party
        gameStateManager::instance()->getPartyMembers().clear();

        // Use the real addCharacterFromFile to build a party from 3 files
        for (int i = 0; i < 3; ++i) {
            QString path = QString("data/characters/BuildHero%1.json").arg(i + 1);
            check(gameStateManager::instance()->addCharacterFromFile(path),
                  QString("add BuildHero%1 to party").arg(i + 1));
        }

        auto& members = gameStateManager::instance()->getPartyMembers();
        check(members.size() == 3, "party built with 3 characters",
              QString::number(members.size()));
        check(members[0].name == "BuildHero1", "first character is BuildHero1");
        check(members[1].name == "BuildHero2", "second character is BuildHero2");
        check(members[2].name == "BuildHero3", "third character is BuildHero3");

        // Verify inventories are independent
        check(members[0].inventory.size() == 1, "first character has 1 item");
        check(members[1].inventory.size() == 1, "second character has 1 item");
        check(members[0].inventory[0].name == "Item1", "first has Item1");
        check(members[1].inventory[0].name == "Item2", "second has Item2");

        // Cleanup
        for (int i = 0; i < 4; ++i) {
            QFile::remove(QString("data/characters/BuildHero%1.json").arg(i + 1));
        }
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[238] v5.2.0 slice 5.2: party size is limited to MAX_PARTY_SIZE");
    {
        // MAX_PARTY_SIZE is 4 — try to add 5 characters
        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();

        for (int i = 0; i < 5; ++i) {
            Character c;
            c.name = QString("Overflow%1").arg(i + 1);
            c.level = 1;
            c.hp = 10;
            c.maxHp = 10;
            c.isAlive = true;
            gameStateManager::instance()->addCharacterToParty(c);
        }

        check(members.size() <= GameConstants::MAX_PARTY_SIZE,
              "party size respects MAX_PARTY_SIZE",
              QString::number(members.size()));

        members.clear();
    }

    section("[239] v5.2.0 slice 5.2: each party member has independent inventory");
    {
        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();

        Character c1;
        c1.name = "Inv1";
        c1.isAlive = true;
        HeldItem sword;
        sword.name = "Sword";
        c1.inventory.append(sword);

        Character c2;
        c2.name = "Inv2";
        c2.isAlive = true;
        HeldItem shield;
        shield.name = "Shield";
        c2.inventory.append(shield);

        members.append(c1);
        members.append(c2);

        // Verify inventories are independent
        check(members[0].inventory.size() == 1, "first member has 1 item");
        check(members[1].inventory.size() == 1, "second member has 1 item");
        check(members[0].inventory[0].name == "Sword", "first member has Sword");
        check(members[1].inventory[0].name == "Shield", "second member has Shield");

        // Modify one inventory, verify the other is unchanged
        members[0].inventory.append(shield);
        check(members[0].inventory.size() == 2, "first member now has 2 items");
        check(members[1].inventory.size() == 1, "second member still has 1 item");

        members.clear();
    }

}
