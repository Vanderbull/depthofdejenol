#pragma once

#include <QString>
#include <QVector>

// Skill definition used by the v6.1.0 skill acquisition system.
struct SkillDef {
    QString name;          // internal skill name (unique)
    QString description;   // short description shown in UI
    int cost = 1;          // skill points required to raise level by 1
    QString prerequisite;  // skill name that must be learned first, or "" for none
    QString type;          // category, e.g. Combat, Craft, Social, Survival
    QString icon;          // art file path (unused for now)
};

// Returns the static list of skill definitions.
QVector<SkillDef> skillDefs();

// CraftingRecipe: defines what can be created via artisan skills.
struct CraftingRecipe
{
    QString itemName;           // Item that gets crafted
    QString skill;              // Required artisan skill
    int levelRequirement = 1;   // Minimum skill level required
    QStringList ingredients;    // Required ingredients
    double successRate = 0.75;  // Base success rate (0-1)
};

// CompanionSkill: defines trainable companion skills
struct CompanionSkill
{
    QString name;           // skill name (e.g. "Guard")
    QString description;    // description
    int level = 1;          // starting level
    int cost = 1;           // skill points required to raise level by 1
    QString type;           // category (e.g. Combat, Survival, Social)
    int xpRequired = 100;   // XP required to level up
};

// Returns the companionSkill table (name -> skill definition).
QVector<CompanionSkill> companionSkills();

// Returns the craftingRecipe table (itemName -> recipe).
QVector<CraftingRecipe> craftingRecipes();

