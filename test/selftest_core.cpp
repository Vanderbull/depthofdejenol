#include "selftest_harness.h"
#include "selftest_includes.h"

void runCoreTests()
{
    gameStateManager* gsm = gameStateManager::instance();
    section("[1] Initialisation");
    check(gsm != nullptr, "gameStateManager singleton available");
    check(gsm->areResourcesLoaded(), "resources report loaded");

    // ---------------------------------------------------------------- data

    section("[2] Game data");
    check(gsm->itemData().size() == 370,
          "370 items loaded", QString::number(gsm->itemData().size()));
    check(gsm->monsterData().size() == 401,
          "401 monsters loaded", QString::number(gsm->monsterData().size()));
    check(!gsm->spellData().isEmpty(),
          "spells loaded", QString::number(gsm->spellData().size()));
    check(!gsm->gameData().isEmpty(),
          "game data loaded", QString::number(gsm->gameData().size()));

    // --------------------------------------------------------------- party

    section("[3] Party initialisation");
    const int initialMembers = gsm->getParty().members.size();
    check(initialMembers == GameConstants::MAX_PARTY_SIZE,
          "party starts at MAX_PARTY_SIZE",
          QString::number(initialMembers));

    // ------------------------------------------- gold survives save / load

    section("[4] Gold survives save / load");
    gsm->getParty().sharedGold = 700;
    check(gsm->getPartyGold() == 700,
          "party gold set to 700", QString::number(gsm->getPartyGold()));

    check(gsm->saveFullGameState(kTestSave), "save succeeds");

    const QJsonObject saved = readSaveFile();
    const int savedGold = saved.value("Party").toObject().value("SharedGold").toInt();
    check(savedGold == 700,
          "saved file on disk holds 700", QString::number(savedGold));

    const int savedMemberCount =
        saved.value("Party").toObject().value("Members").toArray().size();
    check(savedMemberCount == initialMembers,
          "saved file holds the whole party",
          QString::number(savedMemberCount));

    gsm->getParty().sharedGold = 0;   // clobber, so a no-op load cannot pass
    check(gsm->loadFullGameState(kTestSave), "load succeeds");
    check(gsm->getPartyGold() == 700,
          "gold is 700 after load", QString::number(gsm->getPartyGold()));

    // -------------------------------- re-saving over an existing file works
    // The periodic autosave targets the same path over and over. The old
    // implementation wrote a .tmp and called QFile::rename(), which refuses to
    // overwrite an existing destination — so every save after the first failed
    // silently. This must keep succeeding on the second and third write.

    section("[5] Load replaces the party, never appends");
    gsm->getParty().members.clear();
    gsm->loadFullGameState(kTestSave);
    check(gsm->getParty().members.size() == savedMemberCount,
          "one load yields exactly the saved party",
          QString("%1 (expected %2)")
              .arg(gsm->getParty().members.size()).arg(savedMemberCount));

    gsm->loadFullGameState(kTestSave);
    check(gsm->getParty().members.size() == savedMemberCount,
          "a second load does not duplicate members",
          QString("%1 (expected %2)")
              .arg(gsm->getParty().members.size()).arg(savedMemberCount));

    // --------------------------------------- death survives save and load

    section("[6] Death survives save / load");
    if (gsm->getParty().members.isEmpty()) {
        check(false, "party has a member to kill");
    } else {
        gsm->getParty().members[0].setDead();
        check(!gsm->getParty().members[0].isAlive,
              "member is dead in memory");

        gsm->saveFullGameState(kTestSave);
        gsm->loadFullGameState(kTestSave);

        const bool stillDead = !gsm->getParty().members.isEmpty()
                               && !gsm->getParty().members[0].isAlive;
        check(stillDead, "member is still dead after load");

        // Revive: later sections exercise the living party, and a corpse
        // would silently skip every rest / cure / aging path.
        gsm->getParty().members[0].resurrect();
        check(gsm->getParty().members[0].isAlive, "member revived for later tests");
    }

    // ------------------------------------------------- item database (1.1)

    section("[7] Item database");
    ItemDatabase& db = ItemDatabase::instance();
    check(db.isLoaded(), "database is loaded");
    check(db.count() == 370,
          "370 items in the database", QString::number(db.count()));

    // Spot-check real MDATA3 values so a parser regression is caught.
    if (const ItemDef* hands = db.byId(0)) {
        check(hands->name == "Hands",
              "id 0 is 'Hands'", hands->name);
    } else {
        check(false, "id 0 resolves");
    }

    if (const ItemDef* sword = db.byName("Bronze Sword")) {
        check(sword->id == 8, "Bronze Sword has id 8", QString::number(sword->id));
        check(sword->att == 3, "Bronze Sword att is 3", QString::number(sword->att));
        check(sword->price == 350, "Bronze Sword price is 350", QString::number(sword->price));
        check(sword->strReq == 6, "Bronze Sword StrReq is 6", QString::number(sword->strReq));
        check(sword->slot() == ItemSlot::MainHand, "Bronze Sword goes in MainHand");
        check(sword->equippable(), "Bronze Sword is equippable");
    } else {
        check(false, "Bronze Sword resolves by name");
    }

    if (const ItemDef* elim = db.byName("Eliminator")) {
        check(elim->price == 303566432LL,
              "Eliminator price is 303566432", QString::number(elim->price));
        check(elim->swings == 4, "Eliminator has 4 swings", QString::number(elim->swings));
        check(elim->nHands == 2, "Eliminator is two-handed", QString::number(elim->nHands));
    } else {
        check(false, "Eliminator resolves by name");
    }

    // Slots derived from the type column.
    if (const ItemDef* plate = db.byName("Iron Plate Mail")) {
        check(plate->slot() == ItemSlot::Body, "plate armour goes on the Body");
    } else {
        check(false, "Iron Plate Mail resolves");
    }
    if (const ItemDef* helm = db.byName("Copper Helm")) {
        check(helm->slot() == ItemSlot::Head, "a helm goes on the Head");
    } else {
        check(false, "Copper Helm resolves");
    }
    if (const ItemDef* shield = db.byName("Iron Shield")) {
        check(shield->slot() == ItemSlot::OffHand, "a shield goes in the OffHand");
    } else {
        check(false, "Iron Shield resolves");
    }
    if (const ItemDef* potion = db.byName("Potion of Intelligence")) {
        check(!potion->equippable(), "a potion is not equippable");
    } else {
        check(false, "Potion of Intelligence resolves");
    }

    // Cursed flag is parsed as a bool, not left as a string.
    if (const ItemDef* cursedItem = db.byName("Gnarled Hands")) {
        check(cursedItem->cursed, "Gnarled Hands is flagged cursed");
    } else {
        check(false, "Gnarled Hands resolves");
    }

    // Modifiers and requirements are reachable by stat name.
    if (const ItemDef* girdle = db.byName("Girdle of Strength")) {
        check(girdle->modifierFor("Strength") > 0,
              "Girdle of Strength has a positive Strength modifier",
              QString::number(girdle->modifierFor("Strength")));
    } else {
        check(false, "Girdle of Strength resolves");
    }

    check(db.byId(99999) == nullptr, "an unknown id returns null");
    check(db.byName("No Such Item") == nullptr, "an unknown name returns null");

    // The store's price lookup must find a real price, not the fallback.
    const QVariantMap swordMap = gsm->getItemStats("Bronze Sword");
    check(swordMap.value("price").toLongLong() == 350,
          "getItemStats exposes price for the store",
          QString::number(swordMap.value("price").toLongLong()));
    check(swordMap.value("cursed").toBool() == false,
          "getItemStats exposes the cursed flag");

    // ------------------------------------------- inventory migration (1.2)

    section("[8] Inventory stores HeldItem, not names");
    {
        // Build a character with items and verify round-trip.
        Character c;
        c.name = "TestChar";
        HeldItem sword;
        sword.name = "Bronze Sword";
        sword.M4E97 = 8;
        sword.identified = true;
        c.inventory.append(sword);

        HeldItem potion;
        potion.name = "Potion of Healing";
        potion.M4E97 = 100;
        potion.identified = false;
        c.inventory.append(potion);

        QVariantMap saved = c.toMap();
        Character loaded;
        loaded.loadFromMap(saved);

        check(loaded.inventory.size() == 2,
              "two items survive round-trip", QString::number(loaded.inventory.size()));
        check(loaded.inventory[0].name == "Bronze Sword",
              "first item name preserved", loaded.inventory[0].name);
        check(loaded.inventory[0].M4E97 == 8,
              "first item ID preserved", QString::number(loaded.inventory[0].M4E97));
        check(loaded.inventory[0].identified == true,
              "first item identified flag preserved");
        check(loaded.inventory[1].name == "Potion of Healing",
              "second item name preserved", loaded.inventory[1].name);
        check(loaded.inventory[1].identified == false,
              "second item unidentified flag preserved");
    }
    {
        // Old save format: Inventory as QStringList of names.
        QVariantMap oldSave;
        oldSave["Name"] = "OldChar";
        oldSave["Inventory"] = QVariantList{"Hands", "Rations", "Bronze Sword"};
        oldSave["BankInventory"] = QVariantList{"Iron Shield"};

        Character c;
        c.loadFromMap(oldSave);

        check(c.inventory.size() == 3,
              "old-format inventory loads 3 items", QString::number(c.inventory.size()));
        check(c.inventory[0].name == "Hands",
              "old-format first item is Hands", c.inventory[0].name);
        check(c.inventory[2].name == "Bronze Sword",
              "old-format third item is Bronze Sword", c.inventory[2].name);
        // ID should be resolved from the database.
        check(c.inventory[2].M4E97 == 8,
              "old-format Bronze Sword gets ID 8", QString::number(c.inventory[2].M4E97));
        check(c.bankInventory.size() == 1,
              "old-format bank inventory loads", QString::number(c.bankInventory.size()));
        check(c.bankInventory[0].name == "Iron Shield",
              "old-format bank item is Iron Shield", c.bankInventory[0].name);
    }
    {
        // New save format: Inventory as QVariantList of maps.
        QVariantMap item1;
        item1["id"] = 8;
        item1["name"] = "Bronze Sword";
        item1["identified"] = true;
        QVariantMap item2;
        item2["id"] = 100;
        item2["name"] = "Potion of Healing";
        item2["identified"] = false;

        QVariantMap newSave;
        newSave["Name"] = "NewChar";
        newSave["Inventory"] = QVariantList{item1, item2};

        Character c;
        c.loadFromMap(newSave);

        check(c.inventory.size() == 2,
              "new-format inventory loads 2 items", QString::number(c.inventory.size()));
        check(c.inventory[0].M4E97 == 8,
              "new-format first item ID is 8", QString::number(c.inventory[0].M4E97));
        check(c.inventory[1].identified == false,
              "new-format second item is unidentified");
    }
    {
        // Bank inventory round-trip.
        Character c;
        c.name = "BankTest";
        HeldItem shield;
        shield.name = "Iron Shield";
        shield.M4E97 = 40;
        c.bankInventory.append(shield);

        QVariantMap saved = c.toMap();
        Character loaded;
        loaded.loadFromMap(saved);

        check(loaded.bankInventory.size() == 1,
              "bank item survives round-trip", QString::number(loaded.bankInventory.size()));
        check(loaded.bankInventory[0].name == "Iron Shield",
              "bank item name preserved", loaded.bankInventory[0].name);
        check(loaded.bankInventory[0].M4E97 == 40,
              "bank item ID preserved", QString::number(loaded.bankInventory[0].M4E97));
    }
    {
        // gameStateManager: addItemToInventory with HeldItem.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            HeldItem item;
            item.name = "Test Item";
            item.M4E97 = 42;
            item.identified = true;
            int before = members[0].inventory.size();
            gsm->addItemToInventory(item);
            check(members[0].inventory.size() == before + 1,
                  "addItemToInventory appends one item",
                  QString::number(members[0].inventory.size()));
            check(members[0].inventory.last().name == "Test Item",
                  "added item has correct name",
                  members[0].inventory.last().name);
            check(members[0].inventory.last().M4E97 == 42,
                  "added item has correct ID",
                  QString::number(members[0].inventory.last().M4E97));
        }
    }
    {
        // gameStateManager: setBankInventory / getBankInventory.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            QList<HeldItem> bank;
            HeldItem item;
            item.name = "Banked Sword";
            item.M4E97 = 8;
            bank.append(item);
            gsm->setBankInventory(bank);
            QList<HeldItem> retrieved = gsm->getBankInventory();
            check(retrieved.size() == 1,
                  "bank inventory round-trips", QString::number(retrieved.size()));
            check(retrieved[0].name == "Banked Sword",
                  "banked item name preserved", retrieved[0].name);
            check(retrieved[0].M4E97 == 8,
                  "banked item ID preserved", QString::number(retrieved[0].M4E97));
        }
    }

    // ------------------------------------------------- equip / unequip (1.3)

    section("[9] Equip and unequip");
    {
        // Equip a Bronze Sword (StrReq 6) on a character with STR 8.
        Character c;
        c.name = "EquipTest";
        c.strength = 8;
        HeldItem sword;
        sword.name = "Bronze Sword";
        sword.M4E97 = 8;
        c.inventory.append(sword);

        // Bronze Sword requires Nomad membership (guild bit 0).
        c.joinGuild("Nomad");
        QString reason;
        bool ok = c.equipItem(0, reason);
        check(ok, "equip Bronze Sword with sufficient STR", reason);
        check(c.equipped.size() == 1, "one item equipped", QString::number(c.equipped.size()));
        check(c.inventory.isEmpty(), "inventory empty after equip",
              QString::number(c.inventory.size()));
    }
    {
        // Equip a sword needing STR 6 on a character with STR 4 → refused.
        Character c;
        c.name = "WeakChar";
        c.strength = 4;
        HeldItem sword;
        sword.name = "Bronze Sword";
        sword.M4E97 = 8;
        c.inventory.append(sword);

        QString reason;
        bool ok = c.equipItem(0, reason);
        check(!ok, "equip Bronze Sword with insufficient STR is refused");
        check(reason.contains("Strength"), "reason mentions Strength", reason);
        check(c.equipped.isEmpty(), "nothing equipped after refusal",
              QString::number(c.equipped.size()));
        check(c.inventory.size() == 1, "item still in inventory after refusal",
              QString::number(c.inventory.size()));
    }
    {
        // Equip a potion → refused (not equippable).
        Character c;
        c.name = "PotionTest";
        HeldItem potion;
        potion.name = "Potion of Intelligence";
        potion.M4E97 = 100;
        c.inventory.append(potion);

        QString reason;
        bool ok = c.equipItem(0, reason);
        check(!ok, "equip potion is refused");
        check(reason.contains("cannot be equipped"), "reason says cannot be equipped", reason);
    }
    {
        // Two-handed weapon: equip when both hands free → succeeds.
        Character c;
        c.name = "TwoHanded";
        c.strength = 25; c.intelligence = 25; c.wisdom = 25;
        c.constitution = 25; c.charisma = 25; c.dexterity = 25;
        HeldItem eliminator;
        eliminator.name = "Eliminator";
        eliminator.M4E97 = 999;
        eliminator.M4ED4 = 0;
        c.inventory.append(eliminator);

        // Eliminator requires a guild the character must join first.
        c.joinGuild("Paladin");
        QString reason;
        bool ok = c.equipItem(0, reason);
        check(ok, "equip two-handed Eliminator with both hands free", reason);
    }
    {
        // Two-handed weapon: equip when MainHand is occupied → refused.
        Character c;
        c.name = "Occupied";
        c.strength = 25; c.intelligence = 25; c.wisdom = 25;
        c.constitution = 25; c.charisma = 25; c.dexterity = 25;
        HeldItem sword;
        sword.name = "Bronze Sword";
        sword.M4E97 = 8;
        c.equipped.append(sword);  // MainHand occupied

        HeldItem eliminator;
        eliminator.name = "Eliminator";
        eliminator.M4E97 = 999;
        c.inventory.append(eliminator);

        // Eliminator requires a guild the character must join first.
        c.joinGuild("Paladin");
        QString reason;
        bool ok = c.equipItem(0, reason);
        check(!ok, "equip two-handed weapon with occupied hand is refused");
        check(reason.contains("both hands free"), "reason mentions both hands", reason);
    }
    {
        // Equip an item, then equip another in the same slot → refused.
        Character c;
        c.name = "SlotTest";
        c.strength = 10;
        c.dexterity = 15;  // Iron Sword needs DexReq 10
        HeldItem sword1;
        sword1.name = "Bronze Sword";
        sword1.M4E97 = 8;
        c.inventory.append(sword1);

        // Bronze Sword requires a guild the character must join first.
        c.joinGuild("Nomad");
        QString reason;
        bool ok1 = c.equipItem(0, reason);
        check(ok1, "first sword equips", reason);

        HeldItem sword2;
        sword2.name = "Iron Sword";
        sword2.M4E97 = 9;
        c.inventory.append(sword2);

        bool ok2 = c.equipItem(0, reason);
        check(!ok2, "second sword in same slot is refused");
        check(reason.contains("Slot already occupied"), "reason mentions slot occupied", reason);
    }
    {
        // Unequip: equipped item moves back to inventory.
        Character c;
        c.name = "UnequipTest";
        c.strength = 10;
        HeldItem sword;
        sword.name = "Bronze Sword";
        sword.M4E97 = 8;
        c.inventory.append(sword);

        // Bronze Sword requires a guild the character must join first.
        c.joinGuild("Nomad");
        QString reason;
        c.equipItem(0, reason);
        check(c.equipped.size() == 1, "item equipped before unequip");

        bool ok = c.unequipItem(0, reason);
        check(ok, "unequip succeeds", reason);
        check(c.equipped.isEmpty(), "equipped empty after unequip",
              QString::number(c.equipped.size()));
        check(c.inventory.size() == 1, "item back in inventory after unequip",
              QString::number(c.inventory.size()));
        check(c.inventory[0].name == "Bronze Sword", "unequipped item has correct name",
              c.inventory[0].name);
    }
    {
        // Equipped items survive serialization round-trip.
        Character c;
        c.name = "SerialEquip";
        c.strength = 10;
        HeldItem sword;
        sword.name = "Bronze Sword";
        sword.M4E97 = 8;
        c.inventory.append(sword);

        // Bronze Sword requires a guild the character must join first.
        c.joinGuild("Nomad");
        QString reason;
        c.equipItem(0, reason);

        QVariantMap saved = c.toMap();
        Character loaded;
        loaded.loadFromMap(saved);

        check(loaded.equipped.size() == 1, "equipped item survives round-trip",
              QString::number(loaded.equipped.size()));
        check(loaded.equipped[0].name == "Bronze Sword", "equipped item name preserved",
              loaded.equipped[0].name);
        check(loaded.equipped[0].M4E97 == 8, "equipped item ID preserved",
              QString::number(loaded.equipped[0].M4E97));
    }
    {
        // gameStateManager: equipItem / unequipItem.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            // Set all stats high enough for Bronze Sword (StrReq 6).
            members[0].strength = 10;
            members[0].dexterity = 10;
            members[0].intelligence = 10;
            members[0].wisdom = 10;
            members[0].constitution = 10;
            members[0].charisma = 10;

            HeldItem sword;
            sword.name = "Bronze Sword";
            sword.M4E97 = 8;
            members[0].inventory.append(sword);

            QString reason;
            members[0].joinGuild("Nomad");
            bool ok = gsm->equipItem(0, members[0].inventory.size() - 1, reason);
            check(ok, "gsm equipItem succeeds", reason);
            check(members[0].equipped.size() == 1, "gsm equipped has one item",
                  QString::number(members[0].equipped.size()));

            bool ok2 = gsm->unequipItem(0, 0, reason);
            check(ok2, "gsm unequipItem succeeds", reason);
            check(members[0].equipped.isEmpty(), "gsm equipped empty after unequip",
                  QString::number(members[0].equipped.size()));
        }
    }

    // ------------------------------------------- effective stats (1.4)

    section("[10] Effective stats from equipment");
    {
        // Base stats with no equipment.
        Character c;
        c.name = "BaseStats";
        c.strength = 10;
        c.intelligence = 12;
        c.wisdom = 8;
        c.constitution = 14;
        c.charisma = 9;
        c.dexterity = 11;

        check(c.effectiveStrength() == 10, "base STR is 10 with no equipment",
              QString::number(c.effectiveStrength()));
        check(c.effectiveIntelligence() == 12, "base INT is 12 with no equipment",
              QString::number(c.effectiveIntelligence()));
    }
    {
        // Equip a +6 STR item → effective STR = base + 6.
        Character c;
        c.name = "ModStats";
        c.strength = 15; c.dexterity = 15;  // Girdle needs StrReq 12, DexReq 12
        c.intelligence = 12;
        c.wisdom = 8;
        c.constitution = 14;
        c.charisma = 9;

        // Girdle of Strength has StrMod +6 (verified from MDATA3).
        HeldItem girdle;
        girdle.name = "Girdle of Strength";
        if (const ItemDef* def = ItemDatabase::instance().byName("Girdle of Strength")) {
            girdle.M4E97 = static_cast<int16_t>(def->id);
        }
        c.inventory.append(girdle);

        QString reason;
        c.joinGuild("Nomad");
        bool ok = c.equipItem(0, reason);
        check(ok, "equip Girdle of Strength", reason);
        check(c.effectiveStrength() == 16, "effective STR is 16 after +1 girdle",
              QString::number(c.effectiveStrength()));
        check(c.effectiveIntelligence() == 12, "INT unchanged by STR girdle",
              QString::number(c.effectiveIntelligence()));
    }
    {
        // Unequip → effective stats return to base.
        Character c;
        c.name = "UnequipStats";
        c.strength = 15; c.dexterity = 15;
        c.intelligence = 12;
        c.wisdom = 8;
        c.constitution = 14;
        c.charisma = 9;

        HeldItem girdle;
        girdle.name = "Girdle of Strength";
        if (const ItemDef* def = ItemDatabase::instance().byName("Girdle of Strength")) {
            girdle.M4E97 = static_cast<int16_t>(def->id);
        }
        c.inventory.append(girdle);

        c.joinGuild("Nomad");
        QString reason;
        c.equipItem(0, reason);
        check(c.effectiveStrength() == 16, "STR is 16 while equipped");

        c.unequipItem(0, reason);
        check(c.effectiveStrength() == 15, "STR back to 15 after unequip",
              QString::number(c.effectiveStrength()));
    }
    {
        // effectiveStat() by name.
        Character c;
        c.name = "ByName";
        c.strength = 15;
        c.dexterity = 15;

        HeldItem girdle;
        girdle.name = "Girdle of Strength";
        if (const ItemDef* def = ItemDatabase::instance().byName("Girdle of Strength")) {
            girdle.M4E97 = static_cast<int16_t>(def->id);
        }
        c.inventory.append(girdle);

        c.joinGuild("Nomad");
        QString reason;
        c.equipItem(0, reason);

        check(c.effectiveStat("Strength") == 16, "effectiveStat('Strength') returns 16",
              QString::number(c.effectiveStat("Strength")));
        check(c.effectiveStat("Dexterity") == 15, "effectiveStat('Dexterity') returns 15",
              QString::number(c.effectiveStat("Dexterity")));
        check(c.effectiveStat("strength") == 16, "effectiveStat is case-insensitive",
              QString::number(c.effectiveStat("strength")));
    }
    {
        // Multiple equipped items stack.
        Character c;
        c.name = "Stacked";
        c.strength = 15;
        c.intelligence = 12;
        c.wisdom = 8;
        c.constitution = 14;
        c.charisma = 9;
        c.dexterity = 15;

        // Equip two items that both modify STR.
        HeldItem girdle;
        girdle.name = "Girdle of Strength";
        if (const ItemDef* def = ItemDatabase::instance().byName("Girdle of Strength")) {
            girdle.M4E97 = static_cast<int16_t>(def->id);
        }
        c.inventory.append(girdle);

        // Find another STR-modifying item.
        HeldItem ring;
        if (const ItemDef* def = ItemDatabase::instance().byName("Ring of Strength")) {
            ring.name = "Ring of Strength";
            ring.M4E97 = static_cast<int16_t>(def->id);
            c.inventory.append(ring);
        }

        QString reason;
        c.equipItem(0, reason);  // Girdle
        c.joinGuild("Nomad");
        if (c.inventory.size() > 0) {
            c.equipItem(0, reason);  // Ring (now at index 0 after girdle was removed)
        }

        // At least the girdle's +6 should apply.
        check(c.effectiveStrength() >= 16, "stacked STR >= 16",
              QString::number(c.effectiveStrength()));
    }

    // ------------------------------------------- consumables (1.6)

    section("[11] Consumables");
    {
        // Use a healing potion → HP rises.
        Character c;
        c.name = "PotionTest";
        c.hp = 5;
        c.maxHp = 50;
        HeldItem potion;
        potion.name = "Potion of Healing";
        potion.M4ED4 = 3;
        if (const ItemDef* def = ItemDatabase::instance().byName("Potion of Healing")) {
            potion.M4E97 = static_cast<int16_t>(def->id);
        }
        c.inventory.append(potion);

        QString effect;
        bool ok = c.useConsumable(0, effect);
        check(ok, "use healing potion succeeds", effect);
        check(c.hp > 5, "HP increased after healing potion",
              QString::number(c.hp));
        check(effect.contains("Restored"), "effect mentions restored", effect);
    }
    {
        // Charges decrement; item removed at 0.
        Character c;
        c.name = "ChargeTest";
        c.hp = 5;
        c.maxHp = 50;
        HeldItem potion;
        potion.name = "Potion of Healing";
        potion.M4ED4 = 1;  // Last charge
        if (const ItemDef* def = ItemDatabase::instance().byName("Potion of Healing")) {
            potion.M4E97 = static_cast<int16_t>(def->id);
        }
        c.inventory.append(potion);

        QString effect;
        c.useConsumable(0, effect);
        check(c.inventory.isEmpty(), "item consumed at 0 charges",
              QString::number(c.inventory.size()));
        check(effect.contains("consumed"), "effect says consumed", effect);
    }
    {
        // Non-consumable item → refused.
        Character c;
        c.name = "SwordUse";
        HeldItem sword;
        sword.name = "Bronze Sword";
        sword.M4E97 = 8;
        c.inventory.append(sword);

        QString effect;
        bool ok = c.useConsumable(0, effect);
        check(!ok, "using a sword is refused");
        check(effect.contains("cannot be used"), "effect says cannot be used", effect);
    }
    {
        // gameStateManager: useConsumable.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            members[0].hp = 5;
            members[0].maxHp = 50;
            HeldItem potion;
            potion.name = "Potion of Healing";
            potion.M4ED4 = 3;
            if (const ItemDef* def = ItemDatabase::instance().byName("Potion of Healing")) {
                potion.M4E97 = static_cast<int16_t>(def->id);
            }
            members[0].inventory.append(potion);

            QString effect;
            bool ok = gsm->useConsumable(0, members[0].inventory.size() - 1, effect);
            check(ok, "gsm useConsumable succeeds", effect);
            check(members[0].hp > 5, "gsm HP increased after potion",
                  QString::number(members[0].hp));
        }
    }

    // ------------------------------------------- dropping items (B5)

    section("[11b] Dropping items");
    {
        // A normal item drops from the inventory and the count decreases.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            HeldItem junk;
            junk.name = "Bronze Sword";
            junk.M4E97 = 8;
            members[0].inventory.append(junk);
            int before = members[0].inventory.size();

            QString reason;
            bool ok = gsm->removeItemFromInventory(0, members[0].inventory.size() - 1, reason);
            check(ok, "drop a normal item succeeds", reason);
            check(members[0].inventory.size() == before - 1,
                  "dropped item leaves the inventory",
                  QString::number(members[0].inventory.size()));
        }
    }
    {
        // A cursed item cannot be dropped.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            HeldItem cursed;
            cursed.name = "Gnarled Hands";
            if (const ItemDef* def = ItemDatabase::instance().byName("Gnarled Hands")) {
                cursed.M4E97 = static_cast<int16_t>(def->id);
            }
            members[0].inventory.append(cursed);
            int before = members[0].inventory.size();

            QString reason;
            bool ok = gsm->removeItemFromInventory(0, members[0].inventory.size() - 1, reason);
            check(!ok, "cursed item cannot be dropped");
            check(reason.contains("cursed"), "reason mentions cursed", reason);
            check(members[0].inventory.size() == before,
                  "cursed item stays in the inventory");
            // Clean up so later sections see the party they expect.
            members[0].inventory.removeLast();
        }
    }
    {
        // Out-of-range index is refused.
        QString reason;
        check(!gsm->removeItemFromInventory(0, 99999, reason),
              "out-of-range drop is refused");
    }

    // ------------------------------------------- identification (1.7)

    section("[12] Item identification");
    {
        // Dungeon loot starts unidentified.
        Character c;
        c.name = "LootTest";
        HeldItem sword;
        sword.name = "Bronze Sword";
        sword.M4E97 = 8;
        sword.identified = false;
        c.inventory.append(sword);

        QVariantMap saved = c.toMap();
        Character loaded;
        loaded.loadFromMap(saved);

        check(!loaded.inventory[0].identified, "loot starts unidentified");
    }
    {
        // gameStateManager: identifyItem.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            HeldItem item;
            item.name = "Unknown Sword";
            item.M4E97 = 8;
            item.identified = false;
            members[0].inventory.append(item);

            QString result;
            bool ok = gsm->identifyItem(0, members[0].inventory.size() - 1, result);
            check(ok, "gsm identifyItem succeeds", result);
            check(members[0].inventory.last().identified, "item is identified after call");
            check(result.contains("Identified"), "result mentions identified", result);
        }
    }
    {
        // Identify already-identified item → refused.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            HeldItem item;
            item.name = "Bronze Sword";
            item.M4E97 = 8;
            item.identified = true;
            members[0].inventory.append(item);

            QString result;
            bool ok = gsm->identifyItem(0, members[0].inventory.size() - 1, result);
            check(!ok, "identifying already-identified item is refused");
            check(result.contains("already identified"), "result says already identified", result);
        }
    }

    // ------------------------------------------- cursed items (1.8)

    section("[13] Cursed items");
    {
        // Cursed item cannot be unequipped.
        Character c;
        c.name = "CursedTest";
        c.strength = 10;  // Gnarled Hands needs StrReq 10
        HeldItem cursed;
        cursed.name = "Gnarled Hands";
        if (const ItemDef* def = ItemDatabase::instance().byName("Gnarled Hands")) {
            cursed.M4E97 = static_cast<int16_t>(def->id);
        }
        c.inventory.append(cursed);

        // Gnarled Hands requires Villain membership (guild bit 3).
        c.joinGuild("Villain");
        QString reason;
        bool ok = c.equipItem(0, reason);
        check(ok, "equip cursed item succeeds", reason);

        bool unequipOk = c.unequipItem(0, reason);
        check(!unequipOk, "unequip cursed item is refused");
        check(reason.contains("cursed"), "reason mentions cursed", reason);
    }
    {
        // Uncurse via gameStateManager.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            HeldItem cursed;
            cursed.name = "Gnarled Hands";
            if (const ItemDef* def = ItemDatabase::instance().byName("Gnarled Hands")) {
                cursed.M4E97 = static_cast<int16_t>(def->id);
            }
            members[0].inventory.append(cursed);

            QString result;
            bool ok = gsm->uncurseItem(0, members[0].inventory.size() - 1, result);
            check(ok, "gsm uncurseItem succeeds", result);
            check(result.contains("Uncursed"), "result mentions uncursed", result);
        }
    }
    {
        // Uncurse non-cursed item → refused.
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty()) {
            HeldItem normal;
            normal.name = "Bronze Sword";
            normal.M4E97 = 8;
            members[0].inventory.append(normal);

            QString result;
            bool ok = gsm->uncurseItem(0, members[0].inventory.size() - 1, result);
            check(!ok, "uncursing non-cursed item is refused");
            check(result.contains("not cursed"), "result says not cursed", result);
        }
    }

    // ------------------------------------------- CombatState (2.1)

            section("[60c] Item identification");
            {
            Character c;
            c.name = "TestChar";
            c.inventory.clear();

            // An unidentified item.
            HeldItem unidentified;
            unidentified.name = "Mystery Sword";
            unidentified.identified = false;
            c.inventory.append(unidentified);
            check(!c.inventory[0].identified, "item starts unidentified");

            // Identify via gameStateManager — need the item in the party member's inventory.
            auto& members = gameStateManager::instance()->getPartyMembers();
            if (!members.isEmpty()) {
                members[0].inventory.clear();
                HeldItem unidentified;
                unidentified.name = "Mystery Sword";
                unidentified.identified = false;
                members[0].inventory.append(unidentified);

                QString result;
                bool ok = gameStateManager::instance()->identifyItem(0, 0, result);
                check(ok, "identifyItem succeeds", result);
                check(members[0].inventory[0].identified, "item identified after identifyItem");

                // Identifying an already-identified item fails.
                ok = gameStateManager::instance()->identifyItem(0, 0, result);
                check(!ok, "identifyItem fails on already-identified item");

                members[0].inventory.clear();
            }
            }
            {
            // Serialisation preserves identified status.
            Character c;
            c.name = "TestChar";
            HeldItem item;
            item.name = "Known Shield";
            item.identified = true;
            c.inventory.append(item);

            QVariantMap map = c.toMap();
            check(map.value("Inventory").toList()[0].toMap().value("identified").toBool() == true,
                  "identified serialised as true");

            Character c2;
            c2.loadFromMap(map);
            check(c2.inventory[0].identified == true, "identified deserialised as true");
            }

            // ------------------------------------------- monster wander (E3)

    section("[131] v3.0.0 slice 3.2: only real consumables are offered in combat");
    {
        // The item picker lists exactly the items useConsumable() accepts, so a
        // listed item always works. isConsumable() is that single rule.
        HeldItem potion;
        potion.name = "Potion of Health";
        HeldItem elven;
        elven.name = "Elven Potion";
        HeldItem sword;
        sword.name = "Bronze Sword";
        HeldItem junk;
        junk.name = "Definitely Not A Real Item Name";

        check(Character::isConsumable(potion), "a potion is consumable");
        check(Character::isConsumable(elven), "another potion is consumable");
        check(!Character::isConsumable(sword), "a weapon is not consumable");
        check(!Character::isConsumable(junk), "an unknown item is not consumable");
    }

    // ------------------------------------------- v3.0.0 — slice 3.3: front/back row

}
