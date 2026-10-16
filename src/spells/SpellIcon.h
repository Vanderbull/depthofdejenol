#ifndef SPELLICON_H
#define SPELLICON_H

#include <QPixmap>
#include <QString>
#include <QCache>

// Laddar och cache:ar spell-ikoner från resources/images/spells/.
// Varje spell-skola har en egen ikon (fire.png, cold.png, etc.).
class SpellIcon {
public:
    static QPixmap iconForSchool(const QString& school);
    static QPixmap iconForSpell(const QString& iconPath);
    static void clearCache();

private:
    static QCache<QString, QPixmap> s_cache;
};

#endif // SPELLICON_H
