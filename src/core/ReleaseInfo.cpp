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
    return "2026-10-13";
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
        "Combat is tactical: pick targets, see each monster, use front/back rows",
        "Level-ups grant spendable stat points; the pool persists",
        "The main quest chain advances from play and grants its rewards",
        "Play time is tracked and the Hall of Records sorts by it",
        "Saves are versioned, migrated, and newer saves are refused",
        "Combat gives visual feedback on hits and deaths",
        "Core loop wired end to end: fight, loot, equip, level, descend",
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
        {"6.3.0", "2026-10-13 — UI Wiring Repair: restored 14 DungeonDialog action connects, removed redundant library_dialog connect, audited disabled buttons (all intentional), verified AgingRules wiring"},
        {"6.2.0", "2026-10-13 — Dungeon Floor Fidelity: theme-driven tile mix, monsterLevelBonus, fog/rotator/extinguisher tiles wired, dead declarations removed"},
        {"6.1.0", "2026-10-12 — Spell Wiring & Loot Integrity: unified spell resolution, Kill/Banish schools wired, DoT spells, monster loot from level+tier, removed dead awardBattleLoot"},
        {"6.0.0", "2026-10-12 — Spell & Balance Fix: mana +5/level, XP curve 30*n^1.2, Thunderclap/Storm spells, instant kill fix, monster XP fix"},
        {"5.9.0", "2026-10-12 — Dead Code Removal: removed 17 standalone prototype files from src/"},
        {"5.8.0", "2026-10-12 — Wire & Cleanup Round 4: removed dead SenderWindow class"},
        {"5.7.0", "2026-10-12 — Wire & Cleanup Round 3: removed dead ShortcutHelp class"},
        {"5.6.0", "2026-10-12 — Wire & Cleanup Round 2: removed dead DungeonMap, hid helplesson dwarf image placeholder, options backup timestamp updates"},
        {"5.5.0", "2026-10-12 — Wire & Cleanup: arena status effects tick, PlayerExperience wiring, partyUpdated signal test, RaceData guild eligibility test, removed dead code (bankpane/, legacy PartyManager)"},
        {"5.4.0", "2026-10-12 — Main Menu & Save System redesign: New Game requires a character, Load/Save complete game state, Assemble Party dialog replaces Character List"},
        {"5.3.0", "2026-10-12 — Aging & Lifecycle: characters age over time with race-specific decay and max age, coming-of-age ceremony at 18, marriage system with spouse tracking, children and family lineage, descendant inheritance, retirement eligibility at 70, all lifecycle data persists in JSON saves"},
        {"5.2.0", "2026-10-12 — Savegame revamp: autosave uses PartyManager as single source of truth, party building from individual character files (1–4 members with their own inventory/equipment/hunger/guild data), CharacterListDialog preview with party size and clear selection, LoadPartyDialog with party preview showing members and levels, JSON-only save format (legacy .lua/.txt removed)"},
        {"5.1.0", "2026-10-12 — Main Menu redesign: Continue button with party preview, Load Party dialog, clear visual hierarchy, fixed grid collision, removed debug text"},
        {"5.0.0", "2026-10-12 — Survival: hunger is visible, food exists and can be bought and eaten, starvation death is a real death with a body and journal entry, one HP source of truth, a town resupply loop, and a whole party can be loaded at once (all 8 slices 5.1–5.8)"},
        {"4.0.0", "2026-10-11 — Endgame depth and replayability: New Game Plus carries the party and scales rewards, the Abyss mega-dungeon and Arena are wired, 25 achievements unlock and persist, and a balance pass fixes monster AC and dexterity so a full run is winnable in 10–15 hours (all 7 slices 4.1–4.6)"},
        {"3.0.0", "2026-10-11 — Tactical combat and character growth: all 9 v3.0.0 slices wired (target selection, per-monster display, front/back row, stat allocation, quest chain driver, play time, save versioning, combat visual feedback, repo hygiene)"},
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
