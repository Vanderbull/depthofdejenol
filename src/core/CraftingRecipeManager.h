#ifndef CRAFTINGRECIPEMANAGER_H
#define CRAFTINGRECIPEMANAGER_H

#include <QString>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <QCoreApplication>

struct CraftingRecipe {
    QString name;
    QStringList inputs; // list of required item names (order doesn't matter)
    QString description;
};

class CraftingRecipeManager
{
public:
    static CraftingRecipeManager& instance();
    bool load();
    QString getResult(const QStringList& inputs) const; // inputs: list of item names (exact match, order doesn't matter)
    QVector<CraftingRecipe> recipes() const { return m_recipes; }

private:
    CraftingRecipeManager() {}
    QVector<CraftingRecipe> m_recipes;
};

#endif // CRAFTINGRECIPEMANAGER_H
