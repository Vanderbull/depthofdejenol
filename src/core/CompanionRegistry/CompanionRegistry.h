// CompanionRegistry.h - Standalone Subsystem
#pragma once
#include <QObject>
#include <QList>
#include <QSharedPointer>
#include "CompanionModel.h"

class CompanionRegistry : public QObject {
    Q_OBJECT
public:
    explicit CompanionRegistry(QObject *parent = nullptr);
    
    // Completely independent of player/city state
    void registerCompanion(QSharedPointer<Companion> companion);
    void removeCompanion(const QString &id);
    
    QSharedPointer<Companion> getCompanion(const QString &id) const;
    QList<QSharedPointer<Companion>> getAllCompanions() const;

signals:
    void companionStateChanged(const QString &id, const QString &newState);
    void companionMutedOrLost(const QString &id);

private:
    QList<QSharedPointer<Companion>> m_companions;
};
