// CompanionInventoryModel.h
#pragma once
#include <QAbstractListModel>
#include <QVector>
#include "Item.h"
#include <QObject>

class CompanionInventoryModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit CompanionInventoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    
    bool addItem(const Item &item);
    bool removeItem(int index);
    int totalWeight() const;
    int maxCapacity() const;

private:
    QVector<Item> m_satchel;
    int m_maxCapacity = 50; // Independent limit per companion
};
