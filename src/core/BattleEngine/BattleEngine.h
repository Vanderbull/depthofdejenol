#pragma once

#include <QObject>
#include <QVector>
#include <QPoint>
#include <QString>
#include <QVariant>
#include <QVariantMap>

struct UnitStack {
    QString name;
    int count;
    int maxHealth;
    int currentHealth;
    int attack;
    int defense;
    int speed;
    QPoint position;
    bool isPlayerControlled;
};

class BattleEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(int gridWidth READ gridWidth CONSTANT)
    Q_PROPERTY(int gridHeight READ gridHeight CONSTANT)

public:
    BattleEngine(QObject *parent = nullptr) : QObject(parent) {
        // Initialize sample armies
        units.append({"Swordsmen", 10, 30, 30, 10, 10, 5, QPoint(1, 2), true});
        units.append({"Skeleton Archers", 15, 20, 20, 8, 6, 6, QPoint(8, 2), false});
    }

    int gridWidth() const { return 12; }
    int gridHeight() const { return 10; }

    Q_INVOKABLE QVariantMap getUnitAt(int x, int y) {
        for (const auto &unit : units) {
            if (unit.position == QPoint(x, y)) {
                return {
                    {"name", unit.name},
                    {"count", unit.count},
                    {"isPlayer", unit.isPlayerControlled},
                    {"hp", unit.currentHealth}
                };
            }
        }
        return {};
    }

    Q_INVOKABLE bool moveUnit(int index, int targetX, int targetY) {
        if (index < 0 || index >= units.size()) return false;
        units[index].position = QPoint(targetX, targetY);
        emit battlefieldChanged();
        return true;
    }

signals:
    void battlefieldChanged();

private:
    QVector<UnitStack> units;
};
