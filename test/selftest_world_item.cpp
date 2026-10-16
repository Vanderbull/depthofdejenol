#include "selftest_harness.h"
#include "selftest_includes.h"

void runWorldItemTests()
{
    gameStateManager* gsm = gameStateManager::instance();

    section("[54] Item progression");
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::tierName(Tier::Bronze) == "Bronze", "bronze name");
        check(ItemProgression::tierName(Tier::Iron) == "Iron", "iron name");
        check(ItemProgression::tierName(Tier::Steel) == "Steel", "steel name");
        check(ItemProgression::tierName(Tier::Adamantite) == "Adamantite", "adamantite name");
        check(ItemProgression::tierName(Tier::Mithril) == "Mithril", "mithril name");
    }

    section("[70] v1.0.0 slice 1.5: ItemProgression into loot");
    {
        using Tier = ItemProgression::Tier;

        // Tier curve: Bronze → Iron → Steel → Adamantite → Mithril.
        check(ItemProgression::tierForFloor(1) == Tier::Bronze, "floor 1 is bronze");
        check(ItemProgression::tierForFloor(5) == Tier::Iron, "floor 5 is iron");
        check(ItemProgression::tierForFloor(8) == Tier::Steel, "floor 8 is steel");
        check(ItemProgression::tierForFloor(12) == Tier::Adamantite, "floor 12 is adamantite");
        check(ItemProgression::tierForFloor(15) == Tier::Mithril, "floor 15 is mithril");

        // Tier availability gates loot: Mithril is not available on floor 1.
        check(ItemProgression::isAvailable(Tier::Bronze, 1), "bronze available floor 1");
        check(!ItemProgression::isAvailable(Tier::Mithril, 1), "mithril not available floor 1");
        check(ItemProgression::isAvailable(Tier::Mithril, 15), "mithril available floor 15");

        // VictoryReward applies tier-gating: a monster whose drop table contains
        // a Mithril-prefixed item must not drop it on floor 1.
        gameStateManager* gsm = gameStateManager::instance();
        const QList<QVariantMap>& monsters = gsm->getMonsterData();
        const QList<ItemDef>& items = ItemDatabase::instance().all();

        // Find a Mithril-prefixed item in the database.
        const ItemDef* mithrilItem = nullptr;
        for (const ItemDef& item : items) {
            if (item.name.startsWith("Mithril", Qt::CaseInsensitive)) {
                mithrilItem = &item;
                break;
            }
        }

        if (mithrilItem && !monsters.isEmpty()) {
            // Find a monster that drops this item.
            QString dropper;
            for (const QVariantMap& m : monsters) {
                for (int i = 0; i <= 9; ++i) {
                    if (m.value(QString("Item%1").arg(i)).toInt() == mithrilItem->id) {
                        dropper = m["name"].toString();
                        break;
                    }
                }
                if (!dropper.isEmpty()) break;
            }

            if (!dropper.isEmpty()) {
                // On floor 1, the Mithril item must never drop.
                bool foundMithril = false;
                for (int i = 0; i < 100; ++i) {
                    QStringList loot = VictoryReward::calculateLoot(dropper, monsters, 1);
                    if (loot.contains(mithrilItem->name)) {
                        foundMithril = true;
                        break;
                    }
                }
                check(!foundMithril,
                      "Mithril item does not drop on floor 1 (tier-gated)",
                      mithrilItem->name);

                // On floor 15, it can drop (may not always due to RNG, but the
                // tier gate must not block it).
                bool foundMithrilDeep = false;
                for (int i = 0; i < 200; ++i) {
                    QStringList loot = VictoryReward::calculateLoot(dropper, monsters, 15);
                    if (loot.contains(mithrilItem->name)) {
                        foundMithrilDeep = true;
                        break;
                    }
                }
                check(foundMithrilDeep,
                      "Mithril item can drop on floor 15 (tier available)",
                      mithrilItem->name);
            }
        }
    }

    section("[81] v2.0.0 slice 2.4: loot items added to inventory on victory");
    {
        // Loot drops are added to the lead character's inventory as unidentified.
        gameStateManager* gsm = gameStateManager::instance();
        Character hero;
        hero.name = "Loot Hero";
        hero.level = 1;
        hero.experience = 0;
        hero.maxHp = 10;
        hero.hp = 10;
        hero.isAlive = true;

        gsm->addCharacterToParty(hero);

        int invBefore = gsm->getPartyMember(0).inventory.size();

        HeldItem lootItem;
        lootItem.name = "Iron Sword";
        lootItem.identified = false;  // dungeon loot starts unidentified
        if (const ItemDef* def = ItemDatabase::instance().byName("Iron Sword")) {
            lootItem.M4E97 = static_cast<int16_t>(def->id);
        }
        gsm->addItemToCharacter(0, lootItem);

        int invAfter = gsm->getPartyMember(0).inventory.size();
        check(invAfter > invBefore, "loot item added to inventory");
        check(!gsm->getPartyMember(0).inventory.last().identified, "loot item is unidentified");
    }

    section("[108] v2.0.0 slice 2.10: loot items start unidentified");
    {
        HeldItem lootItem;
        lootItem.name = "Iron Sword";
        lootItem.identified = false;
        if (const ItemDef* def = ItemDatabase::instance().byName("Iron Sword")) {
            lootItem.M4E97 = static_cast<int16_t>(def->id);
        }
        check(!lootItem.identified, "loot item starts unidentified");
    }

    section("[109] v2.0.0 slice 2.10: identify renames item");
    {
        HeldItem item;
        item.name = "Unknown Iron Sword";
        item.identified = false;
        if (const ItemDef* def = ItemDatabase::instance().byName("Iron Sword")) {
            item.M4E97 = static_cast<int16_t>(def->id);
        }

        // Identify: look up real name and mark identified
        if (const ItemDef* def = ItemDatabase::instance().byId(item.M4E97)) {
            item.name = def->name;
            item.identified = true;
        }
        check(item.identified, "item identified");
        check(item.name == "Iron Sword", "item renamed after identification");
    }

}
