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
        "Core loop wired end to end: fight, loot, equip, level, descend",
        "Combat starts on encounter; flee removes the monster from the map",
        "Monster spellcasting, status effects (poison, blind, confusion, fire)",
        "Loot drops start unidentified; identify/uncurse at the General Store",
        "Bestiary unlocks a monster only once you have met it",
        "Journal records victories, level-ups, quests, deaths and identification",
        "Gold sinks charged for resurrection, identification, uncursing, guild leveling, rest and cures",
        "Aging causes stat decay and death by old age; the body reaches the Morgue"
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
        {"2.0.0", "2026-10-10 — Core loop complete: all 14 v2.0.0 slices wired (combat, equipment, XP, loot, death, persistence, monster spells, status effects, town, item ID, bestiary, journal, gold sinks, aging)"},
        {"1.0.0", "2026-10-10 — First finishable release: victory sequence, Hall of Records, New Game Plus, and the six orphaned systems wired in"},
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
