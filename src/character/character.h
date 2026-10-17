#ifndef CHARACTER_H
#define CHARACTER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariantMap>
#include <QDateTime>
#include <cstdint>

#include "src/core/GameConstants.h"

// ItemDef lives in ItemDatabase.h; only a forward declaration is needed here
// so character.h does not pull in the whole item table.
struct ItemDef;

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
    bool identified = true;  // false = unidentified (shows as "Unknown X")
    QString name;            // cached display name (resolved from ItemDatabase)
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

    // v3.0.0 slice 3.4: unspent stat points. Level-ups add to this pool and the
    // player spends it on the character sheet, so a build is a choice rather
    // than a random roll.
    int statPoints = 0;

    // v3.0.0 slice 3.6: total play time in seconds. Persisted so the Hall of
    // Records can sort by it.
    int playTimeSeconds = 0;

    // --- Resource Pools ---
    int mana = 50;
    int maxMana = 50;
    int hunger = 100;       // 0 = starving, 100 = full
    int maxHunger = 100;

    // --- Lifecycle (v5.3.0 slice 5.2) ---
    QString spouse;         // name of married partner ("" = unmarried)
    QStringList children;   // names of children

    // --- Status Effects ---
    uint statusFlags = 0;
    bool isAlive = true;

    // --- Location Data ---
    int dungeonLevel = 0;
    int dungeonX = 0;
    int dungeonY = 0;
    int row = 0;

    // --- Guild progression ---
    // Maps guild name -> guild level. A character can belong to several guilds;
    // total power is the sum of all guild levels.
    QMap<QString, int> guildLevels;
    QMap<QString, qint64> guildExperience;  // XP earned within each guild

    // --- Inventory ---
    QList<HeldItem> inventory;
    QList<HeldItem> bankInventory;
    QList<HeldItem> equipped;  // 12 slots, index = ItemSlot::Slot

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

    // --- Equip / Unequip ---
    // Returns true on success. On failure, `reason` explains why.
    bool equipItem(int inventoryIndex, QString& reason);
    bool unequipItem(int slotIndex, QString& reason);

    // --- Effective stats (base + equipped modifiers) ---
    int effectiveStrength() const;
    int effectiveIntelligence() const;
    int effectiveWisdom() const;
    int effectiveConstitution() const;
    int effectiveCharisma() const;
    int effectiveDexterity() const;
    int effectiveStat(const QString& statName) const;

    // --- Stat allocation (v3.0.0 slice 3.4) ---
    // Spend one unspent point on a stat. Refuses when the pool is empty, the
    // stat name is unknown, or the stat is already at `cap`. Returns true when
    // a point was spent.
    bool spendStatPoint(const QString& statName, int cap);

    // The race-imposed cap for a stat by display name (e.g. "Strength").
    // Returns 0 when the race or stat is unknown, meaning "no cap known".
    int raceCapForStat(const QString& statName) const;
    // The base value of a stat by name (0 when unknown). Not the effective
    // value: spending a point must raise the base, not the equipment bonus.
    int baseStat(const QString& statName) const;

    // --- Consumables ---
    // Use a consumable item. Returns true on success.
    bool useConsumable(int inventoryIndex, QString& effectDescription);

    // Apply a consumable's own stat modifiers (Potion of Might and friends).
    // Returns a human-readable summary of what changed.
    QString applyConsumableStatBoost(const ItemDef& def);

    // Can this inventory slot be used as a consumable? Potions (23), scrolls
    // (24) and tomes (25) are the usable types; the same rule useConsumable()
    // enforces, exposed so the combat item picker can list exactly those.
    static bool isConsumable(const HeldItem& item);

    // --- Lifecycle (v5.3.0 slice 5.2) ---
    // Marriage eligibility: must be 18 or older and not already married.
    bool isEligibleForMarriage() const {
        return age >= 18 && spouse.isEmpty();
    }
    // Marry a partner. Returns false if already married or too young.
    bool marry(const QString& partnerName) {
        if (!isEligibleForMarriage()) return false;
        spouse = partnerName;
        return true;
    }
    // Add a child by name. Rejects duplicates.
    bool addChild(const QString& childName) {
        if (children.contains(childName)) return false;
        children.append(childName);
        return true;
    }
    // Retirement eligibility: 70 or older.
    bool canRetire() const {
        return age >= 70;
    }

    // Inherit from an ancestor: gain a fraction of their stats and gold.
    // Stats take the max of current and half the ancestor's value; gold
    // receives a 10% inheritance bonus.
    void inheritFrom(const Character& ancestor) {
        strength     = qMax(strength, ancestor.strength / 2);
        intelligence = qMax(intelligence, ancestor.intelligence / 2);
        wisdom       = qMax(wisdom, ancestor.wisdom / 2);
        constitution = qMax(constitution, ancestor.constitution / 2);
        charisma     = qMax(charisma, ancestor.charisma / 2);
        dexterity    = qMax(dexterity, ancestor.dexterity / 2);
        gold        += qMax(1, ancestor.gold / 10);
    }

    // --- Food / hunger ---
    // The item type that is real food. It is deliberately NOT 22 (which is
    // Amulets) — eating jewellery was the old bug. See ItemDef::typeName().
    static const int FOOD_TYPE = 35;
    // True when this inventory slot holds food that can be eaten.
    static bool isFood(const HeldItem& item);
    // Inventory indices of every food item in `inventory`, so the eat picker
    // offers exactly what eatFood() accepts.
    static QList<int> foodIndices(const QList<HeldItem>& inventory);
    // True when the inventory holds at least one food item.
    static bool hasFood(const QList<HeldItem>& inventory);

    // Decrease hunger by `amount` (clamped to 0). Starvation damage is
    // applied separately by the caller so it can route through combat state.
    void consumeHunger(int amount);
    // Restore hunger by `amount` (clamped to maxHunger).
    void restoreHunger(int amount);
    // True when hunger has hit zero and the character is taking starvation
    // damage each turn.
    bool isStarving() const { return hunger <= 0; }
    // Hunger as a percentage of the maximum (0 when maxHunger is 0).
    int hungerPercent() const {
        return (maxHunger > 0) ? (hunger * 100 / maxHunger) : 0;
    }
    // True when hunger is running low but has not yet hit zero. This is the
    // band the UI warns on, so the player sees the danger before starving.
    static const int HUNGER_LOW_PERCENT = 20;
    bool isHungerLow() const {
        return !isStarving() && hungerPercent() <= HUNGER_LOW_PERCENT;
    }
    // Eat a food item from the inventory. Returns true on success.
    bool eatFood(int inventoryIndex, QString& effectDescription);

    // --- Guild progression ---
    // Get the level in a specific guild (0 if not a member).
    int guildLevel(const QString& guildName) const;
    // Join a guild (starts at level 1).
    void joinGuild(const QString& guildName);
    // Increment the level in a guild. Returns the new level.
    int incrementGuildLevel(const QString& guildName);
    // Total power = sum of all guild levels.
    int totalGuildLevels() const;
    // XP required to reach the next guild level.
    qint64 guildXpToNextLevel(const QString& guildName) const;
    // Add XP to a guild. Returns true if a level was gained.
    bool addGuildExperience(const QString& guildName, qint64 amount);
};

// --- Party ---

struct Party {
    QList<Character> members;
    int sharedGold = 0;

    QVariantMap toMap() const;
    void loadFromMap(const QVariantMap& map);
};

// Status bits live in one namespace so they cannot collide: Character has a
// single statusFlags field, and GameConstants::EntityStatus already defines
// Poisoned/Blinded/OnFire/Dead for it.
namespace StatusFlag {
    constexpr uint None     = 0;
    constexpr uint Poisoned = GameConstants::Poisoned;
    constexpr uint Blinded  = GameConstants::Blinded;
    constexpr uint OnFire   = GameConstants::OnFire;
    constexpr uint Dead     = GameConstants::Dead;
    constexpr uint Snared   = GameConstants::Snared;
}

#endif // CHARACTER_H