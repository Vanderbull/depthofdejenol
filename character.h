#ifndef CHARACTER_H
#define CHARACTER_H

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDateTime>
#include <cstdint>

#include "src/core/GameConstants.h"

// --- Supporting structs (from MTypes.h) ---

struct HeldItem {
    int16_t M4E97;  // ID
    int16_t M4EA1;  // alignment
    int16_t M4ED4;  // charges
    int16_t M5467;  // equipped
    int16_t M4EBF;
    int16_t M56DB;
    int16_t M577E;
    int16_t M4EEE;
    int16_t M572A;
};

struct GuildStatus {
    int16_t M4E82;
    int64_t M579B;
    int16_t M55CB;
    int16_t M57A9;
    int16_t M57B2;
    float   M4E97;
    float   M4EA1;
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

// --- Character: runtime fields + all MTypes.h binary fields ---

struct Character {

    // ===== Runtime fields (existing) =====
    GameConstants::EntityStatuses status = GameConstants::Normal;

    // --- Basic Info ---
    QString name;
    QString race;          // runtime: race name string
    int age = 18;          // runtime: years
    int level = 1;         // runtime: level
    int experience = 0;    // runtime: XP
    int hp = 10;           // runtime: current HP
    int maxHp = 10;        // runtime: max HP
    int gold = 0;          // runtime: gold on hand

    // --- Stats ---
    int strength = 8;
    int intelligence = 8;
    int wisdom = 8;
    int constitution = 8;
    int charisma = 8;
    int dexterity = 8;

    // --- Resource Pools ---
    int mana = 50;
    int maxMana = 50;

    // --- Status Effects ---
    uint statusFlags = 0;
    bool isAlive = true;

    // --- Location Data ---
    int dungeonLevel = 0;
    int dungeonX = 0;
    int dungeonY = 0;
    int row = 0;

    // --- Inventory ---
    QStringList inventory = {"Hands"};

    // ===== MTypes.h binary fields (full record) =====

    // Basic info (binary layout)
    int16_t M5821;     // race ID
    int16_t M4EBF;     // alignment (int16)
    int16_t M5829;     // sex
    float   M5577;     // daysOld (float)
    int16_t M5830;     // currentSP
    int16_t M5470;     // level (int16)
    int16_t M583F;     // currentX
    int16_t M5845;     // currentY
    float   M4E97;     // atk (float)
    float   M4EA1;     // def (float)

    // Stats arrays
    int16_t stats[7];       // base stats (7×int16)
    int16_t modStats[7];    // modified stats (7×int16)

    // Inventory and bank (41 slots each)
    HeldItem M5857[41];    // Inventory
    HeldItem M5860[41];    // Bank

    // Equipped items and misc
    int16_t M586D[36];     // equipped items (36×int16)
    int16_t M56F7;         // currentHP
    float   M587C;         //
    float   M588A;         //
    int16_t M5899;         //
    int16_t M58A6;         //
    int64_t M579B;         // goldInBank (currency)
    int64_t M4F2D;         // totalExperience (currency)
    int16_t M4EAC;         //
    int16_t M4EB4;         //
    int64_t M58AF;         // (currency)
    int16_t M56AF;         //
    int16_t M58BB;         //

    // Guild status (16 entries)
    GuildStatus M5584[16];

    // Companions (5 entries)
    Companion M58CB[5];

    // Misc fields
    int16_t M5700;         //
    int16_t M58DC;         //
    int16_t M58E9;         //
    QString M58F9;         //
    int64_t M5906[9];      // kills, deaths, comp kills, quests, play time, creation date, +3
    int16_t M5914[6];      // character options
    int16_t M56B9[8];      // status effects
    int16_t resistances[12]; // ResFire..ResSpecial
    int32_t M591F;         //
    int16_t M592F[12];     // temp resistances
    int16_t M5483;         //
    int16_t M5943;         //
    QString M594C;         //
    T57EF   M5958[21];     // saved window states
    int16_t M5962;         //
    int32_t M5970;         //
    int32_t M5977;         //
    int16_t M5985[12];     // resists from items
    int16_t M5997[2];      // items in each hand
    int16_t M59A1;         //
    int16_t M59B0[11];     // buffer slots
    int16_t M59BB;         //
    int16_t M59CB;         //
    int16_t M59D9;         //
    T4E6B   M59E7;          //
    int16_t M59F4[3];      //

    // --- Serialization (runtime fields) ---
    QVariantMap toMap() const;
    void loadFromMap(const QVariantMap &map);

    // --- Logic ---
    void addExperience(int amount);
    void setDead();
    void resurrect();
    void addStatus(uint flag);
    void removeStatus(uint flag);
};

// --- Party ---

struct Party {
    QList<Character> members;
    int sharedGold = 0;

    QVariantMap toMap() const;
    void loadFromMap(const QVariantMap& map);
};

namespace StatusFlag {
    constexpr uint None = 0;
    constexpr uint Dead = 1;
}

#endif // CHARACTER_H