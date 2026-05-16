#ifndef SAVEGAME_UTILS_H
#define SAVEGAME_UTILS_H

#include <QString>

namespace SavegameUtils {

    /**
     * @brief Verifies that a character save file exists and contains valid structural headers.
     * @param characterName The name of the character (or file path).
     * @return true if valid, false if corrupt/missing details.
     */
    bool verifySaveGame(const QString& characterName);

    /**
     * @brief Inspects a character save file, inserting missing critical attributes with defaults if corrupt.
     * @param characterName The name of the character (or file path).
     * @return true if successfully repaired or modified, false otherwise.
     */
    bool repairSaveGame(const QString& characterName);

} // namespace SavegameUtils

#endif // SAVEGAME_UTILS_H
