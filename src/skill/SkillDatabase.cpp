#include "SkillDatabase.h"

// Provides the static list of skills available for allocation in v6.1.0.
QVector<SkillDef> skillDefs()
{
    QVector<SkillDef> skills;

    // --- Combat skills ---
    skills.append({QStringLiteral("Swordsmanship"),
                   QStringLiteral("Improves melee attack and damage"),
                   1, QString(), QStringLiteral("Combat"), QStringLiteral("sword.png")});
    skills.append({QStringLiteral("Archery"),
                   QStringLiteral("Improves ranged attack and accuracy"),
                   1, QString(), QStringLiteral("Combat"), QStringLiteral("bow.png")});
    skills.append({QStringLiteral("Stealth"),
                   QStringLiteral("Reduces detection chance"),
                   1, QString(), QStringLiteral("Stealth"), QStringLiteral("cloak.png")});

    // --- Survival / thief skills ---
    skills.append({QStringLiteral("Thievery"),
                   QStringLiteral("Improves ability to pick locks and disarm traps"),
                   1, QString(), QStringLiteral("Combat"), QStringLiteral("thief.png")});
    skills.append({QStringLiteral("PickLock"),
                   QStringLiteral("Improves ability to pick locks"),
                   2, QString("Thievery"), QStringLiteral("Combat"), QStringLiteral("lockpick.png")});

    // --- Craft skills ---
    skills.append({QStringLiteral("Alchemy"),
                   QStringLiteral("Creates potions and poisons"),
                   1, QString(), QStringLiteral("Craft"), QStringLiteral("flask.png")});
    skills.append({QStringLiteral("Smithing"),
                   QStringLiteral("Creates and repairs weapons and armor"),
                   1, QString(), QStringLiteral("Craft"), QStringLiteral("hammer.png")});
    skills.append({QStringLiteral("Enchanting"),
                   QStringLiteral("Enchants items with magical properties"),
                   1, QString(), QStringLiteral("Craft"), QStringLiteral("wand.png")});
    skills.append({QStringLiteral("Tannery"),
                   QStringLiteral("Converts hides into leather goods"),
                   1, QString(), QStringLiteral("Craft"), QStringLiteral("leather.png")});
    skills.append({QStringLiteral("Tailoring"),
                   QStringLiteral("Creates and repairs clothing"),
                   1, QString(), QStringLiteral("Craft"), QStringLiteral("sewing.png")});

    // --- Social skills ---
    skills.append({QStringLiteral("Persuasion"),
                   QStringLiteral("Improves persuasion rolls"),
                   1, QString(), QStringLiteral("Social"), QStringLiteral("speech.png")});
    skills.append({QStringLiteral("Intimidation"),
                   QStringLiteral("Improves intimidation rolls"),
                   1, QString(), QStringLiteral("Social"), QStringLiteral("intimidate.png")});
    skills.append({QStringLiteral("Deception"),
                   QStringLiteral("Improves deception and lying rolls"),
                   1, QString(), QStringLiteral("Social"), QStringLiteral("deceive.png")});
    skills.append({QStringLiteral("Diplomacy"),
                   QStringLiteral("Improves negotiation and diplomacy rolls"),
                   1, QString(), QStringLiteral("Social"), QStringLiteral("diplomacy.png")});
    skills.append({QStringLiteral("Religion"),
                   QStringLiteral("Improves religious and ritual rolls"),
                   1, QString(), QStringLiteral("Social"), QStringLiteral("religion.png")});

    // --- Advanced skills (require prerequisite) ---
    skills.append({QStringLiteral("Advanced Swordsmanship"),
                   QStringLiteral("Improves advanced melee techniques"),
                   2, QString("Swordsmanship"), QStringLiteral("Combat"), QStringLiteral("sword2.png")});

    // --- Survival skills ---
    skills.append({QStringLiteral("First Aid"),
                   QStringLiteral("Heals allies faster"),
                   1, QString(), QStringLiteral("Survival"), QStringLiteral("medkit.png")});
    skills.append({QStringLiteral("Survival"),
                   QStringLiteral("Tracks creatures, finds food, and survives the wild"),
                   1, QString(), QStringLiteral("Survival"), QStringLiteral("compass.png")});
    skills.append({QStringLiteral("Detection"),
                   QStringLiteral("Notice hidden doors, traps, and creatures"),
                   1, QString(), QStringLiteral("Survival"), QStringLiteral("sight.png")});
    skills.append({QStringLiteral("Search"),
                   QStringLiteral("Find hidden items and tracks"),
                   1, QString(), QStringLiteral("Survival"), QStringLiteral("search.png")});
    skills.append({QStringLiteral("Forage"),
                   QStringLiteral("Find food and water from the environment"),
                   1, QString(), QStringLiteral("Survival"), QStringLiteral("forage.png")});
    skills.append({QStringLiteral("DisarmTrap"),
                   QStringLiteral("Disable a trap before it springs"),
                   2, QString("Thievery"), QStringLiteral("Combat"), QStringLiteral("trap.png")});

    // --- Combat skill feats and maneuvers ---
    // Cost 0, no prerequisite: these are granted by feats or by training a
    // special skill rather than by the skill point allocation pool. They are
    // Combat-type skills so they appear in the skill menu, but they cannot be
    // learned via spendSkillPoints().
    skills.append({QStringLiteral("Parry"),
                   QStringLiteral("Parry an attack: the next blow against you is halved"),
                   0, QString(), QStringLiteral("Combat"), QStringLiteral("parry.png")});
    skills.append({QStringLiteral("Dodge"),
                   QStringLiteral("Dodge an attack: the next blow against you misses"),
                   0, QString(), QStringLiteral("Combat"), QStringLiteral("dodge.png")});
    skills.append({QStringLiteral("DualWield"),
                   QStringLiteral("Ready two weapons: make one extra attack in this turn"),
                   0, QString(), QStringLiteral("Combat"), QStringLiteral("dualwield.png")});
    skills.append({QStringLiteral("ShieldBash"),
                   QStringLiteral("Bash with a shield: stun the target and deal 1d6 shock damage"),
                   0, QString(), QStringLiteral("Combat"), QStringLiteral("shieldbash.png")});

    return skills;
}

