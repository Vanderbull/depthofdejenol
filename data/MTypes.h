#pragma once

// Qt includes
#include <QString>
#include <QVector>
#include <stdint.h> // Keep for fixed-width integers
#include <array>

enum Guild {
	GLD_SORCERER,
    GLD_NOMAD,
    GLD_WARRIOR,
    GLD_PALADIN,
    GLD_NINJA,
    GLD_VILLIAN,
    GLD_SEEKER,
    GLD_THIEF,
    GLD_SCAVENGER,
    GLD_HEALER
};

enum MonsterType {
    MON_HUMANOID,
    MON_CLEANUP,
    MON_DEMON,
    MON_DEVIL,
    MON_ELEMENTAL,
    MON_REPTILE,
    MON_DRAGON,
    MON_ANIMAL,
    MON_INSECT,
    MON_UNDEAD,
    MON_AQUATIC,
    MON_GIANT,
    MON_MYTHICAL,
    MON_LYCANTHROPE,
    MON_THIEF,
    MON_MAGE,
    MON_WARRIOR,
    MON_INDIGINI,
    MON_NUMKINDS
};

enum Alignment {
    GOOD,
    NEUTRAL,
    EVIL
};

enum Stats {
    STAT_STR,
    STAT_INT,
    STAT_WIS,
    STAT_CON,
    STAT_CHA,
    STAT_DEX
};

enum Restances {
    RES_FIRE,
    RES_COLD,
    RES_ELECTRIC,
    RES_MIND,
    RES_DISEASE,
    RES_POISON,
    RES_MAGIC,
    RES_STONE,
    RES_PARALYSIS,
    RES_DRAIN,
    RES_ACID
};

// --- STRUCTS (std::string -> QString, std::vector -> QVector) ---

struct GameData {
    struct RaceInfo {
        QString name;
        int16_t minStats[7];
        int16_t maxStats[7];
        int16_t m_resistences[12];
        int32_t m_alignement;
        int16_t m_size;
        int16_t m_bonusPoints;
        int16_t m_maxAge;
        float m_expFactor;
    };
    struct GuildInfo {
        QString name;
        int16_t m_averageHits;
        int16_t MaxLevel;
        int16_t MH;
        float expFactor;
        int16_t u5;
        int16_t reqStats[7];
        int32_t alignment;
        float abilityrates[7];
        int16_t u9;
        float u10;
        int16_t questPercentage;
        float spellstuff[19][2];
        int32_t RaceMask;
        int16_t u14;
        float levelMod;
        float u16;
        float u17;
        int16_t u18;
        float u19;
        int16_t u20;
        int16_t u21;
    };

    struct ItemSubtype {
        QString name;
        int16_t itemtype;
    };

    struct ItemType {
        QString name;
        int16_t IsEquipable;
    };

    struct MonsterSubtype {
        QString name;
        int16_t MonsterType;
    };

    struct MonsterType {
        QString name;
        int16_t unused;
    };

    QVector<RaceInfo> races;
    QVector<GuildInfo> guilds;
    QVector<ItemSubtype> itemSubtypes;
    QVector<ItemType> itemTypes;
    QVector<MonsterSubtype> monsterSubtypes;
    QVector<MonsterType> monsterTypes;
};

struct Spell {
    QString name;
    int16_t ID;
    int16_t category;
    int16_t level;
    int16_t u4;
    int16_t u5;
    int16_t killEffect;
    int16_t affectMonster;
    int16_t affectGroup;
    int16_t damage1;
    int16_t damage2;
    int16_t specialEffect;
    int16_t required[7];
    int16_t resistedBy;
};

struct Item {
    QString name;
    int16_t ID;
    int16_t att;
    int16_t def;
    int32_t price;
    int16_t floor;
    int16_t rarity;
    int32_t abilities;
    int16_t swings;
    int16_t specialType;
    int16_t spellIndex;
    int16_t spellID;
    int16_t charges;
    int16_t deleted;
    int32_t guilds;
    int16_t levelScale;
    float damageMod;
    int32_t alignmentFlags;
    int16_t nHands;
    int16_t type;
    int32_t resistanceFlags;
    int16_t statsRequired[7];
    int16_t statsMod[7];
    int16_t cursed;
    int16_t spellLvl;
    int16_t classRestricted;
};

struct HeldItem { //HeldItem
    int16_t M4E97; //ID
    int16_t M4EA1; //alignment
    int16_t M4ED4; //charges
    int16_t M5467; //equipped
    int16_t M4EBF;
    int16_t M56DB;
    int16_t M577E;
    int16_t M4EEE;
    int16_t M572A;
};

struct GuildStatus { //GuildStatus
    int16_t M4E82;
    int64_t M579B;
    int16_t M55CB;
    int16_t M57A9;
    int16_t M57B2;
    float M4E97;
    float M4EA1;
};

struct Companion {
    QString M548B;
    int16_t M4ED4;
    int16_t M57D4;
    int16_t M4EAC;
    int16_t M4EB4;
    int16_t M4EBF;
    int16_t M4E97;
    int16_t M4EA1;
    int16_t M57E1;
    int16_t M4EEE;
};

struct T57EF {
    int32_t M425F;
    int32_t M4258;
    int32_t M4216;
    int32_t M420C;
    int16_t M5801;
};

struct T4E6B {
    int16_t M4E78;
    int16_t M4E7D;
    int16_t M4E82;
};

