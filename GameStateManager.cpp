// GameStateManager.cpp
#include "GameStateManager.h"
#include <QDebug> // For logging/debugging

// Static pointer to hold the single instance
GameStateManager* gsm_instance = nullptr;

// Public static accessor function
GameStateManager* GameStateManager::instance()
{
    if (!gsm_instance) {
        gsm_instance = new GameStateManager();
    }
    return gsm_instance;
}

// Private constructor
GameStateManager::GameStateManager(QObject *parent) 
    : QObject(parent) 
{
    // Initialize default state data
    m_gameStateData["ResourcesLoaded"] = false;
    m_gameStateData["GameVersion"] = "1.1.7.6.450";
    m_gameStateData["PlayerScore"] = 0;
    
    qDebug() << "GameStateManager initialized.";
}

// Implement public interface methods
void GameStateManager::setGameValue(const QString& key, const QVariant& value)
{
    m_gameStateData[key] = value;
    qDebug() << "State updated:" << key << "=" << value;
}

QVariant GameStateManager::getGameValue(const QString& key) const
{
    return m_gameStateData.value(key);
}

bool GameStateManager::areResourcesLoaded() const
{
    return m_gameStateData.value("ResourcesLoaded").toBool();
}
