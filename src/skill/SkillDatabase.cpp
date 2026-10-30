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

    // --- Craft skills ---
    skills.append({QStringLiteral("Alchemy"),
                   QStringLiteral("Creates potions and poisons"),
                   1, QString(), QStringLiteral("Craft"), QStringLiteral("flask.png")});
    skills.append({QStringLiteral("Smithing"),
                   QStringLiteral("Creates and repairs weapons and armor"),
                   1, QString(), QStringLiteral("Craft"), QStringLiteral("hammer.png")});

    // --- Social skills ---
    skills.append({QStringLiteral("Persuasion"),
                   QStringLiteral("Improves persuasion rolls"),
                   1, QString(), QStringLiteral("Social"), QStringLiteral("speech.png")});

    // --- Advanced skills (require prerequisite) ---
    skills.append({QStringLiteral("Advanced Swordsmanship"),
                   QStringLiteral("Improves advanced melee techniques"),
                   2, QString("Swordsmanship"), QStringLiteral("Combat"), QStringLiteral("sword2.png")});

    // --- Survival skills ---
    skills.append({QStringLiteral("First Aid"),
                   QStringLiteral("Heals allies faster"),
                   1, QString(), QStringLiteral("Survival"), QStringLiteral("medkit.png")});

    return skills;
}
