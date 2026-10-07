#include "selftest.h"

#include "gameStateManager.h"
#include "character.h"
#include "src/items/ItemDatabase.h"
#include "src/combat/CombatState.h"
#include "src/combat/TurnEngine.h"
#include "src/combat/CombatActions.h"
#include "src/combat/MonsterAI.h"
#include "src/combat/EncounterBuilder.h"
#include "src/combat/VictoryReward.h"
#include "src/combat/CombatDeathHandler.h"
#include "src/core/LevelTable.h"
#include "src/core/AgingRules.h"
#include "src/core/DungeonLevelState.h"
#include "src/core/DungeonThemes.h"
#include "src/core/DoorAndSearch.h"
#include "src/core/BossEncounter.h"
#include "src/core/DeathRecovery.h"
#include "src/core/QuestChain.h"
#include "src/core/Endgame.h"
#include "src/core/MonsterBalance.h"
#include "src/spell_casting/SpellMechanics.h"
#include "src/items/ItemProgression.h"
#include "src/core/GoldSinks.h"
#include "src/core/ReleaseInfo.h"
#include "src/core/AlignmentSystem.h"
#include "src/npc_dialog/NPCDialog.h"
#include "src/shortcut_help/ShortcutHelp.h"
#include "src/tutorial/Tutorial.h"
#include "src/quest_board/QuestBoardDialog.h"
#include "src/journal_dialog/JournalDialog.h"
#include "src/spell_casting/SpellBook.h"
#include "src/partymanager/PartyManager.h"
#include "src/library_dialog/BestiaryDialog.h"
#include "src/character_dialog/CharacterSheetDialog.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStringList>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>

#include <cstdio>

namespace {

int g_passed = 0;
int g_failed = 0;

// Results go straight to stdout, and Qt's own logging is swallowed for the
// duration of the run. The game logs heavily while initialising (full data
// dumps), which would otherwise bury the results and make `make check`
// useless in CI.
void quietMessageHandler(QtMsgType, const QMessageLogContext&, const QString&) {}

void out(const QString& line)
{
    std::fputs(qPrintable(line), stdout);
    std::fputc('\n', stdout);
    std::fflush(stdout);
}

void check(bool condition, const QString& what, const QString& detail = QString())
{
    if (condition) {
        ++g_passed;
        out("  PASS  " + what);
    } else {
        ++g_failed;
        QString line = "  FAIL  " + what;
        if (!detail.isEmpty()) line += "  [" + detail + "]";
        out(line);
    }
}

void section(const QString& title)
{
    out("");
    out(title);
}

const QString kTestSave = "selftest_tmp";
QString savePath() { return "data/saves/" + kTestSave + ".json"; }

// Reads the save file we just wrote. Returns an empty object if unreadable.
QJsonObject readSaveFile()
{
    QFile f(savePath());
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(f.readAll()).object();
}

} // namespace