struct Character {
    QString name;
    int16_t M5821; //race
    int16_t M4EBF; //alignment
    int16_t M5829; //sex
    float M5577; //36
    int16_t M5830; //40
    int16_t M5470; //level
    int16_t M583F; //x
    int16_t M5845; //y
    float M4E97; //48
    float M4EA1; //52
    int16_t stats[7];//stats
    int16_t modStats[7];//more stats?
    HeldItem M5857[41]; //Inventory
    HeldItem M5860[41]; //Bank
    int16_t M586D[36];
    int16_t M56F7;
    float M587C;
    float M588A;
    int16_t M5899;
    int16_t M58A6;
    int64_t M579B;
    int64_t M4F2D;
    int16_t M4EAC;
    int16_t M4EB4;
    int64_t M58AF;
    int16_t M56AF;
    int16_t M58BB;
    GuildStatus M5584[16];//GuildStatus
    Companion M58CB[5]; //companions?
    int16_t M5700;
    int16_t M58DC;
    int16_t M58E9;
    QString M58F9;
    int64_t M5906[9];
    int16_t M5914[6];
    int16_t M56B9[8];
    int16_t resistances[12];
    int32_t M591F;
    int16_t M592F[12];
    int16_t M5483;
    int16_t M5943;
    QString M594C;
    T57EF M5958[21];
    int16_t M5962;
    int32_t M5970;
    int32_t M5977;
    int16_t M5985[12];
    int16_t M5997[2];
    int16_t M59A1;
    int16_t M59B0[11];
    int16_t M59BB;
    int16_t M59CB;
    int16_t M59D9;
    T4E6B M59E7;
    int16_t M59F4[3];
};

struct Monster {
    QString name;

    int16_t att;
    int16_t def;
    int16_t id;
    int16_t hits;
    int16_t numGroups;
    int16_t picID;
    int16_t lockedChance;
    int16_t levelFound;

    int16_t resistances[12]; //only first 11 used
    int32_t specialPropertyFlags;
    int32_t specialAttackFlags;
    int32_t spellFlags;

    int16_t chance;
    int16_t boxChance[4]; //only first 3 used
    int16_t alignment; // 0 Good - 2 Evil
    int16_t ingroup;
    int32_t goldFactor; //wabbit's u11
    int32_t trapFlags; //wabbit's u12
    int16_t guildlevel;

    int16_t stats[7]; //only first 6 used
    int16_t type; //0 Humanoid - 17 indigini
    float damageMod;
    int16_t companionType; //0 Humanoid - 17 indigini
    int16_t companionSpawnMode; //wabbit's u18
    int16_t companionID; //id of specific monster
    int16_t items[11]; //only first 10 used
    int16_t subtype; //wabbit's u21
    int16_t companionSubtype; //wabbit's u22
    int16_t size;
    int16_t itemDropLevel;
    int16_t itemChance;
};

struct Automap {
    int16_t deepestLevel = 0;
    // std::array<int32_t, 900> replaced with a fixed-size C-style array
    QVector<std::array<int32_t, 900>> flags; 	// NOTE: This structure is unusual; it creates a vector of arrays.
                                 		// You might consider changing the array to a QVector if size is variable,
                                 		// or use a flat QVector<int32_t> and calculate the index if it's one large map.
};

struct GuildLog {
    int16_t guildID;
    int32_t date;
    QString message;
};

struct Floor {
    enum {
        WALL_EAST,
        WALL_NORTH,
        DOOR_EAST,
        DOOR_NORTH,
        SECRET_DOOR_EAST,
        SECRET_DOOR_NORTH,
        FACE_NORTH,
        FACE_EAST,
        FACE_SOUTH,
        FACE_WEST,
        EXTINGUISHER,
        PIT,
        STAIRS_UP,
        STAIRS_DOWN,
        TELEPORTER,
        WATER,
        QUICKSAND,
        ROTATOR,
        ANTIMAGIC,
        ROCK,
        FOG,
        CHUTE,
        STUD,
        EXPLORED //only used by automap
    };

    struct Field {
        int16_t areaID;
        int64_t flags;
    };

    struct Area {
        int32_t spawnMask;
        int16_t lairID;
    };

    struct Chute {
        int16_t x, y, depth;
    };

    struct Teleporter {
        int16_t x, y, x2, y2, z2;
    };

    int16_t width;
    int16_t height;
    int16_t levelNum;
    QVector<Field> fields;
    QVector<Area> areas;
    QVector<Teleporter> teleporters;
    QVector<Chute> chutes;
};

struct Library {
    struct Record {
        int16_t ID;
        int16_t known;
        int64_t nSeen;
        QString name;
        int16_t monID; //ID of monster that dropped item, or 0 for monsters
        int32_t lastSeen;
        QString location;
        int16_t nDeaths; //0xFFFF for items
    };
    int16_t monstersFound;
    int16_t itemsFound;
    QVector<Record> monsters;
    QVector<Record> items;
};

struct HallRecord {
    int64_t value;
    QString name;
    int32_t date;
};

// --- TYPEDEFS ---

typedef QVector<Spell> Spells;
typedef QVector<Item> Items;
typedef QVector<Character> Characters;
typedef QVector<Monster> Monsters;

typedef QVector<GuildLog> GuildLogs;
typedef QVector<Floor> Dungeon;

typedef QVector<HallRecord> HallRecords;
