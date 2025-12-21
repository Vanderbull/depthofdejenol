// GameStateManager.h
#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include <QObject>
#include <QVariantMap> 
#include <QtGlobal>    

class GameStateManager : public QObject
{
    Q_OBJECT

private:
    // Private constructor
    explicit GameStateManager(QObject *parent = nullptr);
    
    // Prevent copy and assignment
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;

public:
    // Improved thread-safe static accessor
    static GameStateManager* instance();
    
signals:
    void gameValueChanged(const QString& key, const QVariant& value);

private:
    QVariantMap m_gameStateData;

public:
    void setGameValue(const QString& key, const QVariant& value);
    QVariant getGameValue(const QString& key) const;
    void addCharacterExperience(qulonglong amount);
    void logGuildAction(const QString& actionDescription);
    void printAllGameState() const;
    bool areResourcesLoaded() const;

    // Character Status Methods
    void setCharacterPoisoned(bool isPoisoned);
    bool isCharacterPoisoned() const;
    void setCharacterBlinded(bool isBlinded);
    bool isCharacterBlinded() const;
    static QStringList statNames() {
        return {"Strength", "Intelligence", "Wisdom", "Constitution", "Charisma", "Dexterity"};
    }
    static int defaultStatPoints() { return 5; }
    static QStringList alignmentNames() {
        return {"Good", "Neutral", "Evil"};
    }
    static QStringList sexOptions() {
        return {"Male", "Female"};
    }
    static int defaultAlignmentIndex() { 
        return 1; // Default to "Neutral"
    }
};

#endif // GAMESTATEMANAGER_H
