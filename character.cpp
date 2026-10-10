#include "character.h"
#include "src/items/ItemDatabase.h"
#include "src/core/LevelTable.h"

// --- Character Implementation ---

// Serialize a HeldItem to a QVariantMap.
static QVariantMap heldItemToMap(const HeldItem& item) {
    QVariantMap m;
    m["id"]         = item.M4E97;
    m["name"]       = item.name;
    m["charges"]    = item.M4ED4;
    m["equipped"]   = item.M5467;
    m["identified"] = item.identified;
    return m;
}

// Deserialize a HeldItem from a QVariantMap.
static HeldItem heldItemFromMap(const QVariantMap& m) {
    HeldItem item;
    item.M4E97      = static_cast<int16_t>(m.value("id", 0).toInt());
    item.name       = m.value("name").toString();
    item.M4ED4      = static_cast<int16_t>(m.value("charges", 0).toInt());
    item.M5467      = static_cast<int16_t>(m.value("equipped", 0).toInt());
    item.identified = m.value("identified", true).toBool();
    return item;
}

// Convert a QStringList of item names (old save format) to QList<HeldItem>.
static QList<HeldItem> inventoryFromNames(const QStringList& names) {
    QList<HeldItem> result;
    for (const QString& name : names) {
        HeldItem item;
        item.name = name;
        if (const ItemDef* def = ItemDatabase::instance().byName(name)) {
            item.M4E97 = static_cast<int16_t>(def->id);
        }
        result.append(item);
    }
    return result;
}

// Load inventory from a QVariant that may be:
//   - QStringList (old save format: list of item names)
//   - QVariantList of strings (same, but stored as QVariantList)
//   - QVariantList of maps (new save format: serialized HeldItem maps)
static QList<HeldItem> loadInventoryFromVariant(const QVariant& var) {
    if (var.typeId() == QMetaType::QStringList) {
        return inventoryFromNames(var.toStringList());
    }
    if (var.typeId() == QMetaType::QVariantList) {
        QVariantList list = var.toList();
        if (list.isEmpty()) return {};
        // Check the first element: if it's a string, treat as old format.
        if (list.first().typeId() == QMetaType::QString) {
            QStringList names;
            for (const QVariant& v : list) names << v.toString();
            return inventoryFromNames(names);
        }
        // New format: list of maps.
        QList<HeldItem> result;
        for (const QVariant& v : list) {
            result.append(heldItemFromMap(v.toMap()));
        }
        return result;
    }
    return {};
}

QVariantMap Character::toMap() const {
    QVariantMap map;
    map["Age"]          = age;
    map["Alignment"]    = "Neutral";
    map["Name"]         = name;
    map["Race"]         = race;
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
    map["Hunger"]       = hunger;
    map["MaxHunger"]    = maxHunger;
    map["StatusFlags"] = statusFlags;
    map["isAlive"]      = isAlive;
    map["DungeonLevel"] = dungeonLevel;
    map["DungeonX"]     = dungeonX;
    map["DungeonY"]     = dungeonY;
    map["row"]          = row;

    // Serialize guild levels.
    QVariantMap guildMap;
    for (auto it = guildLevels.constBegin(); it != guildLevels.constEnd(); ++it) {
        guildMap[it.key()] = it.value();
    }
    map["GuildLevels"] = guildMap;

    QVariantMap guildXpMap;
    for (auto it = guildExperience.constBegin(); it != guildExperience.constEnd(); ++it) {
        guildXpMap[it.key()] = QVariant::fromValue(it.value());
    }
    map["GuildExperience"] = guildXpMap;

    // Serialize inventory as a list of maps.
    QVariantList invList;
    for (const HeldItem& item : inventory) {
        invList.append(heldItemToMap(item));
    }
    map["Inventory"] = invList;

    // Serialize bank inventory.
    QVariantList bankList;
    for (const HeldItem& item : bankInventory) {
        bankList.append(heldItemToMap(item));
    }
    map["BankInventory"] = bankList;

    // Serialize equipped items.
    QVariantList eqList;
    for (const HeldItem& item : equipped) {
        eqList.append(heldItemToMap(item));
    }
    map["Equipped"] = eqList;

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

    // Default full so saves written before hunger existed do not start starving.
    hunger       = map.value("Hunger", 100).toInt();
    maxHunger    = map.value("MaxHunger", 100).toInt();

    statusFlags  = map.value("StatusFlags", StatusFlag::None).toUInt();
    // Default true so saves written before this field existed still load as alive.
    isAlive      = map.value("isAlive", true).toBool();
    
    dungeonLevel = map.value("DungeonLevel", 0).toInt();
    dungeonX     = map.value("DungeonX", 0).toInt();
    dungeonY     = map.value("DungeonY", 0).toInt();

    row          = map.value("row", 0).toInt();

    // Load guild levels.
    guildLevels.clear();
    QVariantMap guildMap = map.value("GuildLevels").toMap();
    for (auto it = guildMap.constBegin(); it != guildMap.constEnd(); ++it) {
        guildLevels[it.key()] = it.value().toInt();
    }

    guildExperience.clear();
    QVariantMap guildXpMap = map.value("GuildExperience").toMap();
    for (auto it = guildXpMap.constBegin(); it != guildXpMap.constEnd(); ++it) {
        guildExperience[it.key()] = it.value().toLongLong();
    }

    // Load inventory — handle old (QStringList or QVariantList of strings) and
    // new (QVariantList of maps) formats.
    inventory = loadInventoryFromVariant(map.value("Inventory"));
    bankInventory = loadInventoryFromVariant(map.value("BankInventory"));

    // Load equipped items.
    equipped.clear();
    QVariant eqVar = map.value("Equipped");
    if (eqVar.typeId() == QMetaType::QVariantList) {
        for (const QVariant& v : eqVar.toList()) {
            equipped.append(heldItemFromMap(v.toMap()));
        }
    }
}