QVector<CraftingRecipe> craftingRecipes()
{
    QVector<CraftingRecipe> recipes;
    // Alchemy recipes
    recipes.append({QStringLiteral("Potion of Strength"), QStringLiteral("Alchemy"), 2, QStringList{ QStringLiteral("Herbs"), QStringLiteral("Spirit") }, 0.85});
    recipes.append({QStringLiteral("Magic Potion"), QStringLiteral("Alchemy"), 3, QStringList{ QStringLiteral("Herbs"), QStringLiteral("Spirit"), QStringLiteral("Satchel") }, 0.75});
    recipes.append({QStringLiteral("Poison Vial"), QStringLiteral("Alchemy"), 2, QStringList{ QStringLiteral("Herbs"), QStringLiteral("Venom") }, 0.65});
    // Smithing recipes
    recipes.append({QStringLiteral("Steel Sword"), QStringLiteral("Smithing"), 3, QStringList{ QStringLiteral("Iron Ingot"), QStringLiteral("Wood") }, 0.70});
    recipes.append({QStringLiteral("Steel Shield"), QStringLiteral("Smithing"), 2, QStringList{ QStringLiteral("Iron Ingot"), QStringLiteral("Wood") }, 0.75});
    // Enchanting recipes
    recipes.append({QStringLiteral("Enchanted Bow"), QStringLiteral("Enchanting"), 3, QStringList{ QStringLiteral("Bow"), QStringLiteral("Magic Crystal") }, 0.60});
    recipes.append({QStringLiteral("Enchanted Armor"), QStringLiteral("Enchanting"), 3, QStringList{ QStringLiteral("Chainmail"), QStringLiteral("Magic Crystal") }, 0.65});
    // Tannery recipes
    recipes.append({QStringLiteral("Leather Armor"), QStringLiteral("Tannery"), 2, QStringList{ QStringLiteral("Hides") }, 0.80});
    // Tailoring recipes
    recipes.append({QStringLiteral("Leather Armor"), QStringLiteral("Tailoring"), 1, QStringList{ QStringLiteral("Hides") }, 0.70});
    recipes.append({QStringLiteral("Chainmail"), QStringLiteral("Tailoring"), 3, QStringList{ QStringLiteral("Hides"), QStringLiteral("Iron Ingot") }, 0.60});
    return recipes;
}
