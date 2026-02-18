#include "DataRegistry.h"
#include "GameConstants.h"

class StaticDataManager {
public:
    static StaticDataManager& instance() { static StaticDataManager i; return i; }

    void loadAllData() {
        // Moved from GameStateManager::loadGameData
        loadJson("tools/gamedataconverter/data/MDATA1.js"); 
        loadCsv("tools/monsterconverter/MDATA5.csv", GameConstants::CAT_MONSTERS);
        loadCsv("tools/itemconverter/data/MDATA3.csv", GameConstants::CAT_ITEMS);
        loadRaceDefinitions();
    }

    // Type-safe getters instead of raw QVariant lookups
    GameConstants::RaceStats getRaceStats(const QString& raceName) const;
    QVariantMap getMonster(int id) const;

private:
    DataRegistry m_registry;
    QVector<GameConstants::RaceStats> m_raceDefinitions;
    
    void loadJson(const QString& path); // Logic from DataLoader.h
    void loadCsv(const QString& path, const QString& category);
};
