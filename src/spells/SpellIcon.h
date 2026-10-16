#ifndef SPELLICON_H
#define SPELLICON_H

#include <QPixmap>
#include <QString>
#include <QCache>

// Loads and caches spell icons from resources/images/spells/.
// Each spell school has its own icon (fire.png, cold.png, etc.).
class SpellIcon {
public:
    static QPixmap iconForSchool(const QString& school);
    static QPixmap iconForSpell(const QString& iconPath);
    static void clearCache();

private:
    static QCache<QString, QPixmap> s_cache;
};

#endif // SPELLICON_H
