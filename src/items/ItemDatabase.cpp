#include "ItemDatabase.h"

#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QTextStream>

namespace {

// MDATA3 column values are parsed by header name (see loadFromCsv), so the
// column order can change without breaking anything.
int toInt(const QString& s) { return s.trimmed().toInt(); }
double toDouble(const QString& s) { return s.trimmed().toDouble(); }
qint64 toInt64(const QString& s) { return s.trimmed().toLongLong(); }

} // namespace

namespace ItemSlot {

QString name(Slot slot)
{
    switch (slot) {
    case MainHand:      return QStringLiteral("Main Hand");
    case OffHand:       return QStringLiteral("Off Hand");
    case Head:          return QStringLiteral("Head");
    case Hands:         return QStringLiteral("Hands");
    case Body:          return QStringLiteral("Body");
    case Cloak:         return QStringLiteral("Cloak");
    case Wrist:         return QStringLiteral("Wrist");
    case Waist:         return QStringLiteral("Waist");
    case Feet:          return QStringLiteral("Feet");
    case Ring:          return QStringLiteral("Ring");
    case Neck:          return QStringLiteral("Neck");
    default:            return QStringLiteral("None");
    }
}

} // namespace ItemSlot

// ---------------------------------------------------------------------------
// ItemDef
// ---------------------------------------------------------------------------

ItemSlot::Slot ItemDef::slot() const
{
    // Groupings taken from the MDATA3 type column. Weapons all share MainHand;
    // which hand they actually occupy is decided by nHands at equip time.
    switch (type) {
    case 0: case 1: case 3: case 4: case 5:
    case 6: case 7:                     // Hands, Dagger, Sword, Staff, Mace, Axe, Hammer
        return ItemSlot::MainHand;
    case 2:  return ItemSlot::OffHand;  // Cross
    case 11: return ItemSlot::OffHand;  // Shield
    case 12: case 13:                   // Cap, Helmet
        return ItemSlot::Head;
    case 14: case 15:                   // Gloves, Gauntlets
        return ItemSlot::Hands;
    case 8: case 9: case 10:            // Leather, Chain, Plate
        return ItemSlot::Body;
    case 16: return ItemSlot::Cloak;
    case 17: return ItemSlot::Wrist;    // Bracers
    case 18: case 19:                   // Sash, Girdle
        return ItemSlot::Waist;
    case 20: return ItemSlot::Feet;     // Boots
    case 21: return ItemSlot::Ring;
    case 22: return ItemSlot::Neck;     // Amulet
    default: return ItemSlot::NotEquippable;
    }
}

bool ItemDef::equippable() const
{
    return slot() != ItemSlot::NotEquippable;
}

QString ItemDef::typeName() const
{
    switch (type) {
    case 0:  return QStringLiteral("Hands");
    case 1:  return QStringLiteral("Dagger");
    case 2:  return QStringLiteral("Cross");
    case 3:  return QStringLiteral("Sword");
    case 4:  return QStringLiteral("Staff");
    case 5:  return QStringLiteral("Mace");
    case 6:  return QStringLiteral("Axe");
    case 7:  return QStringLiteral("Hammer");
    case 8:  return QStringLiteral("Leather Armor");
    case 9:  return QStringLiteral("Chain Armor");
    case 10: return QStringLiteral("Plate Armor");
    case 11: return QStringLiteral("Shield");
    case 12: return QStringLiteral("Cap");
    case 13: return QStringLiteral("Helmet");
    case 14: return QStringLiteral("Gloves");
    case 15: return QStringLiteral("Gauntlets");
    case 16: return QStringLiteral("Cloak");
    case 17: return QStringLiteral("Bracers");
    case 18: return QStringLiteral("Sash");
    case 19: return QStringLiteral("Girdle");
    case 20: return QStringLiteral("Boots");
    case 21: return QStringLiteral("Ring");
    case 22: return QStringLiteral("Amulet");
    case 23: return QStringLiteral("Potion");
    case 24: return QStringLiteral("Scroll");
    case 25: return QStringLiteral("Tome");
    case 26: return QStringLiteral("Dust");
    case 27: return QStringLiteral("Crystal");
    case 28: return QStringLiteral("Rod");
    case 29: return QStringLiteral("Stone");
    case 30: return QStringLiteral("Sphere");
    case 31: return QStringLiteral("Cube");
    case 32: return QStringLiteral("Object");
    case 33: return QStringLiteral("Curio");
    case 34: return QStringLiteral("Crest");
    default: return QStringLiteral("Item");
    }
}

