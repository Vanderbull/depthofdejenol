#include "character.h"

// --- Character Implementation ---

QVariantMap Character::toMap() const {
    QVariantMap map;
    map["Name"]         = name;
    map["Race"]         = race;
    map["Age"]          = age;
    map["Level"]        = level;
    map["Experience"]   = experience;
    map["HP"]           = hp;
    map["MaxHP"]        = maxHp;
    map["Gold"]         = gold;
    
    map["Strength"]     = strength;
    map["Intelligence"] = intelligence;
    map["Wisdom"]       = wisdom;
    map["Constitution"] = constitution;
    map["Charisma"]     = charisma;
    map["Dexterity"]    = dexterity;

    map["Mana"]         = mana;
    map["MaxMana"]      = maxMana;
    
    map["StatusFlags"] = statusFlags;
//    map["Poisoned"]     = poisoned;
//    map["Blinded"]      = blinded;
//    map["Diseased"]     = diseased;
//    map["isAlive"]      = isAlive;
    
    map["DungeonLevel"] = dungeonLevel;
    map["DungeonX"]     = dungeonX;
    map["DungeonY"]     = dungeonY;
    
    map["Inventory"]    = inventory;
    map["row"]          = row;
    return map;
}

void Character::loadFromMap(const QVariantMap &map) {
    name         = map.value("Name").toString();
    race         = map.value("Race").toString();
    age          = map.value("Age", 18).toInt();
    level        = map.value("Level", 1).toInt();
    experience   = map.value("Experience", 0).toInt();
    hp           = map.value("HP").toInt();
    maxHp        = map.value("MaxHP").toInt();
    gold         = map.value("Gold").toInt();
    
    strength     = map.value("Strength").toInt();
    intelligence = map.value("Intelligence").toInt();
    wisdom       = map.value("Wisdom").toInt();
    constitution = map.value("Constitution").toInt();
    charisma     = map.value("Charisma").toInt();
    dexterity    = map.value("Dexterity").toInt();

    mana         = map.value("Mana", 0).toInt();
    maxMana      = map.value("MaxMana", 0).toInt();
    
    statusFlags  = map.value("StatusFlags", StatusFlag::None).toUInt();
//    poisoned     = map.value("Poisoned", false).toBool();
//    blinded      = map.value("Blinded", false).toBool();
//    diseased     = map.value("Diseased", false).toBool();
//    isAlive      = map.value("isAlive", true).toBool();
    
    dungeonLevel = map.value("DungeonLevel", 0).toInt();
    dungeonX     = map.value("DungeonX", 0).toInt();
    dungeonY     = map.value("DungeonY", 0).toInt();

    row          = map.value("row", 0).toInt();
    inventory    = map.value("Inventory").toStringList();
}

void Character::addStatus(uint flag) { statusFlags |= flag; }
void Character::removeStatus(uint flag) { statusFlags &= ~flag; }

void Character::setDead() {
    addStatus(StatusFlag::Dead);
    hp = 0;
    isAlive = false;
    dungeonLevel = 0; 
}

void Character::resurrect() {
    removeStatus(StatusFlag::Dead);
    isAlive = true;
    if (hp <= 0) hp = 1;
}

void Character::addExperience(int amount) {
    if (amount <= 0 || !isAlive) return;

    experience += amount;

    // Basic level-up logic: Level * 1000 threshold
    // Example: Level 1 needs 1000 XP to hit Level 2
    int nextLevelThreshold = level * 1000;

    while (experience >= nextLevelThreshold) {
        experience -= nextLevelThreshold;
        level++;
        
        // Boost stats on level up
        maxHp += 5;
        hp = maxHp; // Heal on level up
        maxMana += 2;
        mana = maxMana;
        
        // Recalculate next threshold for the new level
        nextLevelThreshold = level * 1000;
    }
}

// --- Party Implementation ---

QVariantMap Party::toMap() const {
    QVariantMap map;
    QVariantList charList;

    for (int i = 0; i < members.size(); ++i) {
        charList.append(members.at(i).toMap());
    }
    
    map["Members"] = charList;
    map["SharedGold"] = sharedGold;
    return map;
}

void Party::loadFromMap(const QVariantMap &map) {
    sharedGold = map.value("SharedGold", 0).toInt();
    QVariantList charList = map.value("Members").toList();
    
    members.clear();
    for (int i = 0; i < charList.size(); ++i) {
        Character c;
        c.loadFromMap(charList.at(i).toMap());
        members.append(c);
    }
}
