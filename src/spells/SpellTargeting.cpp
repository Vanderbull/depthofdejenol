#include "SpellTargeting.h"

int SpellTargeting::targetCount(const QString& category, int casterLevel) {
    const SpellCategory::Def* def = SpellCategory::find(category);
    if (!def) return 1;

    switch (def->targeting) {
    case SpellCategory::Targeting::Single:
        return 1;
    case SpellCategory::Targeting::Group:
        return 4;  // MORDOR: "will cause 4 Monsters in 1 Group"
    case SpellCategory::Targeting::Area:
        return 1 + casterLevel / 3;
    case SpellCategory::Targeting::Self:
        return 0;
    }
    return 1;
}

int SpellTargeting::maxTargets() {
    return 8;  // MORDOR: "8 Monsters in 4 Groups" max
}

bool SpellTargeting::isMultiTarget(const QString& category) {
    return targetCount(category, 1) > 1;
}
