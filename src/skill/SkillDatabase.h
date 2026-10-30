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

