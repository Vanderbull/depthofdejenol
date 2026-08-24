#include "CompanionInventoryModel.h"

CompanionInventoryModel::CompanionInventoryModel(QObject *parent)
    : QAbstractListModel(parent) {}

int CompanionInventoryModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_satchel.size();
}

QVariant CompanionInventoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_satchel.size()) return QVariant{};

    const auto &item = m_satchel.at(index.row());
    if (role == Qt::DisplayRole) {
        return QString("%1 (%2 kg)").arg(item.name).arg(item.weight);
    }
    return QVariant{};
}

bool CompanionInventoryModel::addItem(const Item &item) {
    if (totalWeight() + item.weight > m_maxCapacity) return false;

    beginInsertRows(QModelIndex(), m_satchel.size(), m_satchel.size());
    m_satchel.append(item);
    endInsertRows();
    return true;
}

bool CompanionInventoryModel::removeItem(int index) {
    if (index < 0 || index >= m_satchel.size()) return false;

    beginRemoveRows(QModelIndex(), index, index);
    m_satchel.removeAt(index);
    endRemoveRows();
    return true;
}

int CompanionInventoryModel::totalWeight() const {
    int sum = 0;
    for (const auto &item : m_satchel) sum += item.weight;
    return sum;
}

int CompanionInventoryModel::maxCapacity() const { return m_maxCapacity; }
void CompanionInventoryModel::setMaxCapacity(int capacity) { m_maxCapacity = capacity; }
