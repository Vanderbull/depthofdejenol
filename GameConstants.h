#ifndef GAMECONSTANTS_H
#define GAMECONSTANTS_H

#include <QStringList>
#include <QMap>

namespace GameConstants {
    // Race & Age Limits
    inline QMap<QString, int> getRaceAgeLimits() {
        return { {"Human", 80}, {"Elf", 500}, {"Dwarf", 200}, {"Gnome", 150} };
    }

    const int DEFAULT_ALIGNMENT_INDEX = 1; // Neutral
    const int DEFAULT_STAT_POINTS = 5;      //
    // Character Options
    const QStringList STAT_NAMES = {"Strength", "Intelligence", "Wisdom", "Constitution", "Charisma", "Dexterity"};
    const QStringList ALIGNMENT_NAMES = {"Good", "Neutral", "Evil"};
    const QStringList SEX_OPTIONS = {"Male", "Female"};
    
    // Guild Data
    const QStringList GUILD_NAMES = {
        "Nomad", "Paladin", "Warrior", "Villain", "Seeker", 
        "Thief", "Scavenger", "Mage", "Sorcerer", "Wizard", "Healer", "Ninja"
    };

    inline QMap<QString, QString> getGuildMasters() {
        return {
            {"Nomad", "Goch"}, {"Paladin", "Tuadar"}, {"Warrior", "Spore"},
            {"Mage", "Ge'nal"}, {"Healer", "Theshal"}, {"Ninja", "Shadowblade"}
        };
    }
    // Library Dialog
    const QString CATEGORY_MAGIC = "Magic Books";
    const QString CATEGORY_MONSTERS = "Creatures";
    const QString CATEGORY_ITEMS = "Items";
}

#endif
