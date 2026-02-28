#ifndef GAMECONSTANTS_H
#define GAMECONSTANTS_H

#include <QStringList>
#include <QMap>
#include <QVector>
#include <QString>

namespace GameConstants {

    // Measurements based on the spritesheet proportions
    static constexpr int SPRITE_WIDTH = 80;   
    static constexpr int SPRITE_HEIGHT = 90;  
    static constexpr int KERNING = 48; // Adjust this to tighten/loosen spacing
    
    static constexpr int FONT_CHAR_WIDTH = 32;
    static constexpr int FONT_CHAR_HEIGHT = 42;
    
    // --- Add these for the Registry ---
    const QString CAT_RACES = "Races";
    const QString CAT_GUILDS = "Guilds";
    const QString CAT_MASTERS = "GuildMasters";

    // --- Race Stat Types ---
    struct RaceStat 
    {
        int start;
        int min;
        int max;
    };

    enum AlignmentStatus 
    {
        AS_Allowed,
        AS_NotAllowed
    };

    struct RaceStats 
    {
        QString raceName;
        int maxAge;
        int experience;
        RaceStat strength;
        RaceStat intelligence;
        RaceStat wisdom;
        RaceStat constitution;
        RaceStat charisma;
        RaceStat dexterity;
        AlignmentStatus good;
        AlignmentStatus neutral;
        AlignmentStatus evil;
        QMap<QString, AlignmentStatus> guildEligibility; 
    };

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
