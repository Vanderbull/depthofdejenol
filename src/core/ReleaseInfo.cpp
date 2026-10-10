#include "ReleaseInfo.h"
#include "version.h"
#include <QPair>

QString ReleaseInfo::version() {
    return QString::fromLatin1(GameConstants::SEMANTIC_VERSION);
}

QString ReleaseInfo::versionString() {
    return "v" + version();
}

QString ReleaseInfo::releaseDate() {
    return "2026-10-07";
}

QStringList ReleaseInfo::releaseNotes() {
    return {
        "Phase 1: Core systems — items, equipment, combat, spells, guilds, save/load",
        "Phase 2: Combat — equipment-driven attacks, monster AI, group encounters, status effects",
        "Phase 3: Progression — XP table, level-up gains, guilds, spell learning, aging",
        "Phase 4: Dungeon depth — 15 floors, 15 themes, bosses, keys, secret doors, respawn",
        "Phase 5: Death and consequences — body carrying, resurrection, party wipe, rescue",
        "Phase 6: Win condition — quest chain, final boss, victory, Hall of Records, New Game Plus",
        "Phase 7: Town and quality of life — tavern, quest board, NPCs, alignment, character sheet, bestiary, journal, sound, shortcuts, tutorial",
        "Phase 8: Balance and release — difficulty curve, spell differentiation, item progression, gold sinks, packaging"
    };
}

QStringList ReleaseInfo::changes() {
    return {
        "Monster difficulty curve: floor 1 beatable at level 1, floor 15 needs a real party",
        "Spell differentiation: fire AoE, cold slow, lightning chain, mind crowd control",
        "Item progression: Bronze → Iron → Steel → Adamantite → Mithril",
        "Gold sinks: resurrection, identification, uncursing, guild leveling",
        "Release packaging: version 1.0.0, release notes, system requirements"
    };
}

QString ReleaseInfo::systemRequirements() {
    return "Qt 6.0+, 2 GB RAM, 500 MB disk space";
}

QString ReleaseInfo::installerName() {
    return "blacklands-" + version() + "-linux-x64";
}

bool ReleaseInfo::isRelease() {
#ifdef QT_NO_DEBUG
    return true;
#else
    return false;
#endif
}

QString ReleaseInfo::banner() {
    return "Depth of Dejenol " + versionString();
}

QList<QPair<QString, QString>> ReleaseInfo::versionHistory() {
    return {
        {"0.1.0", "2026-10-10 — Phase 0: security hardening and latent breakage fixes"},
        {"0.0", "2026-10-07 — Game systems built (8 phases)"},
        {"0.9.0", "2026-10-01 — Phase 7 complete"},
        {"0.8.0", "2026-09-28 — Phase 6 complete"},
        {"0.7.0", "2026-09-20 — Phase 5 complete"},
        {"0.6.0", "2026-09-15 — Phase 4 complete"},
        {"0.5.0", "2026-09-10 — Phase 3 complete"},
        {"0.4.0", "2026-09-05 — Phase 2 complete"},
        {"0.3.0", "2026-09-01 — Phase 1 complete"}
    };
}