int ItemDef::requirementFor(const QString& statName) const
{
    const QString s = statName.trimmed().toLower();
    if (s == "strength")     return strReq;
    if (s == "intelligence") return intReq;
    if (s == "wisdom")       return wisReq;
    if (s == "constitution") return conReq;
    if (s == "charisma")     return chaReq;
    if (s == "dexterity")    return dexReq;
    return 0;
}

int ItemDef::modifierFor(const QString& statName) const
{
    const QString s = statName.trimmed().toLower();
    if (s == "strength")     return strMod;
    if (s == "intelligence") return intMod;
    if (s == "wisdom")       return wisMod;
    if (s == "constitution") return conMod;
    if (s == "charisma")     return chaMod;
    if (s == "dexterity")    return dexMod;
    return 0;
}

// ---------------------------------------------------------------------------
// ItemDatabase
// ---------------------------------------------------------------------------

ItemDatabase& ItemDatabase::instance()
{
    static ItemDatabase db;
    return db;
}

bool ItemDatabase::loadFromCsv(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ItemDatabase: could not open" << filePath;
        return false;
    }

    QTextStream in(&file);

    auto parseCsvLine = [](const QString& line) -> QStringList {
        QStringList fields;
        QString current;
        bool inQuotes = false;
        for (const QChar ch : line) {
            if (ch == '"') {
                inQuotes = !inQuotes;
            } else if (ch == ',' && !inQuotes) {
                fields.append(current.trimmed());
                current.clear();
            } else {
                current.append(ch);
            }
        }
        fields.append(current.trimmed());
        return fields;
    };

    if (in.atEnd()) return false;

    QStringList headers = parseCsvLine(in.readLine());
    for (QString& h : headers) h = h.trimmed();

    // Build into locals first so a failed load leaves the database untouched.
    QList<ItemDef> items;
    items.reserve(400);

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        const QStringList values = parseCsvLine(line);
        if (values.isEmpty()) continue;

        ItemDef item;
        for (int i = 0; i < headers.size() && i < values.size(); ++i) {
            const QString& key = headers.at(i);
            const QString& v   = values.at(i);

            if (key == "name")                 item.name = v;
            else if (key == "ID")              item.id = toInt(v);
            else if (key == "att")             item.att = toInt(v);
            else if (key == "def")             item.def = toInt(v);
            else if (key == "price")           item.price = toInt64(v);
            else if (key == "floor")           item.floor = toInt(v);
            else if (key == "rarity")          item.rarity = toInt(v);
            else if (key == "abilities")       item.abilities = toInt(v);
            else if (key == "swings")          item.swings = toInt(v);
            else if (key == "specialType")     item.specialType = toInt(v);
            else if (key == "spellIndex")      item.spellIndex = toInt(v);
            else if (key == "spellID")         item.spellID = toInt(v);
            else if (key == "charges")         item.charges = toInt(v);
            else if (key == "guilds")          item.guilds = toInt(v);
            else if (key == "levelScale")      item.levelScale = toInt(v);
            else if (key == "damageMod")       item.damageMod = toDouble(v);
            else if (key == "alignmentFlags")  item.alignmentFlags = toInt(v);
            else if (key == "nHands")          item.nHands = toInt(v);
            else if (key == "type")            item.type = toInt(v);
            else if (key == "resistanceFlags") item.resistanceFlags = toInt(v);
            else if (key == "StrReq")          item.strReq = toInt(v);
            else if (key == "IntReq")          item.intReq = toInt(v);
            else if (key == "WisReq")          item.wisReq = toInt(v);
            else if (key == "ConReq")          item.conReq = toInt(v);
            else if (key == "ChaReq")          item.chaReq = toInt(v);
            else if (key == "DexReq")          item.dexReq = toInt(v);
            else if (key == "StrMod")          item.strMod = toInt(v);
            else if (key == "IntMod")          item.intMod = toInt(v);
            else if (key == "WisMod")          item.wisMod = toInt(v);
            else if (key == "ConMod")          item.conMod = toInt(v);
            else if (key == "ChaMod")          item.chaMod = toInt(v);
            else if (key == "DexMod")          item.dexMod = toInt(v);
            else if (key == "cursed")          item.cursed = (toInt(v) == 1);
            else if (key == "spellLvl")        item.spellLvl = toInt(v);
            else if (key == "classRestricted") item.classRestricted = toInt(v);
        }

        if (item.name.isEmpty()) continue;
        items.append(item);
    }

    if (items.isEmpty()) {
        qWarning() << "ItemDatabase: no items parsed from" << filePath;
        return false;
    }

    // Build the lookup indexes, then swap everything in at once.
    QHash<int, int> byId;
    QHash<QString, int> byName;
    byId.reserve(items.size());
    byName.reserve(items.size());
    for (int i = 0; i < items.size(); ++i) {
        byId.insert(items.at(i).id, i);
        byName.insert(items.at(i).name, i);
    }

    m_items       = items;
    m_indexById   = byId;
    m_indexByName = byName;

    qDebug() << "ItemDatabase: loaded" << m_items.size() << "items from" << filePath;
    return true;
}

