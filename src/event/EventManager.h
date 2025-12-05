#pragma once
#include <QObject>
#include <QVector>
#include <QString>
#include <QJsonObject>

struct GameEvent {
    QString id;
    QString type; // "Dungeon", "Town", "Character"
    QString description;
    QString trigger; // e.g., "ENTER_LEVEL_5", "VISIT_BANK", "PARTY_HAS_ITEM_SWORD"
    QString effect; // e.g., "SPAWN_MONSTER", "GIVE_ITEM", "CHANGE_PRICES"
    bool resolved;
    QJsonObject extraData;
};

class EventManager : public QObject {
    Q_OBJECT
public:
    static EventManager* instance(); // For singleton access

    EventManager(QObject* parent = nullptr);
    void loadEvents(const QString& filename);
    void update(const QString& trigger, QJsonObject context = {}); // Call when game state changes
    QVector<GameEvent> currentEvents() const;

signals:
    void eventTriggered(const GameEvent& event);

private:
    QVector<GameEvent> m_allEvents;
    QVector<GameEvent> m_activeEvents;
    void checkEvents(const QString& trigger, QJsonObject context);
};
