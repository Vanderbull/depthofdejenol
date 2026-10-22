#include "src/character/character.h"
#include "src/items/ItemDatabase.h"
#include "src/items/ConsumableResolver.h"
#include "src/core/LevelTable.h"
#include "src/core/ResistanceNames.h"
#include "src/core/gameStateManager.h"
#include "src/race_data/RaceData.h"
#include <QRandomGenerator>

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
    map["StatPoints"]   = statPoints;
    map["PlayTimeSeconds"] = playTimeSeconds;
    map["Mana"]         = mana;
    map["MaxMana"]      = maxMana;
    map["Hunger"]       = hunger;
    map["MaxHunger"]    = maxHunger;
    map["Spouse"]       = spouse;
    map["Children"]     = children;
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

    // Resistances are granted by Resist/Protection consumables and consulted
    // by the combat damage path, so they must survive a save/load round trip.
    QVariantList resList;
    for (int i = 0; i < 12; ++i) resList.append(resistances[i]);
    map["Resistances"] = resList;

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
    // Default 0: saves written before stat points existed simply have none.
    statPoints   = map.value("StatPoints", 0).toInt();
    playTimeSeconds = map.value("PlayTimeSeconds", 0).toInt();

    mana         = map.value("Mana", 0).toInt();
    maxMana      = map.value("MaxMana", 0).toInt();

    // Default full so saves written before hunger existed do not start starving.
    hunger       = map.value("Hunger", 100).toInt();
    maxHunger    = map.value("MaxHunger", 100).toInt();

    // Lifecycle (v5.3.0 slice 5.2)
    spouse       = map.value("Spouse").toString();
    children     = map.value("Children").toStringList();

    statusFlags  = map.value("StatusFlags", StatusFlag::None).toUInt();
    // Default true so saves written before this field existed still load as alive.
    isAlive      = map.value("isAlive", true).toBool();

    // Repair incomplete HP from legacy saves. MaxHP is never legitimately 0, so
    // a 0/0 character would load as permanently dead. Without this an old save
    // is unplayable.
    if (maxHp <= 0) maxHp = 10;
    if (isAlive && hp <= 0) hp = maxHp;
    
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

    // Load resistances. Older saves have no entry, in which case every slot
    // stays at zero rather than being treated as a corrupt save.
    const QVariant resVar = map.value("Resistances");
    if (resVar.typeId() == QMetaType::QVariantList) {
        const QVariantList resList = resVar.toList();
        for (int i = 0; i < 12 && i < resList.size(); ++i) {
            resistances[i] = int16_t(qBound(0, resList.at(i).toInt(), 100));
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

int Character::baseStat(const QString& statName) const {
    const QString s = statName.trimmed().toLower();
    if (s == "strength")     return strength;
    if (s == "intelligence") return intelligence;
    if (s == "wisdom")       return wisdom;
    if (s == "constitution") return constitution;
    if (s == "charisma")     return charisma;
    if (s == "dexterity")    return dexterity;
    return 0;
}

int Character::raceCapForStat(const QString& statName) const {
    const QString s = statName.trimmed().toLower();
    const QVector<RaceStats> races = loadRaceData();
    for (const RaceStats& r : races) {
        if (r.raceName.compare(race, Qt::CaseInsensitive) != 0) continue;
        if (s == "strength")     return r.strength.max;
        if (s == "intelligence") return r.intelligence.max;
        if (s == "wisdom")       return r.wisdom.max;
        if (s == "constitution") return r.constitution.max;
        if (s == "charisma")     return r.charisma.max;
        if (s == "dexterity")    return r.dexterity.max;
    }
    return 0;  // race or stat not found — "no cap known"
}

bool Character::spendStatPoint(const QString& statName, int cap) {
    if (statPoints <= 0) return false;

    const QString s = statName.trimmed().toLower();
    int* target = nullptr;
    if (s == "strength")          target = &strength;
    else if (s == "intelligence") target = &intelligence;
    else if (s == "wisdom")       target = &wisdom;
    else if (s == "constitution") target = &constitution;
    else if (s == "charisma")     target = &charisma;
    else if (s == "dexterity")    target = &dexterity;
    if (!target) return false;

    // A cap of 0 or less means "no cap known" — still allow the spend rather
    // than silently blocking progress.
    if (cap > 0 && *target >= cap) return false;

    *target += 1;
    statPoints -= 1;
    return true;
}

// --- Consumables ---

bool Character::isConsumable(const HeldItem& item) {
    const ItemDef* def = ItemDatabase::instance().byName(item.name);
    if (!def) return false;
    // Only potions (23), scrolls (24) and tomes (25) are consumable — the same
    // rule useConsumable() enforces.
    return def->type == 23 || def->type == 24 || def->type == 25;
}

bool Character::isFood(const HeldItem& item) {
    const ItemDef* def = ItemDatabase::instance().byName(item.name);
    if (!def) return false;
    // Food is its own type (35). Type 22 is Amulets — the old eatFood() ate
    // jewellery because it gated on 22, and no real food item existed.
    return def->type == FOOD_TYPE;
}

QList<int> Character::foodIndices(const QList<HeldItem>& inventory) {
    QList<int> indices;
    for (int i = 0; i < inventory.size(); ++i) {
        if (isFood(inventory[i])) indices.append(i);
    }
    return indices;
}

bool Character::hasFood(const QList<HeldItem>& inventory) {
    for (const HeldItem& item : inventory) {
        if (isFood(item)) return true;
    }
    return false;
}

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

    // Resolve what the item does from the item data — never from its name.
    // `spellID` indexes the MDATA2 spell table (105 rows, verified: every
    // row's ID equals its row index); items that declare no spell carry their
    // effect in their own stat modifiers. The old code matched on
    // nameLower.contains("healing") and silently no-op'd for any item whose
    // name lacked the right keyword.
    const QVariantMap spellRow = (def->spellID > 0)
        ? gameStateManager::instance()->getSpell(def->spellID)
        : QVariantMap();
    const ConsumableResolver::Resolved effect =
        ConsumableResolver::resolve(*def, spellRow);

    switch (effect.effect) {
    case ConsumableResolver::Effect::Heal: {
        // Roll the spell's own damage range (MDATA2 damage1-damage2). A spell
        // with a 0-0 range cannot heal, so it is classified as a cure instead.
        int roll = effect.amountMin;
        if (effect.amountMax > effect.amountMin) {
            roll += QRandomGenerator::global()->bounded(effect.amountMax - effect.amountMin + 1);
        }
        const int actual = qMin(qMax(0, roll), maxHp - hp);
        hp += actual;
        effectDescription = actual > 0
            ? QString("Restored %1 HP.").arg(actual)
            : QString("Already at full health.");
        break;
    }

    case ConsumableResolver::Effect::CurePoison:
        // The flag is StatusFlag::Poisoned (1 << 0); the old literal 0x02 was
        // Blinded, so an earlier version cured the wrong status.
        if (statusFlags & StatusFlag::Poisoned) {
            statusFlags &= ~StatusFlag::Poisoned;
            effectDescription = "Poison cured!";
        } else {
            effectDescription = "No poison to cure.";
        }
        break;

    case ConsumableResolver::Effect::StatBoost:
        effectDescription = applyConsumableStatBoost(*def);
        break;

    case ConsumableResolver::Effect::Resist: {
        // "Resist Fire" names its own element; the value goes into that slot.
        // Previously this only set the description, so the potion did nothing.
        const QString type = ResistanceNames::fromSpellName(effect.spellName);
        const int idx = ResistanceNames::indexOf(type);
        if (idx < 0 || idx >= 12) {
            // The manual names three resistance types the combat damage model
            // has no damage school for (Stone, Drain, Acidity), so those
            // potions have nothing to reduce. Say so rather than claim
            // protection the character does not get.
            effectDescription = QString("%1 grants no resistance in this game.")
                                    .arg(effect.spellName);
            break;
        }
        resistances[idx] = qMax(resistances[idx],
                                int16_t(GameConstants::RESIST_GRANT_PERCENT));
        effectDescription = QString("You feel protected — %1 resistance now %2%.")
                                .arg(type).arg(resistances[idx]);
        break;
    }

    case ConsumableResolver::Effect::Protection:
        // A general ward rather than a single element; it covers the physical
        // and weapon schools, which are the ones no Resist spell names.
        resistances[ResistanceNames::indexOf(QStringLiteral("Physical"))] =
            qMax(resistances[ResistanceNames::indexOf(QStringLiteral("Physical"))],
                 int16_t(GameConstants::PROTECTION_GRANT_PERCENT));
        resistances[ResistanceNames::indexOf(QStringLiteral("Weapon"))] =
            qMax(resistances[ResistanceNames::indexOf(QStringLiteral("Weapon"))],
                 int16_t(GameConstants::PROTECTION_GRANT_PERCENT));
        effectDescription = QString("A ward settles over you — physical and "
                                    "weapon damage reduced by %1%.")
                                .arg(GameConstants::PROTECTION_GRANT_PERCENT);
        break;

    case ConsumableResolver::Effect::Damage:
        // An offensive scroll only means something with a target in front of
        // you; CombatActions::useItem applies it to the combat target.
        effectDescription = QString("%1 crackles with power — use it in combat.")
                                .arg(effect.spellName);
        break;

    case ConsumableResolver::Effect::Charm:
        // The charm school's damage range is its power, not HP damage — these
        // scrolls turn monsters friendly. Needs a target, like Damage.
        effectDescription = QString("%1 beckons — use it in combat to charm a creature.")
                                .arg(effect.spellName);
        break;

    case ConsumableResolver::Effect::Banish:
        effectDescription = QString("%1 rings out — use it in combat to banish a creature.")
                                .arg(effect.spellName);
        break;

    case ConsumableResolver::Effect::Dispel:
        effectDescription = QString("%1 unravels — use it in combat to dispel a creature.")
                                .arg(effect.spellName);
        break;

    case ConsumableResolver::Effect::Teleport:
        teleportTurns = 10 + (def->spellLvl * 2);
        effectDescription = QString("The world lurches. Teleport active for %1 turns.").arg(teleportTurns);
        break;

    case ConsumableResolver::Effect::Levitate:
        levitationTurns = 10 + (def->spellLvl * 2);
        effectDescription = QString("Your feet leave the ground. Levitation active for %1 turns.").arg(levitationTurns);
        break;

    case ConsumableResolver::Effect::Invisibility:
        invisibilityTurns = 8 + (def->spellLvl * 2);
        effectDescription = QString("You fade from sight. Invisibility active for %1 turns.").arg(invisibilityTurns);
        break;

    case ConsumableResolver::Effect::SeeInvisible:
        seeInvisibleTurns = 12 + (def->spellLvl * 2);
        effectDescription = QString("Hidden things stand out. True sight active for %1 turns.").arg(seeInvisibleTurns);
        break;

    case ConsumableResolver::Effect::Bind:
        bindTurns = 6 + def->spellLvl;
        effectDescription = QString("%1 binds your will. Binding active for %1 turns.")
                                .arg(effect.spellName).arg(bindTurns);
        break;

    case ConsumableResolver::Effect::Reveal:
        effectDescription = QString("%1 reveals what was hidden.")
                                .arg(effect.spellName);
        break;

    case ConsumableResolver::Effect::Unknown:
        // Say so plainly rather than pretending the item did something.
        effectDescription = QString("Used %1. (%2 is not yet implemented)")
                                .arg(item.name, ConsumableResolver::effectName(effect.effect));
        break;
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

QString Character::applyConsumableStatBoost(const ItemDef& def) {
    // The stat-potions declare their effect in their own modifiers. They are
    // the most expensive items in the game (Potion of Might ≈ 3.2M gold, Aard
    // of Being ≈ 15M, Dragon's Blood ≈ 188M), so a permanent change is the
    // intended design — this is the classic Mordor stat-potion mechanic.
    struct Mod { const char* label; int* stat; int mod; };
    const Mod mods[] = {
        { "Strength",     &strength,     def.strMod },
        { "Intelligence", &intelligence, def.intMod },
        { "Wisdom",       &wisdom,       def.wisMod },
        { "Constitution", &constitution, def.conMod },
        { "Charisma",     &charisma,     def.chaMod },
        { "Dexterity",    &dexterity,    def.dexMod },
    };

    QStringList changes;
    for (const Mod& m : mods) {
        if (m.mod == 0) continue;
        *m.stat = qMax(1, *m.stat + m.mod);
        changes << QString("%1 %+2").arg(QLatin1String(m.label)).arg(m.mod);
    }

    if (changes.isEmpty()) {
        return QString("Used %1. (no stat change)").arg(def.name);
    }
    return changes.join(QStringLiteral(", ")) + ".";
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

    // Only real food (type 35) can be eaten. The old check used type 22, which
    // is Amulets — so it accepted jewellery and rejected every actual meal.
    if (!isFood(item)) {
        effectDescription = QString("%1 is not food.").arg(item.name);
        return false;
    }

    // Food restores hunger based on spellLvl (10 + spellLvl*5, min 10).
    int restoreAmount = qMax(10, 10 + def->spellLvl * 5);
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
        maxMana += 5;
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