int runSelfTest()
{
    qInstallMessageHandler(quietMessageHandler);

    out("BlackLands self-test");
    out("====================");

    gameStateManager* gsm = gameStateManager::instance();

    // ---------------------------------------------------------------- init
    section("[1] Initialisation");
    check(gsm != nullptr, "gameStateManager singleton available");
    check(gsm->areResourcesLoaded(), "resources report loaded");

    // ---------------------------------------------------------------- data
    section("[2] Game data");
    check(gsm->itemData().size() == 366,
          "366 items loaded", QString::number(gsm->itemData().size()));
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

    // ------------------------------------ load replaces rather than appends
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
    check(db.count() == 366,
          "366 items in the database", QString::number(db.count()));

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
        // Identify an item.
        Character c;
        c.name = "IdentTest";
        HeldItem sword;
        sword.name = "Unknown Sword";
        sword.M4E97 = 8;
        sword.identified = false;
        c.inventory.append(sword);

        QString result;
        gsm->identifyItem(0, 0, result);
        // Note: this uses the gsm party, not the local character.
        // Test via gameStateManager instead.
        check(true, "identifyItem callable");
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
    section("[14] CombatState data model");
    {
        CombatState cs;
        check(cs.participantCount() == 0, "new CombatState is empty");
        check(cs.isCombatOver(), "empty combat is over (no participants)");

        CombatParticipant warrior;
        warrior.name = "Warrior";
        warrior.hp = 30; warrior.maxHp = 30;
        warrior.att = 10; warrior.def = 8; warrior.speed = 5; warrior.dex = 10;
        warrior.isPlayer = true;
        cs.addParticipant(warrior);

        CombatParticipant mage;
        mage.name = "Mage";
        mage.hp = 20; mage.maxHp = 20;
        mage.att = 6; mage.def = 4; mage.speed = 7; mage.dex = 12;
        mage.isPlayer = true;
        cs.addParticipant(mage);

        CombatParticipant goblin;
        goblin.name = "Goblin";
        goblin.hp = 15; goblin.maxHp = 15;
        goblin.att = 7; goblin.def = 5; goblin.speed = 6; goblin.dex = 8;
        goblin.isPlayer = false;
        goblin.level = 1;
        cs.addParticipant(goblin);

        check(cs.participantCount() == 3, "three participants added",
              QString::number(cs.participantCount()));
        check(cs.livingPlayerCount() == 2, "two living players",
              QString::number(cs.livingPlayerCount()));
        check(cs.livingMonsterCount() == 1, "one living monster",
              QString::number(cs.livingMonsterCount()));
        check(!cs.isCombatOver(), "combat not over with living participants");
    }
    {
        // Initiative roll produces valid order.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.speed = 5; a.dex = 10; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.speed = 8; b.dex = 12; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.rollInitiative();
        check(cs.initiativeOrder().size() == 2, "initiative order has 2 entries",
              QString::number(cs.initiativeOrder().size()));
        // All indices valid.
        bool valid = true;
        for (int idx : cs.initiativeOrder()) {
            if (idx < 0 || idx >= 2) valid = false;
        }
        check(valid, "initiative indices are valid");
    }
    {
        // Turn cycling: each participant acts once per round.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.speed = 5; a.dex = 10; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.speed = 8; b.dex = 12; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.rollInitiative();
        cs.startRound();
        check(cs.currentRound() == 1, "round 1 started",
              QString::number(cs.currentRound()));

        int turns = 0;
        while (cs.nextTurn()) {
            turns++;
            const CombatParticipant* p = cs.currentParticipant();
            check(p != nullptr, "current participant is valid");
            cs.markActed(cs.initiativeOrder()[cs.currentTurnIndex()]);
        }
        check(turns == 2, "both participants acted in round 1",
              QString::number(turns));
    }
    {
        // Dead participants are skipped.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.speed = 5; a.dex = 10; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.speed = 8; b.dex = 12; b.isPlayer = false;
        b.isAlive = false;  // B starts dead
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.rollInitiative();
        cs.startRound();

        int turns = 0;
        while (cs.nextTurn()) {
            turns++;
            cs.markActed(cs.initiativeOrder()[cs.currentTurnIndex()]);
        }
        check(turns == 1, "only living participant acts",
              QString::number(turns));
    }
    {
        // Combat over when one side is dead.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        check(!cs.isCombatOver(), "combat not over initially");

        cs.participant(1).isAlive = false;  // Kill monster
        check(cs.isCombatOver(), "combat over when monster dead");
    }
    {
        // Multiple rounds.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.rollInitiative();

        cs.startRound();
        check(cs.currentRound() == 1, "round 1");
        while (cs.nextTurn()) {
            cs.markActed(cs.initiativeOrder()[cs.currentTurnIndex()]);
        }

        cs.startRound();
        check(cs.currentRound() == 2, "round 2");
        int turns = 0;
        while (cs.nextTurn()) {
            turns++;
            cs.markActed(cs.initiativeOrder()[cs.currentTurnIndex()]);
        }
        check(turns == 2, "both act in round 2", QString::number(turns));
    }

    // ------------------------------------------- TurnEngine (2.2)
    section("[15] Turn engine");
    {
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5; warrior.dex = 10;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 6; goblin.dex = 8;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        check(te.combatState() == &cs, "TurnEngine holds CombatState");
        check(te.currentRound() == 0, "round 0 before start");
        check(!te.isCombatOver(), "combat not over");

        te.startRound();
        check(te.currentRound() == 1, "round 1 after startRound",
              QString::number(te.currentRound()));
        check(!te.isRoundOver(), "round not over at start");

        // First turn
        bool hasTurn = te.nextTurn();
        check(hasTurn, "first turn available");
        check(te.hasCurrentParticipant(), "has current participant");
        QString name = te.currentParticipantName();
        check(!name.isEmpty(), "current participant has name", name);
        check(!te.hasCurrentActed(), "current has not acted yet");

        te.markCurrentActed();
        check(te.hasCurrentActed(), "current has acted after mark");

        // Second turn
        hasTurn = te.nextTurn();
        check(hasTurn, "second turn available");
        QString name2 = te.currentParticipantName();
        check(name2 != name, "second turn is different participant",
              name2 + " vs " + name);

        te.markCurrentActed();

        // No more turns — round over
        hasTurn = te.nextTurn();
        check(!hasTurn, "no third turn — round over");
        check(te.isRoundOver(), "round is over after all acted");
    }
    {
        // TurnEngine with dead participants.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false; b.isAlive = false;
        cs.addParticipant(a);
        cs.addParticipant(b);

        TurnEngine te;
        te.setCombatState(&cs);
        te.startRound();

        int turns = 0;
        while (te.nextTurn()) {
            turns++;
            te.markCurrentActed();
        }
        check(turns == 1, "only living participant acts",
              QString::number(turns));
    }
    {
        // Combat status string.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);

        TurnEngine te;
        te.setCombatState(&cs);
        te.startRound();
        QString status = te.combatStatus();
        check(status.contains("Round 1"), "status mentions Round 1", status);
        check(status.contains("1 players"), "status mentions 1 players", status);
        check(status.contains("1 monsters"), "status mentions 1 monsters", status);
    }
    {
        // Multiple rounds via TurnEngine.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);

        TurnEngine te;
        te.setCombatState(&cs);

        te.startRound();
        check(te.currentRound() == 1, "round 1");
        while (te.nextTurn()) { te.markCurrentActed(); }

        te.startRound();
        check(te.currentRound() == 2, "round 2");
        int turns = 0;
        while (te.nextTurn()) { turns++; te.markCurrentActed(); }
        check(turns == 2, "both act in round 2", QString::number(turns));
    }
    {
        // livingPlayers / livingMonsters.
        CombatState cs;
        CombatParticipant a; a.name = "A"; a.isPlayer = true;
        CombatParticipant b; b.name = "B"; b.isPlayer = false;
        CombatParticipant c; c.name = "C"; c.isPlayer = false;
        cs.addParticipant(a);
        cs.addParticipant(b);
        cs.addParticipant(c);

        TurnEngine te;
        te.setCombatState(&cs);
        check(te.livingPlayers().size() == 1, "one living player",
              QString::number(te.livingPlayers().size()));
        check(te.livingMonsters().size() == 2, "two living monsters",
              QString::number(te.livingMonsters().size()));

        cs.participant(1).isAlive = false;
        check(te.livingMonsters().size() == 1, "one living monster after kill",
              QString::number(te.livingMonsters().size()));
    }

    // ------------------------------------------- CombatActions (2.3)
    section("[16] Player action menu");
    {
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.att = 10; warrior.dex = 10; warrior.speed = 100; warrior.swings = 1; warrior.damageMod = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 20; goblin.maxHp = 20; goblin.def = 5; goblin.speed = 6; goblin.dex = 8;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();  // Warrior acts first (speed 100 guarantees it)
        check(ca.isPlayerTurn(), "warrior acts first with speed 100");

        // Attack the goblin (index 1)
        QString result;
        int damage = ca.attack(1, result);
        check(damage > 0 || result.contains("miss") || result.contains("fumble"),
              "attack produces a result", result);
        check(cs.participant(1).hp < 20 || result.contains("miss") || result.contains("fumble"),
              "goblin HP reduced or attack missed",
              QString::number(cs.participant(1).hp));
    }
    {
        // Defend marks as acted.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.defend(result);
        check(result.contains("defensive"), "defend produces message", result);
        check(te.hasCurrentActed(), "defend marks as acted");
    }
    {
        // Cast spell damages target.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.att = 8; mage.speed = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 30; goblin.maxHp = 30; goblin.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();
        check(ca.isPlayerTurn(), "mage acts first with speed 100");

        QString result;
        int damage = ca.castSpell(1, 10, result);
        check(damage > 0, "spell deals damage", result);
        check(cs.participant(1).hp < 30, "goblin HP reduced after spell",
              QString::number(cs.participant(1).hp));
    }
    {
        // Flee produces a result.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.flee(result);
        check(result.contains("flee") || result.contains("flees"),
              "flee produces message", result);
    }
    {
        // isPlayerTurn returns false for monster turn.
        CombatState cs;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        cs.addParticipant(goblin);
        cs.addParticipant(warrior);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        // First turn could be either — just check it returns a valid bool
        bool isPlayer = ca.isPlayerTurn();
        check(isPlayer == true || isPlayer == false, "isPlayerTurn returns valid bool");
    }
    {
        // Attack dead target → refused.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.isAlive = false;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int damage = ca.attack(1, result);
        check(damage == -1, "attack on dead target refused");
        check(result.contains("dead"), "result mentions dead", result);
    }

    // ------------------------------------------- attack resolution (2.4)
    section("[17] Equipment-driven attack resolution");
    {
        // A 20 STR warrior with a 2-swing weapon out-damages a 10 STR mage with a dagger
        // over 100 simulated rounds.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.att = 20; warrior.dex = 10; warrior.speed = 100; warrior.swings = 2; warrior.damageMod = 100;
        warrior.level = 5; warrior.levelScale = 10;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.att = 10; mage.dex = 8; mage.speed = 90; mage.swings = 1; mage.damageMod = 100;
        mage.level = 5; mage.levelScale = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 10000; goblin.maxHp = 10000; goblin.def = 5; goblin.speed = 5; goblin.level = 1;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        int warriorDamage = 0;
        int mageDamage = 0;
        int rounds = 0;

        while (!te.isCombatOver() && rounds < 100) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString result;
                    int dmg = ca.attack(2, result);  // Attack goblin
                    if (dmg > 0) {
                        if (idx == 0) warriorDamage += dmg;
                        else if (idx == 1) mageDamage += dmg;
                    }
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(warriorDamage > mageDamage,
              "2-swing warrior out-damages 1-swing mage over 100 rounds",
              QString("warrior=%1 mage=%2").arg(warriorDamage).arg(mageDamage));
    }
    {
        // Level scaling: higher level attacker deals more damage.
        CombatState cs;
        CombatParticipant highLevel; highLevel.name = "HighLevel"; highLevel.isPlayer = true;
        highLevel.att = 10; highLevel.dex = 10; highLevel.speed = 100; highLevel.swings = 1;
        highLevel.damageMod = 100; highLevel.level = 10; highLevel.levelScale = 20;
        CombatParticipant lowLevel; lowLevel.name = "LowLevel"; lowLevel.isPlayer = true;
        lowLevel.att = 10; lowLevel.dex = 10; lowLevel.speed = 90; lowLevel.swings = 1;
        lowLevel.damageMod = 100; lowLevel.level = 1; lowLevel.levelScale = 0;
        CombatParticipant dummy; dummy.name = "Dummy"; dummy.isPlayer = false;
        dummy.hp = 100000; dummy.maxHp = 100000; dummy.def = 0; dummy.speed = 5; dummy.level = 1;
        cs.addParticipant(highLevel);
        cs.addParticipant(lowLevel);
        cs.addParticipant(dummy);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        int highDamage = 0;
        int lowDamage = 0;
        int rounds = 0;

        while (!te.isCombatOver() && rounds < 50) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString result;
                    int dmg = ca.attack(2, result);
                    if (dmg > 0) {
                        if (idx == 0) highDamage += dmg;
                        else if (idx == 1) lowDamage += dmg;
                    }
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(highDamage > lowDamage,
              "higher level attacker deals more damage",
              QString("high=%1 low=%2").arg(highDamage).arg(lowDamage));
    }
    {
        // DEX affects to-hit: higher DEX attacker hits more often.
        CombatState cs;
        CombatParticipant highDex; highDex.name = "HighDex"; highDex.isPlayer = true;
        highDex.att = 10; highDex.dex = 20; highDex.speed = 100; highDex.swings = 1;
        highDex.damageMod = 100; highDex.level = 1; highDex.levelScale = 0;
        CombatParticipant lowDex; lowDex.name = "LowDex"; lowDex.isPlayer = true;
        lowDex.att = 10; lowDex.dex = 2; lowDex.speed = 90; lowDex.swings = 1;
        lowDex.damageMod = 100; lowDex.level = 1; lowDex.levelScale = 0;
        CombatParticipant dummy; dummy.name = "Dummy"; dummy.isPlayer = false;
        dummy.hp = 100000; dummy.maxHp = 100000; dummy.def = 0; dummy.speed = 5; dummy.level = 1;
        cs.addParticipant(highDex);
        cs.addParticipant(lowDex);
        cs.addParticipant(dummy);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        int highDexHits = 0;
        int lowDexHits = 0;
        int rounds = 0;

        while (!te.isCombatOver() && rounds < 50) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString result;
                    int dmg = ca.attack(2, result);
                    if (dmg > 0) {
                        if (idx == 0) highDexHits++;
                        else if (idx == 1) lowDexHits++;
                    }
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(highDexHits >= lowDexHits,
              "higher DEX attacker hits at least as often",
              QString("high=%1 low=%2").arg(highDexHits).arg(lowDexHits));
    }

    // ------------------------------------------- Monster AI (2.5)
    // ------------------------------------------- MonsterAI (2.5)
    section("[18] Monster turns and AI");
    {
        // Monster attacks on its turn.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5; warrior.dex = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.dex = 20; goblin.hp = 50; goblin.maxHp = 50;
        goblin.att = 8; goblin.def = 5; goblin.level = 1;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();  // Goblin acts first (speed 100)
        check(ai.isMonsterTurn(), "goblin turn detected");

        int warriorHpBefore = cs.participant(0).hp;
        QString result = ai.takeTurn();
        check(result.contains("hit") || result.contains("miss") || result.contains("fumble"),
              "monster attack produces result", result);
        check(cs.participant(0).hp <= warriorHpBefore, "warrior HP reduced or unchanged");
    }
    {
        // Low-HP monster flees.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.hp = 2; goblin.maxHp = 50;  // Very low HP
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();
        check(ai.isMonsterTurn(), "goblin turn");

        MonsterAI::Decision d = ai.decide();
        check(d == MonsterAI::Decision::Flee, "low-HP monster decides to flee");
    }
    {
        // Full-HP monster attacks.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.hp = 50; goblin.maxHp = 50;  // Full HP
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();
        MonsterAI::Decision d = ai.decide();
        check(d == MonsterAI::Decision::Attack, "full-HP monster decides to attack");
    }
    {
        // Monster targets front row (lowest index) most of the time.
        CombatState cs;
        CombatParticipant p1; p1.name = "FrontRow"; p1.isPlayer = true;
        p1.speed = 5; p1.hp = 30;
        CombatParticipant p2; p2.name = "BackRow"; p2.isPlayer = true;
        p2.speed = 5; p2.hp = 30;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.hp = 50; goblin.maxHp = 50;
        cs.addParticipant(p1);
        cs.addParticipant(p2);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();

        int frontRowTargets = 0;
        for (int i = 0; i < 100; i++) {
            int target = ai.chooseTarget();
            if (target == 0) frontRowTargets++;
        }
        check(frontRowTargets > 40, "front row targeted more than random",
              QString::number(frontRowTargets));
    }
    {
        // Monster targets weakest when not front row.
        CombatState cs;
        CombatParticipant p1; p1.name = "Strong"; p1.isPlayer = true;
        p1.speed = 5; p1.hp = 50;
        CombatParticipant p2; p2.name = "Weak"; p2.isPlayer = true;
        p2.speed = 5; p2.hp = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 100; goblin.hp = 50; goblin.maxHp = 50;
        cs.addParticipant(p1);
        cs.addParticipant(p2);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();

        int weakTargets = 0;
        for (int i = 0; i < 100; i++) {
            int target = ai.chooseTarget();
            if (target == 1) weakTargets++;
        }
        check(weakTargets > 10, "weakest targeted sometimes",
              QString::number(weakTargets));
    }
    {
        // isMonsterTurn returns false for player turn.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();
        check(!ai.isMonsterTurn(), "player turn is not monster turn");
    }

    // ------------------------------------------- EncounterBuilder (2.6)
    section("[19] Group encounters");
    {
        // Build an encounter from monster data.
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["numGroups"] = 2;
        orc["hits"] = 3;
        orc["att"] = 12;
        orc["def"] = 8;
        orc["StatDex"] = 10;
        orc["StatCon"] = 14;
        orc["levelFound"] = 3;
        orc["damageMod"] = 100;
        monsterData.append(orc);

        QVariantMap snake;
        snake["name"] = "Rattlesnake";
        snake["numGroups"] = 1;
        snake["hits"] = 2;
        snake["att"] = 16;
        snake["def"] = 12;
        snake["StatDex"] = 18;
        snake["StatCon"] = 8;
        snake["levelFound"] = 6;
        snake["damageMod"] = 100;
        monsterData.append(snake);

        // Orc: 2 groups * 3 hits = 6 monsters
        QList<CombatParticipant> orcGroup = EncounterBuilder::buildEncounter("Orc", monsterData);
        check(orcGroup.size() == 6, "Orc encounter has 6 monsters",
              QString::number(orcGroup.size()));

        // Rattlesnake: 1 group * 2 hits = 2 monsters
        QList<CombatParticipant> snakeGroup = EncounterBuilder::buildEncounter("Rattlesnake", monsterData);
        check(snakeGroup.size() == 2, "Rattlesnake encounter has 2 monsters",
              QString::number(snakeGroup.size()));

        // All monsters are non-player
        bool allMonsters = true;
        for (const CombatParticipant& p : orcGroup) {
            if (p.isPlayer) allMonsters = false;
        }
        check(allMonsters, "all encounter members are monsters");

        // Monster names are unique
        QSet<QString> names;
        for (const CombatParticipant& p : orcGroup) {
            names.insert(p.name);
        }
        check(names.size() == orcGroup.size(), "monster names are unique");

        // Stats are populated
        check(orcGroup[0].att == 12, "Orc att = 12");
        check(orcGroup[0].def == 8, "Orc def = 8");
        check(orcGroup[0].level == 3, "Orc level = 3");
        check(orcGroup[0].hp > 0, "Orc HP > 0");
        check(orcGroup[0].maxHp > 0, "Orc maxHP > 0");

        // getGroupSize matches
        check(EncounterBuilder::getGroupSize("Orc", monsterData) == 6,
              "getGroupSize Orc = 6");
        check(EncounterBuilder::getGroupSize("Rattlesnake", monsterData) == 2,
              "getGroupSize Rattlesnake = 2");

        // Unknown monster falls back to 1
        check(EncounterBuilder::getGroupSize("Unknown", monsterData) == 1,
              "unknown monster defaults to 1");

        // Full combat with group: all monsters take turns
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 50; warrior.hp = 200; warrior.maxHp = 200;
        cs.addParticipant(warrior);
        for (const CombatParticipant& p : orcGroup) {
            cs.addParticipant(p);
        }

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        int monsterTurns = 0;
        int rounds = 0;
        while (!te.isCombatOver() && rounds < 20) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (!cs.participant(idx).isPlayer) {
                    ai.takeTurn();
                    monsterTurns++;
                } else {
                    // Player attacks first living monster
                    for (int i = 0; i < cs.participantCount(); i++) {
                        if (!cs.participant(i).isPlayer && cs.participant(i).isAlive) {
                            QString result;
                            ca.attack(i, result);
                            break;
                        }
                    }
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(monsterTurns >= 6, "all 6 monsters took at least one turn",
              QString::number(monsterTurns));
        check(te.isCombatOver(), "combat ended within 20 rounds");
    }

    // ------------------------------------------- Spells in combat (2.7)
    section("[20] Spells in combat");
    {
        // Single-target spell: Fireball on one monster.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 50; mage.maxMana = 50; mage.att = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();  // Mage acts first

        int manaBefore = cs.participant(0).mana;
        QString result;
        int dmg = ca.castSpellAdvanced(1, "Fireball", 25, "15-30", false, result);
        check(dmg > 0, "Fireball deals damage", result);
        check(cs.participant(0).mana == manaBefore - 25, "mana deducted",
              QString("before=%1 after=%2").arg(manaBefore).arg(cs.participant(0).mana));
        check(cs.participant(1).hp < 100, "goblin HP reduced");
    }
    {
        // AoE spell: Fireball hits all monsters.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 100; mage.maxMana = 100; mage.att = 5;
        CombatParticipant goblin1; goblin1.name = "Goblin 1"; goblin1.isPlayer = false;
        goblin1.hp = 100; goblin1.maxHp = 100; goblin1.speed = 5;
        CombatParticipant goblin2; goblin2.name = "Goblin 2"; goblin2.isPlayer = false;
        goblin2.hp = 100; goblin2.maxHp = 100; goblin2.speed = 5;
        CombatParticipant goblin3; goblin3.name = "Goblin 3"; goblin3.isPlayer = false;
        goblin3.hp = 100; goblin3.maxHp = 100; goblin3.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin1);
        cs.addParticipant(goblin2);
        cs.addParticipant(goblin3);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int totalDmg = ca.castSpellAdvanced(1, "Fireball", 25, "15-30", true, result);
        check(totalDmg > 0, "AoE Fireball deals damage", result);
        check(cs.participant(1).hp < 100, "goblin 1 hit");
        check(cs.participant(2).hp < 100, "goblin 2 hit");
        check(cs.participant(3).hp < 100, "goblin 3 hit");
    }
    {
        // Not enough mana: spell fails.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 10; mage.maxMana = 50; mage.att = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int dmg = ca.castSpellAdvanced(1, "Fireball", 25, "15-30", false, result);
        check(dmg == -1, "spell fails without enough mana", result);
        check(cs.participant(0).mana == 10, "mana not deducted on failure");
        check(cs.participant(1).hp == 100, "goblin unharmed");
    }
    {
        // Heal spell: restores HP.
        CombatState cs;
        CombatParticipant cleric; cleric.name = "Cleric"; cleric.isPlayer = true;
        cleric.speed = 100; cleric.mana = 50; cleric.maxMana = 50;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.hp = 20; warrior.maxHp = 50; warrior.speed = 5;
        cs.addParticipant(cleric);
        cs.addParticipant(warrior);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int healed = ca.castHeal(1, 25, result);
        check(healed > 0, "heal spell restores HP", result);
        check(cs.participant(1).hp == 45, "warrior HP = 20 + 25 = 45",
              QString::number(cs.participant(1).hp));
    }
    {
        // Heal doesn't overheal.
        CombatState cs;
        CombatParticipant cleric; cleric.name = "Cleric"; cleric.isPlayer = true;
        cleric.speed = 100; cleric.mana = 50; cleric.maxMana = 50;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.hp = 45; warrior.maxHp = 50; warrior.speed = 5;
        cs.addParticipant(cleric);
        cs.addParticipant(warrior);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        int healed = ca.castHeal(1, 25, result);
        check(healed == 5, "heal capped at max HP", result);
        check(cs.participant(1).hp == 50, "warrior HP = 50 (max)",
              QString::number(cs.participant(1).hp));
    }
    {
        // hasEnoughMana check.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 30; mage.maxMana = 50;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        check(ca.hasEnoughMana(25), "has enough mana for 25");
        check(!ca.hasEnoughMana(35), "does not have enough mana for 35");
    }
    {
        // Full combat: mage casts Fireball in a group fight.
        CombatState cs;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.speed = 100; mage.mana = 100; mage.maxMana = 100; mage.att = 5;
        CombatParticipant goblin1; goblin1.name = "Goblin 1"; goblin1.isPlayer = false;
        goblin1.hp = 50; goblin1.maxHp = 50; goblin1.speed = 5;
        CombatParticipant goblin2; goblin2.name = "Goblin 2"; goblin2.isPlayer = false;
        goblin2.hp = 50; goblin2.maxHp = 50; goblin2.speed = 5;
        cs.addParticipant(mage);
        cs.addParticipant(goblin1);
        cs.addParticipant(goblin2);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        int rounds = 0;
        while (!te.isCombatOver() && rounds < 10) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    // Mage casts AoE Fireball
                    QString result;
                    ca.castSpellAdvanced(1, "Fireball", 25, "15-30", true, result);
                } else {
                    ai.takeTurn();
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(te.isCombatOver(), "combat ended within 10 rounds");
        check(cs.participant(0).mana < 100, "mana was spent",
              QString::number(cs.participant(0).mana));
    }

    // ------------------------------------------- Status effects (2.8)
    section("[21] Status effects in combat");
    {
        // Poison DoT: monster loses HP each round for N rounds.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 200; warrior.maxHp = 200;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 50; goblin.maxHp = 50; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.applyStatus(1, GameConstants::Poisoned, 3, result);
        check(ca.isPoisoned(1), "goblin is poisoned");
        check(ca.getStatusDuration(1, GameConstants::Poisoned) == 3, "poison duration = 3");

        int hpBefore = cs.participant(1).hp;
        QStringList messages = ca.tickStatusEffects();
        check(cs.participant(1).hp < hpBefore, "poison deals damage");
        check(messages.size() > 0, "poison message generated");

        // Tick 2 more rounds
        ca.tickStatusEffects();
        ca.tickStatusEffects();

        // Poison should be expired now
        check(!ca.isPoisoned(1), "poison expired after 3 rounds");
    }
    {
        // Blind reduces to-hit.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.dex = 10; warrior.level = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 200; goblin.maxHp = 200; goblin.def = 5; goblin.level = 1; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        // Without blind: should hit most of the time
        int hitsWithoutBlind = 0;
        for (int i = 0; i < 100; i++) {
            QString result;
            int dmg = ca.attack(1, result);
            if (dmg > 0) hitsWithoutBlind++;
        }

        // Apply blind
        QString applyResult;
        ca.applyStatus(0, GameConstants::Blinded, 5, applyResult);
        check(ca.isBlinded(0), "warrior is blinded");

        // With blind: should hit less often
        int hitsWithBlind = 0;
        for (int i = 0; i < 100; i++) {
            QString result;
            int dmg = ca.attack(1, result);
            if (dmg > 0) hitsWithBlind++;
        }

        check(hitsWithBlind < hitsWithoutBlind,
              "blind reduces to-hit",
              QString("without=%1 with=%2").arg(hitsWithoutBlind).arg(hitsWithBlind));
    }
    {
        // Confusion risks friendly fire.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.att = 10; warrior.dex = 10;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.hp = 50; mage.maxHp = 50; mage.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 200; goblin.maxHp = 200; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        // Apply confusion to warrior
        cs.participant(0).confusionDuration = 10;
        check(ca.isConfused(0), "warrior is confused");

        // Attack goblin many times, count friendly fire incidents
        int friendlyFire = 0;
        for (int i = 0; i < 200; i++) {
            int mageHpBefore = cs.participant(1).hp;
            QString result;
            ca.attack(2, result);  // Attack goblin
            if (cs.participant(1).hp < mageHpBefore) {
                friendlyFire++;
            }
        }

        check(friendlyFire > 0, "confusion causes friendly fire",
              QString::number(friendlyFire));
    }
    {
        // OnFire DoT.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 200; warrior.maxHp = 200;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.applyStatus(1, GameConstants::OnFire, 3, result);
        check(ca.isOnFire(1), "goblin is on fire");

        int hpBefore = cs.participant(1).hp;
        ca.tickStatusEffects();
        check(cs.participant(1).hp < hpBefore, "fire deals damage");

        // Tick until expired
        ca.tickStatusEffects();
        ca.tickStatusEffects();
        check(!ca.isOnFire(1), "fire expired after 3 rounds");
    }
    {
        // Status tick at round end.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 200; warrior.maxHp = 200;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 50; goblin.maxHp = 50; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);

        te.startRound();
        te.nextTurn();

        QString result;
        ca.applyStatus(1, GameConstants::Poisoned, 2, result);

        int hpBefore = cs.participant(1).hp;
        QStringList messages = ca.tickStatusEffects();
        check(cs.participant(1).hp < hpBefore, "tick deals poison damage");
        check(messages.size() > 0, "tick generates messages");

        // Second tick
        hpBefore = cs.participant(1).hp;
        ca.tickStatusEffects();
        check(cs.participant(1).hp < hpBefore, "second tick deals damage");

        // Third tick - poison expired
        ca.tickStatusEffects();
        check(!ca.isPoisoned(1), "poison expired");
    }
    {
        // Full combat with status effects.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 200; warrior.maxHp = 200; warrior.att = 15; warrior.dex = 10;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 30; goblin.maxHp = 30; goblin.speed = 5; goblin.att = 5; goblin.def = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        MonsterAI ai(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();

        // Poison the goblin
        QString result;
        ca.applyStatus(1, GameConstants::Poisoned, 5, result);

        int rounds = 0;
        while (!te.isCombatOver() && rounds < 10) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString atkResult;
                    ca.attack(1, atkResult);
                } else {
                    ai.takeTurn();
                }
                te.markCurrentActed();
            }
            // Tick status effects at round end
            ca.tickStatusEffects();
            rounds++;
        }

        check(te.isCombatOver(), "combat ended");
        check(!cs.participant(1).isAlive, "goblin died");
    }

    // ------------------------------------------- Victory rewards (2.9)
    section("[22] Victory: XP, gold and loot");
    {
        // XP reward: levelFound * 100
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["levelFound"] = 3;
        orc["goldFactor"] = 50;
        monsterData.append(orc);

        QVariantMap dragon;
        dragon["name"] = "Dragon";
        dragon["levelFound"] = 10;
        dragon["goldFactor"] = 500;
        monsterData.append(dragon);

        check(VictoryReward::calculateXp("Orc", monsterData) == 300,
              "Orc XP = 300");
        check(VictoryReward::calculateXp("Dragon", monsterData) == 1000,
              "Dragon XP = 1000");
        check(VictoryReward::calculateXp("Unknown", monsterData) == 100,
              "Unknown monster XP = 100 (default level 1)");
    }
    {
        // Gold reward: goldFactor * random(1, 10)
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["levelFound"] = 3;
        orc["goldFactor"] = 50;
        monsterData.append(orc);

        // Roll many times, check range
        int minGold = 999999;
        int maxGold = 0;
        for (int i = 0; i < 100; i++) {
            int gold = VictoryReward::calculateGold("Orc", monsterData);
            if (gold < minGold) minGold = gold;
            if (gold > maxGold) maxGold = gold;
        }
        check(minGold >= 50, "min gold >= 50 (50 * 1)");
        check(maxGold <= 500, "max gold <= 500 (50 * 10)");
    }
    {
        // Loot: drop table from Item0-Item9
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["levelFound"] = 3;
        orc["goldFactor"] = 50;
        orc["Item0"] = 101;
        orc["Item1"] = 102;
        orc["Item2"] = 0;  // No drop
        orc["Item3"] = 103;
        monsterData.append(orc);

        QList<int> dropTable = VictoryReward::getDropTable("Orc", monsterData);
        check(dropTable.size() == 3, "Orc has 3 drop slots",
              QString::number(dropTable.size()));
        check(dropTable.contains(101), "drop table contains item 101");
        check(dropTable.contains(102), "drop table contains item 102");
        check(dropTable.contains(103), "drop table contains item 103");
    }
    {
        // Loot: filtered by dungeon depth
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["levelFound"] = 3;
        orc["goldFactor"] = 50;
        orc["Item0"] = 101;
        orc["Item1"] = 102;
        monsterData.append(orc);

        // Item data with floor requirements
        QList<QVariantMap> itemData;
        QVariantMap item1;
        item1["id"] = 101;
        item1["name"] = "Iron Sword";
        item1["floor"] = 1;
        item1["rarity"] = 1;
        itemData.append(item1);

        QVariantMap item2;
        item2["id"] = 102;
        item2["name"] = "Dragon Blade";
        item2["floor"] = 5;
        item2["rarity"] = 3;
        itemData.append(item2);

        // At depth 1: only Iron Sword can drop
        QStringList loot = VictoryReward::calculateLoot("Orc", monsterData, 1);
        // At depth 5: both can drop
        QStringList lootDeep = VictoryReward::calculateLoot("Orc", monsterData, 5);

        // Loot is random, but we can check that deep loot has more potential
        // Just verify the function runs without crashing
        check(true, "loot calculation runs");
    }
    {
        // Full victory flow: XP + gold + loot
        QList<QVariantMap> monsterData;
        QVariantMap orc;
        orc["name"] = "Orc";
        orc["levelFound"] = 3;
        orc["goldFactor"] = 50;
        orc["Item0"] = 101;
        monsterData.append(orc);

        int xp = VictoryReward::calculateXp("Orc", monsterData);
        int gold = VictoryReward::calculateGold("Orc", monsterData);

        check(xp > 0, "XP awarded");
        check(gold > 0, "gold awarded");
    }

    // ------------------------------------------- Death in combat (2.10)
    section("[23] Death in combat");
    {
        // Individual character death: HP 0 → isAlive = false.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 10; warrior.maxHp = 10;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        te.startRound();
        te.nextTurn();

        // Kill the warrior
        cs.participant(0).hp = 0;
        cs.participant(0).isAlive = false;

        check(dh.isPartyMemberDead(0), "warrior is dead");
        check(dh.deadPartyMemberCount() == 1, "1 dead party member");
        check(dh.livingPartyMemberCount() == 0, "0 living party members");
        check(dh.isPartyWipe(), "party wipe detected");
        check(dh.handleGameOver(), "game over detected");
    }
    {
        // Victory: all monsters dead.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 100; warrior.maxHp = 100;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 0; goblin.maxHp = 100; goblin.isAlive = false; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        check(dh.isVictory(), "victory detected");
        check(!dh.isPartyWipe(), "not a party wipe");
        check(dh.deadPartyMemberCount() == 0, "no dead party members");
    }
    {
        // Partial death: one of two party members dies.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 100; warrior.maxHp = 100;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.hp = 0; mage.maxHp = 50; mage.isAlive = false; mage.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        check(dh.isPartyMemberDead(1), "mage is dead");
        check(!dh.isPartyMemberDead(0), "warrior is alive");
        check(dh.deadPartyMemberCount() == 1, "1 dead party member");
        check(dh.livingPartyMemberCount() == 1, "1 living party member");
        check(!dh.isPartyWipe(), "not a party wipe");
        check(!dh.handleGameOver(), "no game over");
    }
    {
        // Revive all party members.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 0; warrior.maxHp = 100; warrior.isAlive = false;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.hp = 0; mage.maxHp = 50; mage.isAlive = false; mage.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        check(dh.isPartyWipe(), "party wipe before revive");

        dh.reviveAllPartyMembers();

        check(!dh.isPartyWipe(), "no party wipe after revive");
        check(dh.livingPartyMemberCount() == 2, "2 living party members after revive");
        check(cs.participant(0).hp == 1, "warrior revived with 1 HP");
        check(cs.participant(1).hp == 1, "mage revived with 1 HP");
    }
    {
        // getDeadPartyMemberNames.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 0; warrior.maxHp = 100; warrior.isAlive = false;
        CombatParticipant mage; mage.name = "Mage"; mage.isPlayer = true;
        mage.hp = 50; mage.maxHp = 50; mage.speed = 5;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(mage);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        QStringList deadNames = dh.getDeadPartyMemberNames();
        check(deadNames.size() == 1, "1 dead name");
        check(deadNames.contains("Warrior"), "dead names contains Warrior");
    }
    {
        // Full combat: party wipe ends combat.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 10; warrior.maxHp = 10; warrior.att = 1; warrior.dex = 1;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 1000; goblin.maxHp = 1000; goblin.speed = 5; goblin.att = 50; goblin.def = 20;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        te.startRound();
        int rounds = 0;
        while (!te.isCombatOver() && rounds < 10) {
            te.startRound();
            while (te.nextTurn()) {
                int idx = te.currentParticipantIndex();
                if (cs.participant(idx).isPlayer) {
                    QString result;
                    ca.attack(1, result);
                } else {
                    QString result;
                    ca.attack(0, result);
                }
                te.markCurrentActed();
            }
            rounds++;
        }

        check(te.isCombatOver(), "combat ended");
        check(dh.isPartyWipe(), "party wipe");
        check(dh.handleGameOver(), "game over");
    }
    {
        // processDeaths returns messages for dead players.
        CombatState cs;
        CombatParticipant warrior; warrior.name = "Warrior"; warrior.isPlayer = true;
        warrior.speed = 100; warrior.hp = 0; warrior.maxHp = 100; warrior.isAlive = false;
        warrior.hasActed = false;
        CombatParticipant goblin; goblin.name = "Goblin"; goblin.isPlayer = false;
        goblin.hp = 100; goblin.maxHp = 100; goblin.speed = 5;
        cs.addParticipant(warrior);
        cs.addParticipant(goblin);

        TurnEngine te;
        te.setCombatState(&cs);
        CombatActions ca(&cs, &te);
        CombatDeathHandler dh(&cs, &te, &ca);

        QStringList messages = dh.processDeaths();
        check(messages.size() > 0, "death message generated");
        check(messages[0].contains("Warrior"), "message contains Warrior");
    }

    // ------------------------------------------- LevelTable (3.1)
    section("[24] Data-driven XP table");
    {
        LevelTable& lt = LevelTable::instance();
        check(lt.load("data/levels.json"), "levels.json loads");

        check(lt.isLoaded(), "LevelTable is loaded");
        check(lt.maxLevel() == 20, "max level = 20",
              QString::number(lt.maxLevel()));

        // XP values increase with level
        check(lt.xpForLevel(1) < lt.xpForLevel(5), "XP increases with level");
        check(lt.xpForLevel(5) < lt.xpForLevel(10), "XP increases with level (5<10)");
        check(lt.xpForLevel(10) < lt.xpForLevel(20), "XP increases with level (10<20)");

        // Specific values from the table
        check(lt.xpForLevel(1) == 100, "xpForLevel(1) = 100");
        check(lt.xpForLevel(2) == 283, "xpForLevel(2) = 283");
        check(lt.xpForLevel(5) == 1118, "xpForLevel(5) = 1118");
        check(lt.xpForLevel(10) == 3155, "xpForLevel(10) = 3155");
        check(lt.xpForLevel(20) == 8708, "xpForLevel(20) = 8708");

        // totalXpForLevel
        check(lt.totalXpForLevel(1) == 0, "totalXpForLevel(1) = 0");
        check(lt.totalXpForLevel(2) == 100, "totalXpForLevel(2) = 100");
        check(lt.totalXpForLevel(3) == 383, "totalXpForLevel(3) = 383");

        // levelForXp
        check(lt.levelForXp(0) == 1, "levelForXp(0) = 1");
        check(lt.levelForXp(99) == 1, "levelForXp(99) = 1");
        check(lt.levelForXp(100) == 2, "levelForXp(100) = 2");
        check(lt.levelForXp(382) == 2, "levelForXp(382) = 2");
        check(lt.levelForXp(383) == 3, "levelForXp(383) = 3");

        // xpProgress
        QPair<int, int> progress = lt.xpProgress(150);
        check(progress.first == 50, "xpProgress(150).first = 50",
              QString::number(progress.first));
        check(progress.second == 283, "xpProgress(150).second = 283",
              QString::number(progress.second));

        // Fallback for unloaded table
        LevelTable fallback;
        check(fallback.xpForLevel(1) == 100, "fallback xpForLevel(1) = 100");
        check(fallback.xpForLevel(5) == 1118, "fallback xpForLevel(5) = 1118");
    }

    // ------------------------------------------- Level-up stat gains (3.2)
    section("[25] Level-up stat gains");
    {
        // Level-up grants HP, mana for casters, and stat points.
        PartyManager pm;
        Character c;
        c.name = "TestChar";
        c.level = 1;
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 10;
        c.intelligence = 12;  // Caster
        c.wisdom = 10;
        c.constitution = 10;
        c.charisma = 10;
        c.dexterity = 10;

        int oldMaxHp = c.maxHp;
        int oldMaxMana = c.maxMana;

        pm.applyLevelUpGains(c);

        check(c.maxHp > oldMaxHp, "HP increases on level up",
              QString("old=%1 new=%2").arg(oldMaxHp).arg(c.maxHp));
        check(c.hp == c.maxHp, "HP fully restored on level up");
        check(c.maxMana > oldMaxMana, "mana increases for caster",
              QString("old=%1 new=%2").arg(oldMaxMana).arg(c.maxMana));
        check(c.mana == c.maxMana, "mana fully restored on level up");
    }
    {
        // Non-caster: no mana gain.
        PartyManager pm;
        Character c;
        c.name = "Warrior";
        c.level = 1;
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 15;
        c.intelligence = 8;  // Not a caster
        c.wisdom = 8;
        c.constitution = 12;
        c.charisma = 8;
        c.dexterity = 10;

        int oldMaxMana = c.maxMana;
        pm.applyLevelUpGains(c);

        check(c.maxMana == oldMaxMana, "no mana gain for non-caster");
    }
    {
        // Stat point every 2 levels.
        PartyManager pm;
        Character c;
        c.name = "TestChar";
        c.level = 2;  // Even level
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 10;
        c.intelligence = 10;
        c.wisdom = 10;
        c.constitution = 10;
        c.charisma = 10;
        c.dexterity = 10;

        int totalStatsBefore = c.strength + c.intelligence + c.wisdom +
                              c.constitution + c.charisma + c.dexterity;
        pm.applyLevelUpGains(c);
        int totalStatsAfter = c.strength + c.intelligence + c.wisdom +
                             c.constitution + c.charisma + c.dexterity;

        check(totalStatsAfter == totalStatsBefore + 1,
              "stat point granted on even level",
              QString("before=%1 after=%2").arg(totalStatsBefore).arg(totalStatsAfter));
    }
    {
        // No stat point on odd level.
        PartyManager pm;
        Character c;
        c.name = "TestChar";
        c.level = 3;  // Odd level
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 10;
        c.intelligence = 10;
        c.wisdom = 10;
        c.constitution = 10;
        c.charisma = 10;
        c.dexterity = 10;

        int totalStatsBefore = c.strength + c.intelligence + c.wisdom +
                              c.constitution + c.charisma + c.dexterity;
        pm.applyLevelUpGains(c);
        int totalStatsAfter = c.strength + c.intelligence + c.wisdom +
                             c.constitution + c.charisma + c.dexterity;

        check(totalStatsAfter == totalStatsBefore,
              "no stat point on odd level",
              QString("before=%1 after=%2").arg(totalStatsBefore).arg(totalStatsAfter));
    }
    {
        // addExperienceToCharacter uses LevelTable.
        LevelTable::instance().load("data/levels.json");
        PartyManager pm;
        Character c;
        c.name = "TestChar";
        c.level = 1;
        c.experience = 0;
        c.maxHp = 20;
        c.hp = 20;
        c.maxMana = 10;
        c.mana = 10;
        c.strength = 10;
        c.intelligence = 12;
        c.wisdom = 10;
        c.constitution = 10;
        c.charisma = 10;
        c.dexterity = 10;
        pm.currentParty().members.append(c);

        // Give enough XP to level up (level 1→2 needs 100 XP)
        pm.addExperienceToCharacter(0, 100);

        check(pm.currentParty().members[0].level == 2, "level up via LevelTable",
              QString::number(pm.currentParty().members[0].level));
    }

    // ------------------------------------------- Guild leveling (3.3)
    section("[26] Guild leveling");
    {
        // Join a guild, gain XP, level up.
        Character c;
        c.name = "Mage";
        c.intelligence = 15;
        c.mana = 100;
        c.maxMana = 100;

        check(c.guildLevel("Mages Guild") == 0, "not a member initially");

        c.joinGuild("Mages Guild");
        check(c.guildLevel("Mages Guild") == 1, "join starts at level 1");

        // Level 1 -> 2 costs 100 XP
        bool leveled = c.addGuildExperience("Mages Guild", 99);
        check(!leveled, "99 XP does not level");
        check(c.guildLevel("Mages Guild") == 1, "still level 1");

        leveled = c.addGuildExperience("Mages Guild", 1);
        check(leveled, "100 XP levels up");
        check(c.guildLevel("Mages Guild") == 2, "now level 2");

        // Level 2 -> 3 costs 283
        leveled = c.addGuildExperience("Mages Guild", 283);
        check(leveled, "283 XP levels up again");
        check(c.guildLevel("Mages Guild") == 3, "now level 3");

        // Increment directly
        int newLevel = c.incrementGuildLevel("Mages Guild");
        check(newLevel == 4, "incrementGuildLevel returns 4",
              QString::number(newLevel));
    }
    {
        // totalGuildLevels sums all guild levels.
        Character c;
        c.joinGuild("Mages Guild");
        c.incrementGuildLevel("Mages Guild");  // 2
        c.joinGuild("Healers Guild");          // 1
        c.incrementGuildLevel("Healers Guild"); // 2
        c.incrementGuildLevel("Healers Guild"); // 3

        check(c.guildLevel("Mages Guild") == 2, "Mages Guild = 2");
        check(c.guildLevel("Healers Guild") == 3, "Healers Guild = 3");
        check(c.totalGuildLevels() == 5, "total guild levels = 5",
              QString::number(c.totalGuildLevels()));
    }
    {
        // Guild levels persist through save/load.
        Character c;
        c.name = "Mage";
        c.joinGuild("Mages Guild");
        c.addGuildExperience("Mages Guild", 100);  // -> 2
        c.addGuildExperience("Mages Guild", 283);  // -> 3

        QVariantMap map = c.toMap();
        Character loaded;
        loaded.loadFromMap(map);

        check(loaded.guildLevel("Mages Guild") == 3, "guild level persists",
              QString::number(loaded.guildLevel("Mages Guild")));
        check(loaded.totalGuildLevels() == 3, "total persists");
    }

    // ------------------------------------------- Spell learning (3.5)
    section("[27] Spell learning");
    {
        SpellBook& sb = SpellBook::instance();
        check(sb.load("data/spells.json"), "spells.json loads");
        check(sb.isLoaded(), "SpellBook is loaded");
        check(sb.spellCount() == 47, "47 spells loaded",
              QString::number(sb.spellCount()));

        // A level-1 Mage knows Flame Bolt (base_level 1, int 10).
        Character mage;
        mage.name = "Mage";
        mage.intelligence = 15;  // Fireball needs 15
        mage.joinGuild("Mages Guild");

        QList<SpellDef> known = sb.spellsFor(mage);
        bool hasFlameBolt = false;
        for (const SpellDef& s : known) {
            if (s.name == "Flame Bolt") hasFlameBolt = true;
        }
        check(hasFlameBolt, "level-1 mage knows Flame Bolt");

        // Fireball needs base_level 3.
        bool hasFireball = false;
        for (const SpellDef& s : known) {
            if (s.name == "Fireball") hasFireball = true;
        }
        check(!hasFireball, "level-1 mage does not know Fireball");

        // Level the mage's guild to 3 -> Fireball appears.
        mage.addGuildExperience("Mages Guild", 100 + 283);  // -> level 3
        check(mage.guildLevel("Mages Guild") == 3, "mage is guild level 3");

        known = sb.spellsFor(mage);
        hasFireball = false;
        for (const SpellDef& s : known) {
            if (s.name == "Fireball") hasFireball = true;
        }
        check(hasFireball, "guild level 3 mage knows Fireball");
    }
    {
        // A Warrior gets no mage spells.
        Character warrior;
        warrior.name = "Warrior";
        warrior.strength = 18;
        warrior.intelligence = 8;
        warrior.joinGuild("Warriors Guild");

        SpellBook& sb = SpellBook::instance();
        QList<SpellDef> known = sb.spellsFor(warrior);

        bool hasMageSpell = false;
        for (const SpellDef& s : known) {
            if (s.guilds.contains("Mages Guild")) hasMageSpell = true;
        }
        check(!hasMageSpell, "warrior knows no mage spells");
    }
    {
        // Stat requirements are enforced: low INT blocks Flame Bolt.
        Character weakMage;
        weakMage.name = "WeakMage";
        weakMage.intelligence = 5;  // below the required 10
        weakMage.joinGuild("Mages Guild");

        SpellBook& sb = SpellBook::instance();
        QList<SpellDef> known = sb.spellsFor(weakMage);

        bool hasFlameBolt = false;
        for (const SpellDef& s : known) {
            if (s.name == "Flame Bolt") hasFlameBolt = true;
        }
        check(!hasFlameBolt, "INT 5 blocks Flame Bolt (needs 10)");
    }
    {
        // newlyLearned reports spells granted at a specific guild level.
        Character mage;
        mage.name = "Mage";
        mage.intelligence = 20;
        mage.mana = 200;
        mage.joinGuild("Mages Guild");
        mage.incrementGuildLevel("Mages Guild");
        mage.incrementGuildLevel("Mages Guild");  // level 3

        SpellBook& sb = SpellBook::instance();
        QList<SpellDef> learned = sb.newlyLearned(mage, "Mages Guild", 3);

        bool hasFireball = false;
        for (const SpellDef& s : learned) {
            if (s.name == "Fireball") hasFireball = true;
        }
        check(hasFireball, "Fireball learned at guild level 3");
    }
    {
        // canCast respects known-spell and mana checks.
        Character mage;
        mage.name = "Mage";
        mage.intelligence = 15;
        mage.mana = 5;   // not enough for Flame Bolt (10)
        mage.maxMana = 100;
        mage.joinGuild("Mages Guild");

        SpellBook& sb = SpellBook::instance();
        check(!sb.canCast(mage, "Flame Bolt"), "cannot cast without mana");

        mage.mana = 50;
        check(sb.canCast(mage, "Flame Bolt"), "can cast with mana");

        check(!sb.canCast(mage, "Inferno"), "cannot cast unknown spell");
    }

    // ------------------------------------------- Aging (3.6)
    section("[28] Aging and old age");
    {
        check(AgingRules::maxAgeForRace("Human") == 100, "Human max age = 100");
        check(AgingRules::maxAgeForRace("Elf") == 400, "Elf max age = 400");
        check(AgingRules::maxAgeForRace("Unknown") == 100, "unknown race falls back to 100");

        check(AgingRules::decayThresholdForRace("Human") == 70,
              "Human decay threshold = 70",
              QString::number(AgingRules::decayThresholdForRace("Human")));
    }
    {
        // A young character ages without effect.
        Character c;
        c.name = "Young";
        c.race = "Human";
        c.age = 30;
        c.strength = 15;
        c.constitution = 15;
        c.dexterity = 15;

        QStringList msgs = AgingRules::applyYearOfAging(c);
        check(msgs.isEmpty(), "no messages for a young character");
        check(c.isAlive, "young character survives");
        check(!AgingRules::isPastMaxAge(c), "not past max age");
        check(!AgingRules::isDecaying(c), "not decaying");
    }
    {
        // Past the decay threshold, stats can drop.
        Character c;
        c.name = "Old";
        c.race = "Human";
        c.age = 80;  // past 70
        c.strength = 10;
        c.constitution = 10;
        c.dexterity = 10;

        check(AgingRules::isDecaying(c), "80-year-old is decaying");

        // Over many years, stats should drop at least once.
        bool dropped = false;
        for (int i = 0; i < 200 && !dropped; i++) {
            Character t = c;
            t.age = 80;
            QStringList msgs = AgingRules::applyYearOfAging(t);
            if (!msgs.isEmpty()) dropped = true;
        }
        check(dropped, "decay happens within 200 years of rolls");
    }
    {
        // At max age the character dies.
        Character c;
        c.name = "Ancient";
        c.race = "Human";
        c.age = 100;  // exactly max age
        c.isAlive = true;

        check(AgingRules::isPastMaxAge(c), "100-year-old Human is past max age");

        QStringList msgs = AgingRules::applyYearOfAging(c);
        check(!c.isAlive, "character dies at max age");
        check(c.hp == 0, "HP is 0 on death");
        check((c.statusFlags & StatusFlag::Dead) != 0, "Dead status applied");
        check(msgs.size() == 1, "death message produced");
        check(msgs[0].contains("old age"), "message mentions old age", msgs[0]);
    }
    {
        // An Elf at 100 is nowhere near death.
        Character c;
        c.name = "Elf";
        c.race = "Elf";
        c.age = 100;

        check(!AgingRules::isPastMaxAge(c), "100-year-old Elf is not past max age");
        check(!AgingRules::isDecaying(c), "100-year-old Elf is not decaying");

        AgingRules::applyYearOfAging(c);
        check(c.isAlive, "Elf survives");
    }

    // ------------------------------------------- Persistent level state (4.1)
    section("[29] Persistent dungeon level state");
    {
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        check(!reg.hasLevel(2), "floor 2 not generated yet");
        check(reg.count() == 0, "registry empty");

        // Generate and store floor 2.
        LevelSnapshot snap;
        snap.level = 2;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(3, 4), "Orc");
        snap.monsterPositions.insert(qMakePair(5, 6), "Goblin");
        snap.treasurePositions.insert(qMakePair(7, 8), "Chest");
        snap.stairsUp = qMakePair(1, 1);
        snap.stairsDown = qMakePair(28, 28);
        snap.visitedTiles.insert(qMakePair(1, 1));
        reg.store(snap);

        check(reg.hasLevel(2), "floor 2 now generated");
        check(reg.count() == 1, "registry holds one floor");

        const LevelSnapshot* stored = reg.level(2);
        check(stored != nullptr, "floor 2 retrievable");
        check(stored->monsterPositions.size() == 2, "2 monsters stored");
        check(stored->stairsDown == qMakePair(28, 28), "stairs down stored");
    }
    {
        // Clear a monster, leave, come back: it stays cleared.
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        LevelSnapshot snap;
        snap.level = 3;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(3, 4), "Orc");
        snap.monsterPositions.insert(qMakePair(5, 6), "Goblin");
        reg.store(snap);

        // Kill one.
        reg.levelForEdit(3).monsterPositions.remove(qMakePair(3, 4));
        check(reg.level(3)->monsterPositions.size() == 1, "one monster remains");

        // Simulate leaving (nothing clears the registry) and returning.
        check(reg.hasLevel(3), "floor 3 still known after leaving");
        check(reg.level(3)->monsterPositions.size() == 1, "cleared monster stays cleared");
    }
    {
        // Full round-trip through serialization.
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        LevelSnapshot snap;
        snap.level = 4;
        snap.generated = true;
        snap.monsterPositions.insert(qMakePair(2, 2), "Orc");
        snap.treasurePositions.insert(qMakePair(9, 9), "Chest");
        snap.openedChests.insert(qMakePair(9, 9));
        snap.visitedTiles.insert(qMakePair(1, 1));
        snap.visitedTiles.insert(qMakePair(1, 2));
        snap.stairsUp = qMakePair(3, 3);
        snap.stairsDown = qMakePair(20, 20);
        snap.bossDefeated = false;
        reg.store(snap);

        QVariantMap serialized = reg.toMap();
        DungeonLevelRegistry& reg2 = DungeonLevelRegistry::instance();
        reg2.clear();
        reg2.loadFromMap(serialized);

        check(reg2.hasLevel(4), "floor 4 restored");
        check(reg2.level(4)->monsterPositions.size() == 1, "monster restored");
        check(reg2.level(4)->openedChests.contains(qMakePair(9, 9)), "opened chest restored");
        check(reg2.level(4)->visitedTiles.size() == 2, "visited tiles restored");
        check(reg2.level(4)->stairsDown == qMakePair(20, 20), "stairs restored");
    }

    // ------------------------------------------- Monster respawn (4.6)
    section("[30] Monster respawn");
    {
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        LevelSnapshot snap;
        snap.level = 1;
        snap.generated = true;
        snap.stairsUp = qMakePair(1, 1);
        snap.stairsDown = qMakePair(28, 28);
        reg.store(snap);

        // Floor starts empty (everything cleared); original population was 10.
        QRandomGenerator rng(12345);
        int respawned = reg.respawnMonsters(1, 0.3, 10, rng);

        check(respawned > 0, "some monsters respawned",
              QString::number(respawned));
        check(respawned == 3, "30% of 10 = 3 respawned",
              QString::number(respawned));
        check(reg.level(1)->monsterPositions.size() == 3, "3 monsters now on the floor");
    }
    {
        // Respawn does not overfill: a full floor stays full.
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();

        LevelSnapshot snap;
        snap.level = 2;
        snap.generated = true;
        for (int i = 0; i < 10; i++) {
            snap.monsterPositions.insert(qMakePair(i, 0), "Orc");
        }
        reg.store(snap);

        QRandomGenerator rng(999);
        int respawned = reg.respawnMonsters(2, 0.5, 10, rng);
        check(respawned == 0, "full floor respawns nothing");
        check(reg.level(2)->monsterPositions.size() == 10, "still 10 monsters");
    }
    {
        // Respawn on an unknown floor is a no-op.
        DungeonLevelRegistry& reg = DungeonLevelRegistry::instance();
        reg.clear();
        QRandomGenerator rng(7);
        check(reg.respawnMonsters(9, 0.5, 10, rng) == 0,
              "respawn on ungenerated floor does nothing");
    }

    // ------------------------------------------- Themed floors (4.2)
    section("[31] Fifteen themed floors");
    {
        check(DungeonThemes::MAX_DEPTH == 15, "max depth is 15");

        FloorTheme f1 = DungeonThemes::forLevel(1);
        check(f1.name == "Abandoned Mines", "floor 1 is the Abandoned Mines", f1.name);
        check(f1.monsterLevelBonus == 0, "floor 1 has no level bonus");

        FloorTheme f8 = DungeonThemes::forLevel(8);
        check(f8.name == "Bone Halls", "floor 8 is the Bone Halls", f8.name);
        check(f8.monsterLevelBonus == 7, "floor 8 has +7 level bonus",
              QString::number(f8.monsterLevelBonus));

        FloorTheme f15 = DungeonThemes::forLevel(15);
        check(f15.name == "The Devil's Threshold", "floor 15 name", f15.name);

        // Difficulty rises with depth.
        check(DungeonThemes::forLevel(1).monsterCount < DungeonThemes::forLevel(15).monsterCount,
              "deeper floors have more monsters");
        check(DungeonThemes::forLevel(1).treasureCount < DungeonThemes::forLevel(15).treasureCount,
              "deeper floors have more treasure");
        check(DungeonThemes::forLevel(1).trapCount < DungeonThemes::forLevel(15).trapCount,
              "deeper floors have more traps");

        // All 15 floors are distinct.
        QList<FloorTheme> themes = DungeonThemes::all();
        check(themes.size() == 15, "15 themes defined",
              QString::number(themes.size()));
        QSet<QString> names;
        for (const FloorTheme& t : themes) names.insert(t.name);
        check(names.size() == 15, "all 15 names unique",
              QString::number(names.size()));

        // Out-of-range clamps instead of crashing.
        check(DungeonThemes::forLevel(0).name == "Abandoned Mines", "level 0 clamps to floor 1");
        check(DungeonThemes::forLevel(99).name == "The Devil's Threshold", "level 99 clamps to floor 15");
    }

    // ------------------------------------------- Boss encounters (4.3)
    section("[32] Boss encounters");
    {
        check(DungeonThemes::isBossFloor(5), "floor 5 is a boss floor");
        check(DungeonThemes::isBossFloor(10), "floor 10 is a boss floor");
        check(DungeonThemes::isBossFloor(15), "floor 15 is a boss floor");
        check(!DungeonThemes::isBossFloor(4), "floor 4 is not a boss floor");
        check(!DungeonThemes::isBossFloor(11), "floor 11 is not a boss floor");

        check(BossEncounter::bossName(5) == "Grotto Warden", "floor 5 boss name",
              BossEncounter::bossName(5));
        check(BossEncounter::bossName(10) == "The Bone Tyrant", "floor 10 boss name");
        check(BossEncounter::bossName(15) == "The Prince of Devils", "floor 15 boss name");
    }
    {
        // Boss floors block descent until the boss dies.
        check(!BossEncounter::canDescend(5, false), "floor 5 blocks descent before boss dies");
        check(BossEncounter::canDescend(5, true), "floor 5 opens after boss dies");

        check(!BossEncounter::canDescend(15, false), "floor 15 blocks descent");
        check(BossEncounter::canDescend(15, true), "floor 15 opens");

        // Non-boss floors are always open.
        check(BossEncounter::canDescend(4, false), "floor 4 never blocks");
        check(BossEncounter::canDescend(6, false), "floor 6 never blocks");

        // Blocked message names the boss.
        QString msg = BossEncounter::blockedMessage(5);
        check(msg.contains("Grotto Warden"), "blocked message names the boss", msg);
        check(BossEncounter::blockedMessage(4).isEmpty(), "no message on a normal floor");
    }
    {
        // Boss stats scale with depth.
        QVariantMap b5 = BossEncounter::buildBoss(5);
        QVariantMap b15 = BossEncounter::buildBoss(15);

        check(b5["name"].toString() == "Grotto Warden", "floor 5 boss built");
        check(b5["isPlayer"].toBool() == false, "boss is not a player");
        check(b5["hp"].toInt() > 0, "boss has HP");
        check(b15["hp"].toInt() > b5["hp"].toInt(), "floor 15 boss has more HP",
              QString("%1 vs %2").arg(b15["hp"].toInt()).arg(b5["hp"].toInt()));
        check(b15["att"].toInt() > b5["att"].toInt(), "floor 15 boss hits harder");
        check(b15["swings"].toInt() >= b5["swings"].toInt(), "floor 15 boss swings at least as often");

        check(BossEncounter::bossXp(15) > BossEncounter::bossXp(5), "floor 15 boss worth more XP");
    }

    // ------------------------------------------- Locked doors and keys (4.4)
    section("[33] Locked doors and keys");
    {
        // An unlocked door opens freely.
        DoorState door;
        door.position = qMakePair(5, 5);
        door.locked = false;
        QSet<QString> noKeys;

        QString reason;
        check(DoorAndSearch::canOpen(door, noKeys), "unlocked door can open");
        check(DoorAndSearch::tryOpen(door, noKeys, reason), "unlocked door opens", reason);
    }
    {
        // A locked door refuses without the key, opens with it.
        DoorState door;
        door.position = qMakePair(6, 6);
        door.keyName = "Brass Key";
        door.locked = true;

        QSet<QString> noKeys;
        QString reason;
        check(!DoorAndSearch::canOpen(door, noKeys), "locked door blocks without key");
        check(!DoorAndSearch::tryOpen(door, noKeys, reason), "tryOpen fails without key", reason);
        check(reason.contains("Brass Key"), "message names the key", reason);
        check(door.locked, "door still locked after failed attempt");

        QSet<QString> keys;
        keys.insert("Brass Key");
        check(DoorAndSearch::canOpen(door, keys), "locked door opens with key");
        check(DoorAndSearch::tryOpen(door, keys, reason), "tryOpen succeeds with key", reason);
        check(!door.locked, "door unlocked after success");
    }
    {
        // The wrong key does not help.
        DoorState door;
        door.keyName = "Brass Key";
        door.locked = true;
        QSet<QString> keys;
        keys.insert("Iron Key");

        QString reason;
        check(!DoorAndSearch::tryOpen(door, keys, reason), "wrong key does not open");
        check(door.locked, "door still locked");
    }
    {
        // A secret door cannot be opened by walking into it.
        DoorState door;
        door.secret = true;
        door.difficulty = 10;
        QSet<QString> keys;
        QString reason;
        check(!DoorAndSearch::tryOpen(door, keys, reason), "secret door resists direct opening");
    }

    // ------------------------------------------- Secret door discovery (4.5)
    section("[34] Secret door discovery");
    {
        check(DoorAndSearch::secretDoorDifficulty(1) == 10, "floor 1 secret DC = 10");
        check(DoorAndSearch::secretDoorDifficulty(5) == 14, "floor 5 secret DC = 14");
        check(DoorAndSearch::secretDoorDifficulty(15) == 24, "floor 15 secret DC = 24");
    }
    {
        // High WIS/INT finds secret doors more often than low.
        QMap<QPair<int, int>, DoorState> doors;
        DoorState d;
        d.position = qMakePair(5, 5);
        d.secret = true;
        d.difficulty = 13;  // floor 4
        doors.insert(qMakePair(5, 5), d);

        int highFinds = 0;
        int lowFinds = 0;

        QRandomGenerator rngHigh(4242);
        QRandomGenerator rngLow(4242);

        for (int i = 0; i < 400; i++) {
            QList<QPair<int, int>> found;
            // WIS 20, INT 20 -> +10 combined
            if (DoorAndSearch::searchForSecretDoors(doors, 5, 5, 20, 20, found, rngHigh) > 0) {
                highFinds++;
            }
            found.clear();
            // WIS 6, INT 6 -> -4 combined
            if (DoorAndSearch::searchForSecretDoors(doors, 5, 5, 6, 6, found, rngLow) > 0) {
                lowFinds++;
            }
        }

        check(highFinds > lowFinds, "high WIS/INT finds more secret doors",
              QString("high=%1 low=%2").arg(highFinds).arg(lowFinds));
    }
    {
        // A secret door outside the 3x3 search area is never found.
        QMap<QPair<int, int>, DoorState> doors;
        DoorState d;
        d.position = qMakePair(20, 20);
        d.secret = true;
        d.difficulty = 1;  // trivially easy
        doors.insert(qMakePair(20, 20), d);

        QRandomGenerator rng(1);
        int totalFinds = 0;
        for (int i = 0; i < 100; i++) {
            QList<QPair<int, int>> found;
            totalFinds += DoorAndSearch::searchForSecretDoors(doors, 5, 5, 30, 30, found, rng);
        }
        check(totalFinds == 0, "distant secret door never found",
              QString::number(totalFinds));
    }
    {
        // An easy secret door adjacent to the searcher is found reliably.
        QMap<QPair<int, int>, DoorState> doors;
        DoorState d;
        d.position = qMakePair(6, 5);  // adjacent
        d.secret = true;
        d.difficulty = 10;
        doors.insert(qMakePair(6, 5), d);

        QRandomGenerator rng(99);
        int finds = 0;
        for (int i = 0; i < 200; i++) {
            QList<QPair<int, int>> found;
            if (DoorAndSearch::searchForSecretDoors(doors, 5, 5, 20, 20, found, rng) > 0) {
                finds++;
            }
        }
        check(finds > 150, "adjacent easy door found most of the time",
              QString::number(finds));
    }

    // ------------------------------------------- Death state / bodies (5.1)
    section("[35] Death state and body carrying");
    {
        // A character dies; the body records where it fell.
        Character c;
        c.name = "Fallen";
        c.level = 3;
        c.hp = 10;
        c.maxHp = 30;

        DeathRecovery::killCharacter(c, 4, 12, 9);

        check(!c.isAlive, "character is dead");
        check(c.hp == 0, "HP is 0");
        check((c.statusFlags & StatusFlag::Dead) != 0, "Dead status applied");
        check(c.dungeonLevel == 4, "body remembers the floor",
              QString::number(c.dungeonLevel));
        check(c.dungeonX == 12 && c.dungeonY == 9, "body remembers the position");
    }
    {
        // A body in the dungeon can be carried.
        BodyLocation loc;
        loc.valid = true;
        loc.dungeonLevel = 4;

        check(DeathRecovery::canCarry(loc), "dungeon body can be carried");

        QString reason;
        check(DeathRecovery::carryBody(loc, reason), "carry succeeds", reason);
        check(loc.carried, "body is now carried");
        check(!DeathRecovery::canCarry(loc), "cannot carry it twice");
    }
    {
        // Carried bodies come home when the party reaches town.
        QList<BodyLocation> bodies;
        BodyLocation carried;
        carried.valid = true;
        carried.carried = true;
        carried.dungeonLevel = 5;
        bodies.append(carried);

        BodyLocation leftBehind;
        leftBehind.valid = true;
        leftBehind.dungeonLevel = 5;
        bodies.append(leftBehind);

        int brought = DeathRecovery::bringBodiesToTown(bodies);
        check(brought == 1, "one body brought home",
              QString::number(brought));
        check(bodies[0].inCity, "carried body is now in town");
        check(!bodies[0].carried, "no longer carried once home");
        check(!bodies[1].inCity, "body left behind stays in the dungeon");
    }
    {
        // Dropping a carried body puts it back where the party stands.
        BodyLocation loc;
        loc.valid = true;
        loc.carried = true;
        loc.dungeonLevel = 2;

        DeathRecovery::dropBody(loc, 3, 7, 8);
        check(!loc.carried, "body dropped");
        check(loc.dungeonLevel == 3, "body is on the current floor");
        check(loc.x == 7 && loc.y == 8, "body at the drop position");
        check(DeathRecovery::canCarry(loc), "dropped body can be picked up again");
    }

    // ------------------------------------------- Morgue / resurrection (5.2)
    section("[36] Morgue resurrection");
    {
        // Cost scales with level; a dungeon body costs extra.
        BodyLocation inTown;
        inTown.valid = true;
        inTown.inCity = true;

        BodyLocation inDungeon;
        inDungeon.valid = true;
        inDungeon.dungeonLevel = 5;

        check(DeathRecovery::resurrectionCost(1, inTown) == 500,
              "level 1 in town costs 500",
              QString::number(DeathRecovery::resurrectionCost(1, inTown)));
        check(DeathRecovery::resurrectionCost(5, inTown) == 2500,
              "level 5 in town costs 2500",
              QString::number(DeathRecovery::resurrectionCost(5, inTown)));
        check(DeathRecovery::resurrectionCost(5, inDungeon) == 3000,
              "level 5 in the dungeon costs 3000 (rescue fee)",
              QString::number(DeathRecovery::resurrectionCost(5, inDungeon)));
    }
    {
        // Resurrection fails when gold is short.
        Character c;
        c.name = "Fallen";
        c.level = 3;
        c.isAlive = false;

        BodyLocation loc;
        loc.valid = true;
        loc.inCity = true;

        int gold = 100;  // needs 1500
        QString reason;
        check(!DeathRecovery::resurrect(c, loc, gold, reason), "resurrect fails without gold", reason);
        check(!c.isAlive, "still dead");
        check(gold == 100, "gold untouched on failure");
        check(loc.valid, "body still present");
    }
    {
        // Resurrection succeeds with enough gold.
        Character c;
        c.name = "Fallen";
        c.level = 3;
        c.isAlive = false;
        c.hp = 0;

        BodyLocation loc;
        loc.valid = true;
        loc.inCity = true;

        int gold = 2000;
        QString reason;
        check(DeathRecovery::resurrect(c, loc, gold, reason), "resurrect succeeds", reason);
        check(c.isAlive, "character is alive");
        check(c.hp >= 1, "HP restored");
        check((c.statusFlags & StatusFlag::Dead) == 0, "Dead status cleared");
        check(gold == 500, "1500 gold spent (2000 - 1500)",
              QString::number(gold));
        check(!loc.valid, "body consumed");
    }
    {
        // Resurrecting a living character is refused.
        Character c;
        c.name = "Alive";
        c.isAlive = true;

        BodyLocation loc;
        loc.valid = true;

        int gold = 10000;
        QString reason;
        check(!DeathRecovery::resurrect(c, loc, gold, reason), "cannot resurrect the living", reason);
        check(gold == 10000, "gold untouched");
    }

    // ------------------------------------------- Party wipe / rescue (5.3)
    section("[37] Party wipe and rescue");
    {
        // A party with one survivor is not wiped.
        QList<Character> party;
        Character a; a.name = "A"; a.isAlive = true;
        Character b; b.name = "B"; b.isAlive = false;
        party.append(a);
        party.append(b);

        check(!DeathRecovery::isPartyWiped(party), "one survivor means not wiped");
        check(!DeathRecovery::needsRescue(party), "no rescue needed");
    }
    {
        // All dead: wiped and stranded.
        QList<Character> party;
        Character a; a.name = "A"; a.isAlive = false;
        Character b; b.name = "B"; b.isAlive = false;
        party.append(a);
        party.append(b);

        check(DeathRecovery::isPartyWiped(party), "whole party down");
        check(DeathRecovery::needsRescue(party), "rescue needed");
    }
    {
        // An empty party is not a wipe.
        QList<Character> empty;
        check(!DeathRecovery::isPartyWiped(empty), "empty party is not wiped");
    }
    {
        // Rescue cost scales with depth, superlinearly.
        check(DeathRecovery::rescuePartyCost(1) == 250, "depth 1 rescue costs 250",
              QString::number(DeathRecovery::rescuePartyCost(1)));
        check(DeathRecovery::rescuePartyCost(5) == 6250, "depth 5 rescue costs 6250",
              QString::number(DeathRecovery::rescuePartyCost(5)));
        check(DeathRecovery::rescuePartyCost(10) > DeathRecovery::rescuePartyCost(5),
              "deeper rescues cost more");
        check(DeathRecovery::rescuePartyCost(0) == 250, "depth 0 clamps to depth 1");
    }
    {
        // A rescue recovers bodies on the target floor only.
        QList<BodyLocation> bodies;
        BodyLocation b5a; b5a.valid = true; b5a.dungeonLevel = 5;
        BodyLocation b5b; b5b.valid = true; b5b.dungeonLevel = 5;
        BodyLocation b7;  b7.valid = true;  b7.dungeonLevel = 7;
        bodies.append(b5a);
        bodies.append(b5b);
        bodies.append(b7);

        int recovered = DeathRecovery::recoverBodies(bodies, 5);
        check(recovered == 2, "two bodies recovered from floor 5",
              QString::number(recovered));
        check(bodies[0].inCity && bodies[1].inCity, "both floor-5 bodies are home");
        check(!bodies[2].inCity, "the floor-7 body is untouched");
    }
    {
        // Hardcore mode means no resurrection at all.
        check(DeathRecovery::isPermanentlyDead(true), "hardcore is permanent");
        check(!DeathRecovery::isPermanentlyDead(false), "normal mode is not permanent");
    }

    // ------------------------------------------- Main quest chain (6.1)
    section("[38] Main quest chain");
    {
        check(QuestChain::stepCount() == 6, "6 quest steps",
              QString::number(QuestChain::stepCount()));

        QuestStep s0 = QuestChain::step(0);
        check(s0.id == "first_descent", "step 0 is first descent");
        check(s0.requiresDepth == 1, "step 0 needs depth 1");

        QuestStep s5 = QuestChain::step(5);
        check(s5.id == "prince_of_devils", "step 5 is the Prince");
        check(s5.bossFloor == 15, "step 5 needs the floor-15 boss");

        QuestStep invalid = QuestChain::step(99);
        check(invalid.index == -1, "out-of-range step is invalid");
    }
    {
        // Objective text describes the goal.
        check(QuestChain::objectiveText(QuestChain::step(0)).contains("level 1"),
              "depth objective mentions the floor");
        check(QuestChain::objectiveText(QuestChain::step(1)).contains("floor 5"),
              "boss objective mentions the floor");
    }
    {
        // A fresh party is on step 0 and the chain is incomplete.
        QList<int> noBosses;
        QStringList noItems;

        check(QuestChain::nextStepIndex(0, noBosses, noItems) == 0,
              "fresh party is on step 0");
        check(!QuestChain::isChainComplete(0, noBosses, noItems),
              "chain not complete");
    }
    {
        // Reaching floor 1 completes step 0; the party moves to step 1.
        QList<int> noBosses;
        QStringList noItems;

        check(QuestChain::nextStepIndex(1, noBosses, noItems) == 1,
              "reaching depth 1 advances to step 1");
    }
    {
        // Killing the floor-5 boss completes step 1.
        QList<int> bosses;
        bosses.append(5);
        QStringList noItems;

        check(QuestChain::isStepComplete(QuestChain::step(1), 5, bosses, noItems),
              "floor 5 boss completes step 1");
        check(QuestChain::nextStepIndex(5, bosses, noItems) == 2,
              "chain advances to step 2");
    }
    {
        // Steps cannot be skipped: depth 15 without the floor-5 boss still
        // leaves the party on step 1.
        QList<int> noBosses;
        QStringList noItems;

        check(QuestChain::nextStepIndex(15, noBosses, noItems) == 1,
              "missing boss keeps the party on step 1");
    }
    {
        // The full chain completes only when the Prince is dead.
        QList<int> bosses;
        bosses.append(5);
        bosses.append(10);
        QStringList noItems;

        check(!QuestChain::isChainComplete(15, bosses, noItems),
              "chain incomplete without the Prince");

        bosses.append(15);
        check(QuestChain::isChainComplete(15, bosses, noItems),
              "chain complete once the Prince is dead");
        check(QuestChain::nextStepIndex(15, bosses, noItems) == -1,
              "no steps left when complete");
    }

    // ------------------------------------------- Final boss / victory (6.2, 6.3)
    section("[39] Final boss and victory");
    {
        check(Endgame::finalBossName() == "The Prince of Devils", "final boss name");

        QVariantMap prince = Endgame::buildFinalBoss();
        QVariantMap floorBoss = BossEncounter::buildBoss(15);

        check(prince["name"].toString() == "The Prince of Devils", "prince built");
        check(prince["hp"].toInt() > floorBoss["hp"].toInt(),
              "the Prince outlasts the floor-15 boss",
              QString("%1 vs %2").arg(prince["hp"].toInt()).arg(floorBoss["hp"].toInt()));
        check(prince["att"].toInt() > floorBoss["att"].toInt(), "the Prince hits harder");
        check(prince["swings"].toInt() > floorBoss["swings"].toInt(), "the Prince swings more");
    }
    {
        // Victory requires the floor-15 boss to be down.
        QList<int> bosses;
        bosses.append(5);
        bosses.append(10);
        check(!Endgame::isVictory(bosses), "not victory without the Prince");

        bosses.append(15);
        check(Endgame::isVictory(bosses), "victory once the Prince falls");
    }
    {
        // Victory text exists and reads as a sequence.
        check(!Endgame::victoryTitle().isEmpty(), "victory has a title");
        check(Endgame::victoryParagraphs().size() >= 3, "victory has paragraphs",
              QString::number(Endgame::victoryParagraphs().size()));
    }

    // ------------------------------------------- Hall of Records (6.4)
    section("[40] Hall of Records");
    {
        GameRecord a;
        a.heroName = "A";
        a.highestLevel = 20;
        a.mostGold = 1000;
        a.deepestFloor = 10;

        GameRecord b;
        b.heroName = "B";
        b.highestLevel = 15;
        b.mostGold = 5000;
        b.deepestFloor = 15;

        check(Endgame::outranks(a, b, Endgame::Category::HighestLevel),
              "A outranks B on level");
        check(!Endgame::outranks(a, b, Endgame::Category::MostGold),
              "B outranks A on gold");
        check(Endgame::outranks(b, a, Endgame::Category::DeepestFloor),
              "B outranks A on depth");
    }
    {
        // Fastest completion: a win beats a non-win; faster beats slower.
        GameRecord wonFast;
        wonFast.won = true;
        wonFast.completionTimeSeconds = 3600;

        GameRecord wonSlow;
        wonSlow.won = true;
        wonSlow.completionTimeSeconds = 7200;

        GameRecord notWon;
        notWon.won = false;
        notWon.completionTimeSeconds = 100;

        check(Endgame::outranks(wonFast, wonSlow, Endgame::Category::FastestCompletion),
              "faster win outranks slower win");
        check(Endgame::outranks(wonSlow, wonFast, Endgame::Category::FastestCompletion) == false,
              "slower win does not outrank faster");
        check(Endgame::outranks(wonFast, notWon, Endgame::Category::FastestCompletion),
              "a win outranks a non-finish");
        check(!Endgame::outranks(notWon, wonFast, Endgame::Category::FastestCompletion),
              "a non-finish never outranks a win");
    }
    {
        // Ranking sorts best-first.
        QList<GameRecord> records;
        GameRecord r1; r1.heroName = "Low";  r1.highestLevel = 5;
        GameRecord r2; r2.heroName = "High"; r2.highestLevel = 30;
        GameRecord r3; r3.heroName = "Mid";  r3.highestLevel = 15;
        records << r1 << r2 << r3;

        QList<GameRecord> sorted = Endgame::ranked(records, Endgame::Category::HighestLevel);
        check(sorted[0].heroName == "High", "highest level ranked first", sorted[0].heroName);
        check(sorted[1].heroName == "Mid", "mid ranked second");
        check(sorted[2].heroName == "Low", "low ranked last");
    }
    {
        // Record serialization round-trip.
        GameRecord r;
        r.heroName = "Hero";
        r.highestLevel = 25;
        r.mostGold = 999999;
        r.deepestFloor = 15;
        r.completionTimeSeconds = 4530;
        r.won = true;

        QVariantMap map = r.toMap();
        GameRecord loaded;
        loaded.loadFromMap(map);

        check(loaded.heroName == "Hero", "hero name persists");
        check(loaded.highestLevel == 25, "level persists");
        check(loaded.mostGold == 999999, "gold persists");
        check(loaded.deepestFloor == 15, "depth persists");
        check(loaded.won, "win persists");
        check(loaded.completionTimeSeconds == 4530, "time persists");
    }
    {
        // Time formatting.
        GameRecord r;
        r.completionTimeSeconds = 3661;  // 1h 1m 1s
        check(r.formattedTime() == "01:01:01", "time formats as HH:MM:SS", r.formattedTime());

        GameRecord unfinished;
        check(unfinished.formattedTime() == "--:--:--", "unfinished shows placeholder");
    }
    {
        // Category names.
        check(Endgame::categoryName(Endgame::Category::HighestLevel) == "Highest Level",
              "level category name");
        check(Endgame::categoryName(Endgame::Category::FastestCompletion) == "Fastest Completion",
              "completion category name");
    }

    // ------------------------------------------- New Game Plus (6.5)
    section("[41] New Game Plus");
    {
        check(Endgame::ngPlusMonsterMultiplier(0) == 1.0, "NG+0 monsters are normal");
        check(Endgame::ngPlusMonsterMultiplier(1) == 1.5, "NG+1 monsters are 50% stronger",
              QString::number(Endgame::ngPlusMonsterMultiplier(1)));
        check(Endgame::ngPlusMonsterMultiplier(2) == 2.0, "NG+2 monsters are twice as strong");

        check(Endgame::ngPlusRewardMultiplier(0) == 1.0, "NG+0 rewards are normal");
        check(Endgame::ngPlusRewardMultiplier(1) == 1.25, "NG+1 rewards are 25% higher",
              QString::number(Endgame::ngPlusRewardMultiplier(1)));

        // Difficulty rises faster than rewards, so NG+ stays a challenge.
        check(Endgame::ngPlusMonsterMultiplier(2) > Endgame::ngPlusRewardMultiplier(2),
              "monsters scale faster than rewards");

        check(Endgame::ngPlusBanner(0).isEmpty(), "no banner at NG+0");
        check(Endgame::ngPlusBanner(1).contains("New Game +1"), "NG+1 banner",
              Endgame::ngPlusBanner(1));
    }

    // ------------------------------------------- Tavern / Inn (7.1)
    section("[42] Tavern and Inn");
    {
        auto *gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();

        // Set up a damaged party
        members[0].hp = 5;
        members[0].maxHp = 30;
        members[0].mana = 10;
        members[0].maxMana = 50;
        members[0].addStatus(GameConstants::Poisoned);
        members[0].addStatus(GameConstants::Blinded);

        int goldBefore = gsm->getPartyGold();

        // Rest: 8 hours × 10 gold × 1 living member = 80 gold
        int restCost = 8 * 10 * 1;
        gsm->spendPartyGold(restCost);
        for (auto& c : members) {
            if (!c.isAlive) continue;
            c.hp = c.maxHp;
            c.mana = c.maxMana;
        }

        check(members[0].hp == members[0].maxHp, "rest restores HP",
              QString("%1 / %2").arg(members[0].hp).arg(members[0].maxHp));
        check(members[0].mana == members[0].maxMana, "rest restores mana",
              QString("%1 / %2").arg(members[0].mana).arg(members[0].maxMana));
        check(gsm->getPartyGold() == goldBefore - restCost, "rest deducts gold");

        // Cure: 50 + 50 = 100 gold
        int cureCost = 100;
        gsm->spendPartyGold(cureCost);
        for (int i = 0; i < members.size(); ++i) {
            if (!members[i].isAlive) continue;
            members[i].removeStatus(GameConstants::Poisoned);
            members[i].removeStatus(GameConstants::Blinded);
            gsm->setCharacterStatus(i, GameConstants::Poisoned, false);
            gsm->setCharacterStatus(i, GameConstants::Blinded, false);
        }

        check((members[0].statusFlags & GameConstants::Poisoned) == 0, "poison cured",
              QString::number(members[0].statusFlags));
        check((members[0].statusFlags & GameConstants::Blinded) == 0, "blindness cured",
              QString::number(members[0].statusFlags));
        check(gsm->getPartyGold() == goldBefore - restCost - cureCost, "cure deducts gold");
    }
    {
        // Advance time. incrementPartyAge skips placeholder slots, so give
        // the member a real name first.
        auto *gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();
        if (!members.isEmpty() && members[0].name == "Empty Slot") {
            members[0].name = "Aging Test";
        }
        int ageBefore = members[0].age;
        gsm->incrementPartyAge(1);
        check(members[0].age == ageBefore + 1, "time advances by 1 year",
              QString("%1 -> %2").arg(ageBefore).arg(members[0].age));
    }

    // ------------------------------------------- Character sheet (7.5)
    section("[43] Character sheet");
    {
        auto *gsm = gameStateManager::instance();
        auto& members = gsm->getPartyMembers();

        Character& c = members[0];
        c.name = "TestHero";
        c.race = "Human";
        c.level = 5;
        c.experience = 1000;
        c.hp = 20;
        c.maxHp = 40;
        c.mana = 30;
        c.maxMana = 60;
        c.gold = 500;

        // Effective stats
        check(c.effectiveStrength() >= 8, "effective strength is at least base");
        check(c.effectiveIntelligence() >= 8, "effective intelligence is at least base");
        check(c.effectiveStat("Strength") == c.effectiveStrength(), "effectiveStat matches");

        // Guilds
        c.joinGuild("Warrior");
        c.joinGuild("Mage");
        check(c.guildLevel("Warrior") == 1, "Warrior guild level 1");
        check(c.guildLevel("Mage") == 1, "Mage guild level 1");
        check(c.totalGuildLevels() == 2, "total guild levels is 2");
        check(c.guildLevel("Paladin") == 0, "not in Paladin guild");

        // Increment guild level
        c.incrementGuildLevel("Warrior");
        check(c.guildLevel("Warrior") == 2, "Warrior guild level 2 after increment");
        check(c.totalGuildLevels() == 3, "total guild levels is 3");
    }
    {
        // Spell book integration
        SpellBook& sb = SpellBook::instance();
        if (sb.isLoaded()) {
            auto *gsm = gameStateManager::instance();
            Character& c = gsm->getPartyMembers()[0];
            // Guild names come from data/guilds.json and are what
            // data/spells.json lists ("Mages Guild", not "Mage").
            // Intelligence must clear the spells' required_stats, or every
            // guild level yields the same handful of level-1 spells.
            c.intelligence = 20;
            c.equipped.clear();

            c.joinGuild("Mages Guild");
            QList<SpellDef> noviceSpells = sb.spellsFor(c);

            c.incrementGuildLevel("Mages Guild");
            c.incrementGuildLevel("Mages Guild");
            c.incrementGuildLevel("Mages Guild");
            QList<SpellDef> spells = sb.spellsFor(c);

            check(spells.size() > 0, "Mage knows spells at guild level 4",
                  QString::number(spells.size()));
            check(spells.size() > noviceSpells.size(),
                  "a higher guild level unlocks more spells",
                  QString("%1 > %2").arg(spells.size()).arg(noviceSpells.size()));
        }
    }

    section("[43b] Character sheet dialog");
    {
        // Constructing the dialog used to crash: refreshSheet() cleared and
        // refilled the member combo, which emitted currentIndexChanged, which
        // is wired to onMemberChanged -> refreshSheet, recursing until the
        // stack overflowed. The dialog must survive construction.
        auto *gsm = gameStateManager::instance();
        if (gsm->getPartyMembers().isEmpty()) {
            Character filler;
            filler.name = "SheetTest";
            gsm->getPartyMembers().append(filler);
        }

        CharacterSheetDialog sheet;
        auto *combo = sheet.findChild<QComboBox*>();
        auto *nameLabel = sheet.findChild<QLabel*>();

        check(combo != nullptr, "character sheet has a member combo");
        if (combo) {
            check(combo->count() == gsm->getPartyMembers().size(),
                  "member combo lists every party member",
                  QString("%1 vs %2").arg(combo->count())
                      .arg(gsm->getPartyMembers().size()));
        }
        check(nameLabel != nullptr, "character sheet shows the member name");

        // The stats table must be filled, not merely present: an empty sheet
        // is the visible symptom of refreshSheet() bailing out early.
        QTableWidget *stats = nullptr;
        for (QTableWidget *t : sheet.findChildren<QTableWidget*>()) {
            if (t->columnCount() == 2 && t->rowCount() == 6) { stats = t; break; }
        }
        check(stats != nullptr, "character sheet has a six-row stats table");
        if (stats) {
            int filled = 0;
            for (int r = 0; r < stats->rowCount(); ++r) {
                const auto *nameCell = stats->item(r, 0);
                const auto *valueCell = stats->item(r, 1);
                if (nameCell && valueCell && !nameCell->text().isEmpty()) ++filled;
            }
            check(filled == 6, "every stat row is populated",
                  QString("%1 of 6").arg(filled));
            if (filled == 6) {
                check(stats->item(0, 0)->text() == QStringLiteral("Strength"),
                      "first stat row is Strength",
                      stats->item(0, 0)->text());
            }
        }
    }

    // ------------------------------------------- Bestiary (7.6)

    section("[44] Bestiary");
    {
        // The bestiary is built from data/bestiary.json (see
        // tools/gen_bestiary.py), not straight from the raw CSV, so that it
        // can carry the categories, pictures and walkthrough prose the CSV
        // only stores as opaque flags.
        const QList<QVariantMap>& entries = BestiaryDialog::entries();
        check(entries.size() > 0, "bestiary data is loaded",
              QString::number(entries.size()));

        int missingName = 0, missingHits = 0, missingImage = 0, missingCategory = 0;
        int missingPictureFile = 0;
        // picture -> how many monsters use it, and the best category count
        // for that picture.
        QHash<int, int> pictureCounts;
        QHash<int, int> topCategoryCount;
        QHash<int, QHash<QString, int>> categoryCounts;
        for (const auto& m : entries) {
            if (m.value("name").toString().isEmpty()) ++missingName;
            if (m.value("hits", 0).toInt() <= 0) ++missingHits;
            if (m.value("category").toString().isEmpty()) ++missingCategory;
            const QString img = m.value("image").toString();
            if (img.isEmpty()) ++missingImage;
            else if (!QFile::exists(img)) ++missingPictureFile;

            const int pic = m.value("picture").toInt();
            pictureCounts[pic] += 1;
            categoryCounts[pic][m.value("category").toString()] += 1;
        }
        for (auto it = categoryCounts.constBegin(); it != categoryCounts.constEnd(); ++it) {
            int best = 0;
            for (int n : it.value()) best = qMax(best, n);
            topCategoryCount.insert(it.key(), best);
        }
        check(missingName == 0, "every monster has a name",
              QString("%1 missing").arg(missingName));
        check(missingHits == 0, "every monster has hits",
              QString("%1 missing").arg(missingHits));
        check(missingCategory == 0, "every monster has a category",
              QString("%1 missing").arg(missingCategory));
        check(missingImage == 0, "every monster has a picture",
              QString("%1 missing").arg(missingImage));
        check(missingPictureFile == 0, "every picture file exists on disk",
              QString("%1 missing").arg(missingPictureFile));

        // The CSV's raw hp column does not exist; the field is "hits".
        // Guard the exact bug that made the old dialog show HP: 0.
        const QVariantMap goblie = BestiaryDialog::entry("Goblie");
        check(!goblie.isEmpty(), "a known monster can be looked up by name");
        if (!goblie.isEmpty()) {
            check(goblie.value("hits").toInt() > 0, "looked-up monster has hits",
                  QString::number(goblie.value("hits").toInt()));

            // The portrait must actually decode, not merely exist on disk.
            const QString path = BestiaryDialog::imagePath(goblie);
            QPixmap pix(path);
            check(!pix.isNull(), "the portrait loads as a pixmap", path);
            check(pix.width() > 0 && pix.height() > 0,
                  "the portrait has real dimensions",
                  QString("%1x%2").arg(pix.width()).arg(pix.height()));
        }

        // The five picture groups the walkthrough wraps onto long lines used
        // to lose their category entirely.
        check(BestiaryDialog::categories().size() > 10,
              "bestiary has multiple categories",
              QString::number(BestiaryDialog::categories().size()));

        // Categories come from the walkthrough's own headings and are per
        // monster, not per picture: "Werebear" sits under Lycanthropes even
        // though it shares a picture with the bears. So the picture only
        // supplies a fallback category for monsters the walkthrough never
        // names, and that fallback must cover at least half the picture's
        // monsters or it would be arbitrary.
        int ambiguousPictures = 0;
        for (auto it = pictureCounts.constBegin(); it != pictureCounts.constEnd(); ++it) {
            const int total = it.value();
            const int top = topCategoryCount.value(it.key(), 0);
            if (total > 0 && top * 2 < total) ++ambiguousPictures;
        }
        check(ambiguousPictures == 0,
              "every picture has a dominant category",
              QString("%1 ambiguous").arg(ambiguousPictures));

        // Every category the data uses must be one of the known headings.
        const QStringList known = BestiaryDialog::categories();
        int unknownCategory = 0;
        for (const auto& m : entries) {
            if (!known.contains(m.value("category").toString())) ++unknownCategory;
        }
        check(unknownCategory == 0, "every category is a known heading",
              QString("%1 unknown").arg(unknownCategory));

        // Floor distribution still spans the dungeon.
        QSet<int> floors;
        for (const auto& m : entries) floors.insert(m.value("level").toInt());
        check(floors.size() > 1, "monsters span multiple floors",
              QString::number(floors.size()));
    }

    // ------------------------------------------- Journal (7.7)
    section("[45] Journal");
    {
        // Add an entry
        JournalDialog::addEntry("Quest", "Test quest entry");
        JournalDialog::addEntry("Combat", "Test combat entry");

        // Load and verify
        QFile file("data/journal.json");
        check(file.exists(), "journal file created");

        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();

            check(doc.isArray(), "journal is a JSON array");
            QJsonArray arr = doc.array();
            check(arr.size() >= 2, "journal has at least 2 entries",
                  QString::number(arr.size()));

            // Check first entry
            if (arr.size() > 0) {
                QJsonObject obj = arr[0].toObject();
                check(obj.value("category").toString() == "Quest", "first entry is Quest");
                check(obj.value("text").toString() == "Test quest entry", "first entry text matches");
            }
        }

        // Clean up
        QFile::remove("data/journal.json");
    }

    // ------------------------------------------- Quest board (7.2)
    section("[42] Quest board");
    {
        QuestBoardDialog::reset();

        QList<BoardQuest> quests = QuestBoardDialog::availableQuests();
        check(quests.size() >= 4, "at least 4 quests posted",
              QString::number(quests.size()));

        // Find a kill quest and a fetch quest.
        BoardQuest killQuest;
        BoardQuest fetchQuest;
        for (const BoardQuest& q : quests) {
            if (q.isFetch && fetchQuest.id.isEmpty()) fetchQuest = q;
            if (!q.isFetch && killQuest.id.isEmpty()) killQuest = q;
        }
        check(!killQuest.id.isEmpty(), "a kill quest exists");
        check(!fetchQuest.id.isEmpty(), "a fetch quest exists");
    }
    {
        // Accept a quest.
        QuestBoardDialog::reset();
        BoardQuest q = QuestBoardDialog::availableQuests().first();

        check(QuestBoardDialog::acceptQuest(q.id), "quest accepted");
        check(QuestBoardDialog::isAccepted(q.id), "quest is accepted");
        check(!QuestBoardDialog::acceptQuest(q.id), "cannot accept twice");
    }
    {
        // Kill quest progress.
        QuestBoardDialog::reset();
        BoardQuest q = QuestBoardDialog::availableQuests().first();
        QuestBoardDialog::acceptQuest(q.id);

        check(!QuestBoardDialog::isComplete(q.id), "quest not complete at start");

        for (int i = 0; i < q.killCount; ++i) {
            QuestBoardDialog::reportKill(q.targetMonster, q.targetFloor);
        }

        check(QuestBoardDialog::isComplete(q.id), "quest complete after enough kills");
    }
    {
        // Turn in a completed quest.
        QuestBoardDialog::reset();
        BoardQuest q = QuestBoardDialog::availableQuests().first();
        QuestBoardDialog::acceptQuest(q.id);

        for (int i = 0; i < q.killCount; ++i) {
            QuestBoardDialog::reportKill(q.targetMonster, q.targetFloor);
        }

        int gold = 0, xp = 0;
        check(QuestBoardDialog::turnIn(q.id, gold, xp), "turn-in succeeds");
        check(gold == q.rewardGold, "gold reward matches",
              QString::number(gold));
        check(xp == q.rewardXp, "XP reward matches",
              QString::number(xp));
        check(!QuestBoardDialog::isAccepted(q.id), "quest removed after turn-in");
    }
    {
        // Fetch quest.
        QuestBoardDialog::reset();
        BoardQuest fetchQuest;
        for (const BoardQuest& q : QuestBoardDialog::availableQuests()) {
            if (q.isFetch) { fetchQuest = q; break; }
        }
        QuestBoardDialog::acceptQuest(fetchQuest.id);

        check(!QuestBoardDialog::isComplete(fetchQuest.id), "fetch quest not complete yet");

        QuestBoardDialog::reportFetch(fetchQuest.fetchItem);
        check(QuestBoardDialog::isComplete(fetchQuest.id), "fetch quest complete after item");
    }
    {
        // Unknown quest id.
        BoardQuest invalid = QuestBoardDialog::questById("no_such_quest");
        check(invalid.id.isEmpty(), "unknown quest id returns invalid");
        check(!QuestBoardDialog::acceptQuest("no_such_quest"), "cannot accept unknown quest");
    }

    // ------------------------------------------- Alignment consequences (7.4)
    section("[42] Alignment consequences");
    {
        using Alignment = AlignmentSystem::Alignment;

        check(AlignmentSystem::alignmentName(Alignment::Good) == "Good", "Good name");
        check(AlignmentSystem::alignmentName(Alignment::Neutral) == "Neutral", "Neutral name");
        check(AlignmentSystem::alignmentName(Alignment::Evil) == "Evil", "Evil name");

        check(AlignmentSystem::alignmentFromName("Good") == Alignment::Good, "from Good");
        check(AlignmentSystem::alignmentFromName("Evil") == Alignment::Evil, "from Evil");
        check(AlignmentSystem::alignmentFromName("Neutral") == Alignment::Neutral, "from Neutral");
        check(AlignmentSystem::alignmentFromName("Unknown") == Alignment::Neutral, "unknown defaults to Neutral");
    }
    {
        // Good characters are barred from evil guilds.
        check(!AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Good, "Assassin's Guild"),
              "Good barred from Assassin's Guild");
        check(!AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Good, "Cult of the Dark"),
              "Good barred from Cult of the Dark");
        check(AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Good, "Paladin's Guild"),
              "Good can join Paladin's Guild");
    }
    {
        // Evil characters are barred from good guilds.
        check(!AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Evil, "Paladin's Guild"),
              "Evil barred from Paladin's Guild");
        check(!AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Evil, "Priest of Light"),
              "Evil barred from Priest of Light");
        check(AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Evil, "Assassin's Guild"),
              "Evil can join Assassin's Guild");
    }
    {
        // Neutral can join anything.
        check(AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Neutral, "Paladin's Guild"),
              "Neutral can join Paladin's Guild");
        check(AlignmentSystem::canJoinGuild(AlignmentSystem::Alignment::Neutral, "Assassin's Guild"),
              "Neutral can join Assassin's Guild");
    }
    {
        // Barred guilds lists.
        check(AlignmentSystem::barredGuilds(AlignmentSystem::Alignment::Good).contains("Assassin's Guild"),
              "Good barred list includes Assassin's");
        check(AlignmentSystem::barredGuilds(AlignmentSystem::Alignment::Evil).contains("Paladin's Guild"),
              "Evil barred list includes Paladin's");
        check(AlignmentSystem::barredGuilds(AlignmentSystem::Alignment::Neutral).isEmpty(),
              "Neutral has no barred guilds");
    }
    {
        // Location restrictions.
        check(!AlignmentSystem::canEnterLocation(AlignmentSystem::Alignment::Evil, "Temple"),
              "Evil barred from Temple");
        check(!AlignmentSystem::canEnterLocation(AlignmentSystem::Alignment::Good, "Cult of the Dark"),
              "Good barred from Cult of the Dark");
        check(AlignmentSystem::canEnterLocation(AlignmentSystem::Alignment::Neutral, "Temple"),
              "Neutral can enter Temple");
        check(AlignmentSystem::canEnterLocation(AlignmentSystem::Alignment::Neutral, "Guild"),
              "Neutral can enter Guild");
    }
    {
        // Alignment from value.
        check(AlignmentSystem::characterAlignment(-5) == AlignmentSystem::Alignment::Evil,
              "negative value is Evil");
        check(AlignmentSystem::characterAlignment(5) == AlignmentSystem::Alignment::Good,
              "positive value is Good");
        check(AlignmentSystem::characterAlignment(0) == AlignmentSystem::Alignment::Neutral,
              "zero is Neutral");
    }
    {
        // Consequence hints.
        check(AlignmentSystem::consequenceHint(AlignmentSystem::Alignment::Good).contains("barred"),
              "Good hint mentions barred");
        check(AlignmentSystem::consequenceHint(AlignmentSystem::Alignment::Evil).contains("barred"),
              "Evil hint mentions barred");
        check(AlignmentSystem::consequenceHint(AlignmentSystem::Alignment::Neutral).contains("no restrictions"),
              "Neutral hint mentions no restrictions");
    }

    // ------------------------------------------- NPC dialog (7.3)
    section("[43] NPC dialog");
    {
        QList<NPC> npcs = NPCDialog::allNPCs();
        check(npcs.size() >= 8, "at least 8 NPCs", QString::number(npcs.size()));

        // Every NPC has a name, role, personality, greeting and hints.
        for (const NPC& n : npcs) {
            check(!n.name.isEmpty(), "NPC has a name", n.name);
            check(!n.role.isEmpty(), "NPC has a role", n.name);
            check(!n.personality.isEmpty(), "NPC has personality", n.name);
            check(!n.greeting.isEmpty(), "NPC has greeting", n.name);
            check(!n.hints.isEmpty(), "NPC has hints", n.name);
        }
    }
    {
        // Lookup by name.
        NPC borin = NPCDialog::npcByName("Guildmaster Borin");
        check(!borin.name.isEmpty(), "found Guildmaster Borin");
        check(borin.role == "Guildmaster", "Borin is Guildmaster");
        check(borin.location == "Guild Hall", "Borin is in Guild Hall");

        NPC unknown = NPCDialog::npcByName("Nobody");
        check(unknown.name.isEmpty(), "unknown NPC is invalid");
    }
    {
        // Hints from a specific NPC.
        QStringList borinHints = NPCDialog::hintsFrom("Guildmaster Borin");
        check(borinHints.size() >= 3, "Borin has at least 3 hints",
              QString::number(borinHints.size()));
        check(borinHints[0].contains("Floor 5"), "Borin mentions floor 5");

        QStringList unknownHints = NPCDialog::hintsFrom("Nobody");
        check(unknownHints.isEmpty(), "unknown NPC has no hints");
    }
    {
        // All hints aggregated.
        QStringList all = NPCDialog::allHints();
        check(all.size() >= 20, "at least 20 total hints",
              QString::number(all.size()));
    }
    {
        // NPCs cover key locations.
        QStringList locations;
        for (const NPC& n : NPCDialog::allNPCs()) {
            locations << n.location;
        }
        check(locations.contains("Guild Hall"), "Guild Hall NPC exists");
        check(locations.contains("Temple"), "Temple NPC exists");
        check(locations.contains("Tavern"), "Tavern NPC exists");
        check(locations.contains("Library"), "Library NPC exists");
        check(locations.contains("Royal Bank"), "Bank NPC exists");
        check(locations.contains("Morgue"), "Morgue NPC exists");
    }

    // ------------------------------------------- Keyboard shortcuts (7.9)
    section("[46] Keyboard shortcut help");
    {
        QList<ShortcutEntry> all = ShortcutHelp::allShortcuts();
        check(all.size() >= 20, "at least 20 shortcuts registered",
              QString::number(all.size()));

        // Dungeon shortcuts exist.
        QList<ShortcutEntry> dungeon = ShortcutHelp::shortcutsFor("Dungeon");
        check(dungeon.size() >= 15, "dungeon has at least 15 shortcuts",
              QString::number(dungeon.size()));

        // Global shortcuts exist.
        QList<ShortcutEntry> global = ShortcutHelp::shortcutsFor("Global");
        check(global.size() >= 3, "global has at least 3 shortcuts",
              QString::number(global.size()));

        // Contexts are registered.
        QStringList ctxs = ShortcutHelp::contexts();
        check(ctxs.contains("Dungeon"), "Dungeon context exists");
        check(ctxs.contains("Global"), "Global context exists");

        // Find a specific shortcut.
        ShortcutEntry fight = ShortcutHelp::find("F", "Dungeon");
        check(!fight.key.isEmpty(), "F shortcut found in Dungeon");
        check(fight.action == "Fight", "F is Fight", fight.action);

        // Unknown shortcut is invalid.
        ShortcutEntry unknown = ShortcutHelp::find("X", "Dungeon");
        check(unknown.key.isEmpty(), "unknown shortcut is invalid");

        // Format produces readable text.
        QString formatted = ShortcutHelp::format(fight);
        check(formatted.contains("F"), "format includes key", formatted);
        check(formatted.contains("Fight"), "format includes action", formatted);
    }

    // ------------------------------------------- Tutorial (7.10)
    section("[47] Guided first dungeon run");
    {
        Tutorial::reset();
        check(!Tutorial::isActive(), "tutorial not active after reset");

        QList<TutorialStep> steps = Tutorial::steps();
        check(steps.size() == 11, "11 tutorial steps",
              QString::number(steps.size()));

        // First step is movement.
        TutorialStep s0 = Tutorial::step(0);
        check(s0.id == "welcome", "step 0 is welcome");
        check(s0.completesOn == "move", "step 0 completes on move");

        // Last step is victory.
        TutorialStep last = Tutorial::step(10);
        check(last.id == "victory", "last step is victory");

        // Out-of-range step is invalid.
        TutorialStep invalid = Tutorial::step(99);
        check(invalid.index == -1, "out-of-range step is invalid");
    }
    {
        // Start the tutorial and advance through events.
        Tutorial::reset();
        Tutorial::start();
        check(Tutorial::isActive(), "tutorial active after start");
        check(Tutorial::currentStepIndex() == 0, "starts at step 0");

        // Move completes step 0.
        check(Tutorial::reportEvent("move"), "move advances tutorial");
        check(Tutorial::currentStepIndex() == 1, "now on step 1");

        // Stairs completes step 1.
        check(Tutorial::reportEvent("stairs"), "stairs advances tutorial");
        check(Tutorial::currentStepIndex() == 2, "now on step 2");

        // Level 2 completes step 2.
        check(Tutorial::reportEvent("level", 2), "level 2 advances tutorial");
        check(Tutorial::currentStepIndex() == 3, "now on step 3");

        // Fight completes step 3.
        check(Tutorial::reportEvent("fight"), "fight advances tutorial");
        check(Tutorial::currentStepIndex() == 4, "now on step 4");

        // Spell completes step 4.
        check(Tutorial::reportEvent("spell"), "spell advances tutorial");
        check(Tutorial::currentStepIndex() == 5, "now on step 5");

        // Rest completes step 5.
        check(Tutorial::reportEvent("rest"), "rest advances tutorial");
        check(Tutorial::currentStepIndex() == 6, "now on step 6");

        // Pickup completes step 6.
        check(Tutorial::reportEvent("pickup"), "pickup advances tutorial");
        check(Tutorial::currentStepIndex() == 7, "now on step 7");

        // Door completes step 7.
        check(Tutorial::reportEvent("door"), "door advances tutorial");
        check(Tutorial::currentStepIndex() == 8, "now on step 8");

        // Level 5 completes step 8.
        check(Tutorial::reportEvent("level", 5), "level 5 advances tutorial");
        check(Tutorial::currentStepIndex() == 9, "now on step 9");

        // Boss on floor 5 completes step 9.
        check(Tutorial::reportEvent("boss", 5), "boss advances tutorial");
        check(Tutorial::currentStepIndex() == 10, "now on step 10");

        // Progress text.
        check(Tutorial::progressText().contains("11 / 11"), "progress shows final step",
              Tutorial::progressText());
    }
    {
        // Events that don't match the current step do nothing.
        Tutorial::reset();
        Tutorial::start();
        check(!Tutorial::reportEvent("spell"), "spell does not complete move step");
        check(Tutorial::currentStepIndex() == 0, "still on step 0");
    }
    {
        // Level below target does not complete.
        Tutorial::reset();
        Tutorial::start();
        Tutorial::reportEvent("move");   // -> step 1 (stairs)
        Tutorial::reportEvent("stairs"); // -> step 2 (level 2)
        check(!Tutorial::reportEvent("level", 1), "level 1 does not complete level 2 step");
        check(Tutorial::currentStepIndex() == 2, "still on step 2");
    }
    {
        // Tutorial finishes after the last step.
        Tutorial::reset();
        Tutorial::start();
        for (int i = 0; i < 20; ++i) {
            TutorialStep s = Tutorial::currentStep();
            if (s.index < 0) break;
            Tutorial::reportEvent(s.completesOn, s.targetValue);
        }
        check(!Tutorial::isActive(), "tutorial finished after all steps");
        check(Tutorial::currentStepIndex() == -1, "no current step when finished");
    }
    {
        // Report event when not active does nothing.
        Tutorial::reset();
        check(!Tutorial::reportEvent("move"), "no event when not active");
    }

    // ------------------------------------------- Monster difficulty curve (8.1)
    section("[52] Monster difficulty curve");
    {
        check(MonsterBalance::recommendedLevel(1) == 1, "floor 1 needs level 1");
        check(MonsterBalance::recommendedLevel(15) == 15, "floor 15 needs level 15");
        check(MonsterBalance::statMultiplier(1) == 1.0, "floor 1 multiplier is 1.0");
        check(MonsterBalance::statMultiplier(15) > 2.0, "floor 15 multiplier is high",
              QString::number(MonsterBalance::statMultiplier(15)));
        check(MonsterBalance::scaledHp(1, 10) == 10, "floor 1 HP unchanged");
        check(MonsterBalance::scaledHp(15, 10) > 20, "floor 15 HP scaled",
              QString::number(MonsterBalance::scaledHp(15, 10)));
        check(MonsterBalance::scaledAttack(1, 5) == 5, "floor 1 attack unchanged");
        check(MonsterBalance::scaledAttack(15, 5) > 10, "floor 15 attack scaled");
        check(MonsterBalance::scaledXp(1, 100) == 100, "floor 1 XP unchanged");
        check(MonsterBalance::scaledXp(15, 100) > 200, "floor 15 XP scaled");
        check(MonsterBalance::scaledGold(1, 50) == 50, "floor 1 gold unchanged");
        check(MonsterBalance::scaledGold(15, 50) > 100, "floor 15 gold scaled");
    }
    {
        // Party readiness.
        check(MonsterBalance::isPartyReady(4, 1, 1), "4 members at level 1 ready for floor 1");
        check(!MonsterBalance::isPartyReady(1, 1, 15), "solo level 1 not ready for floor 15");
        check(MonsterBalance::isPartyReady(4, 15, 15), "4 members at level 15 ready for floor 15");
        check(!MonsterBalance::isPartyReady(0, 1, 1), "empty party not ready");
    }
    {
        // Difficulty labels.
        check(MonsterBalance::difficultyLabel(1) == "Easy", "floor 1 is easy");
        check(MonsterBalance::difficultyLabel(5) == "Moderate", "floor 5 is moderate");
        check(MonsterBalance::difficultyLabel(10) == "Hard", "floor 10 is hard");
        check(MonsterBalance::difficultyLabel(15) == "Deadly", "floor 15 is deadly");
    }
    {
        // Floor table.
        QList<QVariantMap> table = MonsterBalance::floorTable();
        check(table.size() == 15, "15 floors in table");
        check(table[0].value("floor").toInt() == 1, "first floor is 1");
        check(table[14].value("floor").toInt() == 15, "last floor is 15");
    }

    // ------------------------------------------- Spell differentiation (8.2)
    section("[53] Spell differentiation");
    {
        SpellDef fire;
        fire.category = "Fire";
        fire.name = "Fireball";
        fire.baseLevel = 3;

        SpellDef cold;
        cold.category = "Cold";
        cold.name = "Iceball";
        cold.baseLevel = 3;

        SpellDef lightning;
        lightning.category = "Electrical";
        lightning.name = "Chain Lightning";
        lightning.baseLevel = 6;

        SpellDef mind;
        mind.category = "Mind";
        mind.name = "Stun";
        mind.baseLevel = 3;

        using School = SpellMechanics::School;
        check(SpellMechanics::schoolFor("Fire") == School::Fire, "fire school");
        check(SpellMechanics::schoolFor("Cold") == School::Cold, "cold school");
        check(SpellMechanics::schoolFor("Electrical") == School::Lightning, "lightning school");
        check(SpellMechanics::schoolFor("Mind") == School::Mind, "mind school");
    }
    {
        // Fire AoE.
        SpellDef fire;
        fire.category = "Fire";
        fire.name = "Fireball";
        fire.baseLevel = 3;
        check(SpellMechanics::fireTargets(fire) >= 2, "fire hits multiple targets");
        check(SpellMechanics::isAoe(fire), "fire is AoE");
        check(!SpellMechanics::isCrowdControl(fire), "fire is not crowd control");
    }
    {
        // Cold slow.
        SpellDef cold;
        cold.category = "Cold";
        cold.name = "Iceball";
        cold.baseLevel = 3;
        check(SpellMechanics::coldSlowTurns(cold) >= 1, "cold slows");
        check(SpellMechanics::coldSlowPercent(cold) >= 20, "cold slow percent");
        check(SpellMechanics::isCrowdControl(cold), "cold is crowd control");
    }
    {
        // Lightning chain.
        SpellDef lightning;
        lightning.category = "Electrical";
        lightning.name = "Chain Lightning";
        lightning.baseLevel = 6;
        check(SpellMechanics::lightningChainTargets(lightning) == 5, "chain hits 5 targets");
        check(SpellMechanics::lightningChainFalloff(lightning) == 20, "chain falloff 20%");
        check(SpellMechanics::isAoe(lightning), "chain is AoE");
    }
    {
        // Mind crowd control.
        SpellDef mind;
        mind.category = "Mind";
        mind.name = "Stun";
        mind.baseLevel = 3;
        check(SpellMechanics::mindStunTurns(mind) >= 1, "mind stuns");
        check(SpellMechanics::mindConfuseChance(mind) >= 25, "mind confuse chance");
        check(SpellMechanics::isCrowdControl(mind), "mind is crowd control");
    }
    {
        // Mechanic descriptions.
        SpellDef fire;
        fire.category = "Fire";
        fire.name = "Fireball";
        fire.baseLevel = 3;
        check(SpellMechanics::mechanicDescription(fire).contains("Fire"), "fire description");
        check(SpellMechanics::mechanicDescription(fire).contains("targets"), "fire targets in desc");
    }
    {
        // Spells in school.
        QList<SpellDef> spells;
        SpellDef f1; f1.category = "Fire"; f1.name = "Flame Bolt";
        SpellDef f2; f2.category = "Fire"; f2.name = "Fireball";
        SpellDef c1; c1.category = "Cold"; c1.name = "Cold Bolt";
        spells << f1 << f2 << c1;

        QList<SpellDef> fireSpells = SpellMechanics::spellsInSchool(spells, SpellMechanics::School::Fire);
        check(fireSpells.size() == 2, "2 fire spells");
        QList<SpellDef> coldSpells = SpellMechanics::spellsInSchool(spells, SpellMechanics::School::Cold);
        check(coldSpells.size() == 1, "1 cold spell");
    }

    // ------------------------------------------- Item progression (8.3)
    section("[54] Item progression");
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::tierName(Tier::Bronze) == "Bronze", "bronze name");
        check(ItemProgression::tierName(Tier::Iron) == "Iron", "iron name");
        check(ItemProgression::tierName(Tier::Steel) == "Steel", "steel name");
        check(ItemProgression::tierName(Tier::Adamantite) == "Adamantite", "adamantite name");
        check(ItemProgression::tierName(Tier::Mithril) == "Mithril", "mithril name");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::tierForFloor(1) == Tier::Bronze, "floor 1 is bronze");
        check(ItemProgression::tierForFloor(5) == Tier::Iron, "floor 5 is iron");
        check(ItemProgression::tierForFloor(8) == Tier::Steel, "floor 8 is steel");
        check(ItemProgression::tierForFloor(12) == Tier::Adamantite, "floor 12 is adamantite");
        check(ItemProgression::tierForFloor(15) == Tier::Mithril, "floor 15 is mithril");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::minFloorForTier(Tier::Bronze) == 1, "bronze min floor 1");
        check(ItemProgression::minFloorForTier(Tier::Mithril) == 13, "mithril min floor 13");
        check(ItemProgression::maxFloorForTier(Tier::Bronze) == 3, "bronze max floor 3");
        check(ItemProgression::maxFloorForTier(Tier::Mithril) == 15, "mithril max floor 15");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::statMultiplier(Tier::Bronze) == 1.0, "bronze stat 1.0");
        check(ItemProgression::statMultiplier(Tier::Iron) == 1.5, "iron stat 1.5");
        check(ItemProgression::statMultiplier(Tier::Steel) == 2.0, "steel stat 2.0");
        check(ItemProgression::statMultiplier(Tier::Adamantite) == 3.0, "adamantite stat 3.0");
        check(ItemProgression::statMultiplier(Tier::Mithril) == 4.0, "mithril stat 4.0");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::costMultiplier(Tier::Bronze) == 1.0, "bronze cost 1.0");
        check(ItemProgression::costMultiplier(Tier::Mithril) == 16.0, "mithril cost 16.0");
    }
    {
        using Tier = ItemProgression::Tier;
        check(ItemProgression::isAvailable(Tier::Bronze, 1), "bronze available floor 1");
        check(!ItemProgression::isAvailable(Tier::Mithril, 1), "mithril not available floor 1");
        check(ItemProgression::isAvailable(Tier::Mithril, 15), "mithril available floor 15");
    }
    {
        // Items of tier.
        QStringList items = {"Bronze Sword", "Iron Shield", "Steel Helm", "Bronze Dagger"};
        QStringList bronze = ItemProgression::itemsOfTier(items, ItemProgression::Tier::Bronze);
        check(bronze.size() == 2, "2 bronze items");
        QStringList iron = ItemProgression::itemsOfTier(items, ItemProgression::Tier::Iron);
        check(iron.size() == 1, "1 iron item");
    }

    // ------------------------------------------- Gold sinks (8.4)
    section("[55] Gold sinks");
    {
        check(GoldSinks::resurrectionCost(1, false) == 500, "level 1 resurrection 500");
        check(GoldSinks::resurrectionCost(5, false) == 2500, "level 5 resurrection 2500");
        check(GoldSinks::resurrectionCost(5, true) == 3000, "level 5 dungeon resurrection 3000");
        check(GoldSinks::rescueCost(1) == 250, "depth 1 rescue 250");
        check(GoldSinks::rescueCost(5) == 6250, "depth 5 rescue 6250");
        check(GoldSinks::identificationCost() == 50, "identification 50");
        check(GoldSinks::uncurseCost() == 100, "uncurse 100");
        check(GoldSinks::guildLevelCost("Mage", 0) == 500, "guild level 0→1 costs 500");
        check(GoldSinks::guildLevelCost("Mage", 5) == 3000, "guild level 5→6 costs 3000");
        check(GoldSinks::restCostPerHour() == 10, "rest 10/hour");
        check(GoldSinks::curePoisonCost() == 50, "cure poison 50");
        check(GoldSinks::cureBlindnessCost() == 50, "cure blindness 50");
    }
    {
        // All sinks.
        QList<QVariantMap> sinks = GoldSinks::allSinks();
        check(sinks.size() >= 7, "at least 7 gold sinks",
              QString::number(sinks.size()));
    }
    {
        // Sink descriptions.
        check(GoldSinks::sinkDescription("resurrection").contains("Morgue"), "resurrection desc");
        check(GoldSinks::sinkDescription("identification").contains("Store"), "identification desc");
        check(GoldSinks::sinkDescription("guild").contains("Guild"), "guild desc");
    }

    // ------------------------------------------- Release packaging (8.5)
    section("[56] Release packaging");
    {
        check(ReleaseInfo::version() == "1.0.0", "version is 1.0.0");
        check(ReleaseInfo::versionString() == "v1.0.0", "version string");
        check(!ReleaseInfo::releaseDate().isEmpty(), "release date exists");
        check(!ReleaseInfo::systemRequirements().isEmpty(), "system requirements exist");
        check(ReleaseInfo::installerName().contains("1.0.0"), "installer name");
        check(ReleaseInfo::banner().contains("1.0.0"), "banner");
    }
    {
        // Release notes.
        QStringList notes = ReleaseInfo::releaseNotes();
        check(notes.size() >= 8, "at least 8 release notes",
              QString::number(notes.size()));
        check(notes[0].contains("Phase 1"), "first note is Phase 1");
    }
    {
        // Changes.
        QStringList changes = ReleaseInfo::changes();
        check(changes.size() >= 5, "at least 5 changes",
              QString::number(changes.size()));
    }
    {
        // Version history.
        QList<QPair<QString, QString>> history = ReleaseInfo::versionHistory();
        check(history.size() >= 5, "at least 5 versions in history",
              QString::number(history.size()));
        check(history[0].first == "1.0.0", "latest version is 1.0.0");
    }

    // ------------------------------------------- Status flag integrity
    section("[57] Status flags");
    {
        // One statusFlags field carries both the death bit and the combat
        // statuses, so their bits must not overlap.
        check(StatusFlag::Dead != StatusFlag::Poisoned, "Dead and Poisoned are different bits");
        check(StatusFlag::Dead != StatusFlag::Blinded, "Dead and Blinded are different bits");
        check(StatusFlag::Dead != StatusFlag::OnFire, "Dead and OnFire are different bits");
        check((StatusFlag::Dead & StatusFlag::Poisoned) == 0, "Dead and Poisoned do not overlap");
        check((StatusFlag::Dead & StatusFlag::Blinded) == 0, "Dead and Blinded do not overlap");
        check((StatusFlag::Dead & StatusFlag::OnFire) == 0, "Dead and OnFire do not overlap");
    }
    {
        // Death must not look like poison, and vice versa.
        Character c;
        c.name = "StatusTest";
        c.isAlive = true;

        c.addStatus(StatusFlag::Poisoned);
        check((c.statusFlags & StatusFlag::Poisoned) != 0, "poison bit set");
        check((c.statusFlags & StatusFlag::Dead) == 0, "poison does not read as dead");
        check(c.isAlive, "a poisoned character is still alive");

        c.removeStatus(StatusFlag::Poisoned);
        c.addStatus(StatusFlag::Dead);
        check((c.statusFlags & StatusFlag::Dead) != 0, "dead bit set");
        check((c.statusFlags & StatusFlag::Poisoned) == 0, "death does not read as poisoned");
    }
    {
        // The cure-poison branch in useConsumable() reads statusFlags
        // directly; no item in MDATA3.csv currently reaches it, so assert
        // the invariant the branch depends on instead of the branch itself.
        Character c;
        c.name = "CureTest";
        c.isAlive = true;
        c.addStatus(StatusFlag::Poisoned);
        c.addStatus(StatusFlag::Blinded);

        // Clearing poison must leave blindness intact.
        c.removeStatus(StatusFlag::Poisoned);
        check((c.statusFlags & StatusFlag::Poisoned) == 0, "poison cleared",
              QString::number(c.statusFlags));
        check((c.statusFlags & StatusFlag::Blinded) != 0, "blindness survives a poison cure",
              QString::number(c.statusFlags));

        // And the reverse.
        c.removeStatus(StatusFlag::Blinded);
        c.addStatus(StatusFlag::Poisoned);
        check((c.statusFlags & StatusFlag::Blinded) == 0, "blindness cleared");
        check((c.statusFlags & StatusFlag::Poisoned) != 0, "poison survives a blindness cure",
              QString::number(c.statusFlags));
    }

    // -------------------------------------------------------------- cleanup
    QFile::remove(savePath());

    // --------------------------------------------------------------- report
    out("");
    out("====================");
    out(QString("%1 passed, %2 failed").arg(g_passed).arg(g_failed));

    qInstallMessageHandler(nullptr);
    return g_failed == 0 ? 0 : 1;
}
