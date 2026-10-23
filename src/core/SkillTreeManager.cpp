#include "SkillTreeManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>

bool SkillTreeManager::load()
{
    QString filePath;
    // Try in-source layout: resources beside executable (if executable placed in source root)
    filePath = QCoreApplication::applicationDirPath() + "/resources/skill_tree.json";
    QFile file(filePath);
    if (file.exists()) {
        // use this path
    } else {
        // Try out-of-source layout: executable in build/bin, resources two levels up
        filePath = QCoreApplication::applicationDirPath() + "/../../resources/skill_tree.json";
        file.setFileName(filePath);
        if (!file.exists()) {
            // Fallback: same directory as executable
            filePath = QCoreApplication::applicationDirPath() + "/skill_tree.json";
            file.setFileName(filePath);
        }
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open skill tree file:" << filePath;
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse skill tree JSON:" << parseError.errorString();
        return false;
    }
    if (!doc.isObject()) {
        qWarning() << "Skill tree JSON is not an object";
        return false;
    }
    QJsonObject obj = doc.object();

    auto loadCategory = [&](const QString& key, QVector<Tier>& outVec) {
        if (obj.contains(key) && obj[key].isArray()) {
            QJsonArray arr = obj[key].toArray();
            outVec.clear();
            for (const QJsonValue &v : arr) {
                if (v.isObject()) {
                    QJsonObject o = v.toObject();
                    Tier t;
                    t.name = o.value("name").toString();
                    t.description = o.value("description").toString();
                    t.bonus = o.value("bonus").toDouble(0.0);
                    outVec.append(t);
                }
            }
        }
    };

    loadCategory("Fighting", m_fightingTree);
    loadCategory("Magical", m_magicalTree);
    loadCategory("Thieving", m_thievingTree);

    qInfo() << "Loaded skill tree: Fighting" << m_fightingTree.size()
            << "tiers, Magical" << m_magicalTree.size()
            << ", Thieving" << m_thievingTree.size();
    return true;
}

double SkillTreeManager::fighterBonus(int points) const
{
    if (points <= 0 || m_fightingTree.isEmpty())
        return 0.0;
    int count = qMin(points, m_fightingTree.size());
    double sum = 0.0;
    for (int i = 0; i < count; ++i)
        sum += m_fightingTree[i].bonus;
    return sum;
}

QString SkillTreeManager::fighterNextTierDesc(int points) const
{
    if (points < 0) return QString();
    if (points >= m_fightingTree.size())
        return QString(); // maxed
    return m_fightingTree[points].description;
}

double SkillTreeManager::magicalBonus(int points) const
{
    if (points <= 0 || m_magicalTree.isEmpty())
        return 0.0;
    int count = qMin(points, m_magicalTree.size());
    double sum = 0.0;
    for (int i = 0; i < count; ++i)
        sum += m_magicalTree[i].bonus;
    return sum;
}

double SkillTreeManager::thievingBonus(int points) const
{
    if (points <= 0 || m_thievingTree.isEmpty())
        return 0.0;
    int count = qMin(points, m_thievingTree.size());
    double sum = 0.0;
    for (int i = 0; i < count; ++i)
        sum += m_thievingTree[i].bonus;
    return sum;
}
