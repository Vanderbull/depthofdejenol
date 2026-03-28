#ifndef CHARACTER_H
#define CHARACTER_H

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDateTime>

#include "src/core/GameConstants.h"

struct Character {

    GameConstants::EntityStatuses status = GameConstants::Normal;
    int row = 0; // 0 for Front, 1 for Back (Default to front)

    // --- Basic Info ---
    QString name;
    QString Race;
    int Age = 18;
    int level = 1;
    int experience = 0;
    int hp = 10;
    int maxHp = 10;
    int Gold = 0;

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
    bool poisoned = false;
    bool blinded = false;
    bool diseased = false;
    bool isAlive = true;
    
    // --- Location Data ---
    int DungeonLevel = 0;
    int DungeonX = 0;
    int DungeonY = 0;

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
