#ifndef CHARACTER_H
#define CHARACTER_H

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDateTime>

struct Character {
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
    QStringList inventory;

    // --- Serialization: TO MAP ---
    // Use this for saving to JSON or passing data to QML/UI
    QVariantMap toMap() const {
        QVariantMap map;
        map["Name"]         = name;
        map["Race"]         = Race;
        map["Age"]          = Age;
        map["Level"]        = level;
        map["Experience"]   = experience;
        map["HP"]           = hp;
        map["MaxHP"]        = maxHp;
        map["Gold"]         = Gold;
        
        map["Strength"]     = strength;
        map["Intelligence"] = intelligence;
        map["Wisdom"]       = wisdom;
        map["Constitution"] = constitution;
        map["Charisma"]     = charisma;
        map["Dexterity"]    = dexterity;

        map["Mana"]         = mana;
        map["MaxMana"]      = maxMana;
        
        map["Poisoned"]     = poisoned;
        map["Blinded"]      = blinded;
        map["Diseased"]     = diseased;
        map["isAlive"]      = isAlive;
        
        map["DungeonLevel"] = DungeonLevel;
        map["DungeonX"]     = DungeonX;
        map["DungeonY"]     = DungeonY;
        
        map["Inventory"]    = inventory;
        return map;
    }

    // --- Serialization: FROM MAP ---
    // Use this when loading from a save file or registry
    void loadFromMap(const QVariantMap &map) {
        name         = map.value("Name").toString();
        Race         = map.value("Race").toString();
        Age          = map.value("Age", 18).toInt();
        level        = map.value("Level", 1).toInt();
        experience   = map.value("Experience", 0).toInt();
        hp           = map.value("HP").toInt();
        maxHp        = map.value("MaxHP").toInt();
        Gold         = map.value("Gold").toInt();
        
        strength     = map.value("Strength").toInt();
        intelligence = map.value("Intelligence").toInt();
        wisdom       = map.value("Wisdom").toInt();
        constitution = map.value("Constitution").toInt();
        charisma     = map.value("Charisma").toInt();
        dexterity    = map.value("Dexterity").toInt();

        mana         = map.value("Mana", 0).toInt();
        maxMana      = map.value("MaxMana", 0).toInt();
        
        poisoned     = map.value("Poisoned", false).toBool();
        blinded      = map.value("Blinded", false).toBool();
        diseased     = map.value("Diseased", false).toBool();
        isAlive      = map.value("isAlive", true).toBool();
        
        DungeonLevel = map.value("DungeonLevel", 0).toInt();
        DungeonX     = map.value("DungeonX", 0).toInt();
        DungeonY     = map.value("DungeonY", 0).toInt();
        
        inventory    = map.value("Inventory").toStringList();
    }
};

#endif // CHARACTER_H
