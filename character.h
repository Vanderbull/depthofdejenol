#ifndef CHARACTER_H
#define CHARACTER_H

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDateTime>

#include "src/core/GameConstants.h"

// --- Status Flags (Bitmask) ---
enum StatusFlag {
    None     = 0,
    Poisoned = 1 << 0, // 1
    Blinded  = 1 << 1, // 2
    Diseased = 1 << 2, // 4
    Dead     = 1 << 3  // 8
};

struct Character {

    GameConstants::EntityStatuses status = GameConstants::Normal;
    int row = 0; // 0 for Front, 1 for Back (Default to front)

    // --- Basic Info ---
    QString name;
    QString race;
    int age = 18;
    int level = 1;
    int experience = 0;
    int hp = 10;
    int maxHp = 10;
    int gold = 0;

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

    uint statusFlags = StatusFlag::None;

    bool isAlive() const {
        return !(statusFlags & StatusFlag::Dead);
    }

    // Helper methods for cleaner logic
    bool hasStatus(StatusFlag flag) const { return statusFlags & flag; }
    void addStatus(StatusFlag flag) { statusFlags |= flag; }
    void removeStatus(StatusFlag flag) { statusFlags &= ~flag; }
    void clearAllStatuses() { statusFlags = StatusFlag::None; }
    void setDead();
    void resurrect();

    // --- Location Data ---
    int dungeonLevel = 0;
    int dungeonX = 0;
    int dungeonY = 0;

    // --- Inventory ---
    //QStringList inventory;
    QStringList inventory = {"Hands"};

    // --- Serialization: TO MAP ---
    // Use this for saving to JSON or passing data to QML/UI
    QVariantMap toMap() const;

    // --- Serialization: FROM MAP ---
    // Use this when loading from a save file or registry
    void loadFromMap(const QVariantMap &map);
};

struct Party {
    QList<Character> members;
    int sharedGold = 0;

    QVariantMap toMap() const;

    void loadFromMap(const QVariantMap &map);
};

#endif // CHARACTER_H
