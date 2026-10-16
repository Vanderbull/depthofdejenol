#include "SpellIcon.h"
#include <QFileInfo>

QCache<QString, QPixmap> SpellIcon::s_cache(64);

QPixmap SpellIcon::iconForSchool(const QString& school)
{
    if (school.isEmpty()) return QPixmap();
    const QString path = QStringLiteral("resources/images/spells/%1.png").arg(school.toLower());
    return iconForSpell(path);
}

QPixmap SpellIcon::iconForSpell(const QString& iconPath)
{
    if (iconPath.isEmpty()) return QPixmap();

    QPixmap* cached = s_cache[iconPath];
    if (cached) return *cached;

    QPixmap pixmap(iconPath);
    if (pixmap.isNull()) return QPixmap();

    pixmap = pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    s_cache.insert(iconPath, new QPixmap(pixmap));
    return pixmap;
}

void SpellIcon::clearCache()
{
    s_cache.clear();
}
