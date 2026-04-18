#ifndef CHARACTER_H
#define CHARACTER_H

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDateTime>

#include "src/core/GameConstants.h"

struct Character {

    GameConstants::EntityStatuses status = GameConstants::Normal;
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

    // --- Status Effects ---
    uint statusFlags = 0;
    bool isAlive = true;

    // --- Location Data ---
    int dungeonLevel = 0;
    int dungeonX = 0;
    int dungeonY = 0;
    int row = 0;

    // --- Inventory ---
    //QStringList inventory;
    QStringList inventory = {"Hands"};

    QVariantMap toMap() const;
    void loadFromMap(const QVariantMap &map);

    // --- Logic ---
    void addExperience(int amount);
    void setDead();
    void resurrect();

    void addStatus(uint flag);
    void removeStatus(uint flag);
};

struct Party {
    QList<Character> members;
    int sharedGold = 0;

    QVariantMap toMap() const;
    void loadFromMap(const QVariantMap &map);
};

namespace StatusFlag {
    const uint None = 0;
    const uint Dead = 1;
}

#endif // CHARACTER_H
