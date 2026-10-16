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

        // No stray temp file should be left behind.
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
        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();

        for (int i = 0; i < 4; ++i) {
            Character c;
            c.name = QString("Hero%1").arg(i + 1);
            c.race = "Human";
            c.level = 5 + i;
            c.hp = 20 + i * 10;
            c.maxHp = 50 + i * 10;
            c.gold = 100 + i * 50;
            c.strength = 10 + i;
            c.dexterity = 12 + i;
            c.hunger = 80 - i * 10;
            c.maxHunger = 100;
            c.isAlive = true;
            c.dungeonLevel = i;
            c.dungeonX = i * 3;
            c.dungeonY = i * 7;

            // Each character gets a unique inventory
            HeldItem sword;
            sword.name = QString("Sword%1").arg(i + 1);
            sword.M4E97 = 100 + i;
            c.inventory.append(sword);

            HeldItem shield;
            shield.name = QString("Shield%1").arg(i + 1);
            shield.M4E97 = 200 + i;
            c.inventory.append(shield);

            // Equipped items
            HeldItem armor;
            armor.name = QString("Armor%1").arg(i + 1);
            armor.M4E97 = 300 + i;
            c.equipped.append(armor);

            members.append(c);
        }

        // Save
        check(gameStateManager::instance()->saveFullGameState("testsave"),
              "saveFullGameState succeeds");

        // Clear and reload
        members.clear();
        check(members.isEmpty(), "party cleared before load");

        check(gameStateManager::instance()->loadFullGameState("testsave"),
              "loadFullGameState succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 4, "party has 4 characters after load",
              QString::number(loaded.size()));

        for (int i = 0; i < 4; ++i) {
            const Character& c = loaded[i];
            check(c.name == QString("Hero%1").arg(i + 1),
                  QString("Hero%1 name preserved").arg(i + 1));
            check(c.level == 5 + i,
                  QString("Hero%1 level preserved").arg(i + 1));
            check(c.hp == 20 + i * 10,
                  QString("Hero%1 HP preserved").arg(i + 1));
            check(c.maxHp == 50 + i * 10,
                  QString("Hero%1 MaxHP preserved").arg(i + 1));
            check(c.gold == 100 + i * 50,
                  QString("Hero%1 gold preserved").arg(i + 1));
            check(c.strength == 10 + i,
                  QString("Hero%1 strength preserved").arg(i + 1));
            check(c.dexterity == 12 + i,
                  QString("Hero%1 dexterity preserved").arg(i + 1));
            check(c.hunger == 80 - i * 10,
                  QString("Hero%1 hunger preserved").arg(i + 1));
            check(c.maxHunger == 100,
                  QString("Hero%1 maxHunger preserved").arg(i + 1));
            check(c.dungeonLevel == i,
                  QString("Hero%1 dungeonLevel preserved").arg(i + 1));
            check(c.dungeonX == i * 3,
                  QString("Hero%1 dungeonX preserved").arg(i + 1));
            check(c.dungeonY == i * 7,
                  QString("Hero%1 dungeonY preserved").arg(i + 1));

            // Inventory
            check(c.inventory.size() == 2,
                  QString("Hero%1 inventory has 2 items").arg(i + 1));
            if (c.inventory.size() == 2) {
                check(c.inventory[0].name == QString("Sword%1").arg(i + 1),
                      QString("Hero%1 sword name preserved").arg(i + 1));
                check(c.inventory[1].name == QString("Shield%1").arg(i + 1),
                      QString("Hero%1 shield name preserved").arg(i + 1));
            }

            // Equipped
            check(c.equipped.size() == 1,
                  QString("Hero%1 equipped has 1 item").arg(i + 1));
            if (c.equipped.size() == 1) {
                check(c.equipped[0].name == QString("Armor%1").arg(i + 1),
                      QString("Hero%1 armor name preserved").arg(i + 1));
            }
        }

        // Cleanup
        QFile::remove("data/saves/testsave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[231] v5.2.0 slice 5.1: party save/load round-trips with all fields");
    {
        // Test savePartyToFile / loadPartyFromFile with a 2-character party
        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();

        Character c1;
        c1.name = "PartyA";
        c1.level = 3;
        c1.hp = 15;
        c1.maxHp = 30;
        c1.gold = 200;
        c1.hunger = 60;
        c1.maxHunger = 100;
        c1.isAlive = true;
        c1.strength = 8;
        c1.dexterity = 14;

        HeldItem potion;
        potion.name = "Potion";
        potion.M4E97 = 42;
        c1.inventory.append(potion);

        Character c2;
        c2.name = "PartyB";
        c2.level = 7;
        c2.hp = 40;
        c2.maxHp = 80;
        c2.gold = 500;
        c2.hunger = 90;
        c2.maxHunger = 100;
        c2.isAlive = true;
        c2.strength = 16;
        c2.dexterity = 10;

        HeldItem ring;
        ring.name = "Ring";
        ring.M4E97 = 88;
        c2.inventory.append(ring);

        members.append(c1);
        members.append(c2);

        QString savePath = "data/saves/test_party.json";
        check(gameStateManager::instance()->savePartyToFile(savePath),
              "savePartyToFile succeeds");

        // Clear and reload
        members.clear();
        check(members.isEmpty(), "party cleared before party load");

        check(gameStateManager::instance()->loadPartyFromFile(savePath),
              "loadPartyFromFile succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 2, "party has 2 characters after party load",
              QString::number(loaded.size()));
        check(loaded[0].name == "PartyA", "PartyA loaded");
        check(loaded[1].name == "PartyB", "PartyB loaded");
        check(loaded[0].inventory.size() == 1, "PartyA inventory preserved");
        check(loaded[1].inventory.size() == 1, "PartyB inventory preserved");
        if (loaded[0].inventory.size() == 1) {
            check(loaded[0].inventory[0].name == "Potion", "PartyA potion preserved");
        }
        if (loaded[1].inventory.size() == 1) {
            check(loaded[1].inventory[0].name == "Ring", "PartyB ring preserved");
        }

        // Cleanup
        QFile::remove(savePath);
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[232] v5.2.0 slice 5.1: autosave works with a living party");
    {
        // Reproduce the exact autosave flow that runs every 30 seconds in-game
        Character hero;
        hero.name = "AutoHero";
        hero.level = 10;
        hero.hp = 100;
        hero.maxHp = 100;
        hero.gold = 500;
        hero.hunger = 50;
        hero.maxHunger = 100;
        hero.isAlive = true;
        hero.strength = 12;
        hero.dexterity = 14;

        HeldItem gem;
        gem.name = "Gem";
        gem.M4E97 = 99;
        hero.inventory.append(gem);

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        // Set CurrentCharacterName so handleAutosave doesn't skip
        gameStateManager::instance()->setGameValue("CurrentCharacterName", "AutoHero");

        // Call the real autosave handler
        gameStateManager::instance()->handleAutosave();

        // Verify the autosave file has the character
        QFile file("data/saves/autosave.json");
        check(file.open(QIODevice::ReadOnly), "autosave file readable");
        if (file.isOpen()) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();
            QVariantMap root = doc.toVariant().toMap();
            QVariantMap party = root.value("Party").toMap();
            QVariantList partyMembers = party.value("Members").toList();
            check(partyMembers.size() == 1, "autosave has 1 member",
                  QString::number(partyMembers.size()));
            if (partyMembers.size() == 1) {
                QVariantMap m = partyMembers[0].toMap();
                check(m.value("Name").toString() == "AutoHero", "autosave has AutoHero");
                check(m.value("HP").toInt() == 100, "autosave has correct HP");
                QVariantList inv = m.value("Inventory").toList();
                check(inv.size() == 1, "autosave has 1 inventory item",
                      QString::number(inv.size()));
                if (inv.size() == 1) {
                    QVariantMap item = inv[0].toMap();
                    check(item.value("name").toString() == "Gem",
                          "autosave has Gem in inventory");
                }
            }
        }

        // Cleanup
        QFile::remove("data/saves/autosave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[233] v5.2.0 slice 5.1: savegame preserves bank inventory");
    {
        Character hero;
        hero.name = "BankHero";
        hero.level = 5;
        hero.hp = 30;
        hero.maxHp = 50;
        hero.gold = 100;
        hero.hunger = 80;
        hero.maxHunger = 100;
        hero.isAlive = true;

        HeldItem bankItem;
        bankItem.name = "BankedItem";
        bankItem.M4E97 = 777;
        hero.bankInventory.append(bankItem);

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        check(gameStateManager::instance()->saveFullGameState("banksave"),
              "save with bank inventory succeeds");

        members.clear();
        check(gameStateManager::instance()->loadFullGameState("banksave"),
              "load with bank inventory succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 1, "1 character loaded");
        if (loaded.size() == 1) {
            check(loaded[0].bankInventory.size() == 1, "bank inventory preserved");
            if (loaded[0].bankInventory.size() == 1) {
                check(loaded[0].bankInventory[0].name == "BankedItem",
                      "bank item name preserved");
            }
        }

        // Cleanup
        QFile::remove("data/saves/banksave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[234] v5.2.0 slice 5.1: savegame preserves equipped items");
    {
        Character hero;
        hero.name = "EquipHero";
        hero.level = 8;
        hero.hp = 80;
        hero.maxHp = 120;
        hero.gold = 300;
        hero.hunger = 70;
        hero.maxHunger = 100;
        hero.isAlive = true;

        HeldItem weapon;
        weapon.name = "DragonSlayer";
        weapon.M4E97 = 1234;
        hero.equipped.append(weapon);

        HeldItem helm;
        helm.name = "IronHelm";
        helm.M4E97 = 5678;
        hero.equipped.append(helm);

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        check(gameStateManager::instance()->saveFullGameState("equipsave"),
              "save with equipped items succeeds");

        members.clear();
        check(gameStateManager::instance()->loadFullGameState("equipsave"),
              "load with equipped items succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 1, "1 character loaded");
        if (loaded.size() == 1) {
            check(loaded[0].equipped.size() == 2, "equipped items preserved");
            if (loaded[0].equipped.size() == 2) {
                check(loaded[0].equipped[0].name == "DragonSlayer", "weapon preserved");
                check(loaded[0].equipped[1].name == "IronHelm", "helmet preserved");
            }
        }

        // Cleanup
        QFile::remove("data/saves/equipsave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[235] v5.2.0 slice 5.1: savegame preserves guild data");
    {
        Character hero;
        hero.name = "GuildHero";
        hero.level = 12;
        hero.hp = 120;
        hero.maxHp = 200;
        hero.gold = 1000;
        hero.hunger = 60;
        hero.maxHunger = 100;
        hero.isAlive = true;

        hero.guildLevels["Fighters"] = 3;
        hero.guildLevels["Mages"] = 5;
        hero.guildExperience["Fighters"] = 5000;
        hero.guildExperience["Mages"] = 12000;

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        check(gameStateManager::instance()->saveFullGameState("guildsave"),
              "save with guild data succeeds");

        members.clear();
        check(gameStateManager::instance()->loadFullGameState("guildsave"),
              "load with guild data succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 1, "1 character loaded");
        if (loaded.size() == 1) {
            check(loaded[0].guildLevels["Fighters"] == 3, "Fighters guild level preserved");
            check(loaded[0].guildLevels["Mages"] == 5, "Mages guild level preserved");
            check(loaded[0].guildExperience["Fighters"] == 5000, "Fighters guild XP preserved");
            check(loaded[0].guildExperience["Mages"] == 12000, "Mages guild XP preserved");
        }

        // Cleanup
        QFile::remove("data/saves/guildsave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

    section("[236] v5.2.0 slice 5.1: savegame preserves play time and NG+ level");
    {
        Character hero;
        hero.name = "TimeHero";
        hero.level = 15;
        hero.hp = 150;
        hero.maxHp = 250;
        hero.gold = 2000;
        hero.hunger = 40;
        hero.maxHunger = 100;
        hero.isAlive = true;
        hero.playTimeSeconds = 3600;

        auto& members = gameStateManager::instance()->getPartyMembers();
        members.clear();
        members.append(hero);

        // Set NG+ level
        gameStateManager::instance()->setGameValue("NGPlusLevel", 3);

        check(gameStateManager::instance()->saveFullGameState("timesave"),
              "save with play time succeeds");

        members.clear();
        // Clear NG+ to verify it loads
        gameStateManager::instance()->setGameValue("NGPlusLevel", 0);
        check(gameStateManager::instance()->loadFullGameState("timesave"),
              "load with play time succeeds");

        auto& loaded = gameStateManager::instance()->getPartyMembers();
        check(loaded.size() == 1, "1 character loaded");
        if (loaded.size() == 1) {
            check(loaded[0].playTimeSeconds == 3600, "play time preserved",
                  QString::number(loaded[0].playTimeSeconds));
        }

        int ngPlus = gameStateManager::instance()->getGameValue("NGPlusLevel").toInt();
        check(ngPlus == 3, "NG+ level preserved", QString::number(ngPlus));

        // Cleanup
        QFile::remove("data/saves/timesave.json");
        gameStateManager::instance()->getPartyMembers().clear();
    }

}
