#include "selftest_harness.h"
#include "src/spell_casting/SpellBook.h"
#include "src/spell_casting/SpellMechanics.h"
#include "src/spells/SpellCategory.h"
#include "src/spells/SpellTargeting.h"
#include "src/spells/SpellResistance.h"
#include "src/spells/SpellScaling.h"

// ---------------------------------------------------------------
// Non-vacuous tests for spell mechanics (v6.5.0 roadmap 1.3)
// Each test FAILS when the corresponding mechanic is reverted.
// ---------------------------------------------------------------

static void testSpellRequiresMinimumStats() {
    section("[S1] Spell requires minimum stats to cast");

    SpellBook& sb = SpellBook::instance();
    check(sb.load("data/spells.json"), "spells.json loads");

    // Create a character with low stats (should not qualify for high-level spells)
    Character weak;
    weak.name = "Weakling";
    weak.intelligence = 5;
    weak.wisdom = 5;
    weak.charisma = 5;
    weak.guildLevels["Mages Guild"] = 1;

    // Create a character with high stats
    Character strong;
    strong.name = "Strongmage";
    strong.intelligence = 30;
    strong.wisdom = 30;
    strong.charisma = 30;
    strong.guildLevels["Mages Guild"] = 10;

    // Flame Bolt requires intelligence 10 — weak should NOT qualify
    const SpellDef* flameBolt = sb.byName("Flame Bolt");
    check(flameBolt != nullptr, "Flame Bolt exists");
    if (flameBolt) {
        QList<SpellDef> weakSpells = sb.spellsFor(weak);
        bool weakKnowsFlameBolt = false;
        for (const SpellDef& s : weakSpells) {
            if (s.name == "Flame Bolt") { weakKnowsFlameBolt = true; break; }
        }
        check(!weakKnowsFlameBolt,
              "Character with Int 5 cannot cast Flame Bolt (requires Int 10)");
    }

    // Strong character should qualify
    if (flameBolt) {
        QList<SpellDef> strongSpells = sb.spellsFor(strong);
        bool strongKnowsFlameBolt = false;
        for (const SpellDef& s : strongSpells) {
            if (s.name == "Flame Bolt") { strongKnowsFlameBolt = true; break; }
        }
        check(strongKnowsFlameBolt,
              "Character with Int 30 can cast Flame Bolt (requires Int 10)");
    }

    // Inferno requires intelligence 30, guild level 7, and Wizards Guild
    const SpellDef* inferno = sb.byName("Inferno");
    check(inferno != nullptr, "Inferno exists");
    if (inferno) {
        // Strong has guild level 10 in Mages Guild but Inferno is Wizards Guild only
        bool strongKnowsInferno = false;
        for (const SpellDef& s : sb.spellsFor(strong)) {
            if (s.name == "Inferno") { strongKnowsInferno = true; break; }
        }
        check(!strongKnowsInferno,
              "Mage cannot cast Inferno (Wizards Guild only)");

        // Create a Wizard with sufficient level
        Character wizard;
        wizard.name = "Wizard";
        wizard.intelligence = 30;
        wizard.guildLevels["Wizards Guild"] = 10;

        bool wizardKnowsInferno = false;
        for (const SpellDef& s : sb.spellsFor(wizard)) {
            if (s.name == "Inferno") { wizardKnowsInferno = true; break; }
        }
        check(wizardKnowsInferno,
              "Wizard with Int 30 and guild level 10 can cast Inferno");
    }
}

static void testSpellLevelScaling() {
    section("[S2] Spell level scales power correctly");

    // At base level, multiplier is 1.0
    check(SpellScaling::powerMultiplier(1, 1) == 1.0,
          "Power multiplier is 1.0 at base level");
    check(SpellScaling::powerMultiplier(3, 3) == 1.0,
          "Power multiplier is 1.0 when caster equals base level");

    // +10% per level above base
    check(SpellScaling::powerMultiplier(2, 1) == 1.1,
          "Power multiplier is 1.1 at one level above base");
    check(SpellScaling::powerMultiplier(4, 1) == 1.3,
          "Power multiplier is 1.3 at three levels above base");

    // Damage scaling
    int base = 20;
    check(SpellScaling::scaleDamage(base, 1, 1) == 20,
          "Base damage at base level");
    check(SpellScaling::scaleDamage(base, 4, 1) == 26,
          "Damage scales up at higher caster level");

    // Healing scaling
    check(SpellScaling::scaleHealing(20, 1, 1) == 20,
          "Base healing at base level");
    check(SpellScaling::scaleHealing(20, 4, 1) == 26,
          "Healing scales up at higher caster level");

    // Duration scaling: +1 turn per 3 levels above base
    check(SpellScaling::scaleDuration(2, 1, 1) == 2,
          "Base duration at base level");
    check(SpellScaling::scaleDuration(2, 7, 1) == 4,
          "Duration increases by 2 at 6 levels above base");
}

