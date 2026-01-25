#include "EventManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

// Static instance for singleton
EventManager* s_instance = nullptr;

EventManager* EventManager::instance()
{
    if (!s_instance)
        s_instance = new EventManager();
    return s_instance;
}

EventManager::EventManager(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<GameEvent>("GameEvent");
}

void EventManager::loadEvents(const QString& filename)
{
    m_allEvents.clear();
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open events file:" << filename;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    for (const auto& ev : doc.array()) {
        QJsonObject o = ev.toObject();
        GameEvent event;
        event.id = o["id"].toString();
        event.type = o["type"].toString();
        event.description = o["description"].toString();
        event.trigger = o["trigger"].toString();
        event.effect = o["effect"].toString();
        event.resolved = o.value("resolved").toBool(false);
        event.extraData = o["extraData"].toObject();
        m_allEvents.append(event);
    }
}

void EventManager::update(const QString& trigger, QJsonObject context)
{
    checkEvents(trigger, context);
}

void EventManager::checkEvents(const QString& trigger, QJsonObject context)
{
    Q_UNUSED(context);
    for (auto& event : m_allEvents) {
        if (!event.resolved && event.trigger == trigger) {
            m_activeEvents.append(event);
            emit eventTriggered(event);
            event.resolved = true; // To prevent retrigger; remove this for repeatable events
        }
    }
}

QVector<GameEvent> EventManager::currentEvents() const
{
    return m_activeEvents;
}
