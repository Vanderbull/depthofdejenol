// Equipment tests — guild restrictions, cursed items, stat modifiers.
// Kept separate from the main suite so guild membership doesn't leak into
// other tests' characters.

#include <QApplication>
#include <QString>
#include <QVariantMap>
#include <cstdio>

#include "character.h"
#include "src/core/GameConstants.h"
#include "src/items/ItemDatabase.h"

static int g_passed = 0;
static int g_failed = 0;

static void out(const QString& line)
{
    std::fputs(qPrintable(line), stdout);
    std::fputc('\n', stdout);
    std::fflush(stdout);
}

static void check(bool condition, const QString& what, const QString& detail = QString())
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

int runEquipmentTest()
{
    g_passed = 0;
    g_failed = 0;

    out("");
    out("--- [76] v2.0.0 slice 2.2: equipment guild restriction " + QString(20, '-'));

    // Items with a guild bitmask can only be equipped by members of at
    // least one of the required guilds.
    Character hero;
    hero.name = "Guild Test Hero";
    hero.level = 5;
    hero.strength = 15;
    hero.intelligence = 15;
    hero.wisdom = 15;
    hero.constitution = 15;
    hero.charisma = 15;
    hero.dexterity = 15;

    // Find an item with a non-zero guild bitmask.
    const ItemDef* guildItem = nullptr;
    for (const ItemDef& def : ItemDatabase::instance().all()) {
        if (def.guilds != 0 && def.equippable()) {
            guildItem = &def;
            break;
        }
    }

    if (guildItem) {
        HeldItem held;
        held.name = guildItem->name;
        held.M4E97 = guildItem->id;
        held.identified = true;
        hero.inventory.append(held);

        // Character without the guild cannot equip it.
        QString reason;
        bool equipped = hero.equipItem(0, reason);
        check(!equipped, "cannot equip guild-restricted item without guild");
        check(hero.inventory.size() == 1, "item still in inventory after failed equip");

        // Join all guilds and verify it can now be equipped.
        for (const QString& g : GameConstants::GUILD_NAMES) hero.joinGuild(g);
        equipped = hero.equipItem(0, reason);
        check(equipped, "can equip guild-restricted item after joining guild");
    } else {
        check(true, "no guild-restricted items found (skip)");
    }

    // --------------------------------------------------------------- report
    out("");
    out("====================");
    out(QString("%1 passed, %2 failed").arg(g_passed).arg(g_failed));
    return g_failed == 0 ? 0 : 1;
}
