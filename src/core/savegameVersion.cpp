#include "savegameVersion.h"
#include <QDebug>

namespace SavegameVersion {

// Migration 0 → 1: v2.0.0 introduced SaveVersion. Legacy saves have no such
// field. Character fields that already existed (StatPoints etc.) are read with
// defaults by Character::loadFromMap, so no per-character work is needed here.
static bool migrateFrom0(QVariantMap& save) {
    // Nothing to do beyond bumping the version — Character::loadFromMap already
    // defaults missing fields. The version bump happens in migrate().
    Q_UNUSED(save)
    return true;
}

// Migration 1 → 3: v3.0.0 added StatPoints, PlayTimeSeconds and Row on
// characters; v4.0.0 added NGPlusLevel. Party::toMap() stores Members as a
// QJsonArray of character maps. Character::loadFromMap() already defaults
// missing fields, but we ensure the new fields are present so any code that
// reads them directly (without going through loadFromMap) sees them too.
static bool migrateFrom1(QVariantMap& save) {
    QVariantMap party = save.value("Party").toMap();
    if (!party.isEmpty()) {
        QVariantList members = party.value("Members").toList();

        bool modified = false;
        for (int i = 0; i < members.size(); ++i) {
            QVariantMap member = members.at(i).toMap();
            if (!member.contains("StatPoints")) {
                member["StatPoints"] = 0;
                modified = true;
            }
            if (!member.contains("PlayTimeSeconds")) {
                member["PlayTimeSeconds"] = 0;
                modified = true;
            }
            // Character::toMap() uses "row" (lowercase) — match it so the field
            // survives a save/load round-trip.
            if (!member.contains("row")) {
                member["row"] = 0;  // CombatRow::Front
                modified = true;
            }
            members[i] = member;
        }
        if (modified) {
            party["Members"] = members;
            save["Party"] = party;
        }
    }

    // v4.0.0 slice 4.1: NGPlusLevel — default to 0 (base game) for old saves.
    if (!save.contains("NGPlusLevel")) {
        save["NGPlusLevel"] = 0;
    }

    return true;
}

bool migrate(QVariantMap& save) {
    int version = versionOf(save);
    bool modified = false;

    while (version < CURRENT_SAVE_VERSION) {
        switch (version) {
        case 0:
            if (!migrateFrom0(save)) return false;
            break;
        case 1:
            if (!migrateFrom1(save)) return false;
            break;
        case 2:
            // v2 saves already have NGPlusLevel from migrateFrom1, but ensure
            // the field is present in case a v2 save was written before 4.1.
            if (!save.contains("NGPlusLevel")) {
                save["NGPlusLevel"] = 0;
            }
            break;
        default:
            qWarning() << "SavegameVersion: no migration from" << version;
            return false;
        }
        version++;
        save["SaveVersion"] = version;
        modified = true;
        qDebug() << "SavegameVersion: migrated to" << version;
    }

    return modified;
}

} // namespace SavegameVersion