const ItemDef* ItemDatabase::byId(int id) const
{
    const auto it = m_indexById.constFind(id);
    if (it == m_indexById.constEnd()) return nullptr;
    return &m_items.at(it.value());
}

const ItemDef* ItemDatabase::byName(const QString& name) const
{
    const auto it = m_indexByName.constFind(name);
    if (it == m_indexByName.constEnd()) return nullptr;
    return &m_items.at(it.value());
}

QVariantMap ItemDatabase::toVariantMap(const ItemDef& item)
{
    QVariantMap m;
    m["ID"]              = item.id;
    m["name"]            = item.name;
    m["att"]             = item.att;
    m["def"]             = item.def;
    m["price"]           = QVariant::fromValue(item.price);
    m["floor"]           = item.floor;
    m["rarity"]          = item.rarity;
    m["abilities"]       = item.abilities;
    m["swings"]          = item.swings;
    m["specialType"]     = item.specialType;
    m["spellIndex"]      = item.spellIndex;
    m["spellID"]         = item.spellID;
    m["charges"]         = item.charges;
    m["guilds"]          = item.guilds;
    m["levelScale"]      = item.levelScale;
    m["damageMod"]       = item.damageMod;
    m["alignmentFlags"]  = item.alignmentFlags;
    m["nHands"]          = item.nHands;
    m["type"]            = item.type;
    m["resistanceFlags"] = item.resistanceFlags;
    m["StrReq"]          = item.strReq;
    m["IntReq"]          = item.intReq;
    m["WisReq"]          = item.wisReq;
    m["ConReq"]          = item.conReq;
    m["ChaReq"]          = item.chaReq;
    m["DexReq"]          = item.dexReq;
    m["StrMod"]          = item.strMod;
    m["IntMod"]          = item.intMod;
    m["WisMod"]          = item.wisMod;
    m["ConMod"]          = item.conMod;
    m["ChaMod"]          = item.chaMod;
    m["DexMod"]          = item.dexMod;
    m["cursed"]          = item.cursed;
    m["spellLvl"]        = item.spellLvl;
    m["classRestricted"] = item.classRestricted;

    // Display helpers.
    m["typeName"] = item.typeName();
    m["slot"]     = static_cast<int>(item.slot());
    m["slotName"] = ItemSlot::name(item.slot());
    return m;
}
