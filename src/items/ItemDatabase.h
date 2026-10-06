#ifndef ITEMDATABASE_H
#define ITEMDATABASE_H

#include <QHash>
#include <QList>
#include <QString>
#include <QVariantMap>

// Equipment slot a given item type occupies. `NotEquippable` is used for
// consumables and miscellaneous objects.
namespace ItemSlot {
enum Slot {
    NotEquippable = -1,
    MainHand = 0,   // unarmed "Hands" plus every weapon
    OffHand,        // shields and held crosses
    Head,
    Hands,
    Body,
    Cloak,
    Wrist,
    Waist,
    Feet,
    Ring,
    Neck,
    Count
};

// Human-readable slot name, for UI and diagnostics.
QString name(Slot slot);
} // namespace ItemSlot

// One row of MDATA3. Field names and types mirror the CSV columns exactly so
// the mapping is obvious when reading either side.
struct ItemDef {
    int     id = -1;
    QString name;

    int     att = 0;            // attack bonus / weapon power
    int     def = 0;            // defence bonus
    qint64  price = 0;          // shop price in gold
    int     floor = 0;          // dungeon depth the item appears on
    int     rarity = 0;         // higher = rarer
    int     abilities = 0;      // ability bitmask
    int     swings = 1;         // attacks per round when wielded
    int     specialType = 0;
    int     spellIndex = 0;
    int     spellID = 0;
    int     charges = 0;        // for consumables / charged items
    int     guilds = 0;         // guild bitmask allowed to use it
    int     levelScale = 0;     // damage scaling with character level
    double  damageMod = 1.0;    // damage multiplier
    int     alignmentFlags = 0;
    int     nHands = 1;         // hands required to wield
    int     type = 0;           // see ItemSlot and typeName()
    int     resistanceFlags = 0;
    int     strReq = 0, intReq = 0, wisReq = 0;
    int     conReq = 0, chaReq = 0, dexReq = 0;
    int     strMod = 0, intMod = 0, wisMod = 0;
    int     conMod = 0, chaMod = 0, dexMod = 0;
    bool    cursed = false;
    int     spellLvl = 0;
    int     classRestricted = -1;

    // --- Derived ---------------------------------------------------------
    ItemSlot::Slot slot() const;   // where this item is worn
    bool equippable() const;       // slot() != NotEquippable
    QString typeName() const;      // e.g. "Sword", "Potion"

    // Requirement / modifier lookups by GameConstants stat name
    // ("Strength", "Intelligence", ...). Returns 0 for an unknown name.
    int requirementFor(const QString& statName) const;
    int modifierFor(const QString& statName) const;
};

// Loads MDATA3 once and serves typed lookups. Consumers should prefer this
// over searching QVariantMaps by name: it is one parse, type-safe, and
// O(1) by id.
class ItemDatabase
{
public:
    static ItemDatabase& instance();

    // Replaces all loaded items. Returns false (and leaves the database
    // untouched) if the file cannot be opened or yields no rows.
    bool loadFromCsv(const QString& filePath);

    bool isLoaded() const { return !m_items.isEmpty(); }
    int  count() const { return m_items.size(); }

    const QList<ItemDef>& all() const { return m_items; }

    // Null when not found — callers must check.
    const ItemDef* byId(int id) const;
    const ItemDef* byName(const QString& name) const;

    // Flat view of an item for UI code that still works in QVariantMaps.
    // Keys mirror MDATA3 (`price`, `cursed`, `type`, ...) plus `typeName`
    // and `slot` for display. Returns an empty map if not found.
    static QVariantMap toVariantMap(const ItemDef& item);

private:
    ItemDatabase() = default;

    QList<ItemDef>       m_items;
    QHash<int, int>      m_indexById;     // id   -> position in m_items
    QHash<QString, int>  m_indexByName;   // name -> position in m_items
};

#endif // ITEMDATABASE_H
