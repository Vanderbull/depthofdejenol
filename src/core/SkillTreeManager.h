#ifndef SKILLTREEMANAGER_H
#define SKILLTREEMANAGER_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class SkillTreeManager
{
public:
    static SkillTreeManager& instance()
    {
        static SkillTreeManager inst;
        return inst;
    }

    // Load from resources/skill_tree.json; call after resources are copied (early in init)
    bool load();

    // Get total bonus for Fighting skill given number of points spent
    double fighterBonus(int points) const;
    // Get description of next tier (or empty if max)
    QString fighterNextTierDesc(int points) const;
    int fighterMaxTier() const { return m_fightingTree.size(); }

    // Similarly for other skills if needed
    double magicalBonus(int points) const;
    double thievingBonus(int points) const;

private:
    SkillTreeManager() {}
    ~SkillTreeManager() {}

    struct Tier { QString name; QString description; double bonus; };
    QVector<Tier> m_fightingTree;
    QVector<Tier> m_magicalTree;
    QVector<Tier> m_thievingTree;
};

#endif // SKILLTREEMANAGER_H