void Character::addStatus(uint flag) { statusFlags |= flag; }
void Character::removeStatus(uint flag) { statusFlags &= ~flag; }

// --- Equip / Unequip ---

bool Character::equipItem(int inventoryIndex, QString& reason) {
    if (inventoryIndex < 0 || inventoryIndex >= inventory.size()) {
        reason = "Invalid inventory index.";
        return false;
    }

    HeldItem item = inventory[inventoryIndex];
    const ItemDef* def = ItemDatabase::instance().byName(item.name);
    if (!def) {
        reason = QString("Unknown item: %1").arg(item.name);
        return false;
    }

    if (!def->equippable()) {
        reason = QString("%1 cannot be equipped.").arg(item.name);
        return false;
    }

    // Check stat requirements.
    struct { const char* name; int req; int stat; } checks[] = {
        {"Strength",     def->strReq, strength},
        {"Intelligence", def->intReq, intelligence},
        {"Wisdom",       def->wisReq, wisdom},
        {"Constitution", def->conReq, constitution},
        {"Charisma",     def->chaReq, charisma},
        {"Dexterity",    def->dexReq, dexterity},
    };
    for (const auto& c : checks) {
        if (c.req > c.stat) {
            reason = QString("Requires %1 %2 (you have %3).")
                         .arg(c.req).arg(c.name).arg(c.stat);
            return false;
        }
    }

    // Check guild restrictions: each bit in def->guilds maps to a guild in
    // GameConstants::GUILD_NAMES. The character must be a member of at least
    // one of the required guilds.
    if (def->guilds != 0) {
        const QStringList guildNames = GameConstants::GUILD_NAMES;
        bool hasRequiredGuild = false;
        for (int bit = 0; bit < guildNames.size(); ++bit) {
            if (def->guilds & (1 << bit)) {
                const QString& guildName = guildNames.at(bit);
                if (guildLevel(guildName) > 0) {
                    hasRequiredGuild = true;
                    break;
                }
            }
        }
        if (!hasRequiredGuild) {
            reason = QString("%1 requires membership in a specific guild.")
                         .arg(item.name);
            return false;
        }
    }

    // Check nHands: two-handed weapons need both MainHand and OffHand free.
    ItemSlot::Slot slot = def->slot();
    if (def->nHands == 2) {
        // Check if both hands are free.
        bool mainHandFree = true;
        bool offHandFree = true;
        for (const HeldItem& eq : equipped) {
            if (eq.M4E97 == 0) continue; // empty slot
            const ItemDef* eqDef = ItemDatabase::instance().byName(eq.name);
            if (!eqDef) continue;
            if (eqDef->slot() == ItemSlot::MainHand) mainHandFree = false;
            if (eqDef->slot() == ItemSlot::OffHand) offHandFree = false;
        }
        if (!mainHandFree || !offHandFree) {
            reason = "Two-handed weapon requires both hands free.";
            return false;
        }
    } else {
        // Check if the target slot is already occupied.
        for (const HeldItem& eq : equipped) {
            if (eq.M4E97 == 0) continue;
            const ItemDef* eqDef = ItemDatabase::instance().byName(eq.name);
            if (!eqDef) continue;
            if (eqDef->slot() == slot) {
                reason = QString("Slot already occupied by %1.").arg(eq.name);
                return false;
            }
        }
    }

    // Equip: add to equipped list, remove from inventory.
    equipped.append(item);
    inventory.removeAt(inventoryIndex);
    return true;
}

