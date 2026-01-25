#ifndef CHARACTER_H
#define CHARACTER_H

#include <QString>
#include <QStringList>
#include <QVariantMap>

struct Character {
    // Basic Info
    QString name;
    QString Race;
    int Age;
    int level;
    int experience;
    int hp;
    int maxHp;
    int Gold;

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
    
    // Location
    int DungeonLevel;
    int DungeonX;
    int DungeonY;

    // Items
    QStringList inventory;

    // Helper function to map data safely
    void loadFromMap(const QVariantMap &map) {
        name         = map.value("Name").toString();
        level        = map.value("Level").toInt();
        experience   = map.value("Experience").toInt();
        hp           = map.value("HP").toInt();
        maxHp        = map.value("MaxHP").toInt();
        Gold         = map.value("CurrentCharacterGold").toInt();
        
        strength     = map.value("Strength").toInt();
        intelligence = map.value("Intelligence").toInt();
        wisdom       = map.value("Wisdom").toInt();
        constitution = map.value("Constitution").toInt();
        charisma     = map.value("Charisma").toInt();
        dexterity    = map.value("Dexterity").toInt();
        
        DungeonLevel = map.value("DungeonLevel").toInt();
        DungeonX     = map.value("DungeonX").toInt();
        DungeonY     = map.value("DungeonY").toInt();
        
        poisoned     = map.value("Poisoned").toBool();
        blinded      = map.value("Blinded").toBool();
        diseased     = map.value("Diseased").toBool();
        isAlive      = map.value("isAlive").toBool();
        
        inventory    = map.value("Inventory").toStringList();
        Race         = map.value("Race").toString(); // Sync from map
        Age          = map.value("Age", 16).toInt();   // Default starting age
    }
};

#endif // CHARACTER_H
