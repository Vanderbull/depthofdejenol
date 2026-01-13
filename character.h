#ifndef CHARACTER_H
#define CHARACTER_H

#include <QString>
#include <QStringList>
#include <QVariantMap>

struct Character {
    // Basic Info
    QString name;
    int level;
    int experience;
    int hp;
    int maxHp;
    int gold;

    // Stats
    int strength;
    int intelligence;
    int wisdom;
    int constitution;
    int charisma;
    int dexterity;

    // Status Effects
    bool poisoned;
    bool blinded;
    bool diseased;
    bool isAlive;

    // Items
    QStringList inventory;

    // Helper function to map data safely
    void loadFromMap(const QVariantMap &map) {
        name         = map.value("Name").toString();
        level        = map.value("Level").toInt();
        experience   = map.value("Experience").toInt();
        hp           = map.value("HP").toInt();
        maxHp        = map.value("MaxHP").toInt();
        gold         = map.value("Gold").toInt();
        
        strength     = map.value("Strength").toInt();
        intelligence = map.value("Intelligence").toInt();
        wisdom       = map.value("Wisdom").toInt();
        constitution = map.value("Constitution").toInt();
        charisma     = map.value("Charisma").toInt();
        dexterity    = map.value("Dexterity").toInt();
        
        // Using toBool() or comparing to 1 for status
        poisoned     = map.value("Poisoned").toBool();
        blinded      = map.value("Blinded").toBool();
        diseased     = map.value("Diseased").toBool();
        isAlive      = map.value("isAlive").toBool();
        
        inventory    = map.value("Inventory").toStringList();
    }
};

#endif
