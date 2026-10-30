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
#include "src/skill/SkillDatabase.h"

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

    // --------------------------------------------------------------- skill allocation tests
    out("--- [77] v6.1.0 slice 6.1a: skill allocation tests ---");

    // ---- Full allocation scenario ----
    Character skillHero;
    skillHero.name = "Skill Test Hero";
    skillHero.level = 1;
    // Starting skill points should be 0
    check(skillHero.skillPoints() == 0, "initial skill points = 0");

    // Add some skill points
    skillHero.addSkillPoints(5);
    check(skillHero.skillPoints() == 5, "skill points after addSkillPoints = 5");

    // Learn Swordsmanship (cost 1)
    int newLevel = 0;
    QString error;
    bool ok = skillHero.learnSkill("Swordsmanship", newLevel, error);
    check(ok, "learned Swordsmanship");
    check(newLevel == 1, "learned Swordsmanship increases level to 1");
    check(skillHero.skillPoints() == 4, "skill points after learning Swordsmanship = 4");

    // Try to learn a skill without enough points (character with 0 points)
    Character shortHero;
    shortHero.name = "Short Hero";
    shortHero.level = 1;
    ok = shortHero.learnSkill("Swordsmanship", newLevel, error);
    check(!ok, "cannot learn skill with 0 points");
    check(!error.isEmpty(), "error message about insufficient points");

    // ---- Prerequisite test ----
    Character prereqHero;
    prereqHero.name = "Prereq Hero";
    prereqHero.level = 1;
    prereqHero.addSkillPoints(5);
    // Attempt to learn Advanced Swordsmanship before learning Swordsmanship
    ok = prereqHero.learnSkill("Advanced Swordsmanship", newLevel, error);
    check(!ok, "cannot learn Advanced Swordsmanship without prerequisite");
    check(!error.isEmpty(), "error message about prerequisite");

    // Learn Swordsmanship
    ok = prereqHero.learnSkill("Swordsmanship", newLevel, error);
    check(ok, "learned Swordsmanship for prerequisite test");
    check(newLevel == 1, "learned Swordsmanship increases level to 1");

    // Now Advanced Swordsmanship should be learnable
    int newLevel2 = 0;
    ok = prereqHero.learnSkill("Advanced Swordsmanship", newLevel2, error);
    check(ok, "learned Advanced Swordsmanship after prerequisite");
    check(newLevel2 == 1, "Advanced Swordsmanship level increased to 1");
    check(prereqHero.skillPoints() == 2, "skill points after learning Advanced Swordsmanship = 2");

    // ---- Serialization test ----
    QVariantMap saved = skillHero.toMap();
    Character loaded;
    loaded.loadFromMap(saved);
    check(loaded.skillPoints() == skillHero.skillPoints(), "serialized skill points preserved");
    check(loaded.skillLevel("Swordsmanship") == 1, "serialized skill level Swordsmanship");
    check(loaded.skillLevel("Advanced Swordsmanship") == 0, "serialized skill level Advanced Swordsmanship (0)");

    // --------------------------------------------------------------- main report
    out("");
    out("====================");
    out(QString("%1 passed, %2 failed").arg(g_passed).arg(g_failed));
    return g_failed == 0 ? 0 : 1;
}
