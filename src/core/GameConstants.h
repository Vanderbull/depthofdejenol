#ifndef GAMECONSTANTS_H
#define GAMECONSTANTS_H

#include <QStringList>
#include <QMap>
#include <QVector>
#include <QString>

namespace GameConstants {


    enum EntityStatus {
        Normal    = 0,
        Poisoned  = 1 << 0,
        Blinded   = 1 << 1,
        OnFire    = 1 << 2,
        Alive     = 1 << 3,
        Dead      = 1 << 4,
        Snared    = 1 << 5   // immobilized by a snare trap
    };
    Q_DECLARE_FLAGS(EntityStatuses, EntityStatus)

    /* Example
    struct Monster {
      QString name;
      int hp;
      EntityStatuses statuses; // Holds all active effects in one variable
    };
    */

    static const int MAX_PARTY_SIZE = 4;
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

    // Required for m_currentMode in gameStateManager
    enum class GameMode {
        InCity,
        InDungeon,
        Combat,
        MainMenu
    };

    // Required for m_currentCityLocation in gameStateManager
    enum class CityLocation {
        Street,
        Guild,
        GeneralStore,
        Temple,
        Inn,
        Bank,
        Library,
        Confinement,
        Tavern,
        None
    };

    enum AlignmentStatus
    {
        AS_Allowed,
        AS_NotAllowed,
        AS_Required
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

    // --- Boss stat scaling ---
    static constexpr int BASE_BOSS_HP_PER_TIER = 150;
    static constexpr int BASE_BOSS_HP_PER_LEVEL = 50;
    static constexpr int BASE_BOSS_ATT_PER_TIER = 20;
    static constexpr int BASE_BOSS_ATT_PER_LEVEL = 2;
    static constexpr int BASE_BOSS_DEF_PER_TIER = 8;
    static constexpr int BASE_BOSS_DEF_PER_LEVEL = 1;
    static constexpr int BOSS_DAMAGE_MOD = 150;
    static constexpr int BOSS_LEVEL_SCALE = 5;
    static constexpr int BOSS_XP_PER_TIER = 5000;
    static constexpr int BOSS_XP_PER_LEVEL = 250;

    // --- Abyss boss stat scaling ---
    static constexpr int ABYSS_BASE_HP = 2000;
    static constexpr int ABYSS_BASE_ATT = 50;
    static constexpr int ABYSS_BASE_DEF = 20;
    static constexpr int ABYSS_DAMAGE_MOD = 250;
    static constexpr int ABYSS_LEVEL_SCALE = 10;
    static constexpr int ABYSS_LEVEL_BASE = 40;
    static constexpr int ABYSS_LEVEL_PER_FLOOR = 3;

    // --- Resistances ---
    // A Resist <Type> potion/scroll grants this much reduction for its
    // duration; stacking takes the higher value rather than adding, so
    // re-drinking one is never a waste but never breaks the cap.
    static constexpr int RESIST_GRANT_PERCENT = 50;
    // Protection is a general ward rather than a single element.
    static constexpr int PROTECTION_GRANT_PERCENT = 25;
    // A resisted hit never deals less than this, so a 100%-resistant creature
    // is still killable rather than immortal.
    static constexpr int MIN_RESISTED_DAMAGE = 1;

    // --- Quest rewards ---
    static constexpr int QUEST_S0_GOLD = 100;
    static constexpr int QUEST_S0_XP = 100;
    static constexpr int QUEST_S1_GOLD = 1000;
    static constexpr int QUEST_S1_XP = 5000;
    static constexpr int QUEST_S2_GOLD = 2500;
    static constexpr int QUEST_S2_XP = 10000;
    static constexpr int QUEST_S3_GOLD = 5000;
    static constexpr int QUEST_S3_XP = 25000;
    static constexpr int QUEST_S4_GOLD = 10000;
    static constexpr int QUEST_S4_XP = 50000;
    static constexpr int QUEST_S5_GOLD = 50000;
    static constexpr int QUEST_S5_XP = 250000;

    // --- Achievement gold rewards ---
    static constexpr int ACH_FIRST_BLOOD = 100;
    static constexpr int ACH_SLAYER = 500;
    static constexpr int ACH_FLOOR_5 = 250;
    static constexpr int ACH_FLOOR_10 = 500;
    static constexpr int ACH_FLOOR_15 = 1000;
    static constexpr int ACH_BOSS_5 = 500;
    static constexpr int ACH_BOSS_10 = 1000;
    static constexpr int ACH_BOSS_15 = 5000;
    static constexpr int ACH_NG_PLUS_1 = 1000;
    static constexpr int ACH_NG_PLUS_3 = 3000;
    static constexpr int ACH_GOLD_10000 = 500;
    static constexpr int ACH_GOLD_50000 = 2000;
    static constexpr int ACH_DIE_1 = 100;
    static constexpr int ACH_DIE_10 = 500;
    static constexpr int ACH_LEVEL_10 = 500;
    static constexpr int ACH_LEVEL_20 = 1000;
    static constexpr int ACH_LEVEL_30 = 2000;
    static constexpr int ACH_ABYSS_1 = 1000;
    static constexpr int ACH_ABYSS_5 = 3000;
    static constexpr int ACH_ABYSS_10 = 10000;

    // --- Aging ---
    inline const QMap<QString, int>& raceMaxAges() {
        static const QMap<QString, int> ages = {
            {"Human", 100}, {"Elf", 400}, {"Giant", 225},
            {"Gnome", 300}, {"Dwarf", 275}, {"Ogre", 265},
            {"Morloch", 175}, {"Osiri", 325}, {"Troll", 285}
        };
        return ages;
    }
    static constexpr double DECAY_THRESHOLD_RATIO = 0.7;

}
Q_DECLARE_OPERATORS_FOR_FLAGS(GameConstants::EntityStatuses)

#endif