bool Character::unequipItem(int slotIndex, QString& reason) {
    if (slotIndex < 0 || slotIndex >= equipped.size()) {
        reason = "Invalid slot index.";
        return false;
    }

    HeldItem item = equipped[slotIndex];
    if (item.M4E97 == 0) {
        reason = "Slot is empty.";
        return false;
    }

    // Cursed items cannot be unequipped.
    if (const ItemDef* def = ItemDatabase::instance().byName(item.name)) {
        if (def->cursed) {
            reason = QString("%1 is cursed and cannot be removed. Uncurse it first.").arg(item.name);
            return false;
        }
    }

    // Move back to inventory.
    inventory.append(item);
    equipped.removeAt(slotIndex);
    return true;
}

// --- Effective stats (base + equipped modifiers) ---

int Character::effectiveStrength() const {
    int total = strength;
    for (const HeldItem& item : equipped) {
        if (const ItemDef* def = ItemDatabase::instance().byName(item.name)) {
            total += def->strMod;
        }
    }
    return total;
}

int Character::effectiveIntelligence() const {
    int total = intelligence;
    for (const HeldItem& item : equipped) {
        if (const ItemDef* def = ItemDatabase::instance().byName(item.name)) {
            total += def->intMod;
        }
    }
    return total;
}

int Character::effectiveWisdom() const {
    int total = wisdom;
    for (const HeldItem& item : equipped) {
        if (const ItemDef* def = ItemDatabase::instance().byName(item.name)) {
            total += def->wisMod;
        }
    }
    return total;
}

int Character::effectiveConstitution() const {
    int total = constitution;
    for (const HeldItem& item : equipped) {
        if (const ItemDef* def = ItemDatabase::instance().byName(item.name)) {
            total += def->conMod;
        }
    }
    return total;
}

int Character::effectiveCharisma() const {
    int total = charisma;
    for (const HeldItem& item : equipped) {
        if (const ItemDef* def = ItemDatabase::instance().byName(item.name)) {
            total += def->chaMod;
        }
    }
    return total;
}

int Character::effectiveDexterity() const {
    int total = dexterity;
    for (const HeldItem& item : equipped) {
        if (const ItemDef* def = ItemDatabase::instance().byName(item.name)) {
            total += def->dexMod;
        }
    }
    return total;
}

int Character::effectiveStat(const QString& statName) const {
    const QString s = statName.trimmed().toLower();
    if (s == "strength")     return effectiveStrength();
    if (s == "intelligence") return effectiveIntelligence();
    if (s == "wisdom")       return effectiveWisdom();
    if (s == "constitution") return effectiveConstitution();
    if (s == "charisma")     return effectiveCharisma();
    if (s == "dexterity")    return effectiveDexterity();
    return 0;
}

// --- Consumables ---

bool Character::useConsumable(int inventoryIndex, QString& effectDescription) {
    if (inventoryIndex < 0 || inventoryIndex >= inventory.size()) {
        effectDescription = "Invalid inventory index.";
        return false;
    }

    HeldItem item = inventory[inventoryIndex];
    const ItemDef* def = ItemDatabase::instance().byName(item.name);
    if (!def) {
        effectDescription = QString("Unknown item: %1").arg(item.name);
        return false;
    }

    // Only potions (23), scrolls (24), and tomes (25) are consumable.
    if (def->type != 23 && def->type != 24 && def->type != 25) {
        effectDescription = QString("%1 cannot be used.").arg(item.name);
        return false;
    }

    // Apply effect based on spellIndex / spellID.
    // For now, healing potions restore HP, mana potions restore mana.
    // This is a simplified system — full spell resolution comes in Phase 2.
    QString nameLower = item.name.toLower();
    if (nameLower.contains("healing") || nameLower.contains("health")) {
        int healAmount = 10 + (def->spellLvl * 5);
        hp = qMin(maxHp, hp + healAmount);
        effectDescription = QString("Restored %1 HP.").arg(healAmount);
    } else if (nameLower.contains("mana")) {
        int manaAmount = 10 + (def->spellLvl * 5);
        mana = qMin(maxMana, mana + manaAmount);
        effectDescription = QString("Restored %1 mana.").arg(manaAmount);
    } else if (nameLower.contains("strength")) {
        // Temporary buff — for now just show a message.
        // Full buff system comes in Phase 2.
        effectDescription = "You feel stronger! (Buff not yet implemented)";
    } else if (nameLower.contains("intelligence")) {
        effectDescription = "Your mind sharpens! (Buff not yet implemented)";
    } else if (nameLower.contains("cure") || nameLower.contains("poison")) {
        // Cure poison. The flag is StatusFlag::Poisoned (1 << 0); the old
        // literal 0x02 was Blinded, so this used to cure the wrong status.
        if (statusFlags & StatusFlag::Poisoned) {
            statusFlags &= ~StatusFlag::Poisoned;
            effectDescription = "Poison cured!";
        } else {
            effectDescription = "No poison to cure.";
        }
    } else {
        effectDescription = QString("Used %1. (Effect not yet implemented)").arg(item.name);
    }

    // Decrement charges.
    item.M4ED4--;
    if (item.M4ED4 <= 0) {
        inventory.removeAt(inventoryIndex);
        effectDescription += " The item is consumed.";
    } else {
        inventory[inventoryIndex] = item;
    }

    return true;
}

