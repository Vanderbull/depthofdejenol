#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QJsonObject>

// Simple struct for event data
struct GameEvent {
    QString id;
    QString type; // "Dungeon", "Town", "Character"
    QString description;
    QString trigger;
    QString effect;
    bool resolved;
    QJsonObject extraData;
};

Q_DECLARE_METATYPE(GameEvent) // Enables use in queued signals/slots

class EventManager : public QObject
{
    Q_OBJECT
public:
    static EventManager* instance();

    EventManager(QObject* parent = nullptr);

    void loadEvents(const QString& filename);
    void update(const QString& trigger, QJsonObject context = QJsonObject());
    QVector<GameEvent> currentEvents() const;

signals:
    void eventTriggered(const GameEvent& event);

private:
    QVector<GameEvent> m_allEvents;
    QVector<GameEvent> m_activeEvents;

    void checkEvents(const QString& trigger, QJsonObject context);
};
