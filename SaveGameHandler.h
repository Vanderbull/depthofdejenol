#ifndef SAVEGAMEHANDLER_H
#define SAVEGAMEHANDLER_H

#include <QString>

class SaveGameHandler {
public:
    static bool verifySave(const QString& characterName);
    static bool repairSave(const QString& characterName);
};

#endif
