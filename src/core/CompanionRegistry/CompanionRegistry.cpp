#include "CompanionRegistry.h"
#include "Companion.h"

CompanionRegistry::CompanionRegistry(QObject *parent) : QObject(parent) {}

void CompanionRegistry::registerCompanion(QSharedPointer<Companion> companion) {
    if (!companion) return;
    m_companions.append(companion);
    emit companionRegistered(companion->id());
}

void CompanionRegistry::unregisterCompanion(const QString &id) {
    for (auto it = m_companions.begin(); it != m_companions.end(); ++it) {
        if ((*it)->id() == id) {
            m_companions.erase(it);
            emit companionRemoved(id);
            break;
        }
    }
}

QSharedPointer<Companion> CompanionRegistry::getCompanion(const QString &id) const {
    for (const auto &c : m_companions) {
        if (c->id() == id) return c;
    }
    return QSharedPointer<Companion>{};
}

QList<QSharedPointer<Companion>> CompanionRegistry::getAllCompanions() const {
    return m_companions;
}