static void testSpellResistance() {
    section("[S3] Spell resistance roll works");

    // Spells with no resistance always succeed
    check(!SpellResistance::hasResistance("Heal"),
          "Heal has no resistance");
    check(!SpellResistance::hasResistance("Protection"),
          "Protection has no resistance");

    // Spells with resistance can be resisted
    check(SpellResistance::hasResistance("Fire"),
          "Fire has resistance");
    check(SpellResistance::hasResistance("Charm"),
          "Charm has resistance");
    check(SpellResistance::hasResistance("Mind"),
          "Mind has resistance");

    // Base resistance chance is 20%
    check(SpellResistance::resistanceChance("Fire", 5, 5) == 20,
          "Resistance chance is 20% when caster equals monster level");

    // +5% per level the monster is above the caster
    check(SpellResistance::resistanceChance("Fire", 5, 6) == 25,
          "Resistance chance increases by 5% when monster is 1 level higher");
    check(SpellResistance::resistanceChance("Fire", 5, 8) == 35,
          "Resistance chance increases by 15% when monster is 3 levels higher");

    // Caster above monster level — still 20% base
    check(SpellResistance::resistanceChance("Fire", 8, 5) == 20,
          "Resistance chance stays at 20% when caster is above monster");

    // Resistance roll: deterministic when chance is 0 or 100
    check(!SpellResistance::isResisted("Heal", 5, 10),
          "Heal is never resisted (no resistance type)");
    check(SpellResistance::isResisted("Fire", 1, 100),
          "Fire is always resisted when monster is 99 levels higher (chance >= 100)");
}

static void testSpellTargeting() {
    section("[S4] Spell targets correct number of monsters");

    // Single-target spells
    check(SpellTargeting::targetCount("Electrical", 5) == 1,
          "Electrical spells target 1 monster");
    check(SpellTargeting::targetCount("Damage", 5) == 1,
          "Damage spells target 1 monster");
    check(SpellTargeting::targetCount("Kill", 5) == 1,
          "Kill spells target 1 monster");
    check(!SpellTargeting::isMultiTarget("Electrical"),
          "Electrical is not multi-target");

    // Group spells: 4 targets (MORDOR: "4 Monsters in 1 Group")
    check(SpellTargeting::targetCount("Charm", 1) == 4,
          "Charm spells target 4 monsters");
    check(SpellTargeting::targetCount("Charm", 10) == 4,
          "Charm spells still target 4 monsters regardless of level");
    check(SpellTargeting::isMultiTarget("Charm"),
          "Charm is multi-target");

    // Area spells: 1 + casterLevel / 3
    check(SpellTargeting::targetCount("Fire", 1) == 1,
          "Fire at level 1 targets 1 monster");
    check(SpellTargeting::targetCount("Fire", 3) == 2,
          "Fire at level 3 targets 2 monsters");
    check(SpellTargeting::targetCount("Fire", 6) == 3,
          "Fire at level 6 targets 3 monsters");
    check(SpellTargeting::targetCount("Fire", 10) == 4,
          "Fire at level 10 targets 4 monsters");
    // isMultiTarget uses default caster level 1, so Fire is single-target there.
    // At higher caster levels, Fire becomes multi-target.
    check(SpellTargeting::targetCount("Fire", 1) == 1,
          "Fire at level 1 targets exactly 1 monster");
    check(SpellTargeting::targetCount("Fire", 6) > 1,
          "Fire at level 6 targets multiple monsters");

    // Self-targeting spells
    check(SpellTargeting::targetCount("Heal", 5) == 0,
          "Heal targets self (0 monsters)");
    check(SpellTargeting::targetCount("Protection", 5) == 0,
          "Protection targets self (0 monsters)");
    check(SpellTargeting::targetCount("Movement", 5) == 0,
          "Movement targets self (0 monsters)");

    // Max targets cap
    check(SpellTargeting::maxTargets() == 8,
          "Max targets is 8");
}

static void testSpellCategoryRegistry() {
    section("[S5] Spell category registry loads all categories");

    QStringList names = SpellCategory::names();
    check(names.size() == 18,
          "All 18 spell categories are registered");

    // Verify each category from MORDOR exists
    const char* expected[] = {
        "Fire", "Cold", "Electrical", "Element", "Damage", "Kill",
        "Banish", "Charm", "Bind", "Mind", "Heal", "Dispel",
        "Protection", "Resistant", "Visual", "Magical", "Movement", "Location"
    };
    for (const char* name : expected) {
        check(SpellCategory::find(name) != nullptr,
              QString("Category '%1' exists in registry").arg(name));
    }

    // Verify targeting types
    const SpellCategory::Def* fire = SpellCategory::find("Fire");
    check(fire && fire->targeting == SpellCategory::Targeting::Area,
          "Fire has Area targeting");

    const SpellCategory::Def* charm = SpellCategory::find("Charm");
    check(charm && charm->targeting == SpellCategory::Targeting::Group,
          "Charm has Group targeting");

    const SpellCategory::Def* lightning = SpellCategory::find("Electrical");
    check(lightning && lightning->targeting == SpellCategory::Targeting::Single,
          "Electrical has Single targeting");

    const SpellCategory::Def* heal = SpellCategory::find("Heal");
    check(heal && heal->targeting == SpellCategory::Targeting::Self,
          "Heal has Self targeting");

    // Verify resistance types
    check(fire && fire->resistance == SpellCategory::Resistance::Elemental,
          "Fire has Elemental resistance");
    check(charm && charm->resistance == SpellCategory::Resistance::Charm,
          "Charm has Charm resistance");
    check(heal && heal->resistance == SpellCategory::Resistance::None,
          "Heal has no resistance");

    // Display names
    check(SpellCategory::targetingName(SpellCategory::Targeting::Single) == "Single target",
          "Single targeting display name");
    check(SpellCategory::targetingName(SpellCategory::Targeting::Area) == "Area",
          "Area targeting display name");
    check(SpellCategory::resistanceName(SpellCategory::Resistance::Elemental) == "Elemental",
          "Elemental resistance display name");
}

void runSpellMechanicTests() {
    testSpellRequiresMinimumStats();
    testSpellLevelScaling();
    testSpellResistance();
    testSpellTargeting();
    testSpellCategoryRegistry();
}
