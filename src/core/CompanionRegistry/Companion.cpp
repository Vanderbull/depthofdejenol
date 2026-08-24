#include "Companion.h"
#include "CompanionInventoryModel.h"

Companion::Companion(QString id, QString name, QObject *parent)
    : QObject(parent), m_id(std::move(id)), m_name(std::move(name)) {
    m_inventory = new CompanionInventoryModel(this);
}

QString Companion::id() const { return m_id; }
QString Companion::name() const { return m_name; }

int Companion::morale() const { return m_morale; }
void Companion::setMorale(int morale) {
    int clamped = qBound(0, morale, 100);
    if (m_morale != clamped) {
        m_morale = clamped;
        emit moraleUpdated(m_morale);
    }
}

int Companion::loyalty() const { return m_loyalty; }
void Companion::setLoyalty(int loyalty) {
    int clamped = qBound(0, loyalty, 100);
    if (m_loyalty != clamped) {
        m_loyalty = clamped;
        emit loyaltyUpdated(m_loyalty);
    }
}

Companion::Status Companion::currentStatus() const { return m_currentStatus; }
void Companion::setCurrentStatus(Status status) {
    if (m_currentStatus != status) {
        m_currentStatus = status;
        emit statusChanged(m_currentStatus);
    }
}

CompanionInventoryModel* Companion::inventoryModel() const { return m_inventory; }

void Companion::updateTick() {
    // Handle background fatigue, passive recovery, or wayfaring logic here
}
