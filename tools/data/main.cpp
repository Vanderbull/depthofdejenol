#include <QCoreApplication>
#include <QDebug>
#include <QVector>
#include <QString>

// Include the converted types and functions
#include "MTypes.h"
#include "MLoader.h" // Assuming this includes the external function declarations

int main(int argc, char *argv[])
{
    // Use QCoreApplication since this is a console/data loading application
    QCoreApplication a(argc, argv);

    qDebug() << "--- Starting Data Load Test ---";

    // Define dummy filenames (in a real app, these would be the actual file paths)
    const QString DATA1 = "MDATA1.MDR";
    const QString DATA2 = "MDATA2.MDR";
    const QString DATA3 = "MDATA3.MDR";
    const QString DATA4 = "MDATA4.MDR";
    const QString DATA5 = "MDATA5.MDR";
    const QString DATA8 = "MDATA8.MDR";
    const QString DATA9 = "MDATA9.MDR";
    const QString DATA11 = "MDATA11.MDR";
    const QString DATA13 = "MDATA13.MDR";
    const QString DATA14 = "MDATA14.MDR";

    try {
        // 1. loadGameInfo (Not Implemented - will assert/throw if called in original C++)
        // GameData gameInfo = loadGameInfo(DATA1);
        // qDebug() << "GameData loaded (Skipped: Not Implemented).";

        // 2. loadSpells
        Spells spells = loadSpells(DATA2);
        qDebug() << "✅ Spells loaded. Count: " << spells.size();

        // 3. loadItems
        Items items = loadItems(DATA3);
        qDebug() << "✅ Items loaded. Count: " << items.size();

        // 4. loadCharacters (Not Implemented - will assert/throw)
        // Characters characters = loadCharacters(DATA4);
        // qDebug() << "Characters loaded (Skipped: Not Implemented).";

        // 5. loadMonsters
        Monsters monsters = loadMonsters(DATA5);
        qDebug() << "✅ Monsters loaded. Count: " << monsters.size();

        // 6. loadAutomap
        Automap automap = loadAutomap(DATA8);
        qDebug() << "✅ Automap loaded. Deepest Level: " << automap.deepestLevel;

        // 7. loadGuildLogs
        GuildLogs logs = loadGuildLogs(DATA9);
        qDebug() << "✅ GuildLogs loaded. Count: " << logs.size();

        // 8. loadDungeon
        Dungeon dungeon = loadDungeon(DATA11);
        qDebug() << "✅ Dungeon loaded. Levels: " << dungeon.size();

        // 9. loadLibrary
        Library library = loadLibrary(DATA13);
        qDebug() << "✅ Library loaded. Monster Records: " << library.monsters.size();

        // 10. loadHallRecords
        HallRecords hallRecords = loadHallRecords(DATA14);
        qDebug() << "✅ HallRecords loaded. Count: " << hallRecords.size();

    } catch (const char* msg) {
        qCritical() << "❌ FATAL ERROR during data loading:" << msg;
        return 1;
    } catch (const std::string& msg) {
        qCritical() << "❌ FATAL ERROR during data loading:" << QString::fromStdString(msg);
        return 1;
    }

    qDebug() << "--- Data Load Test Complete ---";

    // Since this is a simple test, we can just return 0 instead of running the event loop
    return 0;
}
