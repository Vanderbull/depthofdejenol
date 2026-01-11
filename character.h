
// Character.h
#ifndef CHARACTER_H
#define CHARACTER_H

#include <QString>
#include <QVariantMap>

struct Character {
    QVariantMap map;
    Character()
    {
        map["name"] = "NoName";
        map["level"] = 0;
        map["experience"] = 0;
        map["hp"] = 0;
        map["maxhp"] = 0;
        map["gold"] = 1500;
        map["Strength"] = 0;
        map["Intelligence"] = 0;
        map["Wisdom"] = 0;
        map["Constitution"] = 0;
        map["Charisma"] = 0;
        map["Dexterity"] = 0;
        map["poisoned"] = 0;
        map["blinded"] = 0;
        map["diseased"] = 0;
        map["isAlive"] = 0;
        map["inventory"] = QStringList();
    }
};

#endif