// --- Food / hunger ---

void Character::consumeHunger(int amount) {
    if (amount <= 0) return;
    hunger = qMax(0, hunger - amount);
}

void Character::restoreHunger(int amount) {
    if (amount <= 0) return;
    hunger = qMin(maxHunger, hunger + amount);
}

bool Character::eatFood(int inventoryIndex, QString& effectDescription) {
    if (inventoryIndex < 0 || inventoryIndex >= inventory.size()) {
        effectDescription = "Invalid inventory index.";
        return false;
    }

    HeldItem item = inventory[inventoryIndex];
    const ItemDef* def = ItemDatabase::instance().byName(item.name);
    if (!def) {
        effectDescription = QString("Unknown item: %1").arg(item.name);
        return false;
    }

    // Only food items (type 22) can be eaten.
    if (def->type != 22) {
        effectDescription = QString("%1 is not food.").arg(item.name);
        return false;
    }

    // Food restores hunger based on spellLvl (5 + spellLvl*3, min 5).
    int restoreAmount = qMax(5, 5 + def->spellLvl * 3);
    restoreHunger(restoreAmount);
    effectDescription = QString("You eat the %1. (+%2 hunger)")
                            .arg(item.name).arg(restoreAmount);

    // Decrement charges.
    item.M4ED4--;
    if (item.M4ED4 <= 0) {
        inventory.removeAt(inventoryIndex);
        effectDescription += " The food is consumed.";
    } else {
        inventory[inventoryIndex] = item;
    }

    return true;
}

void Character::setDead() {
    addStatus(StatusFlag::Dead);
    hp = 0;
    isAlive = false;
    // The body stays where it fell; dungeonLevel/X/Y are the recovery marker.
}

void Character::resurrect() {
    removeStatus(StatusFlag::Dead);
    isAlive = true;
    if (hp <= 0) hp = 1;
}

void Character::addExperience(int amount) {
    if (amount <= 0 || !isAlive) return;

    experience += amount;

    // Level-up thresholds come from the data-driven table (data/levels.json).
    LevelTable& lt = LevelTable::instance();
    int newLevel = lt.levelForXp(experience);

    while (level < newLevel) {
        level++;

        // Boost stats on level up
        maxHp += 5;
        hp = maxHp; // Heal on level up
        maxMana += 2;
        mana = maxMana;
    }
}

// --- Guild progression ---

int Character::guildLevel(const QString& guildName) const {
    return guildLevels.value(guildName, 0);
}

void Character::joinGuild(const QString& guildName) {
    if (!guildLevels.contains(guildName)) {
        guildLevels[guildName] = 1;
        guildExperience[guildName] = 0;
    }
}

int Character::incrementGuildLevel(const QString& guildName) {
    if (!guildLevels.contains(guildName)) {
        joinGuild(guildName);
        return guildLevels.value(guildName, 1);
    }
    guildLevels[guildName] += 1;
    return guildLevels[guildName];
}

int Character::totalGuildLevels() const {
    int total = 0;
    for (auto it = guildLevels.constBegin(); it != guildLevels.constEnd(); ++it) {
        total += it.value();
    }
    return total;
}

qint64 Character::guildXpToNextLevel(const QString& guildName) const {
    int lvl = guildLevel(guildName);
    if (lvl <= 0) return 0;
    // Guild levels use the same curve as character levels.
    return static_cast<qint64>(LevelTable::instance().xpForLevel(lvl));
}

bool Character::addGuildExperience(const QString& guildName, qint64 amount) {
    if (amount <= 0) return false;
    if (!guildLevels.contains(guildName)) {
        joinGuild(guildName);
    }

    guildExperience[guildName] += amount;

    // Level up while the guild XP pool covers the next level's cost.
    bool leveled = false;
    while (guildExperience[guildName] >= guildXpToNextLevel(guildName)) {
        guildExperience[guildName] -= guildXpToNextLevel(guildName);
        guildLevels[guildName] += 1;
        leveled = true;
    }
    return leveled;
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
    map["PLUTTEN"] = "PLUTTEN";
    return map;
}

void Party::loadFromMap(const QVariantMap &map) {
    // Replacing a party means replacing it: without this clear, every load
    // appends the saved members to whatever is already in memory and the
    // party doubles in size each time.
    members.clear();

    sharedGold = map.value("SharedGold", 0).toInt();
    QVariantList charList = map.value("Members").toList();

    for (int i = 0; i < charList.size(); ++i) {
        Character c;
        c.loadFromMap(charList.at(i).toMap());
        members.append(c);
    }
}
