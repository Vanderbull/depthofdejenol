// GameStateManager.h
#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include <QObject>
#include <QVariantMap> // Useful for holding flexible, common data types
#include <QtGlobal>    // For qulonglong

class GameStateManager : public QObject
{
    Q_OBJECT

private:
    // Private constructor to prevent direct instantiation
    explicit GameStateManager(QObject *parent = nullptr);
    
    // Prevent copy and assignment
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;

public:
    // **The public static method to access the single instance**
    static GameStateManager* instance();
    
    // Core state data members
private:
    // Example: A map to store various game data
    QVariantMap m_gameStateData;

public:
    // Public interface to manage state
    void setGameValue(const QString& key, const QVariant& value);
    QVariant getGameValue(const QString& key) const;

    // Public method to add experience to the current character
    void addCharacterExperience(qulonglong amount);
    
    // Public method to add an entry to the guild action log
    void logGuildAction(const QString& actionDescription);
    
    // Public method to dump the entire game state to debug output
    void printAllGameState() const; 

    // Example: Check if all resources are loaded
    bool areResourcesLoaded() const; 

};

#endif // GAMESTATEMANAGER_H
