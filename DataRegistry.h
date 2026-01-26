#ifndef DATAREGISTRY_H
#define DATAREGISTRY_H

#include <QString>
#include <QMap>
#include <QList>
#include <QVariant>
#include <QDebug>

class DataRegistry {
public:
    // Adds a list of data under a specific category (e.g., "Races", "Items")
    void registerData(const QString& category, const QVariantList& dataList) {
        m_storage[category] = dataList;
        qDebug() << "Registered" << dataList.size() << "entries for category:" << category;
    }

    // Retrieves a full list for a category
    QVariantList getData(const QString& category) const {
        return m_storage.value(category);
    }

    // Quick lookup for a specific item by index within a category
    QVariant getEntry(const QString& category, int index) const {
        const QVariantList& list = m_storage[category];
        if (index >= 0 && index < list.size()) {
            return list.at(index);
        }
        return QVariant();
    }

    // Clears all data (useful for reload/new game)
    void clear() {
        m_storage.clear();
    }

private:
    // The "Master Map" that replaces the 15 individual QLists
    QMap<QString, QVariantList> m_storage;
};

#endif
