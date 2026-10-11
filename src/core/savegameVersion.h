#ifndef SAVEGAME_VERSION_H
#define SAVEGAME_VERSION_H

#include <QVariantMap>

namespace SavegameVersion {

// The current save schema version. Bump whenever a slice persists a new field.
// See VERSIONING.md for the policy.
constexpr int CURRENT_SAVE_VERSION = 3;

// The highest version this build can load. A save with a higher version is refused.
constexpr int MAX_LOADABLE_VERSION = 3;

/**
 * @brief Returns the SaveVersion field from a loaded map. Missing = 0 (legacy).
 */
inline int versionOf(const QVariantMap& save) {
    return save.value("SaveVersion", 0).toInt();
}

/**
 * @brief Runs all pending migrations on the given map, in order.
 * Returns true if the map was modified.
 */
bool migrate(QVariantMap& save);

/**
 * @brief Returns true if a save at this version can be loaded by this build.
 */
inline bool isLoadable(int version) {
    return version >= 0 && version <= MAX_LOADABLE_VERSION;
}

} // namespace SavegameVersion

#endif // SAVEGAME_VERSION_H
