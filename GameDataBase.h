class GameDatabase {
public:
    static GameDatabase& instance() {
        static GameDatabase inst;
        return inst;
    }

    // Move all "Load" methods here
    void loadAll(const QString& rootPath);
    
    // Move "Get" methods here
    QVariantMap getMonster(int id) const;
    QVariantMap getItem(int id) const;
    GameConstants::RaceStats getStatsForRace(const QString& name) const;

private:
    QList<QVariantMap> m_monsters;
    QList<QVariantMap> m_items;
    QList<QVariantMap> m_spells;
    QVector<GameConstants::RaceStats> m_races;
};
